
#ifndef OUTSTREAM_H
#define OUTSTREAM_H

#include <streambuf>
#include <iostream>
#include <vector>

class buf : public std::streambuf {
 
  private:
    int max_size;
  public:  
    std::vector<std::string> data;
 
  protected:
    virtual int overflow(int c);
    virtual int sync();

  public: 
    buf();
};

#endif
