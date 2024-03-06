# KonaneBot

## Description
  Our Konane Bot uses Min-Max Trees to provide move generation. We used a Single 64-bit integer to represent our board state
  and used a handmade arena based c style.

## Code Base
- `main.c` This contains our programs entry point and handles logic for the command line arguments
- `alllocators.c/h` This contains the implementation of the arena and pool allocator using malloc as a backing allocator for the arena
- `bitmoves.h` This is a deprecated file that was automatically generated to provide bitmasks for move generation
- `boardio.c/h` This file handles input from standard in and out 
- `meta.c` This is a deprecated meta program that generated bitmoves.h
- `types.h` This file contains all of our primitive types such as StateNode and typedefs of C's Integer types for ease of use
- `agent.c/h` This files contains the logic of our agent and implements the move generation and agent search