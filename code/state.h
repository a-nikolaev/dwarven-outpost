
#ifndef STATE_H
#define STATE_H

#include <cstdlib>
#include <iostream>
#include <vector>

#include "bot.h"
#include "outstream.h"

enum Game_status {RUNNING, SUCCESS, FAILURE};

struct Object {Place pl; int wait; Action action; Dir dir; Loc dst; int durability; std::vector<Loc> path; };

struct State {
  // dimensions
  char part;
  int rows;
  int cols;
  int num;
  // map
  std::vector <std::vector<Object> > map;
  std::vector <std::vector<double> > smell;

  // resurces
  int lumber;
  int apples;
  int pumpkins;

  // status
  int time;
  Game_status status;

  // UI
  bool play;

  std::vector<int> prng_seq;
  int prng_index;
};

void init(State &st, char part, int rows, int cols, int number);

void update(State &st, std::ostream &gamelog);

bool is_night(State &st);

int largest_structure(State &st);
int count_place(State &st, Place pl);

#endif
