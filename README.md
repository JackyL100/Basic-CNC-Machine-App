# Basic-CNC-Machine-App
WIP
The code for a basic CNC machine I made. 
It contains the C++ code used to create a crude 2D CAD application that will connect to an ESP32 via WiFi.
Once a design is complete, the computer will send drawing instructions to the ESP32 one by one.
The ESP32 is connected to an Arduino that will actually follow the instructions.
The Arduino uses a stepper motor driver board to control the stepper motors of the machine.

Notes about current version:
The only line that uses the mouse is the mouse
curved lines must use the terminal
deleting lines must use the terminal and the user must keep track of the number of line to be deleted
