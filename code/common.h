
#ifndef COMMON_H
#define COMMON_H

enum Dir {WEST, EAST, NORTH, SOUTH};

enum Action {IDLE, WALK, CHOP, PICK, BUILD, ATTACK};

enum Place {UNKNOWN, EMPTY, DWARF, FENCE, PINE_TREE, APPLE_TREE, ZOMBIE, PUMPKIN_ZOMBIE, PUMPKIN, PUMPKIN_ZOMBIE_BABY};

struct Loc {int r; int c;}; // row and column

const int log_line_length = 70; 
#endif 
