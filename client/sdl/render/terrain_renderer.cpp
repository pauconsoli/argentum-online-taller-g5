#include "terrain_renderer.h"

#include <cstdio>

TerrainRenderer::TerrainRenderer(Renderer* renderer, SpriteManager* sprite_manager,
                                 const Camera& camera):
    renderer_(renderer), sprite_manager_(sprite_manager), camera_(camera) {}

TerrainRenderer::~TerrainRenderer() {
    if (terrain_cache_)
        SDL_DestroyTexture(terrain_cache_);
}

// Punto de entrada del renderizado de terreno.
// Hace dos pasadas sobre la región visible, pero solo cuando la cámara cruzó un tile boundary.
// Si la posición en tiles no cambió, salta ambas pasadas y blitea la textura cacheada.
void TerrainRenderer::draw(int start_col, int end_col, int start_row, int end_row, int tile_w,
                           int tile_h, const ClientMap& client_map, int screen_w, int screen_h) {
    SDL_Renderer* sdl_r = renderer_->get_sdl_renderer();

    bool size_changed = (cache_w_ != screen_w || cache_h_ != screen_h);
    bool pos_changed = (cache_col_ != start_col || cache_row_ != start_row);

    if (terrain_cache_ == nullptr || size_changed || pos_changed) {
        if (terrain_cache_ == nullptr || size_changed) {
            if (terrain_cache_)
                SDL_DestroyTexture(terrain_cache_);
            terrain_cache_ = SDL_CreateTexture(sdl_r, SDL_PIXELFORMAT_RGBA8888,
                                               SDL_TEXTUREACCESS_TARGET, screen_w, screen_h);
        }

        SDL_SetRenderTarget(sdl_r, terrain_cache_);
        SDL_SetRenderDrawColor(sdl_r, 0, 0, 0, 255);
        SDL_RenderClear(sdl_r);
        draw_base_tiles(start_col, end_col, start_row, end_row, tile_w, tile_h, client_map);
        draw_terrain_transitions(start_col, end_col, start_row, end_row, tile_w, tile_h,
                                 client_map);
        SDL_SetRenderTarget(sdl_r, nullptr);

        cache_col_ = start_col;
        cache_row_ = start_row;
        cache_w_ = screen_w;
        cache_h_ = screen_h;
    }

    SDL_RenderCopy(sdl_r, terrain_cache_, nullptr, nullptr);
}

// se pinta el tile base
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

    // Paso 2: comprueba los cuatro vecinos cardinales.
    bool v_n = has_terrain(client_map, col, row - 1, vecino);
    bool v_s = has_terrain(client_map, col, row + 1, vecino);
    bool v_e = has_terrain(client_map, col + 1, row, vecino);
    bool v_w = has_terrain(client_map, col - 1, row, vecino);

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
    auto draw_corner = [&](double angulo) {
        if (corner_tex)
            renderer_->draw_frame_rotated(corner_tex, 0, 0, tile_w, tile_h, tx, ty, angulo);
    };

    // Paso 4a: overlays de borde para cada cardinal con vecino.
    if (v_n)
        draw_edge(0.0);
    if (v_e)
        draw_edge(90.0);
    if (v_s)
        draw_edge(180.0);
    if (v_w)
        draw_edge(270.0);

    // Paso 4b: esquinas interiores — vecino solo en diagonal, sin vecino en los lados adyacentes.
    // Si ya hay borde en un cardinal no se dibuja corner sobre esa diagonal (evita doble overlay).
    if (!v_n && !v_w && has_terrain(client_map, col - 1, row - 1, vecino))
        draw_corner(0.0);
    if (!v_n && !v_e && has_terrain(client_map, col + 1, row - 1, vecino))
        draw_corner(90.0);
    if (!v_s && !v_e && has_terrain(client_map, col + 1, row + 1, vecino))
        draw_corner(180.0);
    if (!v_s && !v_w && has_terrain(client_map, col - 1, row + 1, vecino))
        draw_corner(270.0);
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
// Bitmask N/S/E/O de vecinos SAND: elige la pieza correcta entre las 6 disponibles.
// Las combinaciones sin pieza (ej. NO, SE, 3+ cardinales) no dibujan nada.
void TerrainRenderer::draw_ao_costa_overlay(int col, int row, int tile_w, int tile_h, int tx,
                                            int ty, const ClientMap& client_map) {
    // Actúa sobre tiles de AGUA con vecinos ARENA:
    // la pieza se dibuja sobre el tile de agua, cubriendo la línea dura agua→arena.
    if (client_map.at(col, row).terrain != TerrainType::WATER)
        return;

    bool s_n = has_terrain(client_map, col, row - 1, TerrainType::SAND);
    bool s_s = has_terrain(client_map, col, row + 1, TerrainType::SAND);
    bool s_e = has_terrain(client_map, col + 1, row, TerrainType::SAND);
    bool s_o = has_terrain(client_map, col - 1, row, TerrainType::SAND);

    const char* key = nullptr;
    // Ángulos convexos del lago (esquina de agua con dos cardinales de arena adyacentes)
    if (s_s && s_o && !s_n && !s_e)
        key = "ao_angulo_ne";  // esquina NE del lago
    else if (s_n && s_e && !s_s && !s_o)
        key = "ao_angulo_so";  // esquina SO del lago
    else if (s_s && s_e && !s_n && !s_o)
        key = "ao_angulo_se";  // esquina NW del lago
    else if (s_n && s_o && !s_s && !s_e)
        key = "ao_angulo_nw";  // esquina SE del lago
    // Bordes rectos (exactamente un cardinal con arena)
    else if (s_s && !s_n && !s_e && !s_o)
        key = "ao_costa_norte";
    else if (s_n && !s_s && !s_e && !s_o)
        key = "ao_costa_sur";
    else if (s_o && !s_n && !s_s && !s_e)
        key = "ao_costa_este";
    else if (s_e && !s_n && !s_s && !s_o)
        key = "ao_costa_oeste";
    // Resto (NO, SE, 3+ cardinales de arena): sin pieza todavía, no se dibuja nada.

    if (!key)
        return;
    SDL_Texture* tex = sprite_manager_->get_transition_overlay(key);
    if (tex)
        renderer_->draw_frame(tex, 0, 0, tile_w, tile_h, tx, ty);
}

// Dibuja los overlays de transición arena→pasto sobre tiles de SAND con vecinos GRASS.
// Bordes rectos: piezas AO (ao_pasto_sur/este + espejado). Cada cardinal es independiente;
// si hay 2+ cardinales se acumulan, cubriendo el caso convexa sin pieza especial.
// Esquinas cóncavas (pasto solo en diagonal): overlay sintético viejo rotado.
void TerrainRenderer::draw_ao_pasto_overlay(int col, int row, int tile_w, int tile_h, int tx,
                                            int ty, const ClientMap& client_map) {
    if (client_map.at(col, row).terrain != TerrainType::SAND)
        return;

    bool g_n = has_terrain(client_map, col, row - 1, TerrainType::GRASS);
    bool g_s = has_terrain(client_map, col, row + 1, TerrainType::GRASS);
    bool g_e = has_terrain(client_map, col + 1, row, TerrainType::GRASS);
    bool g_o = has_terrain(client_map, col - 1, row, TerrainType::GRASS);

    // Bordes rectos AO — if independientes: se acumulan cuando hay 2+ cardinales con pasto.
    if (g_s) {
        SDL_Texture* tex = sprite_manager_->get_transition_overlay("ao_pasto_sur");
        if (tex)
            renderer_->draw_frame(tex, 0, 0, tile_w, tile_h, tx, ty);
    }
    if (g_n) {
        SDL_Texture* tex = sprite_manager_->get_transition_overlay("ao_pasto_sur");
        if (tex)
            renderer_->draw_frame_flipped(tex, 0, 0, tile_w, tile_h, tx, ty, SDL_FLIP_VERTICAL);
    }
    if (g_e) {
        SDL_Texture* tex = sprite_manager_->get_transition_overlay("ao_pasto_este");
        if (tex)
            renderer_->draw_frame(tex, 0, 0, tile_w, tile_h, tx, ty);
    }
    if (g_o) {
        SDL_Texture* tex = sprite_manager_->get_transition_overlay("ao_pasto_este");
        if (tex)
            renderer_->draw_frame_flipped(tex, 0, 0, tile_w, tile_h, tx, ty, SDL_FLIP_HORIZONTAL);
    }

    // Esquinas cóncavas: pasto solo en diagonal, sin cardinal adyacente que lo cubra.
    // Usa el overlay sintético viejo en vez de ao_pasto_concava (set 6017, verde muy oscuro).
    // Condición: igual que draw_transition — solo exige que los dos cardinales ADYACENTES
    // a esa diagonal no tengan pasto (puede haber pasto en el cardinal opuesto).
    SDL_Texture* corner =
        sprite_manager_->get_transition_overlay("terrain_overlay_grass_sand_corner");
    if (corner) {
        if (!g_n && !g_o && has_terrain(client_map, col - 1, row - 1, TerrainType::GRASS))
            renderer_->draw_frame_rotated(corner, 0, 0, tile_w, tile_h, tx, ty, 0.0);
        if (!g_n && !g_e && has_terrain(client_map, col + 1, row - 1, TerrainType::GRASS))
            renderer_->draw_frame_rotated(corner, 0, 0, tile_w, tile_h, tx, ty, 90.0);
        if (!g_s && !g_e && has_terrain(client_map, col + 1, row + 1, TerrainType::GRASS))
            renderer_->draw_frame_rotated(corner, 0, 0, tile_w, tile_h, tx, ty, 180.0);
        if (!g_s && !g_o && has_terrain(client_map, col - 1, row + 1, TerrainType::GRASS))
            renderer_->draw_frame_rotated(corner, 0, 0, tile_w, tile_h, tx, ty, 270.0);
    }
}

// Retorna true si la celda (col, row) está dentro del mapa y su terreno es `expected`.
// Retorna false ante cualquier coordenada fuera de límites (sin excepción).
bool TerrainRenderer::has_terrain(const ClientMap& map, int col, int row, TerrainType expected) {
    if (col < 0 || col >= map.get_width() || row < 0 || row >= map.get_height())
        return false;
    return map.at(col, row).terrain == expected;
}
