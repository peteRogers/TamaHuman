#!/usr/bin/env python3
"""Turn a capture log into drawing templates.

1. In the Arduino IDE Serial Monitor (115200 baud), play the game for a while,
   then copy everything and paste it into a text file, e.g. capture.txt
2. Run:  python3 tools/log_to_templates.py capture.txt
3. Open templates/  - every shape the game drew is there as a PNG, already
   scaled up to the size it appears on the OLED (grey = original pixels).
   Copy the ones you want to replace into sprites/, keep the file name, and
   draw over them in black. Delete the grey when you're done (or leave it -
   grey counts as "off").

Needs: pip install pillow
"""
import pathlib, sys
from PIL import Image

SCALE = 3          # match SCALE in TamaHuman.ino
ROOT = pathlib.Path(__file__).resolve().parent.parent
OUT = ROOT / "templates"
OUT.mkdir(exist_ok=True)

if len(sys.argv) < 2:
    sys.exit("usage: python3 tools/log_to_templates.py capture.txt")

count = 0
for line in open(sys.argv[1], errors="ignore"):
    parts = line.split()
    if len(parts) < 4 or parts[0] != "SHAPE":
        continue
    h, w, ht = parts[1], int(parts[2]), int(parts[3])
    rows = [int(r, 16) for r in parts[4:4 + ht]]
    img = Image.new("RGBA", (w * SCALE, ht * SCALE), (0, 0, 0, 0))
    for y, row in enumerate(rows):
        for x in range(w):
            if row >> (w - 1 - x) & 1:
                for dy in range(SCALE):
                    for dx in range(SCALE):
                        img.putpixel((x * SCALE + dx, y * SCALE + dy), (190, 190, 190, 255))
    path = OUT / f"{h}.png"
    if not path.exists():
        img.save(path)
        count += 1
print(f"saved {count} new template(s) in templates/")
