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
#include "parsedReplies.hpp"

class Client;
class Channel;

struct	cmd 
{
	std::vector<std::string> args;
	std::string	buff;
};

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
		~Server();
    	static void SignalHandler(int signum);
    	
		
		void IrcServerInit(std::string port, std::string password);
    	void SerSocket();

    	void AcceptNewClient();
    	void ReceiveNewData(int fd);
		void Registration(Client &cli, cmd &command);
    	void removeClient(int fd);

		Channel *getChannelByName(std::string name);
		Client &findClient(int fd);
		std::vector<cmd> parseBuffer(std::string buff);
		void handlePass(Client &cli, cmd &command);
		void handleNick(Client &cli, cmd &command);
		void handleUser(Client &cli, cmd &command);
		void handleJOIN(cmd &command, Client &cli);
		//getters
		std::string const getHostName();
    	void CloseFds();
};


std::vector<std::string> split(const std::string &s, char delimiter);