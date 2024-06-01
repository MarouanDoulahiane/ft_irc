#pragma once

#include "Server.hpp"
#include "header.hpp"
#include "parsedReplies.hpp"


#define HAVE_PASS 0b1
#define HAVE_NICK 0b10
#define HAVE_USER 0b100
#define HAVE_REGISTERD 0b111

class Channel;

class Client
{
		int				sock;
        std::string     Ipadd;
		std::vector<Channel *> Channels;
	public:
		Client();
		~Client();
		Client(int sock);
		std::string		nick;
		std::string		hostname;
		std::string		user;
		std::string		line;
		std::string		realname;
		std::vector<std::string> invitedChannels;
		int			registerState;		//flag as state of registration
	

	    void SetFd(int fd);
	    void setIpAdd(std::string ipadd);
		void setHostname(std::string hostname);

		void send_message(std::string msg);
		void send_messageCH(char *msg);

	    int GetFd();
		std::string getFullname();
		std::string getIpadd();
		std::string getHostname();
		std::string getInvitedChannels();
		std::vector<Channel *> getChannels();

		// void disconnect();
		void leaveAllChannels(std::string reason);
		void eraseInvitedChannel(std::string channelName);
		void eraseChannel(std::string channelName);

		// bool operator == (const Client &c);
		// bool operator != (const Client &c);

};
	class ClientErrMsgException : public std::exception
	{
	    private:
	        const std::string _cmessage;
	        ClientErrMsgException() throw();
	    public:
	        ClientErrMsgException(std::string msg, Client &bc) throw();
	        virtual ~ClientErrMsgException() throw();
	        Client &_client;
			const std::string getMessage() const;
	        virtual const char* what() const throw()
			{
	            return (_cmessage.c_str());
	        };
	};