#include <iostream>
#include <thread>
#include <chrono>

#include "Client.h"

int main(int argc, char **argv)
{
    std::cout << "Starting client..." << std::endl;
    Client *client = new Client();
    std::cout << "Client started!" << std::endl;

    if (!client->connect_client())
    {
        std::cout << "Connecting failed :(" << std::endl;
        client->setStopped(true);
        delete client;
        return 1;
    }

    while(!client->isStopped() && client->step() == 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::cout << "Stopping client..." << std::endl << "Press [Enter] to exit..." << std::endl;
    delete client;
    return 0;
}