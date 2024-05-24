#include "../../headers/Server.hpp"
#include "../../headers/Client.hpp"

Client::Client()
{
	this->sock = -1;
	this->registerState = 0;
}

Client::Client(int sock)
{
	this->sock = sock;
	this->registerState = 0;
}

void Client::send_messageCH(char *msg)
{
	write(this->sock, msg, strlen(msg));
}
void Client::send_message(std::string msg)
{
	write(this->sock, msg.c_str(), msg.length());
}

Client::~Client()
{
    
}