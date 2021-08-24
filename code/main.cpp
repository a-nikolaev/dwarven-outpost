
#include <unistd.h>
#include <stdio.h>
#include <curses.h>
#include <locale.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>

#include <stdlib.h>
#include <string.h>

#include "state.h"
#include "output.h"

// Chrono
#include <chrono>
#include <thread>

int update_from_input(State &s, std::ostream &gamelog);
void finish(int sig);

/* Run the game */
void run (State &st) {

  buf buf;
  std::ostream gamelog (&buf); // game log

  output(st, buf.data);
  
  { // Bot logic init
    onStart(st.rows, st.cols, st.num, gamelog);
  }

  int k = 0;
  int finished = 0;
  while( !finished ) {
    k++;
    if (k>1) { 
      
      k=0;
      // run the bot's logic and update the state
      if (st.play) {
        update(st, gamelog);
      }

    }
    finished = update_from_input(st, gamelog);
    output(st, buf.data);
    std::this_thread::sleep_for(std::chrono::microseconds(10000));
  }
}

/* Helper function, puts the value v in the interval [min, max] */
int put_in_range(int v, int min, int max) {
  if (v < min)
    return min;
  else if (v > max)
    return max;
  else
    return v;
}

double put_in_range_double(double v, double min, double max) {
  if (v < min)
    return min;
  else if (v > max)
    return max;
  else
    return v;
}

/* Main function. Mostly boilerplate code of setting up the terminal. */
int main(int argc, char* argv[]){

  /*       1    2    3    4   5    6    */
  /* ARGS: PART ROWS COLS NUM SEED FAST*/
  /* Init the game */
  char part = 'a';
  int rows = 20;
  int cols = 20;
  int num = 1;

  if (argc > 1) {
    part = argv[1][0];
    if (part < 'a') part = 'a';
    if (part > 'c') part = 'a';
  }
  
  // Random seed
  if (argc > 5 && (strcmp(argv[5], "-") != 0)) {
    srand(atoi(argv[5]));
  }
  else {
    srand(time(NULL));
    rows += -2 + rand() % 5;
    cols += -2 + rand() % 5;
  }

  switch(part) {
    case 'a':
      num = 1;
      break;
    case 'b':
      num = 6;
      break;
    case 'c':
      num = 6 + rand()%3;
      break;
  }

  if (argc > 2 && (strcmp(argv[2], "-") != 0)) {rows = put_in_range(atoi(argv[2]), 10, 40);}
  if (argc > 3 && (strcmp(argv[3], "-") != 0)) {cols = put_in_range(atoi(argv[3]), 10, 40);}
  if (argc > 4 && (strcmp(argv[4], "-") != 0)) {num  = put_in_range(atoi(argv[4]), 1, 10);}

  // Skip the video simulation?
  bool fast = (argc > 6) && (strcmp(argv[6], "fast") == 0);
 
  // Init the game state
  State st;
  init(st, part, rows, cols, num);

  if (!fast) 
  {
    /* User interface initialization */
    signal(SIGINT, finish);

    /* ncurses initialization */
    setlocale(LC_ALL, "");
    initscr();     /* initialize the library and screen */
    cbreak();      /* put terminal into non-blocking input mode */
    nonl();        /* no NL -> CRNL on output */
    noecho();      /* turn off echo */
    start_color();
    curs_set(0);   /* hide the cursor */
    timeout(0);

    use_default_colors();
    init_pair(0, COLOR_WHITE, COLOR_BLACK);
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_BLACK, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    init_pair(4, COLOR_GREEN, COLOR_BLACK);
    init_pair(5, COLOR_BLUE, COLOR_BLACK);
    init_pair(6, COLOR_YELLOW, COLOR_BLACK);
    init_pair(7, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(8, COLOR_CYAN, COLOR_BLACK);
    
    init_pair(9, COLOR_BLACK, COLOR_GREEN);
    init_pair(10, COLOR_WHITE, COLOR_GREEN);
    init_pair(11, COLOR_RED, COLOR_GREEN);
    
    init_pair(12, COLOR_CYAN, COLOR_BLUE);

    color_set(0, NULL);
    assume_default_colors(COLOR_WHITE, COLOR_BLACK);

    attrset(A_NORMAL | COLOR_PAIR(0));
    refresh();
    clear();
   
    /* Run the game */
    run(st);
  
    /* Restore the teminal state */
    echo();
    curs_set(1);
    clear();
    endwin();
  }
  else {

    buf buf;
    std::ostream gamelog (&buf); // game log

    // init bot logic
    onStart(st.rows, st.cols, st.num, gamelog);
    
    // run the game until it's over
    while(st.status == RUNNING) { update(st, gamelog); }

    if (argc > 7) {
      FILE *ff = fopen(argv[7], "w+");
      
      if (st.status == SUCCESS) 
        fprintf(ff, "success \n");
      else
        fprintf(ff, "failure \n");

      fprintf(ff, "Time: %i \n", st.time);
      fprintf(ff, "Dwarves: %i / %i \n", count_place(st, DWARF), st.num);
      fprintf(ff, "Structure: %i \n", largest_structure(st));
      fprintf(ff, "Lumber: %i \n", st.lumber);
      fprintf(ff, "Apples: %i \n", st.apples);
      fprintf(ff, "Pumpkins: %i \n", st.pumpkins);
        
      fprintf(ff, "\n");

      fclose(ff);
    }
    else {
      if (st.status == SUCCESS) 
        printf("success \n");
      else
        printf("failure \n");

      printf("Time: %i \n", st.time);
      printf("Dwarves: %i / %i \n", count_place(st, DWARF), st.num);
      printf("Structure: %i \n", largest_structure(st));
      printf("Lumber: %i \n", st.lumber);
      printf("Apples: %i \n", st.apples);
      printf("Pumpkins: %i \n", st.pumpkins);
        
      printf("\n");
    }
  }

  return 0;
}

int update_from_input(State &s, std::ostream &gamelog)
{
    int c;
    int finished=0;

    while ( !finished && (c=getch()) != ERR ) {

      switch(c){
        case 'q': case 'Q':
          finished = 1;
          break;
        case 'f': case 'F':
          for(int i = 0; i< 60 * 3; ++i){
            update(s, gamelog);
          }
          break;
        case 's': case 'S':
          s.play = false;
          for(int i = 0; i<1; ++i){
            update(s, gamelog);
          }
          break;
        case 'p': case 'P':
          s.play = !s.play;
          break;
        default:;
      }

    }                
    return finished;
}

/* SIGINT */
void finish(int sig)
{
  echo();
  curs_set(1);
  clear();
  endwin();
  exit(0);
}
