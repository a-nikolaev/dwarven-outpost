
#include "dwarf.h"

Dwarf::Dwarf(int r, int c, int nm, int lmbr, double radius, 
    std::vector< std::vector <Place> > & area, void (*set_action)(Action a, Dir dir, int r, int c) ) 
{
  int rows = area.size();
  int cols = area[0].size();
  
  known.resize(rows);
  for(int rr = 0; rr < rows; ++rr) {
    known[rr].resize(cols);
    for(int cc = 0; cc < cols; ++cc) {
      known[rr][cc] = UNKNOWN;
      if (((double)r-rr)*((double)r-rr) + ((double)c-cc)*((double)c-cc) <= radius * radius)
        known[rr][cc] = area[rr][cc];
    }
  }

  p_set_action = set_action;

  location.r = r;
  location.c = c;
  name_value = nm;

  lumber_value = lmbr;
}
  
int Dwarf::row() {
  return location.r;
}

int Dwarf::col() {
  return location.c;
}

int Dwarf::name() {
  return name_value;
}

int Dwarf::lumber() {
  return lumber_value;
}

Place Dwarf::look(int r, int c) {
  int rows = known.size();
  int cols = known[0].size();
  
  if (r >= 0 && c >= 0 && r < rows && c < cols)
    return known[r][c];
  else
    return UNKNOWN;
}

Place Dwarf::look(Loc loc) {
  return look(loc.r, loc.c);
}

void Dwarf::start_walk(int r, int c) {
  (*p_set_action)(WALK, WEST, r, c);
}

void Dwarf::start_walk(Loc loc) {
  (*p_set_action)(WALK, WEST, loc.r, loc.c);
}

void Dwarf::start_chop(Dir dir) {
  (*p_set_action)(CHOP, dir, 0, 0);
}

void Dwarf::start_pick(Dir dir) {
  (*p_set_action)(PICK, dir, 0, 0);
}

void Dwarf::start_build(Dir dir) {
  (*p_set_action)(BUILD, dir, 0, 0);
}

