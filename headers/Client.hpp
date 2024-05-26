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
		// just a flag as state of registration
		int			registerState;
		Client();
		~Client();
		Client(int sock);
	
	    int GetFd();

	    void SetFd(int fd){sock = fd;} //-> setter for fd
	    void setIpAdd(std::string ipadd){Ipadd = ipadd;}

		void send_message(std::string msg);
		void send_messageCH(char *msg);
		std::string getIpadd();
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