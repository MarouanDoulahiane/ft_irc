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