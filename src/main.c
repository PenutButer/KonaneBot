/* Copyright 2024 Isaac McCracken - All rights reserved
 ......................................................
 ......................................................

 ......................................................
 ......................................................

 ......................................................
 ......................................................
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define THINKING_TIME 10
#define BOARD_WIDTH 8


#include "types.h"
#define BITMOVES_IMPLEMENTATION
#include "bitmoves.h"
#include "boardio.h"
#include "allocators.h"
#include "agent.h"

#include <string.h>

/**
 * @brief This is a debug print for printing the bit board
 * 
 * @param board 
 */
void PrintBitBoard(BitBoard board) {
  for (U8 i = 0; i < 8; i++) {
    for (U8 j = 0; j < 8; j++) {
      char c = (board.rows[i] & (1<<j)) > 0;
      putchar('0' + c); // prints zero or 1
      putchar(' ');
    }
    putchar('\n');
  }
}

void PrintBoard(BitBoard board) {
  for (U8 i = 0; i < 8; i++) {
    for (U8 j = 0; j < 8; j++) {
      U8 color = (i + j) % 2 ;
      char c = (board.rows[i] & (1<<j)) > 0;
      if (c) c = (color)? 'W': 'B';
      else c = 'O';
      putchar(c);
    }
    putchar('\n');
  }
}

// void StateNodePushChild(StateNode *parent, StateNode *child) {
//   if (parent->lastChild) {
//     MyAssert(parent->firstChild);
//     // link parent to new child
//     StateNode *oldLast = parent->lastChild;
//     parent->lastChild = child;
//     // link children to each other
//     child->prev = oldLast;
//     oldLast->next = child;
//     // increase child count (child count could be useless)
//     parent->childCount++;
//   } else {
//     parent->firstChild = child;
//     parent->lastChild = child;
//     parent->childCount = 1;
//   }
// }

void GenerateAllMoves(Arena *arena, StateNode *parent, U64 turn) {
  const BitBoard board = parent->board;
  if (turn == 0) {
    MyAssert(board.whole == allPieces);
    StateNode *node = ArenaPush(arena, sizeof(StateNode));
    node->board = board;
    node->board.whole &= ~1llu;
    StateNodePushChild(parent, node);
    return;
  }

  // for each of our pieces
  for (U8 index = 0; index < 32; index++) {
    Coord coord = CoordFromIndex(index);
    U64 pieceMask = 1llu << index;
    U64 moveMask = 0llu;
    U64 killMask = 0llu;

    // shift one over if its whites turn or multiplu by 2... 8=====D~~~~~**
    if (turn % 2) pieceMask << 1;  

    // skip if we do not have a piece there
    if (!(pieceMask & board.whole)) continue;

    // for each of the four directions we can move for that piece
    for (U8 dir; dir < 4; dir++) {

    }

  }
  
}

int main(int argc, char** argv) {
  
  Bool gaming = Bool_True;
  char *boardFilePath = NULL;
  
  FILE *dump = fopen("dump.txt", "w");

  if (argc > 3) {
    printf("Dude, you got to use this thing properly\n");
    return -1;  
  } else {
    boardFilePath = argv[1];
    
  }

  Arena *arena = ArenaInit(Gigabyte(4)); // Don't worry this won't actually allocate 4 gigabytes

  BitBoard board = BitBoardFromFile(arena, boardFilePath);

  U8 agentPlayer = (*argv[2] == 'W') ?  PlayerKind_White : PlayerKind_Black;
  U64 playerBoard = (agentPlayer == PlayerKind_White) ? board.whole & allWhite : board.whole & allBlack;
  U64 allPlayerBoard = (agentPlayer == PlayerKind_White) ? allWhite : allBlack;
  U8 agentOpponent = (agentPlayer == PlayerKind_White) ? PlayerKind_Black : PlayerKind_White;

  srand(time(NULL));
  
  StateNodePool* stateNodePool = StateNodePoolInit(arena);
  bool blackIsAgent = (agentPlayer == PlayerKind_White) ? false : true;
  int turns = 1;
  
  int depth = 1;
  while (gaming) {
    if (blackIsAgent) {
      agentMove(agentPlayer, &board, stateNodePool, depth);
      // agent
      // input()
      printBoardToConsole(&board);
      mainInput(&board, agentOpponent);
      printBoardToConsole(&board);
    }

    else {
      // input()
      // then
      // agent

      // Somehow this fixes drivercheck
      
      agentMove(agentPlayer, &board, stateNodePool, depth);
      printBoardToConsole(&board);
      mainInput(&board, agentOpponent);
      printBoardToConsole(&board);
    }

    // show board
    // printBoardToConsole(&board);
    turns++;
  }

  BitBoardFilePrint(dump, board);

  // deinitalization
  ArenaDeinit(arena);

  fclose(dump);
  return 0;
}
