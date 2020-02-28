#include <iostream>
#include <thread>
#include <chrono>

#include "Client.h"

int main(int argc, char **argv)
{
    Client *client = new Client();

    while(!client->isStopped() && client->step() == 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    delete client;
    return 0;
}