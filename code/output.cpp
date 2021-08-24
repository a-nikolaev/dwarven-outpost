
#include <curses.h>
#include <string>
#include "output.h"

#define X(r,c) (c)*2 + 6
#define Y(r,c) (r)*1 + 3

#define GRAY_COLOR (A_NORMAL | COLOR_PAIR(1))

#define TREE_COLOR (A_NORMAL | COLOR_PAIR(4))
#define NIGHT_TREE_COLOR (A_NORMAL | COLOR_PAIR(8))
#define FENCE_COLOR (A_NORMAL | COLOR_PAIR(5))

#define DWARF_COLOR (A_BOLD | COLOR_PAIR(8))

#define ZOMBIE_COLOR (A_BOLD | COLOR_PAIR(4))
#define PUMPKIN_COLOR (A_BOLD | COLOR_PAIR(3))

void print_hint(std::string s, char c) {
  bool needs_highlight = true;
  attrset(A_NORMAL | COLOR_PAIR(1));
  addch('[');
  for(unsigned int i = 0; i<s.size(); ++i) {
    if (needs_highlight && s[i] == c) {
      attrset(A_BOLD | COLOR_PAIR(1));
      addch(c);
      attrset(A_NORMAL | COLOR_PAIR(1));
      needs_highlight = false;
    }
    else
      addch(s[i]);
  }
  addch(']');
}

void output(State &st, std::vector<std::string> &logbufdata) {
     
  if (st.lumber > 0) {
    move(Y(-3,0), X(-3,0));
    attrset(A_NORMAL | COLOR_PAIR(4));
    printw("Lumber: %i  ", st.lumber);
  }
  
  if (st.apples > 0) {
    move(Y(-3,10), X(-3,10));
    attrset(A_NORMAL | COLOR_PAIR(6));
    printw("Apples: %i  ", st.apples);
  }

  if (st.pumpkins > 0) {
    move(Y(-3,20), X(-3,20));
    attrset(A_NORMAL | COLOR_PAIR(3));
    printw("Pumpkins: %i  ", st.pumpkins);
  }
  attrset(A_NORMAL | COLOR_PAIR(1));

  int rows = st.rows;
  int cols = st.cols;

  // draw the labels
  for(int r = 0; r < rows; ++r) {

    if (r%5 != 0) continue;

    move(Y(r,-1), X(r,-1));
    printw("%i", r%10);
    
    if (r/10 > 0) {
      move(Y(r,-1), X(r,-1)-1);
      printw("%i", r/10);
    }
  }

  for(int c = 0; c < cols; ++c) {
    
    if (c%5 != 0) continue;
   
    /*
    move(Y(-1,c), X(-1,c));
    printw("%i", c%10);
    
    if (c/10 > 0) {
      move(Y(-1,c)-1, X(-1,c));
      printw("%i", c/10);
    }
    */
    move(Y(-1,c), X(-1,c));
    printw("%i", c);
  }

  int tree_color = TREE_COLOR;
  if (is_night(st)) tree_color = NIGHT_TREE_COLOR;

  // draw the map
  for(int r = 0; r < rows; ++r) {
    for(int c = 0; c < cols; ++c) {
      move(Y(r,c), X(r,c));
      
      switch(st.map[r][c].pl) {
        
        case EMPTY: 
          attrset(tree_color);
          if (false)
          {
            int sm = (int)(st.smell[r][c] + 0.5) / 10;
            if (sm == 0) {
              addch('.');
              addch(' ');
            }
            else {
              attrset(FENCE_COLOR);
              addch('0' + (sm%10));
              addch(' ');
            }
          }
          else {
            addch('.');
            addch(' ');
          }
          break;

        case PINE_TREE: 
          attrset(tree_color);
          addch('Y');
          addch(' ');
          break;
        
        case APPLE_TREE: 
          attrset(tree_color);
          addch('&');
          addch(' ');
          break;

        case DWARF: 
          attrset(DWARF_COLOR);
          //addch('D');
          addch('0' + st.map[r][c].durability);
          addch('/');
          attrset(A_NORMAL | COLOR_PAIR(1));
          //addch(' ');
          break;
        
        case ZOMBIE: 
          attrset(ZOMBIE_COLOR);
          addch('Z');
          addch(' ');
          break;
        
        case PUMPKIN_ZOMBIE: 
          attrset(PUMPKIN_COLOR);
          addch('Z');
          addch(' ');
          break;
        
        case PUMPKIN_ZOMBIE_BABY: 
          attrset(PUMPKIN_COLOR);
          addch('z');
          addch(' ');
          break;
        
        case PUMPKIN: 
          attrset(PUMPKIN_COLOR);
          addch('*');
          addch(' ');
          break;
        
        case FENCE: 
          attrset(FENCE_COLOR);
          addch('#');
          addch(' ');
          break;
        
        default:
          addch(' ');
          addch(' ');
          break;
      }
    }
  }

  {
    Place places[8] = {DWARF, PINE_TREE, APPLE_TREE, FENCE, PUMPKIN, ZOMBIE, PUMPKIN_ZOMBIE, PUMPKIN_ZOMBIE_BABY};
    bool found[8];
    for (int i = 0; i<8; i++) { found[i] = false; }

    for (int r = 0; r < st.rows; r++) {
      for (int c = 0; c < st.cols; c++) {
        Place pl = st.map[r][c].pl;
        for (int i = 0; i<8; i++) { if (places[i] == pl) found[i] = true; }
      }
    }

    int y = 0;
    for(int i = 0; i < 8; i++) {
      if (found[i]) {
        int yy = Y(y, st.cols + 1);
        int xx = X(y, st.cols + 1);
        move(yy, xx);
        y += 1;

        switch(places[i]) {
          
          case PINE_TREE: 
            attrset(tree_color);
            addch('Y');
            addch(' ');
            printw(" PINE_TREE");
            break;
          
          case APPLE_TREE: 
            attrset(tree_color);
            addch('&');
            addch(' ');
            printw(" APPLE_TREE");
            break;

          case DWARF: 
            attrset(DWARF_COLOR);
            addch('0');
            addch('/');
            printw(" DWARF");
            break;
          
          case ZOMBIE: 
            attrset(ZOMBIE_COLOR);
            addch('Z');
            addch(' ');
            printw(" ZOMBIE");
            break;
          
          case PUMPKIN_ZOMBIE: 
            attrset(PUMPKIN_COLOR);
            addch('Z');
            addch(' ');
            printw(" PUMPKIN_ZOMBIE");
            break;
          
          case PUMPKIN_ZOMBIE_BABY: 
            attrset(PUMPKIN_COLOR);
            addch('z');
            addch(' ');
            printw(" PUMPKIN_ZOMBIE_BABY");
            break;
          
          case PUMPKIN: 
            attrset(PUMPKIN_COLOR);
            addch('*');
            addch(' ');
            printw(" PUMPKIN");
            break;
          
          case FENCE: 
            attrset(FENCE_COLOR);
            addch('#');
            addch(' ');
            printw(" FENCE");
            break;

          default:
            break;
        }
      }
      for(int i = y; i < 8; i++) {
        int yy = Y(y, st.cols + 1);
        int xx = X(y, st.cols + 1);
        move(yy, xx);
        printw("                         ");
      }

    }
  }
  

  // Draw the interface

  int yy = Y(rows + 1, 0);
  int xx = 2; //X(rows + 1, 0);

  attrset(A_NORMAL | COLOR_PAIR(1));
  move(yy, xx + 1);
  
  int day = st.time / (60 * 24);
  int hours = (st.time - day*(60*24)) / 60;
  int minutes = st.time % (60);
  printw("Day: %i  Time: %02i:%02i", day, hours, minutes);
  
  move(yy+1, xx + 1);
  if (st.status == FAILURE) {
    attrset(A_BOLD | COLOR_PAIR(3));
    printw("GAME OVER");
    int lrg = largest_structure(st);
    if (lrg > 0)
      printw("   Largest structure: %i", lrg);
  }
  if (st.status == SUCCESS) {
    attrset(A_BOLD | COLOR_PAIR(4));
    if (st.part == 'c')
      printw("SUCCESS");
    else  
      printw("TIME IS UP");
    
    int lrg = largest_structure(st);
    if (lrg > 0)
      printw("   Largest structure: %i", lrg);
  }
  attrset(A_NORMAL | COLOR_PAIR(1));


  move(yy+0, xx+25);
  print_hint("Quit", 'Q');
  
  move(yy+0, xx+35);
  if (st.play)
    print_hint("Pause", 'P');
  else {
    print_hint("Play", 'P');
    addch(' ');
  }

  move(yy+0, xx+45);
  print_hint("Step", 'S') ;
  
  move(yy+0, xx+55);
  print_hint("Fast-forward", 'F') ;
 
  /* Log */
  attrset(GRAY_COLOR);
  move(yy+2, xx);
  for (unsigned int i = 0; i < log_line_length; i++) {
    addch('-');
  }
  move(yy+2+9, xx);
  for (unsigned int i = 0; i < log_line_length; i++) {
    addch('-');
  }
  move(yy+2, xx + log_line_length - 25);
  addch('|');
  printw(" Outpost log ");
  addch('|');

  for (unsigned int i = 0; i < logbufdata.size(); ++i) {
    move(yy+3 + i, xx);
    clrtoeol();
  }

  attrset(A_NORMAL | COLOR_PAIR(1));
  for (unsigned int i = 0; i < logbufdata.size(); ++i) {
    move(yy+3 + i, xx);
    printw("%s", logbufdata[i].c_str());
  }

  refresh();
}
