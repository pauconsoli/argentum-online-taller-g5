#!/usr/bin/env python3
"""Extract inventory item icons from Recursos/Graficos into assets/sprites/items/.

Each item is saved as a 32x32 RGBA PNG named item_N.png.
Items with non-32x32 source sprites are resized to fit 32x32 with padding.
Items whose source PNG is missing are skipped.
"""

import argparse
import re
import sys
from pathlib import Path
from PIL import Image

RECURSOS = Path(__file__).parent.parent / "Recursos"
GRAFICOS_INI = RECURSOS / "init" / "graficos.ini"
OBJ_DAT = RECURSOS / "Dat" / "obj.dat"
GRAFICOS_DIR = RECURSOS / "Graficos"
OUTPUT_DIR = Path(__file__).parent.parent / "assets" / "sprites" / "items"

ICON_SIZE = 32


def load_grh_index(path):
    index = {}
    with open(path, encoding="latin-1") as f:
        for line in f:
            m = re.match(r"Grh(\d+)=(\d+)-(\d+)-(\d+)-(\d+)-(\d+)-(\d+)", line.strip())
            if m:
                gid, frames, filenum, x, y, w, h = (int(v) for v in m.groups())
                if frames == 1:
                    index[gid] = (filenum, x, y, w, h)
    return index


def load_obj_dat(path):
    """Return dict: obj_id -> grh_index (only items with a GrhIndex)."""
    items = {}
    with open(path, encoding="latin-1") as f:
        content = f.read()
    total_m = re.search(r"NumOBJs=(\d+)", content)
    if not total_m:
        raise RuntimeError("NumOBJs not found in obj.dat")
    num_objs = int(total_m.group(1))
    for n in range(1, num_objs + 1):
        section = re.search(rf"\[OBJ{n}\](.*?)(?=\[OBJ|\Z)", content, re.DOTALL)
        if not section:
            continue
        grh_m = re.search(r"GrhIndex=(\d+)", section.group(1), re.I)
        if grh_m:
            items[n] = int(grh_m.group(1))
    return items, num_objs


def get_png(filenum, png_cache):
    if filenum not in png_cache:
        path = GRAFICOS_DIR / f"{filenum}.png"
        if not path.exists():
            return None
        png_cache[filenum] = Image.open(path).convert("RGBA")
    return png_cache[filenum]


def extract_icon(gid, grh_index, png_cache):
    """Return a 32x32 RGBA icon for the given Grh ID, or None on failure."""
    if gid not in grh_index:
        return None
    filenum, x, y, w, h = grh_index[gid]
    img = get_png(filenum, png_cache)
    if img is None:
        return None
    crop = img.crop((x, y, x + w, y + h))
    if w == ICON_SIZE and h == ICON_SIZE:
        return crop
    # Fit into 32x32 with transparent padding
    out = Image.new("RGBA", (ICON_SIZE, ICON_SIZE), (0, 0, 0, 0))
    scale = min(ICON_SIZE / w, ICON_SIZE / h)
    new_w = max(1, int(w * scale))
    new_h = max(1, int(h * scale))
    resized = crop.resize((new_w, new_h), Image.LANCZOS)
    offset_x = (ICON_SIZE - new_w) // 2
    offset_y = (ICON_SIZE - new_h) // 2
    out.paste(resized, (offset_x, offset_y))
    return out


def load_item_names(path, ids):
    """Return dict: obj_id -> name for the given ids."""
    names = {}
    with open(path, encoding="latin-1") as f:
        content = f.read()
    for n in ids:
        m = re.search(rf"\[OBJ{n}\](.*?)(?=\[OBJ|\Z)", content, re.DOTALL)
        if m:
            name_m = re.search(r"Name=(.+)", m.group(1), re.I)
            names[n] = name_m.group(1).strip() if name_m else f"OBJ{n}"
    return names


def main():
    parser = argparse.ArgumentParser(description="Extract item icons from obj.dat")
    parser.add_argument(
        "--ids",
        nargs="+",
        type=int,
        metavar="N",
        help="Extract only these OBJ numbers (default: all)",
    )
    args = parser.parse_args()

    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    print("Loading graficos index...")
    grh_index = load_grh_index(GRAFICOS_INI)
    print(f"  {len(grh_index)} Grh entries loaded")

    print("Loading obj.dat...")
    items, num_objs = load_obj_dat(OBJ_DAT)
    print(f"  {len(items)}/{num_objs} items have a GrhIndex")

    if args.ids:
        filter_set = set(args.ids)
        items = {n: gid for n, gid in items.items() if n in filter_set}
        names = load_item_names(OBJ_DAT, filter_set)
        print(f"  Filtering to {len(items)} items: {sorted(filter_set)}")

    png_cache = {}
    extracted = 0
    skipped = 0
    results = []
    for n, gid in sorted(items.items()):
        icon = extract_icon(gid, grh_index, png_cache)
        if icon is None:
            skipped += 1
            if args.ids:
                results.append((n, names.get(n, "?"), None))
            continue
        out_path = OUTPUT_DIR / f"item_{n}.png"
        icon.save(out_path)
        extracted += 1
        if args.ids:
            results.append((n, names.get(n, "?"), out_path.name))
        elif extracted % 500 == 0:
            print(f"  Extracted {extracted} items...")

    if args.ids and results:
        print()
        print(f"{'OBJ#':<8} {'Archivo':<16} {'Nombre'}")
        print("-" * 60)
        for n, name, fname in results:
            status = fname if fname else "SKIPPED"
            print(f"OBJ{n:<5} {status:<16} {name}")

    print(f"\nDone. {extracted} items extracted to {OUTPUT_DIR} ({skipped} skipped)")


if __name__ == "__main__":
    main()
