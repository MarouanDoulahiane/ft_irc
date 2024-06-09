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
	bool useBuffer(size_t n);
	bool isValidNick();
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
    	std::vector<Client> clients;
    	std::vector<struct pollfd> fds;
		std::vector<std::string> jokes;
		
		
    	static bool Signal;
		//channels--
    public:
    	Server();
		~Server();
    	static void SignalHandler(int signum);
		std::vector<Channel*> channels;
		
		void IrcServerInit(std::string port, std::string password);
    	void SerSocket();

    	void AcceptNewClient();
    	void ReceiveNewData(int fd);
		void Registration(Client &cli, cmd &command);
    	void removeClient(int fd);

		Channel *getChannelByName(std::string name);
		Client &findClient(int fd);
		Client *isClientBef(std::string name);
		std::vector<cmd> parseBuffer(std::string buff);

//      commands
		void handlePass(Client &cli, cmd &command);
		void handleNick(Client &cli, cmd &command);
		void handleUser(Client &cli, cmd &command);
		void handleJOIN(cmd &command, Client &cli);
		void handleInvite(cmd &command, Client &cli);
		void handleMode(cmd &command, Client &cli);
		void handlePRIVMSG(cmd &command, Client &cli);
		void handleTOPIC(cmd &command, Client &cli);
		void handleKICK(cmd &command, Client &cli);
		void handlePART(cmd &command, Client &cli);
		void handleQUIT(cmd &command, Client &cli);
		void handleBOT(Client &cli);
		// channel functions
		Channel *isChannelExisiting(std::string name);
		void  parseFlags(cmd &command, Client &cli, std::string &flagsHolder, std::string &addParams);
		void sendMSG(std::string &target, std::string &text, Client &cli);
		void inviteClinetToChannel(Client &invitedClient, Channel &channel, Client &client);
		void addNewChannel(std::string name,std::string pass, Client &client);
		void applyModeFlags(std::string channelName, std::string modeFlags, std::string addParams, Client &client);
		void storeMode(Channel *channel, char mode, bool setFlag);
		
		//getters
		std::string const getHostName();
    	void CloseFds();
		void removeChannel(std::string name);
};

std::vector<std::string> split(const std::string &s, char delimiter);

