#ifndef CPP_CLIENT_H
#define CPP_CLIENT_H

#include "socket.h"
#include "CircularLineBuffer.h"
#include <thread>
#include <iostream>

#ifndef _WIN32
    #include <pthread.h>
#endif

class Client
{
private:
    bool stop;
    SOCKET clientSocket;
    CircularLineBuffer *inputBuffer, *outputBuffer;
    std::thread inputThread, outputThread;

    inline void startThreads()
    {
        this->inputThread = std::thread(
            &Client::readFromStdinThread,
            this
        );
        this->outputThread = std::thread(
            &Client::readFromSocketThread,
            this
        );
    }

    inline void stopThreads()
    {
        this->inputThread.join();
        this->outputThread.join();
    }

    inline void readFromStdinThread()
    {
        while (!this->isStopped())
        {
            auto res = this->readFromStdin();
            if (res < 0)
            {
                this->setStopped(true);
            }
        }
    }

    inline void readFromSocketThread()
    {
        while (!this->isStopped())
        {
            auto res = this->readFromSocket();
            if (res <= 0)
            {
                this->setStopped(true);
            }
        }
    }

public:
    Client();
    ~Client();

    int step();
    bool isStopped();
    void setStopped(bool val);
    bool connect_client();
    int readFromStdin();
    int readFromSocket();
};


#endif