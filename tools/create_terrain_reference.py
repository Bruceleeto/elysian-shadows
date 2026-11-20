#!/usr/bin/env python3
"""Copy original Loren terrain verbatim, removing scenery and replacing the atlas.

Run with --write to regenerate assets/level/rewrite/terrain_test; otherwise verify.
Tile IDs are deliberately retained so the original tileset physics is unchanged.
"""
import argparse
import collections
from pathlib import Path
import re
import struct

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "assets/level/release/foloi_forest"
TARGET = ROOT / "assets/level/rewrite/terrain_test"
SOURCE_MAP = SOURCE / "area/loren_town/map.bin"
TARGET_MAP = TARGET / "area/terrain/map.bin"

# Atlas inspection: left-hand grass/cliff/path cells, including the actual
# Loren stairs (7/23/39/55). The tree canopy occupies the right-hand cells.
LAND = set(range(1, 8))
for row in range(1, 4):
    LAND.update(range(row * 16, row * 16 + 8))
for row in range(4, 8):
    LAND.update(range(row * 16, row * 16 + 5))
for row in range(8, 16):
    LAND.update(range(row * 16, row * 16 + 4))
# Brown ground-edge tile, outside the main grass block.
LAND.add(58)
WATER = {352, 353, 368, 369, 384, 385, 400, 401}
for row, columns in ((26, 4), (27, 4), (28, 4), (29, 6), (30, 6), (31, 4)):
    WATER.update(range(row * 16, row * 16 + columns))
KEEP = LAND | WATER
STAIRS = {7, 23, 39, 55}


def read_map(path):
    raw = path.read_bytes()
    prefix = 12 if raw.startswith(b"ESMAP\r\n\x1a") else 0
    layers = 3
    if prefix:
        version, layers = struct.unpack_from("<HH", raw, 8)
        assert version == 1 and layers in (3, 4), "Unsupported map version/layers"
    width, height = struct.unpack_from("<HH", raw, prefix + 100)
    assert len(raw) == prefix + 104 + width * height * layers * 4, "Unexpected map format"
    # This reference check compares original terrain in the first three layers.
    # A fourth layer is editable additional geometry, not part of the reference.
    return raw[prefix:], width, height, struct.unpack_from(f"<{width * height * 3}I", raw, prefix + 104)


def original_spawn():
    text = (SOURCE / "area/loren_town/area_table.lua").read_text()
    block = re.search(r"startPosition\s*=\s*Vector3:new\s*\{([^}]+)", text).group(1)
    return [re.search(rf"\b{axis}\s*=\s*([-\d.]+)", block).group(1) for axis in "xyz"]


def generate():
    from PIL import Image

    raw, width, height, entries = read_map(SOURCE_MAP)
    filtered = [value if value & 511 in KEEP else 0 for value in entries]
    TARGET_MAP.parent.mkdir(parents=True, exist_ok=True)
    (TARGET / "tile").mkdir(parents=True, exist_ok=True)
    TARGET_MAP.write_bytes(b"ESMAP\r\n\x1a" + struct.pack("<HH", 1, 4) + raw[:104] +
                           struct.pack(f"<{len(filtered)}I", *filtered) + bytes(width * height * 4))
    # Copy every collider/standOn/damping/animation setting, without reinterpretation.
    (TARGET / "tile/tileset.txt").write_bytes((SOURCE / "tile/tileset.txt").read_bytes())
    atlas = Image.new("RGBA", (512, 1024))
    prototypes = ROOT / "assets/rewrite/prototype/1 Tiles"
    sheets = {i: Image.open(prototypes / f"Tileset{i}.png").convert("RGBA") for i in (1, 2, 3, 4)}
    for index in KEEP:
        # Green land, grey cliffs/path, purple stairs, blue water. Use supplied
        # 32px placeholder pixels, with no scaling or changes to placed tile IDs.
        sheet = 2 if index in WATER else 4 if index in STAIRS else 3 if index in set(range(48, 85)) | {2, 5} else 1
        tile = sheets[sheet].crop((0, 0, 32, 32))
        atlas.paste(tile, (index % 16 * 32, index // 16 * 32))
    atlas.save(TARGET / "tile/tilesheet.png")
    x, y, z = original_spawn()
    (TARGET_MAP.parent / "area_table.lua").write_text(f'''return {{
  nameString = "Loren Town - terrain reference",
  size = {{ width = {width}, height = {height} }},
  useSun = true,
  ambientColor = Vector4:new {{ x = 0.6, y = 0.6, z = 0.6, w = 1.0 }},
  music = {{
    bgm = {{ [1] = "", [2] = "", [3] = "" }},
    aux = {{ [1] = "", [2] = "" }},
    gain = 1.0,
  }},
  activeEntities = {{ }},
  inactiveEntities = {{ }},
  startPosition = Vector3:new {{ x = {x}, y = {y}, z = {z} }},
}}
''')


def verify():
    source, width, height, original = read_map(SOURCE_MAP)
    target, out_width, out_height, copied = read_map(TARGET_MAP)
    assert (width, height) == (150, 150) == (out_width, out_height)
    assert target[:104] == source[:104], "Map dimensions/header changed"
    count = collections.Counter()
    for i, (old, new) in enumerate(zip(original, copied)):
        keep = old & 511 in KEEP
        assert new == (old if keep else 0), f"Changed terrain word at slot {i}"
        if old & 511:
            count["retained" if keep else "removed"] += 1
        if old & 511 in STAIRS:
            assert new == old
            count["stairs"] += 1
    assert count["stairs"] == 68, "Original stair inventory changed"
    assert (TARGET / "tile/tileset.txt").read_bytes() == (SOURCE / "tile/tileset.txt").read_bytes()
    area = (TARGET_MAP.parent / "area_table.lua").read_text()
    assert "activeEntities = { }" in area and "inactiveEntities = { }" in area
    x, y, z = original_spawn()
    assert f"x = {x}, y = {y}, z = {z}" in area
    print(f"PASS: {width}x{height}; {count['retained']} terrain entries unchanged; "
          f"all {count['stairs']} stair entries identical; {count['removed']} scenery entries removed; "
          "original physics settings and spawn preserved; no entities")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--write", action="store_true")
    args = parser.parse_args()
    if not SOURCE_MAP.exists() or (not args.write and not TARGET_MAP.exists()):
        print("SKIP: terrain reference assets unavailable")
        raise SystemExit(77)
    if args.write:
        generate()
    verify()
