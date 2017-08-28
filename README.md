# esp32_color_dmd
Experiment with using an ESP32 to drive 64x32 color LED matrix

I am using Hub75 type matrix boards.

Connect the pints from the ESP32 module as follows:

ESP32 IO pin to HUB75
0 - 1
1 - 2
2 - 3
    4
3 - 5
4 - 6
5 - 7
    8
12 - 9
13 - 10
14 - 11
15 - 12
16 - 13
17 - 14
32 - 15
gnd - 16

The HUB75 inputs are not necessarily 3.3V logic and seem to run off the supply voltage, typically 5V.
However I am able to get decent current and brightness at 3.3V and this makes the input logic levels work.
The output clock rate from the crude bit-banging is running at 10MHz, the refresh rate is thus about 10KHz for a single module.

