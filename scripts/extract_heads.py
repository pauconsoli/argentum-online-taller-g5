#!/usr/bin/env python3
"""Extract head sprites from Recursos/Graficos into assets/sprites/characters/heads/.

Each head is saved as a 27x256 RGBA PNG with 4 direction strips (top to bottom):
  y=0:   South (Head1)
  y=64:  North (Head2)
  y=128: West  (Head3)
  y=192: East  (Head4)
"""

import re
import sys
from pathlib import Path
from PIL import Image

RECURSOS = Path(__file__).parent.parent / "Recursos"
GRAFICOS_INI = RECURSOS / "init" / "graficos.ini"
CABEZAS_INI = RECURSOS / "init" / "cabezas.ini"
GRAFICOS_DIR = RECURSOS / "Graficos"
OUTPUT_DIR = Path(__file__).parent.parent / "assets" / "sprites" / "characters" / "heads"

FRAME_W = 27
FRAME_H = 64
NUM_DIRS = 4


def load_grh_index(path):
    """Return dict: grhId (int) -> (filenum, x, y, w, h)."""
    index = {}
    with open(path, encoding="latin-1") as f:
        for line in f:
            m = re.match(r"Grh(\d+)=(\d+)-(\d+)-(\d+)-(\d+)-(\d+)-(\d+)", line.strip())
            if m:
                gid, frames, filenum, x, y, w, h = (int(v) for v in m.groups())
                if frames == 1:
                    index[gid] = (filenum, x, y, w, h)
    return index


def load_cabezas(path):
    """Return list of (head1_gid, head2_gid, head3_gid, head4_gid) indexed by head number."""
    heads = {}
    with open(path, encoding="latin-1") as f:
        content = f.read()
    total_m = re.search(r"NumHeads=(\d+)", content)
    if not total_m:
        raise RuntimeError("NumHeads not found in cabezas.ini")
    num_heads = int(total_m.group(1))
    for n in range(1, num_heads + 1):
        section = re.search(rf"\[HEAD{n}\](.*?)(?=\[HEAD|\Z)", content, re.DOTALL)
        if not section:
            continue
        text = section.group(1)
        dirs = []
        for d in range(1, 5):
            m = re.search(rf"Head{d}=(\d+)", text)
            dirs.append(int(m.group(1)) if m else 0)
        heads[n] = tuple(dirs)
    return heads, num_heads


def get_png(filenum, png_cache):
    if filenum not in png_cache:
        path = GRAFICOS_DIR / f"{filenum}.png"
        if not path.exists():
            return None
        png_cache[filenum] = Image.open(path).convert("RGBA")
    return png_cache[filenum]


def extract_head(head_dirs, grh_index, png_cache):
    """Build a 27x256 RGBA strip: south, north, west, east (top to bottom)."""
    out = Image.new("RGBA", (FRAME_W, FRAME_H * NUM_DIRS), (0, 0, 0, 0))
    dir_order = [0, 1, 2, 3]  # Head1=south, Head2=north, Head3=west, Head4=east
    for row, dir_idx in enumerate(dir_order):
        gid = head_dirs[dir_idx]
        if gid == 0 or gid not in grh_index:
            continue
        filenum, x, y, w, h = grh_index[gid]
        img = get_png(filenum, png_cache)
        if img is None:
            continue
        crop = img.crop((x, y, x + FRAME_W, y + FRAME_H))
        out.paste(crop, (0, row * FRAME_H))
    return out


def main():
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    print("Loading graficos index...")
    grh_index = load_grh_index(GRAFICOS_INI)
    print(f"  {len(grh_index)} Grh entries loaded")

    print("Loading cabezas.ini...")
    heads, num_heads = load_cabezas(CABEZAS_INI)
    print(f"  {num_heads} heads found")

    png_cache = {}
    missing = 0
    for n in range(1, num_heads + 1):
        if n not in heads:
            missing += 1
            continue
        head_dirs = heads[n]
        strip = extract_head(head_dirs, grh_index, png_cache)
        out_path = OUTPUT_DIR / f"head_{n}.png"
        strip.save(out_path)
        if n % 100 == 0:
            print(f"  Extracted head {n}/{num_heads}")

    print(f"Done. {num_heads - missing} heads extracted to {OUTPUT_DIR}")
    if missing:
        print(f"  {missing} heads missing from cabezas.ini")


if __name__ == "__main__":
    main()
