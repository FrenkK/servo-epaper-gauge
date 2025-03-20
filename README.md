# servo-epaper-gauge
A simple triple gauge using three servos and a 3-color e-paper display used for indicating a solar system state.
The project uses 3 90g servos to move needles and an e-paper display to display both gauge markings and extra information about the system state.

# STEP files
The project includes STEP files for 3d-printing an enclosure:
- Ogrodje za tri.step is the mounting plate for the servos and the e-paper display (Waveshare 3-color 4.2") mounting plate.
- Ohišje za semafor lope.step is the housing with a cover. You have to drill a hole for the power cable in an appropriate spot. If exposed to elements, glue an acryllic plate on the inside using silicone to make it watertight. Finally, slide the mounting plate inside and use a drop of superglue to mount the cover.
- Kazalec za lopo.step contains two versions of needles for the servos, short one is for the two side servos and the long one for the center one.

# Arduino code
This is the code for the ESP32 controller driving the display and servos. The header files are for the GxEPD2 library and contain display definition and pin connections.

