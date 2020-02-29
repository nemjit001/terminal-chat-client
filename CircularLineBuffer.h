#ifndef CPP_CIRCULAR_LINE_BUFFER_H
#define CPP_CIRCULAR_LINE_BUFFER_H

#include <mutex>
#include <string>

class CircularLineBuffer
{
private:
    std::mutex buffer_mtx;
    char *buffer;
    int start_ptr, curr_ptr;
public:
    CircularLineBuffer();
    ~CircularLineBuffer();

    bool write(std::string chars, size_t n_chars);
    std::string read();
};


#endif