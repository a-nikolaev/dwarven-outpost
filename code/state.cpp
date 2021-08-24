
#include "common.h"
#include "state.h"
#include <queue>
#include <utility>
#include <cassert>

using namespace std;

bool in_range(State &st, int r, int c) {
  return (r >= 0 && r < st.rows && c >= 0 && c < st.cols);
}

bool operator==(Loc &a, Loc &b) {
  return a.r == b.r && a.c == b.c;
}

bool is_agent(Object &obj) {
  switch(obj.pl){
    case DWARF: case ZOMBIE: case PUMPKIN_ZOMBIE: case PUMPKIN_ZOMBIE_BABY: return true;
    default: return false;                                                                        
  }
}
          
int default_wait(Place pl) {
  switch(pl) {
    case DWARF: return 3;
    case ZOMBIE: 
    case PUMPKIN_ZOMBIE: 
    case PUMPKIN_ZOMBIE_BABY: return 4;
    default: return 1;
  }
}

bool is_free(State &st, Loc loc) {
  if (not in_range(st, loc.r, loc.c))
    return false;
 
  return st.map[loc.r][loc.c].pl == EMPTY;
} 

int next_prng(State &st) {
  int ans = st.prng_seq[st.prng_index];
  st.prng_index = (st.prng_index + 1) % st.prng_seq.size();
  return ans;
}

Loc random_loc(State &st) {
  Loc loc = {next_prng(st) % st.rows, next_prng(st) % st.cols};
  return loc;
} 

Loc random_boundary_loc(State &st) {
  
  int i = next_prng(st) % (2 * (st.rows + st.cols));
  Loc loc = {0, 0};
  if (i < st.cols) {
    loc.r = 0;
    loc.c = i;
    return loc;
  }
  i -= st.cols;
  
  if (i < st.cols) {
    loc.r = st.rows-1;
    loc.c = i;
    return loc;
  }
  i -= st.cols;
  
  if (i < st.rows) {
    loc.r = i;
    loc.c = 0;
    return loc;
  }
  i -= st.rows;
    
  loc.c = st.cols-1;
  loc.r = i;

  return loc;
} 

Loc translate_loc(Loc loc, Dir dir) { 
  Loc ans = loc;
  switch(dir) {
    case NORTH: ans.r -= 1; return ans;
    case SOUTH: ans.r += 1; return ans;
    case WEST:  ans.c -= 1; return ans;
    case EAST:  ans.c += 1; return ans;
    default: return ans;
  }
}

Place get_place(State &st, Loc loc) {
  if (in_range(st, loc.r, loc.c))
    return st.map[loc.r][loc.c].pl;
  else
    return UNKNOWN;
}

bool is_night(State &st) {
  int day = st.time / (60 * 24);
  int hours = (st.time - day*(60*24)) / 60;
  return (21 <= hours || hours < 6);
}

void init(State &st, char part, int rows, int cols, int number) {
  
  // prng sequence
  for (int i = 0; i < 99371; i++) {
    st.prng_seq.push_back(rand());
  }
  st.prng_index = 0;
  
  st.part = part;
  st.rows = rows;
  st.cols = cols;
  st.num = number;
  st.status = RUNNING;
  st.time = (24 * 60) + 6 * 60;
  st.play = true;

  st.lumber = 0;
  st.apples = 0;
  st.pumpkins = 0;

  Object obj_empty =       {EMPTY,      0, IDLE, WEST, {0, 0},  0, vector<Loc>(0)};
  Object obj_pine_tree =   {PINE_TREE,  0, IDLE, WEST, {0, 0}, 10, vector<Loc>(0)};
  Object obj_apple_tree =  {APPLE_TREE, 0, IDLE, WEST, {0, 0},  2, vector<Loc>(0)};
  Object obj_dwarf =       {DWARF,      0, IDLE, WEST, {0, 0},  0, vector<Loc>(0)};
 
  // set dwarf smell to zero
  st.smell.resize(rows);
  for(int r = 0; r < rows; ++r) {
    st.smell[r].resize(cols);
    for(int c = 0; c < cols; ++c) {
      st.smell[r][c] = 0;
    }
  }

  // initialize the map
  st.map.resize(rows);
  for(int r = 0; r < rows; ++r) {
    st.map[r].resize(cols);
    for(int c = 0; c < cols; ++c) {
      st.map[r][c] = obj_empty;
    }
  }
  
  // place agents
  int dwarf_name = 0;
  for(int i = 0; i < number; i++) {
    int attempts = 20;
    while(attempts > 0) {
      Loc loc = random_loc(st);
      if (is_free(st, loc)) {
        st.map[loc.r][loc.c] = obj_dwarf;
        st.map[loc.r][loc.c].durability = dwarf_name;
        dwarf_name += 1;
        st.map[loc.r][loc.c].wait = rand() % 20;
        break;
      }
      attempts --;
    }
  }
  
  // place trees
  int pine_trees_number = st.rows * st.cols / 10;
  for(int i = 0; i < pine_trees_number; i++) {
    int attempts = 20;
    while(attempts > 0) {
      Loc loc = random_loc(st);
      if (is_free(st, loc)) {
        st.map[loc.r][loc.c] = obj_pine_tree;
        break;
      }
      attempts --;
    }
  }
  
  int apple_trees_number = st.rows * st.cols / 20;
  for(int i = 0; i < apple_trees_number; i++) {
    int attempts = 20;
    while(attempts > 0) {
      Loc loc = random_loc(st);
      if (is_free(st, loc)) {
        st.map[loc.r][loc.c] = obj_apple_tree;
        break;
      }
      attempts --;
    }
  }
  
}

namespace{
  Action scheduled_ac = IDLE;
  Dir scheduled_dir = WEST;
  int scheduled_row = 0;
  int scheduled_col = 0;
  bool scheduling_done = false;

  void reset_action() {
    scheduled_ac = IDLE;
    scheduled_dir = WEST;
    scheduled_row = 0;
    scheduled_col = 0;
    scheduling_done = false;
  }

  void set_action(Action ac, Dir dir, int r, int c) {
    if (scheduling_done) {
      return;
    }
    scheduled_ac = ac;
    scheduled_dir = dir;
    scheduled_row = r;
    scheduled_col = c;
    scheduling_done = true;
  }

}

void shuffle(State & st, std::vector<int> & v) {
  if (v.size() <= 1) return;

  for(size_t i = v.size()-1; i >= 1; i--) {
    int j = next_prng(st) % (i+1);
    int tmp = v[i];
    v[i] = v[j];
    v[j] = tmp;
  }
}

/**
 * Source: https://github.com/dragonslayerx
 */

void find_best_path(vector< vector< pair<int,int> > > &G, int vertexCount, int src, int dst, vector<int> &found_path){
    const int PATH_INF = 1e9 - 1;
    
    priority_queue<pair<int, int>, vector<pair<int, int> >, greater<pair<int, int> > > pq;
    vector<bool>isvisited(vertexCount, false);
    vector<int>dist(vertexCount, PATH_INF);

    dist[src] = 0;
    pq.push(make_pair(0, src));
    while (!pq.empty()){
        pair<int, int> tp = pq.top();
        pq.pop();
        int node = tp.second;
        int d = tp.first;
        if (!isvisited[node]) {
            isvisited[node] = true;
            for (size_t i = 0; i < G[node].size(); i++) {
                int v = G[node][i].first;
                int w = G[node][i].second;
                if (dist[v] > d + w) {
                    dist[v] = d + w;
                    pq.push(make_pair(dist[v], v));
                }
            }
        }
    }
  
    found_path.resize(0);
    int node = dst;
    if(dist[dst] < PATH_INF) {
      
      while(node != src) {
        found_path.push_back(node);
        
        int best_v = src;
        int best_d = PATH_INF;
        for(size_t i = 0; i < G[node].size(); i++) {
          int v = G[node][i].first;
          int w = G[node][i].second;
          int d = dist[v];
          if (d < best_d && dist[v] + w == dist[node]) {
            best_v = v;
            best_d = d;
          }
        }

        node = best_v;
      }
    }
}

int to_vx(State & st, int r, int c) {
  return r * st.cols + c;
}

int to_vx(State & st, Loc loc) {
  return loc.r * st.cols + loc.c;
}

int to_r(State & st, int vx) {
  return vx / st.cols;
}

int to_c(State & st, int vx) {
  return vx % st.cols;
}

void find_path(State &st, Loc src, Loc dst, vector<Loc> &found_path_loc) {

  if ((not in_range(st, src.r, src.c)) || (not in_range(st, dst.r, dst.c))) {
    found_path_loc.resize(0);
    return;
  }

  int num_vxs = st.rows * st.cols;

  vector< vector< pair<int,int> > > g(num_vxs);
  for(int r = 0; r < st.rows; r++) {
    for(int c = 0; c < st.cols; c++) {
      Place pl = st.map[r][c].pl;
      bool it_is_agent = is_agent(st.map[r][c]); 

      if(not (pl==EMPTY || is_agent(st.map[r][c]))) {
        continue;
      }

      int vx = to_vx(st, r, c);

      Loc locs[] = { {r-1, c}, {r+1, c}, {r, c-1}, {r, c+1}};
      for(int i = 0; i < 4; i++) {
        if (in_range(st, locs[i].r, locs[i].c)) {
          
          int nbr_vx = to_vx(st, locs[i]);
          Place nbr_pl = st.map[locs[i].r][locs[i].c].pl;
          bool nbr_is_agent = is_agent(st.map[locs[i].r][locs[i].c]); 
          
          if (pl == EMPTY && nbr_pl == EMPTY) {
            g[vx].push_back(make_pair(nbr_vx, 10));
          }
          else if ( (pl == EMPTY && nbr_is_agent) || (nbr_pl == EMPTY && it_is_agent) ) {
            g[vx].push_back(make_pair(nbr_vx, 100));
          }
          else if ( nbr_is_agent && it_is_agent ) {
            g[vx].push_back(make_pair(nbr_vx, 800));
          }
        }
      }
    }
  }
  vector<int>found_path_vx;
  find_best_path(g, num_vxs, to_vx(st,src), to_vx(st,dst), found_path_vx);

  found_path_loc.resize(0);
  for(size_t i = 0; i < found_path_vx.size(); i++) {
    int vx = found_path_vx[i];
    Loc loc = { to_r(st, vx), to_c(st, vx) };
    found_path_loc.push_back(loc);
  }

}

Dwarf make_dwarf(State &st, int row, int col) {
  
  vector <vector <Place> > area(st.rows);
  for(int r = 0; r < st.rows; ++r) {
    area[r].resize(st.cols);
    for(int c = 0; c < st.cols; ++c) {
      area[r][c] = st.map[r][c].pl;
    }
  }

  int name = st.map[row][col].durability;

  return Dwarf(row, col, name, st.lumber, st.rows + st.cols, area, set_action);  
}

void update_smell(State &st) {

  // add smell
  for(int row = 0; row < st.rows; row++) {
    for(int col = 0; col < st.cols; col++) {
      if (st.map[row][col].pl == DWARF) {
        st.smell[row][col] += 8;
      }
    }
  }
  
  // save old values
  vector <vector <double> > old(st.rows);
  for(int r = 0; r < st.rows; ++r) {
    old[r].resize(st.cols);
    for(int c = 0; c < st.cols; ++c) {
      old[r][c] = st.smell[r][c];
    }
  }

  // move
  for(int r = 0; r < st.rows; ++r) {
    for(int c = 0; c < st.cols; ++c) {
      
      double amount = old[r][c] * 0.1;

      st.smell[r][c] -= 4.0 * amount;

      Loc nbrs[] = {{r-1, c}, {r+1, c}, {r, c-1}, {r, c+1}};

      for (int i = 0; i < 4; i++) {
        int tr = nbrs[i].r;
        int tc = nbrs[i].c;

        if (in_range(st, tr, tc)) {
          st.smell[tr][tc] += amount;
          
          if (st.map[r][c].pl == EMPTY || is_agent(st.map[r][c])) {
            st.smell[r][c] -= amount;
            st.smell[tr][tc] += amount;
          }
        }
      }
    }
  }

}

void add_zombies(State &st) {
  
  int day = st.time / (60 * 24);

  if (is_night(st)) {
    /* night */
    for(int r = 0; r < st.rows; ++r) {
      for(int c = 0; c < st.cols; ++c) {
        Place pl = st.map[r][c].pl;
        if (pl == PUMPKIN_ZOMBIE_BABY) {
          st.map[r][c].pl = PUMPKIN_ZOMBIE;
        }
      }
    }
    if (next_prng(st) % (100/(day+1)) == 0) {
    
      int attempts = 20;
      while(attempts > 0) {
        Loc loc = random_boundary_loc(st);
        assert(in_range(st, loc.r, loc.c));
        if (is_free(st, loc)) {
  
          Object obj_zombie = {ZOMBIE, 0, IDLE, WEST, {0, 0},  0, vector<Loc>(0)};
          if (next_prng(st)%20 < day)
            obj_zombie.pl = PUMPKIN_ZOMBIE;

          st.map[loc.r][loc.c] = obj_zombie;

          break;
        }
        attempts --;
      }

    }
  }
  else {
    /* morning */
    for(int r = 0; r < st.rows; ++r) {
      for(int c = 0; c < st.cols; ++c) {
        Place pl = st.map[r][c].pl;
        if (pl == ZOMBIE || pl == PUMPKIN_ZOMBIE) {
          if (next_prng(st)%10 == 0) {

            if (pl == ZOMBIE) {
              Object obj_empty = {EMPTY, 0, IDLE, WEST, {0, 0},  0, vector<Loc>(0)};
              st.map[r][c] = obj_empty;
            }
            else {
              Object obj_empty = {PUMPKIN, 0, IDLE, WEST, {0, 0},  0, vector<Loc>(0)};
              st.map[r][c] = obj_empty;
            }

          }
        }
      }
    }

  }

}

void update(State &st, std::ostream &gamelog) { 
  // don't do anything if the game had finished
  if (st.status == SUCCESS || st.status == FAILURE) return;

  int dtime = 1;
  st.time += dtime;
  
  int days = st.time / (60 * 24);
  int hours = (st.time - days*(60*24)) / 60;
  int minutes = st.time % (60);

  vector<int>v;
  for(int row = 0; row < st.rows; row++) {
    for(int col = 0; col < st.cols; col++) {
      if (is_agent(st.map[row][col])) {
        if (st.map[row][col].wait <= 0) {
          v.push_back(to_vx(st, row, col));
        }
      }
    }
  }
  shuffle(st, v);

  for(size_t i = 0; i < v.size(); i++) {
    int row = to_r(st, v[i]);
    int col = to_c(st, v[i]);
    Loc cur_loc = {row, col};

    Object & obj = st.map[row][col];
    if ((not is_agent(obj)) || (obj.wait > 0)) {
      continue;
    }
    
    st.map[row][col].wait = default_wait(st.map[row][col].pl);
   
    // Process active actions
    // Walk
    if (obj.action == WALK) {
      if (obj.dst == cur_loc) {
        obj.action = IDLE;
      }
      else if (obj.path.size() == 0){
        find_path(st, {row,col}, obj.dst, obj.path);
        if (obj.path.size() <= 0) {
          obj.action = IDLE;
        }
      }
      else {
        Loc next_loc = obj.path.back();
        if (st.map[next_loc.r][next_loc.c].pl != EMPTY) {
          if (next_prng(st) % 5 == 0) {
            obj.action = IDLE;
          }
          else{
            obj.path.resize(0);
          }
        }
        else {
          obj.path.pop_back();
          Object e = st.map[next_loc.r][next_loc.c];
          st.map[next_loc.r][next_loc.c] = st.map[row][col];
          st.map[row][col] = e;
        }
      }
    }
    // Chop
    else if (obj.action == CHOP) {
      Loc target_loc = translate_loc(cur_loc, obj.dir);
      Place pl = get_place(st, target_loc);
      if (pl == PINE_TREE || pl == APPLE_TREE) {
        int tr = target_loc.r;
        int tc = target_loc.c;
        st.map[tr][tc].durability -= 1;
        st.lumber += 1;

        if (st.map[tr][tc].durability <= 0) {
          st.map[tr][tc].pl = EMPTY;
          obj.action = IDLE;
        }
      }
      else if (pl == FENCE) {
        int tr = target_loc.r;
        int tc = target_loc.c;
        st.lumber += 10;
        st.map[tr][tc].pl = EMPTY;
        obj.action = IDLE;
      }
      else if (pl == PUMPKIN) {
        int tr = target_loc.r;
        int tc = target_loc.c;
        Object obj_zombie = {PUMPKIN_ZOMBIE_BABY, 0, IDLE, WEST, {0, 0},  0, vector<Loc>(0)};
        st.map[tr][tc] = obj_zombie;
        obj.action = IDLE;
      }
      else {
        obj.action = IDLE;
      }
    }
    // Build
    else if (obj.action == BUILD) {
      Loc target_loc = translate_loc(cur_loc, obj.dir);
      Place pl = get_place(st, target_loc);

      int price = 10;

      if (pl == EMPTY && st.lumber >= price) {
        int tr = target_loc.r;
        int tc = target_loc.c;
        st.lumber -= price;
        Object obj_fence = {FENCE, 0, IDLE, WEST, {0, 0},  0, vector<Loc>(0)};
        st.map[tr][tc] = obj_fence;
        obj.action = IDLE;
      }
      else {
        obj.action = IDLE;
      }
    }
    // Pick
    else if (obj.action == PICK) {
      Loc target_loc = translate_loc(cur_loc, obj.dir);
      Place pl = get_place(st, target_loc);
      if (pl == APPLE_TREE) {
        st.apples += 1;
        obj.action = IDLE;
      }
      else if (pl == PUMPKIN) {
        int tr = target_loc.r;
        int tc = target_loc.c;
        st.pumpkins += 1;
        st.map[tr][tc].pl = EMPTY;
        obj.action = IDLE;
      }
      else {
        obj.action = IDLE;
      }
    }
    // Attack
    else if (obj.action == ATTACK) {
      Loc target_loc = translate_loc(cur_loc, obj.dir);
      Place pl = get_place(st, target_loc);
      if (pl == DWARF) {
        int tr = target_loc.r;
        int tc = target_loc.c;
        Object obj_zombie = {ZOMBIE, 0, IDLE, WEST, {0, 0},  0, vector<Loc>(0)};
        st.map[tr][tc] = obj_zombie;
        obj.action = IDLE;
      }
      else {
        obj.action = IDLE;
      }
    }
    // Process Idle
    else if (obj.action == IDLE) {
      switch(st.map[row][col].pl){
        case DWARF:
          {
            reset_action();
            Dwarf dw = make_dwarf(st, row, col);
            onAction(dw, days, hours, minutes, gamelog);
            
            if (scheduling_done) {
              st.map[row][col].action = scheduled_ac;
              st.map[row][col].dir = scheduled_dir;
              st.map[row][col].dst.r = scheduled_row;
              st.map[row][col].dst.c = scheduled_col;
              st.map[row][col].path.resize(0);

              switch(scheduled_ac) {
                case PICK:
                  st.map[row][col].wait = default_wait(st.map[row][col].pl) * 5;
                case BUILD:
                  st.map[row][col].wait = default_wait(st.map[row][col].pl) * 3;
                  break;
                default:
                  break;
              }
            }
          }

          break;
        case ZOMBIE:
        case PUMPKIN_ZOMBIE:
        case PUMPKIN_ZOMBIE_BABY:
          {
            Loc nbr [] = {{row-1, col}, {row+1, col}, {row, col-1}, {row, col+1}};
            Dir dir [] = {NORTH, SOUTH, WEST, EAST};
            double best_smell = -100;
            Loc best_smell_loc = {row, col};
            for(int i = 0; i < 4; i++) {
              Place nbr_pl = get_place(st, nbr[i]);
              if (nbr_pl == DWARF) {
                st.map[row][col].action = ATTACK;
                st.map[row][col].dir = dir[i];
                break;
              }
              else if (nbr_pl != UNKNOWN) {
                double noise = (next_prng(st) % 1000) * 0.001 *  (0.1 * st.smell[row][col] + 0.00001) ;
                double cur_smell = st.smell[nbr[i].r][nbr[i].c] + noise;
                if (cur_smell > best_smell && nbr_pl == EMPTY) {
                  best_smell = cur_smell;
                  best_smell_loc = nbr[i];
                }
              }
            }
            if (st.map[row][col].action == IDLE) {
              st.map[row][col].action = WALK;
              st.map[row][col].dst = best_smell_loc;
            }

          }
          break;
        default:
          break;
      }
    }
  }
 
  // all waiting time drops
  for(int row = 0; row < st.rows; row++) {
    for(int col = 0; col < st.cols; col++) {
      if (is_agent(st.map[row][col])) {
        if (st.map[row][col].wait >= 0) {
          st.map[row][col].wait -= 1;
        }
      }
    }
  }

  // update smell
  update_smell(st);
  add_zombies(st);
    
  int num_dwarves = count_place(st, DWARF);
  if (num_dwarves <= 0) {
    st.status = FAILURE;
    return;
  }
  else if (st.part == 'c' && days >= 8) {
    st.status = SUCCESS;
    return;
  }
  else if (st.part == 'b' && days >= 1 && hours >= 21) {
    st.status = SUCCESS;
    return;
  }
  else if (st.part == 'a' && days >= 1 && hours >= 21) {
    st.status = SUCCESS;
    return;
  }
}

int aux_recurse_structure(State &st, std::vector< std::vector <int> > & g, int group, int r, int c) {
  if (not in_range(st, r, c))
    return 0;
  
  int count = 0;
  if ( st.map[r][c].pl == FENCE && g[r][c] == 0 ) {
    count += 1;
    g[r][c] = group;
    count += aux_recurse_structure(st, g, group, r-1, c);
    count += aux_recurse_structure(st, g, group, r+1, c);
    count += aux_recurse_structure(st, g, group, r, c-1);
    count += aux_recurse_structure(st, g, group, r, c+1);
  }
  return count;

}

int largest_structure(State &st) {
  std::vector< std::vector <int> > g;
  g.resize(st.rows);
  for(int r = 0; r < st.rows; ++r) {
    g[r].resize(st.cols);
    for(int c = 0; c < st.cols; ++c) {
      g[r][c] = 0;
    }
  }
  
  int max_count = 0;

  int group = 1;
  for(int r = 0; r < st.rows; ++r) {
    for(int c = 0; c < st.cols; ++c) {
      if (g[r][c] == 0 && st.map[r][c].pl == FENCE) {
        int count = aux_recurse_structure(st, g, group, r, c);
        if (count > max_count) {
          max_count = count;
        }
        group += 1;
      }
    }
  }

  return max_count;
}

int count_place(State &st, Place pl) {
  int count = 0;
  for(int r = 0; r < st.rows; ++r) {
    for(int c = 0; c < st.cols; ++c) {
      if (st.map[r][c].pl == pl) {
        count += 1;
      }
    }
  }
  return count;
}



