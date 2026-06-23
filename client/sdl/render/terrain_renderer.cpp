#include "terrain_renderer.h"

#include <cstdio>

TerrainRenderer::TerrainRenderer(Renderer* renderer, SpriteManager* sprite_manager,
                                 const Camera& camera):
    renderer_(renderer), sprite_manager_(sprite_manager), camera_(camera) {}

TerrainRenderer::~TerrainRenderer() {
    if (terrain_cache_)
        SDL_DestroyTexture(terrain_cache_);
}

// Retorna qué vecinos cardinales (N/S/E/W) de (col,row) son del terreno `t`.
// Centraliza el patrón de "mirar los 4 vecinos" que antes se repetía en cada overlay.
TerrainRenderer::Cardinals TerrainRenderer::cardinals_of(const ClientMap& map, int col, int row,
                                                         TerrainType t) {
    return {has_terrain(map, col, row - 1, t), has_terrain(map, col, row + 1, t),
            has_terrain(map, col + 1, row, t), has_terrain(map, col - 1, row, t)};
}

// Punto de entrada del renderizado de terreno.
// Si la posición/tamaño no cambiaron, reusa la textura cacheada; si no, la reconstruye.
void TerrainRenderer::draw(int start_col, int end_col, int start_row, int end_row, int tile_w,
                           int tile_h, const ClientMap& client_map, int screen_w, int screen_h) {
    if (cache_is_stale(start_col, start_row, screen_w, screen_h))
        rebuild_cache(start_col, end_col, start_row, end_row, tile_w, tile_h, client_map, screen_w,
                      screen_h);

    SDL_RenderCopy(renderer_->get_sdl_renderer(), terrain_cache_, nullptr, nullptr);
}

// El cache se invalida si no existe, si cambió el tamaño de pantalla,
// o si la cámara cruzó un tile boundary (cambió la celda superior-izquierda visible).
bool TerrainRenderer::cache_is_stale(int start_col, int start_row, int screen_w,
                                     int screen_h) const {
    bool size_changed = (cache_w_ != screen_w || cache_h_ != screen_h);
    bool pos_changed = (cache_col_ != start_col || cache_row_ != start_row);
    return terrain_cache_ == nullptr || size_changed || pos_changed;
}

// Reconstruye la textura cacheada en dos pasadas (tiles base + overlays de transición).
// Solo re-crea la SDL_Texture cuando no existe o cambió el tamaño de pantalla.
void TerrainRenderer::rebuild_cache(int start_col, int end_col, int start_row, int end_row,
                                    int tile_w, int tile_h, const ClientMap& client_map,
                                    int screen_w, int screen_h) {
    SDL_Renderer* sdl_r = renderer_->get_sdl_renderer();

    if (terrain_cache_ == nullptr || cache_w_ != screen_w || cache_h_ != screen_h) {
        if (terrain_cache_)
            SDL_DestroyTexture(terrain_cache_);
        terrain_cache_ = SDL_CreateTexture(sdl_r, SDL_PIXELFORMAT_RGBA8888,
                                           SDL_TEXTUREACCESS_TARGET, screen_w, screen_h);
    }

    SDL_SetRenderTarget(sdl_r, terrain_cache_);
    SDL_SetRenderDrawColor(sdl_r, 0, 0, 0, 255);
    SDL_RenderClear(sdl_r);
    draw_base_tiles(start_col, end_col, start_row, end_row, tile_w, tile_h, client_map);
    draw_terrain_transitions(start_col, end_col, start_row, end_row, tile_w, tile_h, client_map);
    SDL_SetRenderTarget(sdl_r, nullptr);

    cache_col_ = start_col;
    cache_row_ = start_row;
    cache_w_ = screen_w;
    cache_h_ = screen_h;
}

// Pasada 1: pinta el tile base de toda la región visible (incluye el fallback GRASS del borde).
// Debe completarse ANTES de cualquier overlay: un overlay de tipo (p. ej. muro de ciudad) puede
// ser más ancho/alto que su tile y desbordar al vecino; si los base tiles no están todos abajo,
// un tile dibujado después taparía ese desborde.
void TerrainRenderer::draw_base_tiles(int start_col, int end_col, int start_row, int end_row,
                                      int tile_w, int tile_h, const ClientMap& client_map) {
    for (int row = start_row; row <= end_row; row++) {
        for (int col = start_col; col <= end_col; col++) {
            draw_base_tile(col, row, tile_w, tile_h, client_map);
        }
    }
}

// Pasada 2: recorre solo las celdas dentro del mapa y dibuja sus overlays de transición.
// Se omiten celdas fuera de límites porque no tienen vecinos reales para calcular transiciones.
void TerrainRenderer::draw_terrain_transitions(int start_col, int end_col, int start_row,
                                               int end_row, int tile_w, int tile_h,
                                               const ClientMap& client_map) {
    for (int row = start_row; row <= end_row; row++) {
        for (int col = start_col; col <= end_col; col++) {
            bool in_bounds = (col >= 0 && col < client_map.get_width() && row >= 0 &&
                              row < client_map.get_height());
            if (!in_bounds)
                continue;
            draw_overlays_for_tile(col, row, tile_w, tile_h, client_map);
        }
    }
}

// Dibuja el tile base de la celda (col, row):
//   1. Convierte coordenadas de tile a coordenadas de pantalla con la cámara.
//   2. Si la celda está fuera del mapa usa GRASS como fallback (para el borde de la cámara).
//   3. Obtiene la textura del terreno del SpriteManager y la renderiza.
void TerrainRenderer::draw_base_tile(int col, int row, int tile_w, int tile_h,
                                     const ClientMap& client_map) {
    int tx = camera_.get_screen_x(col * tile_w);
    int ty = camera_.get_screen_y(row * tile_h);
    bool in_bounds =
        (col >= 0 && col < client_map.get_width() && row >= 0 && row < client_map.get_height());
    TerrainType terrain = in_bounds ? client_map.at(col, row).terrain : TerrainType::GRASS;

    renderer_->draw_frame(sprite_manager_->get_terrain(terrain), 0, 0, tile_w, tile_h, tx, ty);
}

// Dibuja los overlays de la celda (col, row) en dos etapas:
//   Etapa A — overlay de tipo de terreno (ej. muro de ciudad):
//     - Las entradas de mazmorra se omiten visualmente para evitar artefactos de arte;
//       la lógica de teletransporte sigue funcionando en el servidor.
//     - El overlay se centra horizontalmente y se alinea al borde inferior del tile.
//   Etapa B — overlays de transición entre terrenos adyacentes:
//     - Se aplican en orden de menor a mayor prioridad visual para que en esquinas
//       compartidas el terreno "superior" quede encima (agua < arena < pasto/tierra).
void TerrainRenderer::draw_overlays_for_tile(int col, int row, int tile_w, int tile_h,
                                             const ClientMap& client_map) {
    int tx = camera_.get_screen_x(col * tile_w);
    int ty = camera_.get_screen_y(row * tile_h);
    TerrainType terrain = client_map.at(col, row).terrain;

    // Etapa A: overlay de tipo (muros de ciudad).
    // Las entradas de mazmorra se omiten visualmente
    // — la lógica de teletransporte sigue funcionando porque está en el servidor.
    // TODO(chiaradelaurentis): quitar esta guarda cuando se quiera mostrar las entradas nuevamente.
    bool is_dungeon_entrance =
        (terrain == TerrainType::DUNGEON_ENTRANCE_1 || terrain == TerrainType::DUNGEON_ENTRANCE_2);
    SDL_Texture* type_overlay =
        is_dungeon_entrance ? nullptr : sprite_manager_->get_terrain_overlay(terrain);
    if (type_overlay) {
        int ow, oh;
        SDL_QueryTexture(type_overlay, nullptr, nullptr, &ow, &oh);
        // Centra horizontalmente y pega al borde inferior del tile.
        renderer_->draw_frame(type_overlay, 0, 0, ow, oh, tx + (tile_w - ow) / 2,
                              ty + (tile_h - oh));
    }

    // Etapa B: overlays de transición entre terrenos. El orden va de menor a mayor prioridad visual
    // (agua < arena < pasto/tierra) para que en esquinas compartidas quede encima el terreno
    // "superior" — ver draw_transition para la convención de rotaciones.
    draw_ao_costa_overlay(col, row, tile_w, tile_h, tx, ty, client_map);
    draw_ao_pasto_overlay(col, row, tile_w, tile_h, tx, ty, client_map);
    draw_transition(col, row, tile_w, tile_h, tx, ty, client_map, TerrainType::WATER,
                    TerrainType::GRASS, "terrain_overlay_grass_water");
    draw_transition(col, row, tile_w, tile_h, tx, ty, client_map, TerrainType::DIRT,
                    TerrainType::GRASS, "terrain_overlay_grass_dirt");
}

// Dibuja los overlays de transición entre el terreno `sobre` y su vecino `vecino`
// para la celda (col, row).
//
// Algoritmo:
//   1. Si la celda no es del terreno `sobre`, no hay nada que hacer.
//   2. Detecta qué vecinos cardinales (N/S/E/W) son del tipo `vecino`.
//   3. Por cada vecino cardinal presente, dibuja un overlay de borde (_edge) rotado:
//        0°=N, 90°=E, 180°=S, 270°=W.
//   4. Por cada diagonal donde hay `vecino` pero los dos lados adyacentes NO lo son
//      (esquina interior), dibuja un overlay de esquina (_corner) rotado:
//        0°=NW, 90°=NE, 180°=SE, 270°=SW.
//   Se usan solo dos PNGs base + rotación para cubrir las 8 posibles orientaciones.
//   Falla silenciosamente si los PNGs no existen en el SpriteManager.
void TerrainRenderer::draw_transition(int col, int row, int tile_w, int tile_h, int tx, int ty,
                                      const ClientMap& client_map, TerrainType sobre,
                                      TerrainType vecino, const char* nombre_base) {
    // Paso 1: solo actúa sobre celdas del tipo `sobre`.
    if (client_map.at(col, row).terrain != sobre)
        return;

    // Paso 2: vecinos cardinales del tipo `vecino`.
    Cardinals v = cardinals_of(client_map, col, row, vecino);

    // Paso 3: carga las dos texturas base (edge y corner).
    // Se usan dos PNGs base con rotación clockwise para cubrir las 8 direcciones:
    //   edge:   vecino en borde norte → 0°=N, 90°=E, 180°=S, 270°=W
    //   corner: vecino en esquina NW  → 0°=NW, 90°=NE, 180°=SE, 270°=SW
    char key_edge[64];
    char key_corner[64];
    std::snprintf(key_edge, sizeof(key_edge), "%s_edge", nombre_base);
    std::snprintf(key_corner, sizeof(key_corner), "%s_corner", nombre_base);
    SDL_Texture* edge_tex = sprite_manager_->get_transition_overlay(key_edge);
    SDL_Texture* corner_tex = sprite_manager_->get_transition_overlay(key_corner);

    auto draw_edge = [&](double angulo) {
        if (edge_tex)
            renderer_->draw_frame_rotated(edge_tex, 0, 0, tile_w, tile_h, tx, ty, angulo);
    };

    // Esquina interior: vecino en la diagonal (dcol,drow) y SIN vecino en los dos cardinales
    // adyacentes a esa diagonal (así no se pisa con un borde ya dibujado).
    auto corner_if = [&](bool side_a, bool side_b, int dcol, int drow, double angulo) {
        if (!side_a && !side_b && has_terrain(client_map, col + dcol, row + drow, vecino) &&
            corner_tex)
            renderer_->draw_frame_rotated(corner_tex, 0, 0, tile_w, tile_h, tx, ty, angulo);
    };

    // Paso 4a: overlays de borde para cada cardinal con vecino.
    if (v.n)
        draw_edge(0.0);
    if (v.e)
        draw_edge(90.0);
    if (v.s)
        draw_edge(180.0);
    if (v.w)
        draw_edge(270.0);

    // Paso 4b: esquinas interiores (NW, NE, SE, SW).
    corner_if(v.n, v.w, -1, -1, 0.0);
    corner_if(v.n, v.e, +1, -1, 90.0);
    corner_if(v.s, v.e, +1, +1, 180.0);
    corner_if(v.s, v.w, -1, +1, 270.0);
}

// Dibuja un árbol en la celda (col, row):
//   1. Calcula las coordenadas de pantalla.
//   2. Dibuja la sombra elíptica centrada en la base del tile (borde inferior).
//   3. Dibuja el sprite del árbol encima, alineado al tile completo.
void TerrainRenderer::draw_tree(int col, int row, int tile_w, int tile_h) {
    int tx = camera_.get_screen_x(col * tile_w);
    int ty = camera_.get_screen_y(row * tile_h);
    int shadow_w = static_cast<int>(tile_w * Renderer::SHADOW_TREE_RATIO);
    renderer_->draw_shadow(tx + tile_w / 2, ty + tile_h, shadow_w);
    renderer_->draw_frame(sprite_manager_->get_tree(), 0, 0, tile_w, tile_h, tx, ty);
}

// Dibuja el overlay de costa AO para la celda (col, row) si es WATER con vecinos SAND.
// La pieza se dibuja sobre el tile de agua, cubriendo la línea dura agua→arena con la espuma.
// Se arma un bitmask N/S/E/O de vecinos SAND y se elige la pieza por tabla (switch).
// Las combinaciones sin pieza (0 cardinales, o 3+) caen en el default y no dibujan nada.
void TerrainRenderer::draw_ao_costa_overlay(int col, int row, int tile_w, int tile_h, int tx,
                                            int ty, const ClientMap& client_map) {
    if (client_map.at(col, row).terrain != TerrainType::WATER)
        return;

    Cardinals s = cardinals_of(client_map, col, row, TerrainType::SAND);
    int mask = (s.n << 3) | (s.s << 2) | (s.e << 1) | s.w;

    const char* key = nullptr;
    switch (mask) {
        // Ángulos convexos del lago (dos cardinales de arena adyacentes).
        case 0b0101:
            key = "ao_angulo_ne";
            break;  // S + O
        case 0b1010:
            key = "ao_angulo_so";
            break;  // N + E
        case 0b0110:
            key = "ao_angulo_se";
            break;  // S + E
        case 0b1001:
            key = "ao_angulo_nw";
            break;  // N + O
        // Bordes rectos (exactamente un cardinal con arena).
        case 0b0100:
            key = "ao_costa_norte";
            break;  // S
        case 0b1000:
            key = "ao_costa_sur";
            break;  // N
        case 0b0001:
            key = "ao_costa_este";
            break;  // O
        case 0b0010:
            key = "ao_costa_oeste";
            break;  // E
        // Resto (0 cardinales, 2 opuestos, 3+): sin pieza todavía.
        default:
            break;
    }

    if (!key)
        return;
    SDL_Texture* tex = sprite_manager_->get_transition_overlay(key);
    if (tex)
        renderer_->draw_frame(tex, 0, 0, tile_w, tile_h, tx, ty);
}

// Dibuja los overlays de transición arena→pasto sobre tiles de SAND con vecinos GRASS.
// Bordes rectos: piezas AO (ao_pasto_sur/este + espejado). Cada cardinal es independiente;
// si hay 2+ cardinales se acumulan, cubriendo el caso convexo sin pieza especial.
// Esquinas cóncavas (pasto solo en diagonal): overlay sintético viejo rotado.
void TerrainRenderer::draw_ao_pasto_overlay(int col, int row, int tile_w, int tile_h, int tx,
                                            int ty, const ClientMap& client_map) {
    if (client_map.at(col, row).terrain != TerrainType::SAND)
        return;

    Cardinals g = cardinals_of(client_map, col, row, TerrainType::GRASS);

    // Blitea una pieza de transición por su key, opcionalmente espejada.
    auto blit = [&](const char* key, SDL_RendererFlip flip) {
        SDL_Texture* tex = sprite_manager_->get_transition_overlay(key);
        if (!tex)
            return;
        if (flip == SDL_FLIP_NONE)
            renderer_->draw_frame(tex, 0, 0, tile_w, tile_h, tx, ty);
        else
            renderer_->draw_frame_flipped(tex, 0, 0, tile_w, tile_h, tx, ty, flip);
    };

    // Bordes rectos AO — independientes: se acumulan cuando hay 2+ cardinales con pasto.
    if (g.s)
        blit("ao_pasto_sur", SDL_FLIP_NONE);
    if (g.n)
        blit("ao_pasto_sur", SDL_FLIP_VERTICAL);
    if (g.e)
        blit("ao_pasto_este", SDL_FLIP_NONE);
    if (g.w)
        blit("ao_pasto_este", SDL_FLIP_HORIZONTAL);

    // Esquinas cóncavas: pasto solo en diagonal, sin cardinal adyacente que lo cubra.
    // Usa el overlay sintético viejo en vez de ao_pasto_concava (set 6017, verde muy oscuro).
    SDL_Texture* corner =
        sprite_manager_->get_transition_overlay("terrain_overlay_grass_sand_corner");
    if (!corner)
        return;

    auto corner_if = [&](bool side_a, bool side_b, int dcol, int drow, double angulo) {
        if (!side_a && !side_b &&
            has_terrain(client_map, col + dcol, row + drow, TerrainType::GRASS))
            renderer_->draw_frame_rotated(corner, 0, 0, tile_w, tile_h, tx, ty, angulo);
    };
    corner_if(g.n, g.w, -1, -1, 0.0);
    corner_if(g.n, g.e, +1, -1, 90.0);
    corner_if(g.s, g.e, +1, +1, 180.0);
    corner_if(g.s, g.w, -1, +1, 270.0);
}

// Retorna true si la celda (col, row) está dentro del mapa y su terreno es `expected`.
// Retorna false ante cualquier coordenada fuera de límites (sin excepción).
bool TerrainRenderer::has_terrain(const ClientMap& map, int col, int row, TerrainType expected) {
    if (col < 0 || col >= map.get_width() || row < 0 || row >= map.get_height())
        return false;
    return map.at(col, row).terrain == expected;
}
