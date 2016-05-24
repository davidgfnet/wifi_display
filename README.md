
E-ink wifi display
==================

This project contains the software bits for a e-ink display I created.

You might find some info at my website: https://davidgf.net/page/41/e-ink-wifi-display

Contents
--------

esp8266_firmware: firmware sources for the ESP, built using the xtensa toolchain and the IoT SDK (used v1.5)

media: Some pictures used by the screen

server: PHP hell! Contains some cool JS editor I hacked myself that allows users to design screens

stm32_application: STM32F firmware that actually drives the screen (was builtin in the board!)

stm32f10x: ST includes and libs used to build the stm32 firmware

How to build this shit
----------------------

You will need to build the FW images with their corresponding toolchaings and flash them (shouldn't be that difficult).
To reuse the display driver for another board, just redefine the macros to point to your GPIOs and you are good to go!

The server should be copied to a PHP enabled server, create a config.php and fill it. Also create a screens/ dir and chmod it to be world readable/writtable.

Schematics
----------

I'll update this with some schematics. The ESP uses a GPIO to power gate the STM32 (off by default) which at its turn uses three other GPIOs to power gate the various screen voltage rails.

GDE043A2
--------

This thing uses the Good Diplay device codenamed GDE043A2, you may find stuff googling the web. The main issue though is that there is little support and the datasheet doesn't explain a thing, check gde043a2.c to see how the driver actually works. It seems GDE060BA works all much the same (they seem to be the same device with slightly different pinouts and screen sizes, but same resolution and probably same driver).

