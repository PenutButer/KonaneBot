#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "allocators.h"



Arena *ArenaInit(U64 capacity) {
  capacity = (capacity < Kilobyte(4))? Kilobyte(4):capacity;
  void *ptr = malloc(capacity);
  Arena *arena = ptr;
  arena->buff = (char*)&arena[1];
  arena->align = 8;
  arena->next = NULL;
  arena->pos = 0;
  arena->cap = capacity;
  
  return arena;
}

void ArenaDeinit(Arena *arena) {
  Arena *node = arena;
  while (node) {
    Arena *next = node->next;

    free(node);

    node = next;
  }
  
}


void *ArenaPushNoZero(Arena *arena, U64 size) {
  while (arena->pos + size > arena->cap) {
    if (!arena->next) {
      arena->next = ArenaInit(ARENA_DEFAULT_SIZE);
    }

    arena = arena->next;
  }

  arena->pos = ((arena->pos + arena->align - 1) / arena->align) * arena->align;

  void *result = arena->buff + arena->pos;

  arena->pos += size;

  return result;
}


void *ArenaPush(Arena *arena, U64 size) {
  void *result = ArenaPushNoZero(arena, size);
  memset(result, 0, size);
  return result;
}


void ArenaReset(Arena *arena) {
  Arena *node = arena;
  while (node) {
    Arena *next = node->next;

    node->pos = 0;

    node = next;
  }
  
}


TempArena TempArenaInit(Arena *backing_arena) {
  return (TempArena){
    .arena = backing_arena,
    .pos = backing_arena->pos,
    .align = backing_arena->align,
  };
}


void TempArenaDeinit(TempArena temp_arena) {
  temp_arena.arena->pos = temp_arena.pos;
  temp_arena.arena->align = temp_arena.align;
}

StateNodePool *StateNodePoolInit(Arena *backingArena) {
  StateNodePool *pool = ArenaPushNoZero(backingArena, sizeof(StateNodePool));
  pool->arena = backingArena;
  pool->freeList = NULL;
  return pool;
}

StateNode *StateNodePoolAlloc(StateNodePool *pool) {
  if (pool->freeList) {
    StateNode *node = pool->freeList;
    pool->freeList = pool->freeList->next;
    memset(node, 0, sizeof(StateNode)); // memzero
    return node;

  } else {
    return ArenaPush(pool->arena, sizeof(StateNode));
  }
}

void StateNodePoolFree(StateNodePool *pool, StateNode *node) {
  // StateNode *prev = node->prev;
  // StateNode *next = node->next;

  // if (prev) prev->next = next;
  // if (next) next->prev = prev;

  node->next = pool->freeList;
  pool->freeList = node;
}
