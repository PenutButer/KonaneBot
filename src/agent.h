/*
  USAGE: 
    The files agent.h and agent.c are for implementing the agent itself
  
  COPYRIGHT:
    Copyright 2024 Isaac McCracken - All rights reserved
*/
#ifndef AGENT_H
#define AGENT_H

#include "types.h"
#include "allocators.h"

// REMOVE THIS AFTER DEMO
U64 getUpMove(BitBoard board, char player);

U64 getPlayerEmptySpace(BitBoard board, char player);
void getMovablePieces(U8* pList, U64 jump, BitBoard board, char player); 
StateNode* StateNodeGenerateChildren(StateNodePool *pool, StateNode *parent, char playerKind);
U64 StateNodeCountChildren(StateNode *node);
void StateNodePushChild(StateNode *parent, StateNode *child);
void StateNodeCalcCost(StateNode* node);
void agentMove(U8 agentPlayer, BitBoard* board, StateNodePool *pool, int depth);

// For the minimax functions
// Max and Min functions
static inline int max(int x, int y) {
  return x > y ? x : y;
}

static inline int min(int x, int y) {
  return x < y ? x : y;
}
// Minimax Algorithm functions
I32 minimax(StateNodePool *pool, StateNode* node, I32 depth, I32 alpha, I32 beta, I32 maximizingPlayer);


#endif
