#include "agent.h"
#include "types.h"
#include <stdbool.h>
#include <stdlib.h>
#include "allocators.h"
#include <stdio.h>
#include <string.h>
#include "boardio.h"
#include <time.h>

#define ALL_BLACK     0xAA55AA55AA55AA55
#define ALL_WHITE     0x55AA55AA55AA55AA
#define DEPTH 5
#define EDGE_PIECES   0x7E8181818181817E
#define CORNER_PIECES 0x8100000000000081

#define INT_MAX 127
#define INT_MIN -128

bool shiftValid(U64 jump, U8 shift, bool max);


void freeAllChildrenNodes(StateNodePool* pool, StateNode* node) {
  if (!node) return;

  StateNode* freeNode;
  while (node) {
    freeNode = node;
    freeAllChildrenNodes(pool, freeNode->firstChild);
    node = node->next;
    StateNodePoolFree(pool, freeNode);
  }
}


bool isOver(StateNode* node) {
  U64 whitePieces = 0, blackPieces = 0;
  U64 whiteSpots = getPlayerEmptySpace(node->board, PlayerKind_White);
  U8 counter = 0;
  U64 checker = whiteSpots, jumpSpace;
  while (checker) {
    jumpSpace = checker & 1;
    if (jumpSpace) {
      U64* piecesList = getMovablePieces(jumpSpace << counter, node->board, PlayerKind_White);
      for (int j = 0; j < 4; j++) {
        // If a spot is empty, get if this is reachable by a piece. +1 to score for each
        // piece reachable
        if (piecesList[j]) {
          whitePieces |= (piecesList[j] & ALL_WHITE);
        }
      }
    }
    checker >>= 1;
    counter++;
  }

  // Same thing as white pieces but with black pieces.
  U64 blackSpots = getPlayerEmptySpace(node->board, PlayerKind_Black);
  counter = 0;
  checker = blackSpots;
  while (checker) {
    jumpSpace = checker & 1;
    if (jumpSpace) {
      U64* piecesList = getMovablePieces(jumpSpace << counter, node->board, PlayerKind_Black);
      for (int j = 0; j < 4; j++) {
        // If a spot is empty, get if this is reachable by a piece. +1 to score for each
        // piece reachable
        if (piecesList[j]) {
          blackPieces |= (piecesList[j] & ALL_BLACK);
        }
      }
    }
    checker >>= 1;
    counter++;
  }

  // White pieces can't capture -> set this node as extremely black favoured
  if (!whitePieces) node->score = INT_MIN;

  // White pieces can't capture -> set this node as extremely white favoured
  if (!blackPieces) node->score = INT_MAX;

  // Both pieces can't move, give this state a 0
  if (!whitePieces && !blackPieces) node->score = 0;

  return !(whitePieces && blackPieces);
}


void agentMove(U8 agentPlayer, BitBoard* board, StateNodePool *pool, int depth) {
  // printf("Agent move: ");

  U64 allPlayerBoard = (agentPlayer == PlayerKind_White) ? ALL_WHITE : ALL_BLACK;
  
  char playerStartingMoves[2][3];
  if (agentPlayer == PlayerKind_White) {
    strcpy(playerStartingMoves[0], "D4");
    strcpy(playerStartingMoves[1], "E5");
  } else {
    strcpy(playerStartingMoves[0], "D5");
    strcpy(playerStartingMoves[1], "E4");
  }
  char randomStart[3];
  strcpy(randomStart, playerStartingMoves[rand() % 2]);

  // First move
  if (!((board->whole & allPlayerBoard) ^ allPlayerBoard)) {
    printf("%s\n", randomStart);
    board->whole ^= 1llu<<IndexFromCoord(CoordFromInput(randomStart));
    return;
  }

  // Determine best move: Generate children of possible moves
  StateNode* stateNode = StateNodePoolAlloc(pool);
  stateNode->board = *board;
  StateNodeGenerateChildren(pool, stateNode, agentPlayer);

  // Go through all children and set their score as minimax()
  for (StateNode* child = stateNode->firstChild; child; child=child->next) {
    child->score = minimax(pool, child, depth, INT_MIN, INT_MAX, (agentPlayer == PlayerKind_White) ?
      1 : 0);
  }

  // Go through all children and print their score
  I32 bestScore = (agentPlayer == PlayerKind_White) ? INT_MAX : INT_MIN;
  StateNode* newState = stateNode->firstChild;
  for (StateNode* child = stateNode->firstChild; child; child=child->next) {
    if (agentPlayer == PlayerKind_White && child->score > newState->score) newState = child;
    else if (agentPlayer == PlayerKind_Black && child->score < newState->score) newState = child;
    // printf("State score: %d\n", child->score);
  }
  
  printf("%s\n", newState->move);
  if (newState->move[0] == '\0') {
    printf("Lost");
  }
  *board = newState->board;

  // Free all children of our state node
  freeAllChildrenNodes(pool, stateNode);
}


// score < 0: black favoured (black has more pieces to move)
// score > 0: white favoured (white has more pieces to move)
// score = 0: equal pieces move
void StateNodeCalcCost(StateNode* node) {
  // These hold the pieces that are able to move
  U64 whitePieces = 0, blackPieces = 0;
  U64 whiteEdgePieces = 0, blackEdgePieces = 0;
  U64 whiteCornerPieces = 0, blackCornerPieces = 0;
  
  // Get white pieces empty spots
  U64 whiteSpots = getPlayerEmptySpace(node->board, PlayerKind_White);

  // This algo is really weird.
  // In short, the algo will go through each square.
  // If the square is empty, call getMovablePieces() to get all the pieces that can
  // move into this square.
  // & each direction with ALL_WHITE to get the piece that can move to the empty square
  // | each piece with whitePieces
  // Now we have all the whitePieces that can move
  U8 counter = 0;
  U64 checker = whiteSpots, jumpSpace;
  while (checker) {
    jumpSpace = checker & 1;
    if (jumpSpace) {
      U64* piecesList = getMovablePieces(jumpSpace << counter, node->board, PlayerKind_White);
      for (int j = 0; j < 4; j++) {
        // If a spot is empty, get if this is reachable by a piece. +1 to score for each
        // piece reachable
        if (piecesList[j]) {
          whitePieces |= (piecesList[j] & ALL_WHITE);
        }
      }
      free(piecesList);
    }
    checker >>= 1;
    counter++;
  }

  // Same thing as white pieces but with black pieces.
  U64 blackSpots = getPlayerEmptySpace(node->board, PlayerKind_Black);
  counter = 0;
  checker = blackSpots;
  while (checker) {
    jumpSpace = checker & 1;
    if (jumpSpace) {
      U64* piecesList = getMovablePieces(jumpSpace << counter, node->board, PlayerKind_Black);
      for (int j = 0; j < 4; j++) {
        // If a spot is empty, get if this is reachable by a piece. +1 to score for each
        // piece reachable
        if (piecesList[j]) {
          blackPieces |= (piecesList[j] & ALL_BLACK);
          blackEdgePieces |= (piecesList[j] & ALL_BLACK & EDGE_PIECES);
        }
      }
      free(piecesList);
    }
    checker >>= 1;
    counter++;
  }

  whiteEdgePieces |= (whitePieces & ALL_WHITE & EDGE_PIECES);
  blackEdgePieces |= (blackPieces & ALL_BLACK & EDGE_PIECES);
  whiteCornerPieces |= (whitePieces & ALL_WHITE & CORNER_PIECES);
  blackCornerPieces |= (blackPieces & ALL_BLACK & CORNER_PIECES);

  // printBoardToConsole(&node->board);
  // printf("# of white pieces movable: %d\n", __builtin_popcountll(whitePieces));
  // printf("# of black pieces movable: %d\n", __builtin_popcountll(blackPieces));
  // printf("This state's score: %d\n", __builtin_popcountll(whitePieces) - __builtin_popcountll(blackPieces));

  node->score = ((__builtin_popcountll(whitePieces)-__builtin_popcountll(blackPieces)) + 
                 (3*(__builtin_popcountll(whiteCornerPieces))) - (3*(__builtin_popcountll(blackCornerPieces)))
                 );
}


StateNode* StateNodeGenerateChildren(StateNodePool *pool, StateNode *parent, char playerKind) {
  // 0b01 if black pieces, 0b10 if white pieces

  U64 currentSpace = (playerKind == PlayerKind_White) ? 0x2 : 0x1;
  U64 playerEmpty = getPlayerEmptySpace(parent->board, playerKind);
  U64 allPlayer = (playerKind == PlayerKind_White) ? ALL_WHITE : ALL_BLACK;

  U8 counter = 0;
  U64 checker = playerEmpty, jumpSpace;
  while (checker) {
    jumpSpace = checker & 1;
    if (jumpSpace) {
      U64* piecesList = getMovablePieces(jumpSpace << counter, parent->board, playerKind);
      for (int j = 0; j < 4; j++) {
        // Only create a new state if a move exists in the given direction
        if (piecesList[j]) {
          BitBoard board;
          board.whole = (parent->board.whole)^(piecesList[j]|(jumpSpace << counter));
          StateNode* child = StateNodePoolAlloc(pool);
          child->board = board;

          // Change this to minimax
          // StateNodeCalcCost(child);

          char coord[3];
          bitToTextCoord(piecesList[j] & allPlayer, coord);
          strcat(child->move, coord);
          strcat(child->move, "-");

          bitToTextCoord(jumpSpace << counter, coord);
          strcat(child->move, coord);

          // printf("Child with move %s created\n", child->move);

          StateNodePushChild(parent, child);
        }
      }
      free(piecesList);
    }
    checker >>= 1;
    counter++;
  }
  // printf("Children count: %llu\n", StateNodeCountChildren(parent));

}


// For the minimax functions
I32 minimax(StateNodePool *pool, StateNode* node, I32 depth, I32 alpha, I32 beta, I32 maximizingPlayer) {
  
  // printf("Depth remaining: %d\n", depth);

  if (isOver(node)) {
    return node->score;
  }
  
  if (depth == 0) {
    //Run Evaluation Function
    StateNodeCalcCost(node);
    return node->score;
  }

  if (maximizingPlayer) {
    I32 maxEval = INT_MIN;

    // Generate children as white
    StateNodeGenerateChildren(pool, node, PlayerKind_White);
    
    for (StateNode* child = node->firstChild; child != NULL; child = child->next) {
      I32 eval = minimax(pool, child, depth -1, alpha, beta, false);
      maxEval = max(maxEval, eval);
      alpha = max(alpha, eval);
      if (beta <= alpha) {
        break;
      }
    }
    return maxEval;
  }

  else {
    I32 minEval = INT_MAX;

    // Generate children as black
    StateNodeGenerateChildren(pool, node, PlayerKind_Black);

    for (StateNode* child = node->firstChild; child != NULL; child = child->next) {
      I32 eval = minimax(pool, child, depth -1, alpha, beta, true);
      minEval = min(minEval, eval);
      beta = min(beta, eval);
      if (beta <= alpha) {
        break;
      }
    }
    return minEval;
  }

}



/*
 * This function simply gets all the empty spots the given player can land on.
 */
U64 getPlayerEmptySpace(BitBoard board, char player) {
  return (player == PlayerKind_White) ? (~board.whole) & ALL_WHITE : (~board.whole) & ALL_BLACK;
}

/*
 * Gets all the movable pieces given the empty spots the player can land on. Should
 * be used with getPlayerEmptySpace() and passing one jump at a time to make move
 * calculation easier. Returns an array of movements in each direction.
 * 0 - up
 * 1 - left
 * 2 - down
 * 3 - right
 */
U64* getMovablePieces(U64 jump, BitBoard board, char player) {
  // Not using arena since this is not a state
  U64* piecesModified = malloc(4*sizeof(U64));

  // Initialize values
  piecesModified[0] &= 0;
  piecesModified[1] &= 0;
  piecesModified[2] &= 0;
  piecesModified[3] &= 0;

  U8 dirs = 0xF;
  U64 playerBoard = (player == PlayerKind_White) ? board.whole & ALL_WHITE : board.whole & ALL_BLACK;
  U64 oppBoard = (player == PlayerKind_White) ? board.whole & ALL_BLACK : board.whole & ALL_WHITE;  
  U64 allOppBoard = (player == PlayerKind_White) ? ALL_BLACK : ALL_WHITE;
  U64 allPlayerBoard = (player == PlayerKind_White) ? ALL_WHITE : ALL_BLACK;

  for (int checkRange=0; checkRange < 7; checkRange++) {
    U8 vShift = 8*(checkRange+1), hShift = (checkRange+1);
    U64 checkJump;    

    // Check up direction
    if ((dirs & 0x8) && shiftValid(jump, vShift, true)) {
      checkJump = jump << vShift;
      if (checkJump & playerBoard) {
        dirs &= 0x7;
        piecesModified[0] ^= checkJump;
      }
      else if ((checkRange+1)%2 && checkJump & ~oppBoard) dirs &= 0x7;
      else if (checkJump & oppBoard) piecesModified[0] ^= checkJump;
      // // This spot can be double jumped. Disable other direction until
      // // max spot has been found
      // else {
      //   piecesModified[2] = 0;
      //   dirs &= 0xD;
      // }
    } else dirs &= 0x7;

    // Check left direction
    if ((dirs & 0x4) && shiftValid(jump, hShift, true)) {
      checkJump = jump << hShift;
      // Checks if row is same
      if (!(hShift%2) && (checkJump & allOppBoard) || 
          (hShift%2) && (checkJump & allPlayerBoard)) dirs &= 0xB;
      else if (checkJump & playerBoard) {
        dirs &= 0xB;
        piecesModified[1] ^= checkJump;
      }
      else if (hShift%2 && checkJump & ~oppBoard) dirs &= 0xB;
      else if (checkJump & oppBoard) piecesModified[1] ^= checkJump;
      // else {
      //   piecesModified[3] = 0;
      //   dirs &= 0xE;
      // }
    } else dirs &= 0xB;
    
    // Check down direction
    if ((dirs & 0x2) && shiftValid(jump, vShift, false)) {
      checkJump = jump >> vShift;
      if (checkJump & playerBoard) {
        dirs &= 0xD;
        piecesModified[2] ^= checkJump;
      }
      else if ((checkRange+1)%2 && checkJump & ~oppBoard) dirs &= 0xD;
      else if (checkJump & oppBoard) piecesModified[2] ^= checkJump;
      // else {
      //   piecesModified[0] = 0;
      //   dirs &= 0x7;
      // }
    } else dirs &= 0xD;

    // Check right direction
    if ((dirs & 0x1) && shiftValid(jump, hShift, false)) {
      checkJump = jump >> hShift;
      if (!(hShift%2) && (checkJump & allOppBoard) || 
          (hShift%2) && (checkJump & allPlayerBoard)) dirs &= 0xE;
      else if (checkJump & playerBoard) {
        dirs &= 0xE;
        piecesModified[3] ^= checkJump;
      }
      else if (hShift%2 && (checkJump & ~oppBoard)) dirs &= 0xE;
      else if (checkJump & oppBoard) piecesModified[3] ^= checkJump;
      // else {
      //   piecesModified[1] = 0;
      //   dirs &= 0xB;
      // }
    } else dirs &= 0xE;
  }

  if (!(piecesModified[0] & playerBoard)) piecesModified[0] = 0;
  if (!(piecesModified[1] & playerBoard)) piecesModified[1] = 0;
  if (!(piecesModified[2] & playerBoard)) piecesModified[2] = 0;
  if (!(piecesModified[3] & playerBoard)) piecesModified[3] = 0;

  return piecesModified;
} 

/*
 * Helper function, returns true if given shift is within 64 bits
 */
bool shiftValid(U64 jump, U8 shift, bool max) {
  if (max) return (jump < (0xFFFFFFFFFFFFFFFF >> shift))? true : false;
  return (jump >= (1 << shift))? true : false;
}

void StateNodePushChild(StateNode *parent, StateNode *child) {
  // StateNode* oldChild = parent->firstChild;
  // child->next = oldChild;
  // parent->firstChild = child;

  if (parent->lastChild) {
    MyAssert(parent->firstChild);
    // link parent to new child
    StateNode *oldLast = parent->lastChild;
    parent->lastChild = child;
    // link children to each other
    // child->prev = oldLast;
    oldLast->next = child;
    // increase child count (child count could be useless)
    // parent->childCount++;
  } else {
    parent->firstChild = child;
    parent->lastChild = child;
    // parent->childCount = 1;
  }
}


U64 StateNodeCountChildren(StateNode *node) {
  U64 count = 0;
  StateNode *curr = node->firstChild;
  while (curr) {
    curr = curr->next;
    count++;
  }

  return count;
}