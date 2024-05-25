#pragma once

#include "Server.hpp"


class Client
{
	public:
		std::string		nick;
		int				sock;
		std::string		user;
		std::string		hostname;
		std::string		realname;
		std::string		mode;
		std::string		line;
        std::string     Ipadd;
		std::vector<Channel *> Channels;
		std::vector<std::string> invitedChannels;
		int			registered;
		Client();
		~Client();
		Client(int sock);
	
	    int GetFd();

	    void SetFd(int fd){sock = fd;} //-> setter for fd
	    void setIpAdd(std::string ipadd){Ipadd = ipadd;}

		void send_message(std::string msg);
		void send_messageCH(char *msg);
		// std::string getIpAddress();
		// std::vector<Channel *> getChannels();

		// std::string getFullname();

		// std::string getInvitedChannels();

		// void disconnect();
		// void leaveAllChannels(std::string reason);
		// void eraseInvitedChannel(std::string channelName);
		// void eraseChannel(std::string channelName);

		// bool operator == (const Client &c);
		// bool operator != (const Client &c);


};