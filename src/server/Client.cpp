#include "../../headers/Server.hpp"
#include "../../headers/Client.hpp"


class Client;
Client::Client()
{
	this->sock = -1;
	this->registerState = false;
	std::string nickname = "";
	std::string username = "";
}

Client::Client(int sock)
{
	this->sock = sock;
	this->registerState = false;
}

void Client::eraseChannel(std::string channel)
{
	std::vector<Channel *>::iterator it;
	for (it = this->Channels.begin(); it != this->Channels.end(); it++)
	{
		if ((*it)->getName() == channel)
		{
			this->Channels.erase(it);
			return;
		}
	}

}


// Client::Client(int sock, std::string nick, std::string user, std::string hostname);
void Client::send_messageCH(char *msg)
{
	write(this->sock, msg, strlen(msg));
}
void Client::send_message(std::string msg)
{
	write(this->sock, msg.c_str(), msg.length());
}

int Client::GetFd()
{
	return sock;
}
std::string Client::getIpadd()
{
	return Ipadd;
}

std::string Client::getHostname()
{
	return hostname;
}

void Client::leaveAllChannels(std::string reason)
{
	std::vector<Channel *>::iterator it;
	for (it = this->Channels.begin(); it != this->Channels.end(); it++)
	{
		// (*it)->partClient(*this, reason);
	}
}

Client::~Client()
{
    
}

std::string Client::getInvitedChannels()
{
	std::string ret = "\0";
	for(int i = 0; i < this->invitedChannels.size(); i++)
			ret = invitedChannels[i] + " ";
	return ret;
}

std::vector<Channel *> Client::getChannels()
{
	return this->Channels;
}

std::string Client::getFullname()
{
	return this->nick + "!" + this->user + "@" + this->hostname;
}


void Client::SetFd(int fd)
{
	sock = fd;
}

void Client::setHostname(std::string hostname)
{
	this->hostname = hostname;
}

void Client::setIpAdd(std::string ipadd)
{
	Ipadd = ipadd;
}

///////// need change after

ClientErrMsgException::ClientErrMsgException(std::string msg, Client &client) throw() : _cmessage(msg), _client(client)
{
}

ClientErrMsgException::~ClientErrMsgException() throw()
{

}

const std::string ClientErrMsgException::getMessage() const
{
	return this->_cmessage;
}