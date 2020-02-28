#include "Client.h"

Client::Client()
{
    this->stop = false;
}

Client::~Client()
{
}

int Client::step()
{
    return 0;
}

bool Client::isStopped()
{
    return this->stop;
}