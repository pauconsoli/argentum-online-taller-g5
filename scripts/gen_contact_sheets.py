#!/usr/bin/env python3
"""
Generador de contact sheets para candidatos de sprites AO.
Secciones E (NPCs ciudad), F (Puestitos), G (Criaturas).
No modifica ningún recurso original.
"""

from PIL import Image, ImageDraw, ImageFont
import os

BASE_GFX = "/Users/chiaradelaurentiis/argentum-online-taller-g5/Recursos/Graficos/"
OUT_DIR  = "/Users/chiaradelaurentiis/argentum-online-taller-g5/Recursos/OUTPUT/"
GRAFICOS_INI = "/Users/chiaradelaurentiis/argentum-online-taller-g5/Recursos/init/graficos.ini"

os.makedirs(OUT_DIR, exist_ok=True)

# ── colores de UI ────────────────────────────────────────────────────────────
BG        = (30, 30, 30)
PANEL_BG  = (45, 45, 55)
LABEL_BG  = (20, 20, 60)
C_GRID    = (0, 200, 255, 50)   # azul semitransparente
C_ANCHOR  = (255, 80, 80)
C_BORDER  = (100, 200, 100)
C_HEADER  = (255, 220, 80)
C_WHITE   = (255, 255, 255)
C_GRAY    = (160, 160, 160)
C_ORANGE  = (255, 140, 0)
C_CYAN    = (0, 220, 220)
C_RED     = (255, 80, 80)
TILE      = 32   # tamaño de tile de referencia

# ── carga de graficos.ini ────────────────────────────────────────────────────
_grh_cache: dict[int, tuple] = {}

def load_graficos():
    global _grh_cache
    _grh_cache = {}
    with open(GRAFICOS_INI, "r", encoding="latin-1") as f:
        for line in f:
            line = line.strip()
            if not line.startswith("Grh"):
                continue
            try:
                key, val = line.split("=", 1)
                gid = int(key[3:])
                parts = [int(x) for x in val.split("-")]
            except Exception:
                continue
            nframes = parts[0]
            if nframes == 1:
                # 1-filenum-x-y-w-h
                _grh_cache[gid] = ("static", parts[1], parts[2], parts[3], parts[4], parts[5])
            else:
                # n-grh1-grh2-...-speed
                frames = parts[1:-1]
                speed  = parts[-1]
                _grh_cache[gid] = ("anim", frames, speed)

# ── resolución de GRH a lista de (filenum,x,y,w,h) ──────────────────────────
_img_cache: dict[int, Image.Image] = {}

def open_img(filenum: int) -> Image.Image | None:
    if filenum in _img_cache:
        return _img_cache[filenum]
    path = os.path.join(BASE_GFX, f"{filenum}.png")
    if not os.path.exists(path):
        return None
    img = Image.open(path)
    # convertir paletas a RGBA
    if img.mode == "P":
        img = img.convert("RGBA")
        # detectar color de transparencia (magenta o color índice 0)
        data = img.getdata()
        # heurística: pixeles con R>200 y B>200 y G<50 son magenta → transparentes
        new_data = []
        for px in data:
            r, g, b, a = px
            if r > 200 and b > 200 and g < 60:
                new_data.append((r, g, b, 0))
            elif r == 0 and g == 0 and b == 0:
                new_data.append((0, 0, 0, 0))
            else:
                new_data.append(px)
        img.putdata(new_data)
    elif img.mode != "RGBA":
        img = img.convert("RGBA")
    _img_cache[filenum] = img
    return img

def resolve_grh(gid: int) -> list[tuple[int,int,int,int,int]]:
    """Devuelve lista de (filenum, x, y, w, h) para cada frame del GRH."""
    if gid not in _grh_cache:
        return []
    entry = _grh_cache[gid]
    if entry[0] == "static":
        _, filenum, x, y, w, h = entry
        return [(filenum, x, y, w, h)]
    else:
        _, frames, speed = entry
        result = []
        for fid in frames:
            result.extend(resolve_grh(fid))
        return result

def extract_sprite(filenum: int, x: int, y: int, w: int, h: int) -> Image.Image | None:
    img = open_img(filenum)
    if img is None:
        return None
    W, H = img.size
    x2, y2 = min(x+w, W), min(y+h, H)
    if x >= W or y >= H or w <= 0 or h <= 0:
        return None
    return img.crop((x, y, x2, y2))

# ── helpers de dibujo ────────────────────────────────────────────────────────
try:
    _font_big  = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", 14)
    _font_med  = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", 11)
    _font_sml  = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", 9)
except Exception:
    _font_big = _font_med = _font_sml = ImageFont.load_default()

def draw_label(draw, x, y, text, color=C_WHITE, font=None):
    draw.text((x, y), text, fill=color, font=font or _font_med)

def draw_grid_overlay(canvas: Image.Image, px: int, py: int, w: int, h: int):
    """Dibuja grilla de 32×32 sobre la región (px,py,w,h) del canvas."""
    overlay = Image.new("RGBA", canvas.size, (0,0,0,0))
    d = ImageDraw.Draw(overlay)
    for gx in range(px, px+w, TILE):
        d.line([(gx, py), (gx, py+h)], fill=C_GRID, width=1)
    for gy in range(py, py+h, TILE):
        d.line([(px, gy), (px+w, gy)], fill=C_GRID, width=1)
    canvas.alpha_composite(overlay)

def checkerboard(w: int, h: int, size: int = 8) -> Image.Image:
    """Fondo a cuadros para visualizar transparencia."""
    img = Image.new("RGBA", (w, h))
    d = ImageDraw.Draw(img)
    for y in range(0, h, size):
        for x in range(0, w, size):
            c = (60,60,60,255) if (x//size + y//size) % 2 == 0 else (80,80,80,255)
            d.rectangle([x, y, x+size-1, y+size-1], fill=c)
    return img

def paste_on_checker(sprite: Image.Image, pad: int = 4) -> Image.Image:
    """Pega el sprite sobre fondo a cuadros con padding."""
    if sprite is None:
        spr = Image.new("RGBA", (32, 32))
    else:
        spr = sprite.convert("RGBA")
    W, H = spr.width + pad*2, spr.height + pad*2
    bg = checkerboard(W, H)
    bg.alpha_composite(spr, (pad, pad))
    return bg

def draw_anchor(img: Image.Image, x_center: int, y_bottom: int):
    """Marca el punto de anclaje (pies) con una cruz roja."""
    d = ImageDraw.Draw(img)
    sz = 5
    d.line([(x_center-sz, y_bottom), (x_center+sz, y_bottom)], fill=C_ANCHOR, width=2)
    d.line([(x_center, y_bottom-sz), (x_center, y_bottom+sz)], fill=C_ANCHOR, width=2)

def scale_to_fit(img: Image.Image, max_w: int, max_h: int, nearest: bool = True) -> Image.Image:
    """Escala preservando proporción, sin distorsión."""
    w, h = img.size
    if w == 0 or h == 0:
        return img
    scale = min(max_w/w, max_h/h, 1.0)
    if scale < 1.0:
        nw, nh = max(1, int(w*scale)), max(1, int(h*scale))
        resample = Image.NEAREST if nearest else Image.LANCZOS
        return img.resize((nw, nh), resample)
    return img

# ── construcción de panel individual ─────────────────────────────────────────

PANEL_W = 1100
MARGIN  = 10
ROW_H   = 180   # alto de cada fila de sprites dentro de un panel

def make_panel_header(title: str, subtitle: str, npc_id: str, section: str) -> Image.Image:
    h = 48
    img = Image.new("RGBA", (PANEL_W, h), LABEL_BG)
    d = ImageDraw.Draw(img)
    d.text((8, 4),  f"[{section}] {title}", fill=C_HEADER, font=_font_big)
    d.text((8, 24), f"{npc_id}  —  {subtitle}", fill=C_GRAY,   font=_font_med)
    # borde inferior
    d.line([(0, h-2), (PANEL_W, h-2)], fill=C_BORDER, width=2)
    return img

def make_row_label(text: str, color=C_CYAN) -> Image.Image:
    img = Image.new("RGBA", (PANEL_W, 18), PANEL_BG)
    d = ImageDraw.Draw(img)
    d.text((8, 3), text, fill=color, font=_font_sml)
    return img

def assemble_frames_row(frames: list[Image.Image], labels: list[str],
                         scale: int = 4, max_h: int = 128,
                         show_grid: bool = True) -> Image.Image:
    """Ensambla lista de frames en una fila horizontal con labels."""
    if not frames:
        img = Image.new("RGBA", (PANEL_W, ROW_H), PANEL_BG)
        d = ImageDraw.Draw(img)
        d.text((20, ROW_H//2), "(sin frames resueltos)", fill=C_RED, font=_font_med)
        return img

    cell_w = max(f.width * scale if f else 32 for f in frames)
    cell_w = max(cell_w, TILE * scale)
    cell_h = max_h + 20   # espacio para label

    total_w = max(PANEL_W, len(frames) * (cell_w + 4) + 8)
    img = Image.new("RGBA", (total_w, cell_h), PANEL_BG)

    cx = 4
    for i, (frame, lbl) in enumerate(zip(frames, labels)):
        if frame is None:
            cx += cell_w + 4
            continue
        # escalar sprite
        fw, fh = frame.width, frame.height
        s = min(scale, max_h // max(fh, 1))
        s = max(s, 1)
        upscaled = frame.resize((fw*s, fh*s), Image.NEAREST)
        # fondo a cuadros
        bg = checkerboard(upscaled.width + 4, upscaled.height + 4)
        bg.alpha_composite(upscaled, (2, 2))
        bg_w, bg_h = bg.size
        # pegar en img
        py = (max_h - bg_h) // 2
        if py < 0:
            py = 0
        img.alpha_composite(bg, (cx, py))
        # marca de anclaje (pies)
        anchor_x = cx + bg_w // 2
        anchor_y = py + bg_h - 2
        draw_anchor(img, anchor_x, anchor_y)
        # grilla de tile sobre sprite escalado
        if show_grid:
            d = ImageDraw.Draw(img, "RGBA")
            tile_s = TILE * s
            for gx in range(cx, cx+bg_w, tile_s):
                d.line([(gx, py), (gx, py+bg_h)], fill=(0,200,255,40), width=1)
            for gy in range(py, py+bg_h, tile_s):
                d.line([(cx, gy), (cx+bg_w, gy)], fill=(0,200,255,40), width=1)
        # label
        d = ImageDraw.Draw(img)
        d.text((cx, max_h + 2), lbl, fill=C_GRAY, font=_font_sml)
        cx += cell_w + 4

    return img

def make_thumbnail_row(filenum: int, label: str, max_dim: int = 256) -> Image.Image:
    """Muestra FileNum.png completo escalado como thumbnail de referencia."""
    h = max_dim + 30
    img = Image.new("RGBA", (PANEL_W, h), PANEL_BG)
    d = ImageDraw.Draw(img)

    src = open_img(filenum)
    if src is None:
        d.text((20, 20), f"[ARCHIVO FALTANTE: {filenum}.png]", fill=C_RED, font=_font_med)
        return img

    thumb = scale_to_fit(src.convert("RGBA"), max_dim, max_dim, nearest=True)
    # checkerboard detrás
    bg = checkerboard(thumb.width, thumb.height)
    bg.alpha_composite(thumb)
    img.alpha_composite(bg, (4, 4))

    # info
    d.text((4, bg.height + 8),
           f"{filenum}.png — {src.width}×{src.height}px — {src.mode}  |  {label}",
           fill=C_ORANGE, font=_font_sml)
    return img


# ── construcción de un NPC completo ─────────────────────────────────────────

def build_npc_panel(
    npc_id: str,
    name: str,
    section: str,
    body_id: int,
    walk_dirs: dict | None,   # {dir_name: [list of (fn,x,y,w,h)]}  para Walk1-4
    std_grh: int | None,      # GRH del campo Std
    filenum: int | None,      # FileNum del body (PNG del walk sheet)
    body_idle_grh: int | None,
    attack_grh: int | None,
    extra_sprites: list | None,  # [(gid, label)]
    notes: str = ""
) -> Image.Image:
    parts = []

    # encabezado
    parts.append(make_panel_header(name, f"Body={body_id}  FileNum={filenum}  StdGRH={std_grh}", npc_id, section))

    SCALE = 4

    # ── sección Walk1-4 ──────────────────────────────────────────────────────
    if walk_dirs:
        parts.append(make_row_label("▶ WALK ANIMATIONS (4 direcciones — arriba / derecha / abajo / izquierda)"))
        dir_order = [
            ("↑ Arriba  (Walk1)", walk_dirs.get("up",   [])),
            ("→ Derecha (Walk2)", walk_dirs.get("right",[])),
            ("↓ Abajo   (Walk3)", walk_dirs.get("down", [])),
            ("← Izq     (Walk4)", walk_dirs.get("left", [])),
        ]
        for dir_label, frame_specs in dir_order:
            parts.append(make_row_label(f"   {dir_label}", C_WHITE))
            sprites = []
            lbls    = []
            for i, (fn, x, y, w, h) in enumerate(frame_specs):
                spr = extract_sprite(fn, x, y, w, h)
                sprites.append(spr)
                lbls.append(f"f{i+1}\n{fn}.png\n{x},{y}\n{w}×{h}")
            parts.append(assemble_frames_row(sprites, lbls, scale=SCALE, max_h=140))

        # análisis de direcciones
        found_dirs = sum(1 for _, fs in dir_order if fs)
        parts.append(make_row_label(
            f"  ↳  Direcciones encontradas: {found_dirs}/4  |  "
            f"Frames por dir: {', '.join(str(len(v)) for v in walk_dirs.values())}",
            C_CYAN
        ))

    # ── sección FileNum sprite sheet ─────────────────────────────────────────
    if filenum:
        parts.append(make_row_label(f"▶ SPRITE SHEET (FileNum={filenum}.png — walking animations)"))
        parts.append(make_thumbnail_row(filenum, f"Sheet de animaciones de caminata — body_id={body_id}", max_dim=512))

    # ── sección StdGRH estático ───────────────────────────────────────────────
    if std_grh is not None:
        specs = resolve_grh(std_grh)
        if specs:
            parts.append(make_row_label(f"▶ STD GRH {std_grh} — sprite estático de referencia"))
            sprites = [extract_sprite(*s) for s in specs]
            lbls    = [f"GRH{std_grh}\n{s[0]}.png\n{s[1]},{s[2]}\n{s[3]}×{s[4]}" for s in specs]
            parts.append(assemble_frames_row(sprites, lbls, scale=SCALE, max_h=140))

    # ── BodyIdle ─────────────────────────────────────────────────────────────
    if body_idle_grh is not None:
        specs = resolve_grh(body_idle_grh)
        if specs:
            n = len(specs)
            parts.append(make_row_label(
                f"▶ BODY IDLE — GRH{body_idle_grh}  ({n} frame{'s' if n>1 else ''})  [DIRECT GRH]"
            ))
            sprites = [extract_sprite(*s) for s in specs]
            lbls    = [f"f{i+1}\n{s[0]}.png\n{s[1]},{s[2]}\n{s[3]}×{s[4]}" for i,s in enumerate(specs)]
            parts.append(assemble_frames_row(sprites, lbls, scale=SCALE, max_h=140))

    # ── Ataque ───────────────────────────────────────────────────────────────
    if attack_grh is not None:
        specs = resolve_grh(attack_grh)
        if specs:
            n = len(specs)
            parts.append(make_row_label(
                f"▶ ATAQUE — GRH{attack_grh}  ({n} frame{'s' if n>1 else ''})  [DIRECT GRH]"
            ))
            sprites = [extract_sprite(*s) for s in specs]
            lbls    = [f"f{i+1}\n{s[0]}.png\n{s[1]},{s[2]}\n{s[3]}×{s[4]}" for i,s in enumerate(specs)]
            parts.append(assemble_frames_row(sprites, lbls, scale=SCALE, max_h=140))

    # ── extras ───────────────────────────────────────────────────────────────
    if extra_sprites:
        for gid, elabel in extra_sprites:
            specs = resolve_grh(gid)
            if specs:
                parts.append(make_row_label(f"▶ {elabel} — GRH{gid}  ({len(specs)} frames)"))
                sprites = [extract_sprite(*s) for s in specs]
                lbls    = [f"f{i+1}\n{s[0]}.png\n{s[1]},{s[2]}\n{s[3]}×{s[4]}" for i,s in enumerate(specs)]
                parts.append(assemble_frames_row(sprites, lbls, scale=SCALE, max_h=140))

    # ── notas ────────────────────────────────────────────────────────────────
    if notes:
        note_img = Image.new("RGBA", (PANEL_W, 28), (40,40,20,255))
        d = ImageDraw.Draw(note_img)
        d.text((8, 8), f"⚠  {notes}", fill=C_ORANGE, font=_font_sml)
        parts.append(note_img)

    # ── separador ────────────────────────────────────────────────────────────
    sep = Image.new("RGBA", (PANEL_W, 8), BG)
    parts.append(sep)

    # unir verticalmente
    total_h = sum(p.height for p in parts)
    result = Image.new("RGBA", (PANEL_W, total_h), BG)
    y = 0
    for p in parts:
        pw = min(p.width, PANEL_W)
        region = p.crop((0, 0, pw, p.height))
        result.paste(region, (0, y))
        y += p.height
    return result


# ── definición de todos los NPCs candidatos ──────────────────────────────────

def grh_walk_dir(animated_gid: int) -> list:
    """Resuelve GRH animado a lista de (fn,x,y,w,h)."""
    return resolve_grh(animated_gid)

load_graficos()

npcs = []

# ══════════════════════════════════════════════════════════════════════════════
# SECCIÓN E — NPCs DE CIUDAD
# ══════════════════════════════════════════════════════════════════════════════

# E1 — Banquero NPC3 (Body=4106: Walk1=4622, Walk2=4624, Walk3=4621, Walk4=4623)
npcs.append({
    "npc_id": "NPC3",
    "name": "Banquero <Finanzas Goliath>",
    "section": "E — NPC Ciudad",
    "body_id": 4106,
    "walk_dirs": {
        "up":    grh_walk_dir(4622),
        "right": grh_walk_dir(4624),
        "down":  grh_walk_dir(4621),
        "left":  grh_walk_dir(4623),
    },
    "std_grh": None,
    "filenum": None,
    "body_idle_grh": 4489,
    "attack_grh": None,
    "extra_sprites": None,
    "notes": "Walk1-4 completo desde 4057.png (26×46/frame). BodyIdle: 437.png 32×32."
})

# E2 — Banquera NPC1272 (Body=30: Std=14, FileNum=1006)
npcs.append({
    "npc_id": "NPC1272",
    "name": "Banquera <Goliath Internacional>",
    "section": "E — NPC Ciudad",
    "body_id": 30,
    "walk_dirs": None,
    "std_grh": 14,
    "filenum": 1006,
    "body_idle_grh": 4489,
    "attack_grh": None,
    "extra_sprites": None,
    "notes": "Std/FileNum body. StdGRH=14 (5028.png 736/96 32×32). BodyIdle=4489 (437.png 32×32)."
})

# E3 — Sacerdote NPC1 (Body=117: Std=9, FileNum=1002; BodyIdle=861)
npcs.append({
    "npc_id": "NPC1",
    "name": "Sacerdote",
    "section": "E — NPC Ciudad",
    "body_id": 117,
    "walk_dirs": None,
    "std_grh": 9,
    "filenum": 1002,
    "body_idle_grh": 861,   # animado: 2 frames en 5130.png 96×64
    "attack_grh": None,
    "extra_sprites": None,
    "notes": "BodyIdle animado (2 frames, speed=111): Grh855+856 desde 5130.png."
})

# E4 — Sacerdotisa NPC1289 (Body=3258: Std=8, FileNum=1109; sin BodyIdle)
npcs.append({
    "npc_id": "NPC1289",
    "name": "Xanthe <Sacerdotisa de Forgat>",
    "section": "E — NPC Ciudad",
    "body_id": 3258,
    "walk_dirs": None,
    "std_grh": 8,
    "filenum": 1109,
    "body_idle_grh": None,
    "attack_grh": None,
    "extra_sprites": None,
    "notes": "Std/FileNum body. Sin BodyIdle explícito en npcs.dat."
})

# E5 — Mercader NPC15 (Body=5: Std=5, FileNum=1005)
npcs.append({
    "npc_id": "NPC15",
    "name": "Joseph <Mercader>",
    "section": "E — NPC Ciudad",
    "body_id": 5,
    "walk_dirs": None,
    "std_grh": 5,    # Grh5=5066.png 224/832 96×96
    "filenum": 1005,
    "body_idle_grh": None,
    "attack_grh": None,
    "extra_sprites": None,
    "notes": "StdGRH=5 → 5066.png @ (224,832) 96×96. Walking sheet en 1005.png."
})

# E6 — Mercader Durrot NPC105 (Body=217: Std=13, FileNum=1012)
npcs.append({
    "npc_id": "NPC105",
    "name": "Durrot <Mercader>",
    "section": "E — NPC Ciudad",
    "body_id": 217,
    "walk_dirs": None,
    "std_grh": 13,   # Grh13=5028.png 704/96 32×32
    "filenum": 1012,
    "body_idle_grh": None,
    "attack_grh": None,
    "extra_sprites": None,
    "notes": "StdGRH=13 → 5028.png @ (704,96) 32×32. Walking sheet en 1012.png."
})

# ══════════════════════════════════════════════════════════════════════════════
# SECCIÓN G — CRIATURAS
# ══════════════════════════════════════════════════════════════════════════════

# G1 — Goblin NPC522 (Body=4116: Std=40, FileNum=4047)
npcs.append({
    "npc_id": "NPC522",
    "name": "Goblin",
    "section": "G — Criatura",
    "body_id": 4116,
    "walk_dirs": None,
    "std_grh": 40,   # Grh40=5025.png 128/576 32×96
    "filenum": 4047,
    "body_idle_grh": None,
    "attack_grh": None,
    "extra_sprites": None,
    "notes": "StdGRH=40 → 5025.png @ (128,576) 32×96. Walking sheet en 4047.png (256×256)."
})

# G2 — Esqueleto deambulante NPC677 (Body=4376: Std=96, FileNum=4744; BodyIdle=4377; Ataque=4274)
npcs.append({
    "npc_id": "NPC677",
    "name": "Esqueleto deambulante",
    "section": "G — Criatura",
    "body_id": 4376,
    "walk_dirs": None,
    "std_grh": 96,   # Grh96=101.png 224/32 32×32
    "filenum": 4744,
    "body_idle_grh": 4377,    # Grh4377=4074.png 0/72 32×25
    "attack_grh": 4274,       # Grh4274=2004.png 27/47 27×47
    "extra_sprites": None,
    "notes": "Tres sprites de estado: body(4376), idle(4377), ataque(4274). Walking sheet en 4744.png."
})

# G3 — Esqueleto NPC514 (Body=4548: Walk1-4 completo desde 4079.png)
npcs.append({
    "npc_id": "NPC514",
    "name": "Esqueleto",
    "section": "G — Criatura",
    "body_id": 4548,
    "walk_dirs": {
        "up":    grh_walk_dir(50174),
        "right": grh_walk_dir(50176),
        "down":  grh_walk_dir(50173),
        "left":  grh_walk_dir(50175),
    },
    "std_grh": None,
    "filenum": None,
    "body_idle_grh": None,
    "attack_grh": None,
    "extra_sprites": None,
    "notes": "Walk1-4 completo. 4079.png, frames 25×52. Down=6f, Up=6f, Left=5f, Right=5f(reversed)."
})

# G4 — Zombie NPC507 (Body=4198: Std=127, FileNum=4795; BodyIdle=4197; Ataque=4196)
npcs.append({
    "npc_id": "NPC507",
    "name": "Zombie",
    "section": "G — Criatura",
    "body_id": 4198,
    "walk_dirs": None,
    "std_grh": 127,
    "filenum": 4795,
    "body_idle_grh": 4197,    # Grh4197=1003.png 0/224 32×32
    "attack_grh": 4196,       # Grh4196=5081.png 576/0 96×64
    "extra_sprites": None,
    "notes": "Std=127 FileNum=4795 (RGBA). BodyIdle → 1003.png. Ataque → 5081.png 96×64."
})

# G5 — Araña NPC512 (Body=4208: Std=30, FileNum=4792; BodyIdle=4207; Ataque=4206)
npcs.append({
    "npc_id": "NPC512",
    "name": "Araña",
    "section": "G — Criatura",
    "body_id": 4208,
    "walk_dirs": None,
    "std_grh": 30,
    "filenum": 4792,
    "body_idle_grh": 4207,    # Grh4207=5027.png 0/160 32×32
    "attack_grh": 4206,       # Grh4206=5027.png 128/128 32×32
    "extra_sprites": None,
    "notes": "Std=30 FileNum=4792 (RGBA). BodyIdle y Ataque ambos en 5027.png."
})

# G6 — Araña Negra NPC605 (Body=4424: Std=81, FileNum=4707; BodyIdle=4425)
npcs.append({
    "npc_id": "NPC605",
    "name": "Araña Negra",
    "section": "G — Criatura",
    "body_id": 4424,
    "walk_dirs": None,
    "std_grh": 81,
    "filenum": 4707,
    "body_idle_grh": 4425,    # Grh4425=5058.png 96/256 96×128
    "attack_grh": None,
    "extra_sprites": None,
    "notes": "BodyIdle → 5058.png @ (96,256) 96×128. Walking sheet en 4707.png (512×512)."
})

# G7 — Orco NPC510 (Body=4505: Walk1-4 completo desde 4015.png)
npcs.append({
    "npc_id": "NPC510",
    "name": "Orco",
    "section": "G — Criatura",
    "body_id": 4505,
    "walk_dirs": {
        "up":    grh_walk_dir(55759),
        "right": grh_walk_dir(55761),
        "down":  grh_walk_dir(55758),
        "left":  grh_walk_dir(55760),
    },
    "std_grh": None,
    "filenum": None,
    "body_idle_grh": None,
    "attack_grh": None,
    "extra_sprites": None,
    "notes": "Walk1-4 completo. 4015.png, frames 23-24×48-52. Down=6f, Up=6f, Left=5f, Right=5f."
})

# G8 — Orco Hechicero NPC1220 (Body=205: Std=1, FileNum=1012)
npcs.append({
    "npc_id": "NPC1220",
    "name": "Orco Hechicero",
    "section": "G — Criatura",
    "body_id": 205,
    "walk_dirs": None,
    "std_grh": 1,
    "filenum": 1012,
    "body_idle_grh": None,
    "attack_grh": None,
    "extra_sprites": None,
    "notes": "Std=1 FileNum=1012. Arma=55(GRH), Casco=38(GRH) en npcs.dat."
})

# G9 — Golem de Hielo NPC535 (Body=4536: Std=65, FileNum=4592)
npcs.append({
    "npc_id": "NPC535",
    "name": "Golem de Hielo",
    "section": "G — Criatura",
    "body_id": 4536,
    "walk_dirs": None,
    "std_grh": 65,
    "filenum": 4592,
    "body_idle_grh": None,
    "attack_grh": None,
    "extra_sprites": None,
    "notes": "Std=65, FileNum=4592.png (1024×1024). Speed=0.750 en cuerpos.dat."
})

# G10 — Golem de Piedra NPC536 (Body=4036: Std=65, FileNum=4580)
npcs.append({
    "npc_id": "NPC536",
    "name": "Golem de Piedra",
    "section": "G — Criatura",
    "body_id": 4036,
    "walk_dirs": None,
    "std_grh": 65,
    "filenum": 4580,
    "body_idle_grh": None,
    "attack_grh": None,
    "extra_sprites": None,
    "notes": "Std=65, FileNum=4580.png (1024×1024). Speed=0.750 en cuerpos.dat."
})

# G11 — Golem Infernal NPC944 (Body=4546: Std=65, FileNum=4593)
npcs.append({
    "npc_id": "NPC944",
    "name": "Golem Infernal",
    "section": "G — Criatura",
    "body_id": 4546,
    "walk_dirs": None,
    "std_grh": 65,
    "filenum": 4593,
    "body_idle_grh": None,
    "attack_grh": None,
    "extra_sprites": None,
    "notes": "Std=65, FileNum=4593.png (1024×1024). Speed=0.750 en cuerpos.dat."
})

# ══════════════════════════════════════════════════════════════════════════════
# SECCIÓN F — PUESTITOS (inspección de sheets de ciudad)
# ══════════════════════════════════════════════════════════════════════════════
city_sheets = [5064, 5066, 5034, 5028, 5040, 5032, 5033, 5025, 5035, 5044]

# ── render ───────────────────────────────────────────────────────────────────

print("Generando contact sheets...")

# Un archivo por NPC
for npc in npcs:
    panel = build_npc_panel(
        npc_id        = npc["npc_id"],
        name          = npc["name"],
        section       = npc["section"],
        body_id       = npc["body_id"],
        walk_dirs     = npc.get("walk_dirs"),
        std_grh       = npc.get("std_grh"),
        filenum       = npc.get("filenum"),
        body_idle_grh = npc.get("body_idle_grh"),
        attack_grh    = npc.get("attack_grh"),
        extra_sprites = npc.get("extra_sprites"),
        notes         = npc.get("notes", "")
    )
    fname = f"{OUT_DIR}preview_{npc['npc_id']}_{npc['name'].replace(' ','_').replace('<','').replace('>','').replace('/','')[:30]}.png"
    panel.convert("RGB").save(fname)
    print(f"  ✓ {fname.split('/')[-1]}")

# ── Sección F: city sprite sheets ────────────────────────────────────────────
print("\nGenerando inspección de sheets de ciudad (Sección F)...")
F_COLS = 2
F_SCALE_MAX = 600
cell_w = F_SCALE_MAX + 20
cell_h = F_SCALE_MAX + 50
rows_needed = (len(city_sheets) + F_COLS - 1) // F_COLS
f_img = Image.new("RGB", (cell_w * F_COLS + 20, cell_h * rows_needed + 60), BG)
d = ImageDraw.Draw(f_img)
d.text((10, 10), "SECCIÓN F — Inspección visual de sprite sheets de ciudad (candidatos puestitos)", fill=C_HEADER, font=_font_big)
d.text((10, 30), "Buscar: mesas, mostradores, toldos, puestos, stands. Cruz roja = esquina superior izq del sheet.", fill=C_GRAY, font=_font_sml)

for idx, filenum in enumerate(city_sheets):
    col = idx % F_COLS
    row = idx // F_COLS
    px = col * (cell_w + 10) + 10
    py = row * (cell_h + 10) + 60

    src = open_img(filenum)
    if src is None:
        d.text((px, py+20), f"{filenum}.png FALTANTE", fill=C_RED, font=_font_med)
        continue

    thumb = src.convert("RGB").resize(
        (min(F_SCALE_MAX, src.width), min(F_SCALE_MAX, src.height)),
        Image.NEAREST
    )
    # escalar al cuadrado máximo manteniendo proporción
    ratio = min(F_SCALE_MAX/src.width, F_SCALE_MAX/src.height)
    nw = int(src.width * ratio)
    nh = int(src.height * ratio)
    thumb = src.convert("RGBA").resize((nw, nh), Image.NEAREST)

    # fondo blanco para ver transparencias
    bg = Image.new("RGBA", (nw, nh), (255, 255, 255, 255))
    bg.alpha_composite(thumb)

    f_img.paste(bg.convert("RGB"), (px, py))

    # grilla de tiles escalada
    tile_px = int(32 * ratio)
    d2 = ImageDraw.Draw(f_img)
    for gx in range(px, px+nw, tile_px):
        d2.line([(gx, py), (gx, py+nh)], fill=(0,200,200,), width=1)
    for gy in range(py, py+nh, tile_px):
        d2.line([(px, gy), (px+nw, gy)], fill=(0,200,200,), width=1)

    # label
    d2.rectangle([px, py+nh, px+nw, py+nh+18], fill=(20,20,20))
    d2.text((px+2, py+nh+2), f"{filenum}.png  {src.width}×{src.height}  {src.mode}  ratio={ratio:.2f}x", fill=C_ORANGE, font=_font_sml)

fname_f = f"{OUT_DIR}preview_seccionF_city_sheets.png"
f_img.save(fname_f)
print(f"  ✓ {fname_f.split('/')[-1]}")

print(f"\n✅ Todos los previews generados en {OUT_DIR}")
print(f"   Archivos NPCs: {len(npcs)}")
print(f"   City sheets: {len(city_sheets)}")
