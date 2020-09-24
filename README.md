# SomfyRTS-Gateway-RS
This is a next version of https://github.com/p-m-x/espSomfyRts project. Now it is designed to controll Somfy RTS roller shutters by RS232 interface.

This project uses Arduino Nano board but can be simply changed to other uC

## RS232 ASCII data frame

DEVICE-ID|COMMAND

Where:
DEVICE-ID is a number from 0 to DEVICES_COUNT - 1
COMMAND is on of up,stop,down,prog

Main loop waits for LF character (\n) to complete serial input command.
If there is no next char on serial interface more than 1s [SERIAL_WAIT_FOR_CMD_TIME] it completes without waiting for LF char.