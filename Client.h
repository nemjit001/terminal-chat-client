#ifndef CLIENT_H
#define CLIENT_H

#include <thread>

class Client
{
private:
    bool stop;
public:
    Client();
    ~Client();
    int step();
    bool isStopped();
};


#endif