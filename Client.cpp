#include "Client.h"

int Client::readFromStdin()
{
    return 0;
}

int Client::readFromSocket()
{
    return 1;
}


Client::Client()
{
    this->stop = false;
    this->startThreads();

    sock_init();
}

Client::~Client()
{
    this->stopThreads();
    sock_close(this->clientSocket);
    sock_quit();
}

int Client::step()
{
    return 0;
}

bool Client::isStopped()
{
    return this->stop;
}

void Client::setStopped(bool val)
{
    this->stop = val;
}

bool Client::connect()
{
    return true;
}