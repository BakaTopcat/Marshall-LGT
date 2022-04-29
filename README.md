# Marshall-LGT
Tiny Marshall camera RCP on LGT8F328P board.

The simplified version of the Marshall-Nano RCP. Does not use EEPROM.
Basically it is the remote joystick for camera, only for menu access.

Some MAX drivers on these boards have very weak 3v3 output, unsufficient for powering the OLED, so I'd suggest using separate 5v to 3.3v converter.
