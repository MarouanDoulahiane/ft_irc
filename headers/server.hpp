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
#include "channels.hpp"

class Client;
class Channel;


#define SERVER_PREFIX "IRC"
#define SERVER_SUFFIX "localhost"
#define SERVER_HOSTNAME SERVER_PREFIX + SERVER_SUFFIX
class Server
{
        std::string pass;
    	int Port;
    	int sockFd;
		std::string IrcServhostname;
    	static bool Signal; 
    	std::vector<Client> clients;
    	std::vector<struct pollfd> fds;
		
		
		//channels--
		std::vector<Channel*> channels;
    public:
    	Server();
    	static void SignalHandler(int signum);
    	
		
		void IrcServerInit(std::string port, std::string password);
    	void SerSocket();

    	void AcceptNewClient();
    	void ReceiveNewData(int fd);
    	void removeClient(int fd);

		Channel *getChannelByName(std::string name);

		//getters
		std::string const getHostName();
    	void CloseFds();
};
