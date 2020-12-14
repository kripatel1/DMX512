# DMX-512-A with EF3 protocol (single differential pair)	

The code protocol is written in DMX512.c Code is intended to work on an embedded systems.
Code should be loaded on TM4C123GHPM board and a differential single pair circuit.
All code except the user interface for input commands are interrupt driven function. A combination of UART interrupts and timer interrupts are used for the DMX512 protocol.
The display command is driven using an interrupt
