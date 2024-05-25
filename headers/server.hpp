#pragma once



#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h> 
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <csignal>
#define RED "\e[1;31m"
#define WHI "\e[0;37m"
#define GRE "\e[1;32m"
#define YEL "\e[1;33m"
#include "Client.hpp"

class Client;

class Server
{
    private:
    	int Port;
        std::string pass;
    	int sockFd;
    	static bool Signal; 
    	std::vector<Client> clients;
    	std::vector<struct pollfd> fds; // vector of pollfd
    public:
    	void IrcServerInit(std::string port, std::string password);
    	void SerSocket();
    	void AcceptNewClient();
    	void ReceiveNewData(int fd);
    	static void SignalHandler(int signum);
    	void CloseFds();
    	void removeClient(int fd);
};
