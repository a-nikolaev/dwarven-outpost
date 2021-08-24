
#ifndef BOT_H
#define BOT_H

#include <iostream>
#include "dwarf.h"

void onStart(int rows, int cols, int num, std::ostream &log);
// onStart: Called at the start, if you want to initialize certain global
// variables, or make something else before the actual simulation starts.
//
// Parameters:
//
// rows: number of rows
// cols: number of columns
// num:  number of dwarfs
//
// log:  a cout-like log


void onAction(Dwarf &dwarf, int day, int hours, int minutes, std::ostream &log);
// onAction: Called each time a dwarf is idle and choosing their next action.
//
// Parameters:
//
// dwarf:   dwarf choosing an action
// day:     day (1+)
// hours:   number of hours in 24-hour format (0-23)
// minutes: number of minutes (0-59)
//
// log:     a cout-like log

#endif
