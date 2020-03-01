#include "Client.h"

int Client::readFromStdin()
{
    std::string userInput;
    getline(std::cin, userInput);

    userInput += '\n';

    if (this->inputBuffer->write(userInput.c_str(), userInput.length())) return userInput.length();

    return -1;
}

int Client::readFromSocket()
{
    return 1;
}

Client::Client()
{
    this->inputBuffer = new CircularLineBuffer();
    this->outputBuffer = new CircularLineBuffer();
    this->stop = false;
    this->startThreads();
}

Client::~Client()
{
    this->stopThreads();

    delete this->inputBuffer;
    delete this->outputBuffer;

    sock_close(this->clientSocket);
    sock_quit();
}

int Client::step()
{
    std::string userInput = this->inputBuffer->read();

    if (!sock_valid(this->clientSocket))
    {
        return -1;
    }

    send(this->clientSocket, userInput.c_str(), userInput.length(), 0);
    
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

bool Client::connect_client()
{
    addrinfo hints;
    addrinfo *results;
    char *hostname = "127.0.0.1";
    char *service = "1337";

    memset(&hints, 0, sizeof(hints));
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;
    
    if (sock_init() != 0) return false;

    if (getaddrinfo(hostname, service, &hints, &results) != 0) return false;

    this->clientSocket = socket(results->ai_family, results->ai_socktype, results->ai_protocol);

    try_connect:
    if (connect(this->clientSocket, results->ai_addr, results->ai_addrlen) != 0)
    {
        std::cout << "retrying connection..." << std::endl;
        goto try_connect;
    }
    
    freeaddrinfo(results);
    return true;
}