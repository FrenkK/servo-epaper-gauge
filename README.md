# Triple analog gauge using servos and and e-paper display
A simple triple gauge using three servos and a 3-color e-paper display used for indicating a solar system state.
The project uses 3 90g servos to move needles and an e-paper display to display both gauge markings and extra information about the system state.
Inspired by [Arne van Iterson](https://arnweb.nl/gitea/arne/EinkAnalog).

# What you need
- An ESPP32 dev board
- Waveshare 4.2" 3-color e-paper display (there are two versions, one with screw holes in the back and one with screw holes next to the display, you need the first one or the mounting biard will not fit)
- 3 90g servos
- Some screws from your screw drawer ;)
- A 3D-printer
- Weatherproof filament like ASA or ABS
- Some acryllic to make the window

# STEP files
The project includes STEP files for 3d-printing an enclosure:
- Ogrodje za tri.step is the mounting plate for the servos and the e-paper display (Waveshare 3-color 4.2") mounting plate.
- Ohišje za semafor lope.step is the housing with a cover. Print this in something that will take direct sunlight, like ABS or ASA. You have to drill a hole for the power cable in an appropriate spot. If exposed to elements, glue an acryllic plate on the inside using silicone to make it watertight. Finally, slide the mounting plate in the grooves inside and use a drop of superglue to mount the cover.
- Kazalec za lopo.step contains two versions of needles for the servos, short one is for the two side servos and the long one for the center one.

# Arduino code
This is the code for the ESP32 controller driving the display and servos. The header files are for the GxEPD2 library and contain display definition and pin connections.

# Connections
- E-paper display should be connected to 3.3V and GND power rails of the ESP32 and the pins listed in the 'GxEPD2_wiring_examples.h' header file for the exact ESP32 board you are using.
- Connect the servos power lines to the 5V and GND power rails of the ESP32 board, and then connect the control lines to the pins set in the Arduino code (25, 26, and 27 in my case).
- Finally, connect the ESP32 board to an USB power cable or directly solder a 1A, 5V voltage supply to the 5V and GND power rails of the ESP32 board.

# TBD
- OTA firmware upload
- Better Wi-fi disturbances handling

Happy DIY :)
Franci
