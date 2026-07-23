# Android Programming Quiz 3 - Fan Dashboard HUD

This project extends Quiz 2 without changing the original Quiz 2 source.

Live WebGL version: [Assignment 3 - HUD](https://sebaslin993.github.io/CSD2101/Assignment-3-HUD/)

## Controls

- Top-left upper button: increase fan speed
- Top-left lower button: decrease fan speed
- Top-right upper button: zoom in
- Top-right lower button: zoom out
- Right-middle vertical strip: continuous camera zoom progress, filling blue toward the nearest view
- Click or tap a blade, the hub, or the base to toggle its highlight independently
- Multiple blades can remain highlighted; the vertical pole is not selectable

The entire fan assembly rotates automatically at a fixed rate while the blades spin independently. The speed controls affect only the blades.

Fan speed is clamped to 0-20 and is shown by the 20-block bar at the bottom.
Camera distance is tuned and clamped to 3-9 to match the supplied closest/farthest views. The 0.4 step requires exactly 15 button presses from one limit to the other.

## Bonus Features

### Colour-coded picked-part indicator

A second 20-cell HUD strip above the speed bar reports highlighted parts without fonts or textures. Each highlighted blade lights its matching blade-number cell orange, so multiple selected blades produce multiple orange cells. Separate side swatches turn silver for the base and gold for the motor hub; toggling a part off clears only its own indicator.

## Architecture

- `Scene3D` owns the Phong-lit fan, procedural checkerboard ground, centred camera, and picking.
- The fan uses the supplied `Models` OBJ primitives through TinyObjLoader, with procedural meshes retained only as a runtime fallback.
- `SceneHUD` owns the four screen-space controls, speed bar, continuous zoom bar, and picked-part indicator.
- `Renderer` draws Scene3D with depth testing, disables depth for SceneHUD, then restores depth testing.

The same shared C++ scene code is used by Desktop, Android, and WebGL builds.
