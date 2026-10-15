# TamaHuman

A real Tamagotchi P1 running on an Arduino Uno R4 WiFi with a 128×64 SSD1306 OLED.
You can replace the pet with your own higher-detail drawings. The game logic is
the original, and only the pictures change.

## You need

- Arduino Uno R4 WiFi
- SSD1306 128×64 OLED, I²C version (4 pins)
- 3 push buttons
- A passive buzzer (optional)
- Arduino IDE 2, with **Arduino UNO R4 Boards** (Boards Manager) and **U8g2** (Library Manager)
- Python 3 with Pillow (`pip3 install pillow`) for the drawing tools

## Wiring

| Part | Pin |
|---|---|
| OLED VCC / GND | 5V (or 3.3V, check your module) / GND |
| OLED SDA / SCL | SDA / SCL (next to AREF, same as A4 / A5) |
| Button A (left) | D2 → button → GND |
| Button B (middle) | D3 → button → GND |
| Button C (right) | D4 → button → GND |
| Buzzer | D5 → buzzer → GND |

## First run

1. Open `TamaHuman/TamaHuman.ino` in the Arduino IDE and upload it.
2. An egg appears. Press **B** to set the clock: **A** changes the hour, **B** the minutes, and **C** confirms.
3. The egg hatches a few minutes later. The baby is already replaced by an example stick figure
   (the files in `sprites/` ending in `-baby-example`) to show how it works.

## Drawing your own human

**1. Capture the shapes.** Open the Serial Monitor at 115200 baud and play. Each time the game draws a
pet-sized shape that hasn't been replaced yet, it prints it:

```
SHAPE 0775b7e5 6 6 1e 2d 3f 33 3f 1e
  .####.
  #.##.#
  ...
```

The 8 characters after `SHAPE` are the shape's **hash**, its unique name.

**2. Make templates.** Copy everything from the Serial Monitor into a text file (say `capture.txt`),
then run:

```
python3 tools/log_to_templates.py capture.txt
```

`templates/` now holds a PNG of every shape, scaled up to the size it appears on the OLED.
There's already a set from a test run, covering the egg, hatching and baby.

**3. Draw.** Copy a template into `sprites/` and draw your version in black on top of it, using
Piskel, Aseprite, or any editor that saves PNGs.

- **Keep the hash at the start of the filename.** You can add a note after a dash, like `0775b7e5-baby-front.png`.
- Black pixels are drawn. Anything else (white, grey, transparent) is left off.
- Any size up to about **48×48** works. Your drawing is placed so its **bottom-centre** sits where the
  original shape's bottom-centre was, which keeps the feet on the ground.
- Only draw each pose facing one way. If the pet turns around, your drawing is flipped automatically.

**4. Build and upload.**

```
python3 tools/make_sprites.py
```

This rewrites `TamaHuman/human_sprites.h`. Upload again, and your drawings replace those shapes.

Tip: finish one life stage at a time (the baby has about 6 frames), so you see results quickly.

## Settings (top of `TamaHuman.ino`)

| Setting | What it does |
|---|---|
| `SCALE 3` | Game area 96×48 with the menu icons above and below. Sprites up to ~48×48. |
| `SCALE 4` | Game fills the whole 128×64 screen (icons draw over the top). Sprites up to ~64×64. Also change `SCALE` in `tools/log_to_templates.py`. |
| `CAPTURE 0` | Stop printing shapes once you're done drawing. |
| `CAPTURE_ALL 1` | Also print text, digits and tiny bits (normally skipped). |

## Files

- `TamaHuman/TamaHuman.ino`: the board code (buttons, screen, buzzer)
- `TamaHuman/renderer.h`: finds each shape, swaps in drawings, and runs capture mode
- `TamaHuman/icons.h`: the 8 menu icons (8×8). You can redraw these too.
- `TamaHuman/human_sprites.h`: generated from `sprites/`. Don't edit it by hand.
- `TamaHuman/tama_rom.h`: the game ROM, made with `tools/rom_to_header.py`
- `TamaHuman/src/tamalib/`: the TamaLIB emulator by Jean-Christophe Rona (GPL-2.0)

## Known limits

- The pet resets when the power goes off (there is no save feature yet).
- If two things touch on screen (for example the pet eating food), they count as one new shape.
  That combined shape shows the original pixels until you draw it too.
