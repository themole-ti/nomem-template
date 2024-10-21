# Pong for the TI-99/4A

This is a simple implementation of a Pong clone for the TI-99/4A, meant to serve as an example program that doesn't require the 32k memory expansion, but instead runs on the unexpanded system with its glorious total of 256 bytes of RAM.
If you want to develop your own cartridge games in C that don't require the memory expansion, you can use this as a basis. Just remove the main.c and graphics.h files and replace them with your own sources and you should be good to go!

## Building
Provided you have the gcc compiler for the tms9900 installed (https://github.com/mburkley/tms9900-gcc), simply edit the top of the Makefile so that the BASE_PATH variable points to the directory where your compiler lives. After that, provided you are running in a POSIX-ish environment, issuing a simple `make` command should be enough to build the game.
This will create two files: `pong.rpk`, which can be used with Mame or JS99er, and `pongc.bin` which can be used with Classic99.

## Playing the game
What can I say, it's Pong... It supports 1 and 2 player games, which can be selected on the title screen. First player to score 10 points wins.

## Making your own games
Simply remove main.c and graphics.h in the src/ and include/ directories respectively and replace them with your own game code. 

I've included two headers that are heavily based on the excellent libti99 by Tursi (https://github.com/tursilion) but have been tweaked to follow the conventions of STB-style single header libraries. That is to say, to include the implementation in your binary, you have to make sure that you define the VDP_IMPLEMENTATION and INPUT_IMPLEMENTATION macros in one (and exactly one) of your .c files before including the headers. Check out main.c for an example. 
The headers provide basic functions for showing stuff on the screen and reading the keyboard/joysticks. 
I might add a similar header for sound later on.

## Technical notes
Your data and variables (.data and .bss segments) are located in memory starting at 0x8320, the stack starts at 0x8400 and grows down. That means that your biggest challenge will likely be ensuring your stack doesn't grow into your data, since both combined can never take up more than 224 *bytes*.
To allow you to track memory usage a bit easier, the entire region is initialized with 0xaa by crt0.c which should make it easy to see in any emulator's debugger when you're running out of memory.
Use (and re-use) global variables and define things as constant (which puts them in cartridge memory) wherever possible.

Happy coding!
