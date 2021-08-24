
#include "outstream.h"
#include "common.h"

#include <cstdio> // for EOF

int buf::overflow(int c) {
  if (c == EOF) {
    return EOF;
  }
  else {
    unsigned int max_line_len = log_line_length;
    int last = data.size()-1;
    if (c == '\n' || data[last].size() >= max_line_len) {
      std::string s;
      if (c != '\n') s.push_back(c);
      
      if (data.size() >= (unsigned int)max_size) {
        for(int j = 0; j < last; ++j) {
          data[j] = data[j+1];
        }
        data.pop_back();
      }

      data.push_back(s);
    }
    else{
      data[last].push_back(c);
    }
    
    return c;
  }
}

int buf::sync() {
  return 0;
}

buf::buf() {
  max_size = 8;
  data.resize(1);
  data[0] = "";
}
