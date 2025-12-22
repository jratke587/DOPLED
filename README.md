# DOPLED
The **DOPLED** library allows you to use an ESP32 to control low-speed Data Over Power (DOP) LED strings, such as those distributed by Hello Fairy and Dazzle Bright. These LEDs are unique since they do not use a data wire and can be wired in series to use higher voltages.

For detailed reference, check out the [DOPLED Class Reference](https://jratke587.github.io/DOPLED/classDOPLED.html).
Example sketches are included with the library.

## Supported LEDs
This library drives some 2-wire addressable LEDs that send data over the power wires. While part number of these LEDs is not known, they generally have a cloudy white 5mm conical housing. These are NOT the same LEDs used in Twinkly products.

<img width="1008" height="756" alt="Photo of an indiviual LED from the string" src="https://github.com/user-attachments/assets/d2b60ca7-b27e-48a6-8158-b97da1885c1e" />

These lights are often sold in strings using the 'Hello Fairy', 'Dazzle Bright', or 'Surplife' app.

## LED Behavior
While these LEDs can be addressed individually, strings typically contain multiple LEDs that share the same address which means they are not addressable in the traditional sense. 
For example: 
- Hello Fairy 200 string has 25 addresses repeating sequentially (0, 1, ..., 24, 0, 1, ...)
- Dazzle Bright 500 string has 125 addresses in adjacent groups of 4 (0, 0, 0, 0, 1, 1, 1, 1, ... 124, 124, 124, 124).

There are 4 ways to control these LEDS:
- [DOPLED::fillAll](https://jratke587.github.io/DOPLED/classDOPLED.html#ad8be7b575134303325a3e33c118342dc) - Sets the entire string to a specific color.
- [DOPLED::writePixel](https://jratke587.github.io/DOPLED/classDOPLED.html#a2aba7b99872eb10defcc4e5e705eb41b) - Sets a specific address to a specific color.
- [DOPLED::fillMatchingAddresses](https://jratke587.github.io/DOPLED/classDOPLED.html#ad6524cea9fe8db586872e04ee4e0fa95) - Uses an address and a bitmask to set addresses that match the specified bits.
- [DOPLED::fillRandomGroups](https://jratke587.github.io/DOPLED/classDOPLED.html#a1f824dba50ea91a8cb02e3567cede7cf) - Each LED will assign itself to a group randomly which can then be addressed.

These LEDs use a very slow protocol (up to 14ms per command), so addressing in groups is important for smooth effects.

## Hardware Setup
Remove the original controller and connect the lights via an N-Channel Mosfet as shown in the diagram. Note that the striped wire on the light string is positive.
<img width="1585" height="620" alt="image" src="https://github.com/user-attachments/assets/dd1dcf51-4744-48a8-81f8-61d53201b8bb" />

Altenatively, the existing controller can be modified by removing the resistor connecting the mosfet gate to the microcontroller and soldering a wire to the mosfet gate and to ground. Mosfet gate can then be connected to the ESP32's GPIO pin and the grounds can be connected together.

## Installation Instructions
Click the green 'Code' button at the top of this page, then 'Download ZIP' 

In Arduino IDE, click 'Sketch' -> 'Include Library' -> 'Add .ZIP Library...'


## Keywords
Hello Fairy; Dazzle Bright; Surplife; 2BM5F-HB-1000COI; Linhai Huanbo Lighting Co., Ltd; 2AKBP-HELLOFAIRY; Shenzhen Hysiry Technology Co., Ltd; Hello Fairy BMSL6; CL-600CC; Data Over Power; Powerline Communication; PLC
