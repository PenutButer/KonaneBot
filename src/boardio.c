#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitmoves.h"
#include "types.h"
#include "boardio.h"

// Read the whole file info a buffer located on the arena
// This function was written by Kaiden Kaine in december of 2023
// I have gotten permission to use it.
U8 *LoadFileDataArena(Arena *arena, const char *filepath, U32 *bytes_read) {
  
  FILE *fp = fopen(filepath, "rb");
  
  if (fp) {
    fseek(fp, 0, SEEK_END);
    *bytes_read = ftell(fp);
    rewind(fp);

    U8 *buffer = ArenaPushNoZero(arena, *bytes_read);
    fread(buffer, *bytes_read, 1, fp);
    
    fclose(fp);
    return buffer;

  } else fprintf(stderr, "couldn't open file \"%s\"\n", filepath);
  
  return NULL;
}


// get bitboard of input file. make sure index
// is only either 0 (black pieces) or 1 (white pieces). 
BitBoard BitBoardFromFile(Arena *tempArena, const char* fileName) {
  BitBoard board = { 0 };
  
  // since this data is only temporaly needed
  // We can use the temp arena, after we are done
  // we can use TempArenaDeinit() to reset the 
  // arena to its original state.
  TempArena temp = TempArenaInit(tempArena);
  
  U32 bytesRead = 0;
  U8 *buffer = LoadFileDataArena(tempArena, fileName, &bytesRead);

  U32 row = 0;
  U32 col = 0;
  for (U32 index = 0; index < bytesRead; index++) {
    if (buffer[index] == '\n') {
      col = 0;
      row += 1;
      continue;
    } 
    if (buffer[index] != 'O') board.rows[row] |= 1llu<<col;
    col += 1;
  }
  
  board.whole &= 0;
  U8 start = 63;
  for (U32 index = 0; index < bytesRead; index++) {
    if (buffer[index] == '\n') continue;
    if (buffer[index] == 'O') {
      start--;
      continue;
    }
    board.whole |= (1llu << start--);
  }
  
  // After we are done allocations 
  // we can "free" all the memory 
  // that we used during our calculations
  TempArenaDeinit(temp);
  
  return board;
}


void BitBoardFilePrint(FILE *fp, BitBoard board) {
  U8 counter = 63;
  char c;
  
  for (U8 i = 0; i < 8; i++) {
    for (U8 j = 0; j < 8; j++) {
      if ((i+j)%2 && ((1llu << counter) & board.whole)) c = 'W';
      else if ((1llu << counter) & board.whole) c = 'B';
      else {
        c = 'O';
      }
      counter--;
      putc(c, fp);
    }
    putc('\n', fp);
  }

  // for (U8 i = 0; i < 8; i++) {
  //   for (U8 j = 0; j < 8; j++) {
  //     U8 color = (i + j) % 2 ;
  //     char c = (board.rows[i] & (1<<j)) > 0;
  //     if (c) c = (color)? 'W': 'B';
  //     else c = 'O';
  //     putc(c, fp);
  //   }
  //   putc('\n', fp);
  // }
}

void bitToTextCoord(U64 bit, char* textCoord) {
  U8 row = 0, column = 0;
  
  // row:
  // if 0: '1'
  // if 7: '8'
  while (bit >> 8) {
    row++;
    bit >>= 8;
  }
  
  // column
  // if 0: 'H'
  // if 7: 'A'
  while (bit >> 1) {
    column++;
    bit >>= 1;
  }

  // ColumnRow ('H'-column), ('1' + row)
  textCoord[0] = 'H' - column;
  textCoord[1] = '1' + row;
  textCoord[2] = '\0';
}

void CoordOutputMove(Coord coord) {
  putchar('A' + coord.x);
  putchar('\n');
}


Coord CoordFromEnemyInput(void) {
  I8 x = toupper(getchar());
  I8 y = getchar();
  getchar();
  Coord coord;
  coord.x = (('X' - x)%8) + 1;
  coord.y = (y - '1') + 1;
  return coord;
}


Coord CoordFromInput(char* coord){
  I8 x = coord[0];
  I8 y = coord[1];
  Coord returnCoord;
  returnCoord.x = (('X' - x)%8) + 1;
  returnCoord.y = (y - '1') + 1;
  return returnCoord;
}


// [0] first coord, [1] second coord
Coord* multipleCoordsInput() {
  Coord* coords = malloc(2*sizeof(Coord));
  coords[0] = (Coord){0, 0};
  coords[1] = (Coord){0, 0};

  char c1x = toupper(getchar());
  char c1y = getchar();

  // Remove '-'
  getchar();

  char c2x = toupper(getchar());
  char c2y = getchar();

  // Remove '\n'
  getchar();

  char  coord1[] = {c1x, c1y, '\0'},
        coord2[] = {c2x, c2y, '\0'};

  coords[0] = CoordFromInput(coord1);
  coords[1] = CoordFromInput(coord2);

  return coords;
}


void mainInput(BitBoard* board, char player) {
  U64 allPlayerBoard = (player == PlayerKind_White) ? allWhite : allBlack;
  
  printf("Your move: ");

  // First move
  if (!((board->whole & allPlayerBoard) ^ allPlayerBoard)) {
    Coord enemyStone = CoordFromEnemyInput();
    board->whole ^= (1llu<<IndexFromCoord(enemyStone));
    return;
  }

  Coord* coordMove = multipleCoordsInput();
  Coord coord1 = coordMove[0],
        coord2 = coordMove[1];
  U64 startBit = 1llu << IndexFromCoord(coord1);
  
  if (coord1.y == coord2.y) {
    // Horizontal
    int shiftAmount = abs(coord1.x - coord2.x);
    if (coord1.x - coord2.x < 0) {
      // Left shift
      for (int i = 1; i < shiftAmount; i++) {
        startBit |= (startBit<<1);
      }
    }

    else {
      // Right shift
      for (int i = 1; i < shiftAmount; i++) {
        startBit |= (startBit>>1);
      }
    }
  }
  
  else {
    // Vertical
    int shiftAmount = abs(coord1.y - coord2.y);
    if (coord1.y - coord2.y < 0) {
      // Left shift
      for (int i = 1; i < shiftAmount; i++) {
        startBit |= (startBit<<8);
      }
    }

    else {
      // Right shift
      for (int i = 1; i < shiftAmount; i++) {
        startBit |= (startBit>>8);
      }
    }
  }

  // & entire table and | coord1, coord2
  // example move:
  // WBOBOB
  // bit generation gives -> 111110, but
  // & entire we then get -> 110100
  // coord 1, current white piece
  // coord 2, where white piece will be.
  // after | index of both -> 110110. can use ^ with board now.
  startBit &= board->whole;
  startBit |= 1llu<<IndexFromCoord(coord1);
  startBit |= 1llu<<IndexFromCoord(coord2);
  
  board->whole ^= startBit;
  free(coordMove);
}

void printBoardToConsole(BitBoard* board) {
  int boardShift = 63;
  char colour;

  char background[] = "\033[48;5;94m";

  printf("\n");

  // Row (123...)
  for (int i = 0; i < 10; i++) {
    printf("%s", background);
    // Column (ABC...)
    for (int j = 0; j < 10; j++) {
      
      // Top and bottom letter column printing
      if (i == 0 || i == 9) {
        if (j >= 1 && j <= 8) {
          // 64 is char '@'
          printf("\033[38;5;255m%c ", 64+j);
        }
        else {
          if (j == 0) printf("\033[38;5;255m   ");
          else if (j == 9) printf("\033[38;5;255m  ");
        }
        continue;
      }

      // Column printing
      if (j == 0 || j == 9) {
        // Print row numbers at these spots
        // 57 is char '9'
        if (j == 9) printf("\033[38;5;255m%c ", 57-i);
        else printf("\033[38;5;255m %c ", 57-i);
      }

      else {
        // Print board status
        if (!((board->whole & (1llu << boardShift)) & allPieces)) colour = ' ';
        else if ((board->whole & (1llu << boardShift)) & allBlack) colour = 'B';
        else if ((board->whole & (1llu << boardShift)) & allWhite) colour = 'W';

        if (j == 8 && colour == 'B') printf("\033[38;5;232m%s⬤\033[38;5;255m ", background);
        else if (j == 8 && colour == 'W') printf("\033[38;5;255m%s⬤ ", background);
        else if (colour == 'B') printf("\033[38;5;232m%s⬤ \033[38;5;255m", background);
        else if (colour == 'W') printf("\033[38;5;255m%s⬤ ", background);
        else printf("%s  ", background);
        boardShift--;
      }
    }
    printf("\033[0m\n");
  }

  printf("\n\033[0m");

  return;
}
