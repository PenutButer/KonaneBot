#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>

typedef unsigned char       U8;
typedef unsigned short      U16;
typedef unsigned int        U32;
typedef unsigned long long  U64;

typedef signed char       I8;
typedef signed char       I8;
typedef signed short      I16;
typedef signed int        I32;
typedef signed long long  I64;

#define ROOT2 1.41421356237f // for monte carlo tree search

#define MyAssert(expr) if (!expr) *((U32*)0) = 0xDEAD
#define MOVE_LENGTH 6 // including '\0'


typedef U8 PlayerKind;
enum {
  PlayerKind_White,
  PlayerKind_Black,
};

typedef U8 Bool;
enum {
  Bool_False,
  Bool_True,
};

enum {
  Direction_up,
  Direction_left,
  Direction_down,
  Direction_right,
};


typedef union BitBoard BitBoard;
union BitBoard {
  U8 rows[8];
  U64 whole;
};



// 64 byte per node maybe we can cut down on size
typedef struct StateNode StateNode;
struct StateNode {
  BitBoard   board;
  StateNode *next;
  // StateNode *prev;
  StateNode *firstChild;
  StateNode *lastChild;
  // U64 childCount;
  I8 score;
  char move[MOVE_LENGTH];
};



typedef struct Coord Coord;
struct Coord {
  I8 x;
  I8 y;
};


static inline Coord CoordFromIndex(U8 index) {
  return (Coord){.x = (index % 8), .y = (index / 8)+1};
}

static inline U8 IndexFromCoord(Coord coord) {
  return (coord.x-1) + 8*(coord.y-1);
}


#endif