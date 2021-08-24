
#ifndef DWARF_H
#define DWARF_H

#include "common.h"

#include <vector>

class Dwarf {
  std::vector< std::vector <Place> > known; 
  Loc location;
  int name_value;
  int lumber_value;
  bool started_action;
  void (*p_set_action)(Action a, Dir dir, int r, int c);
public: 
  Dwarf(int r, int c, int nm, int lmbr, double radius, std::vector< std::vector <Place> > & area, void (*set_action)(Action a, Dir dir, int r, int c) );
  
  int row();
  int col();
  int name();
  int lumber();
  Place look(int r, int c);
  Place look(Loc loc);

  void start_walk(int r, int c);
  void start_walk(Loc loc);
  void start_chop(Dir dir);
  void start_pick(Dir dir);
  void start_build(Dir dir);
};

#endif
