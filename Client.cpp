#include "Client.h"

Client::Client()
{
    this->inputBuffer = new CircularLineBuffer();
    this->outputBuffer = new CircularLineBuffer();
    this->stop = false;
}

Client::~Client()
{
    sock_close(this->clientSocket);
    sock_quit();

    this->setStopped(true);
    this->stopThreads();

    delete this->inputBuffer;
    delete this->outputBuffer;
}

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
    size_t buffer_size = 4096;
    char *data = (char *)calloc(buffer_size, sizeof(char));
    int bytesReceived = recv(this->clientSocket, data, buffer_size, 0);
    if (bytesReceived <= 0) return -1;

    if (this->outputBuffer->write(data, bytesReceived)) return bytesReceived;

    return -1;
}

int Client::handleInput(std::string input)
{
    std::string output;

    if (!sock_valid(this->clientSocket)) return -1;

    if (std::regex_match(input, std::regex("!quit[\\w\\d\\s\\W\\D\\S]*")))
    {
        output = "QUIT\n";
        send(this->clientSocket, output.c_str(), output.length(), 0);
        return -1;
    }
    else if (std::regex_match(input, std::regex("@[\\w\\d\\s\\W\\D\\S]*")))
    {
        output = "SEND " + input.substr(1);
    }
    else if (std::regex_match(input, std::regex("!who[\\w\\d\\s\\W\\D\\S]*")))
    {
        output = "WHO\n";
    }
    else if (std::regex_match(input, std::regex("!help[\\w\\d\\s\\W\\D\\S]*")))
    {
        std::cout <<
        "--------------------------------------------------------------------------"
        << std::endl <<
        "Commands available:"
        << std::endl <<
        "!who\t\t|\tlists all connected users"
        << std::endl <<
        "@<username>\t|\tsend a message to another user (you can't message yourself)"
        << std::endl <<
        "!quit\t\t|\texit the chat program"
        << std::endl <<
        "!help\t\t|\tdisplay this help"
        << std::endl <<
        "--------------------------------------------------------------------------"
        << std::endl;
        return 0;
    }

    if (send(this->clientSocket, output.c_str(), output.length(), 0) < 0) return -1;

    return 0;
}

int Client::handleResponse(std::string data)
{
    if (std::regex_match(data, std::regex("BAD-RQST-HDR[\\w\\d\\s\\W\\D\\S]*")))
    {
        std::cout << "Error connecting to server" << std::endl;
    }
    else if (std::regex_match(data, std::regex("WHO-OK[\\w\\d\\s\\W\\D\\S]*")))
    {
        std::cout << "Connected users: " << data.substr(7);
    }
    else if (std::regex_match(data, std::regex("DELIVERY[\\w\\d\\s\\W\\D\\S]*")))
    {
        std::string username, message;
        int startNameIndex = 0, endNameIndex = 0;

        for (int i = 0; i < data.length(); i++)
        {
            if (data.at(i) == ' ')
            {
                if (startNameIndex == 0)
                    startNameIndex = i;
                else if (endNameIndex == 0)
                    endNameIndex = i;
                else
                    break;
            }
        }

        username = data.substr(startNameIndex + 1, endNameIndex - startNameIndex - 1);
        message = data.substr(endNameIndex + 1);

        std::cout << "[" << username << "] " << message;
    }
    else if (std::regex_match(data, std::regex("SEND-OK[\\w\\d\\s\\W\\D\\S]*")))
    {
        std::cout << "Message sent" << std::endl;
    }
    else if (std::regex_match(data, std::regex("SEND-FAIL[\\w\\d\\s\\W\\D\\S]*")))
    {
        std::cout << "Failed to send message";
    }
    else
    {
        std::cout << data;
        return 0;
    }

    return 0;
}

int Client::step()
{
    std::string input = this->inputBuffer->read();
    if (input != "")
    {
        if (this->handleInput(input) != 0) return -1;
    }

    std::string data = this->outputBuffer->read();
    if (data != "");
    {
        if (this->handleResponse(data) != 0) return -1;
    }

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
    std::string hostname;
    std::string service;
    char buffer[4096];

    std::cout << "Enter hostname and port in the format <hostname> <port>" << std::endl;
    std::cin >> hostname >> service;

    memset(&hints, 0, sizeof(hints));
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;
    
    if (sock_init() != 0)
    {
        this->startThreads();
        return false;
    }

    if (getaddrinfo(hostname.c_str(), service.c_str(), &hints, &results) != 0) return false;

    this->clientSocket = socket(results->ai_family, results->ai_socktype, results->ai_protocol);

    for (int i = 0; i < 5 && connect(this->clientSocket, results->ai_addr, results->ai_addrlen) != 0; i++)
    {
        std::cout << "retrying connection..." << std::endl;
        if (i == 4)
        {
            std::cout << "Connecting failed, did you enter the hostname and port correctly?" << std::endl;
            this->startThreads();
            return false;
        }
    }
    
    if (recv(this->clientSocket, buffer, 4096, 0) < 0) return false;

    while (!this->login())
    {
        this->clientSocket = socket(results->ai_family, results->ai_socktype, results->ai_protocol);

        while (connect(this->clientSocket, results->ai_addr, results->ai_addrlen) != 0)
        {
            std::cout << "retrying connection..." << std::endl;
        }
        
        if (recv(this->clientSocket, buffer, 4096, 0) < 0) return false;
    }

    std::cout << buffer;

    freeaddrinfo(results);

    this->startThreads();

    return true;
}

bool Client::login() {
    std::string username;
    std::string login_message;
    std::string ok_message;
    char buffer[4096];

    std::cout << "Enter your username: ";
    std::cin >> username;
    
    memset(buffer, 0, 4096);

    login_message = "HELLO-FROM " + username + "\n";
    ok_message = "HELLO " + username + "\n";

    send(this->clientSocket, login_message.c_str(), login_message.length(), 0);
    if (recv(this->clientSocket, buffer, 4096, 0) < 0) return false;

    if (strcmp(buffer, "FAIL-TAKEN\n") == 0)
    {
        std::cout << "Username already taken, try again!" << std::endl;
        return false;
    }

    if (strcmp(buffer, "FAIL-INVALID\n") == 0)
    {
        std::cout << "Username invalid, try again!" << std::endl;
        return false;
    }

    if (strcmp(buffer, ok_message.c_str()) == 0)
    {
        std::cout << "You have been logged in!" << std::endl;
    }

    return true;
}