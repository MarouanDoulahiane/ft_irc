#include "../../headers/Server.hpp"
#include "../../headers/Client.hpp"

Server::Server()
{
	sockFd = -1;
}

bool Server::Signal = false;

void Server::IrcServerInit(std::string port, std::string password)
{
    this->Port = atoi(port.c_str());
    if (Port >= 0 && Port <= 1023)
        throw std::out_of_range("Invalid Port");
    this->pass = password;
    SerSocket();
	std::cout << GRE << "Server " << sockFd << " Connected" << WHI << std::endl;
	std::cout << "Waiting to accept a connection...\n";
	while (Server::Signal == false)
	{
		if((poll(&fds[0],fds.size(),-1) == -1) && Server::Signal == false)
			throw(std::runtime_error("poll() faild"));
		for (size_t i = 0; i < fds.size(); i++)
		{
			if (fds[i].revents & POLLIN)// check if there is data to read
			{
				if (fds[i].fd == sockFd)
					AcceptNewClient();
				else
					ReceiveNewData(fds[i].fd);
			}
		}
	}
	CloseFds();
}

void Server::SerSocket()
{
    struct sockaddr_in add;
	struct pollfd NewPoll;
    int en = 1;

    add.sin_family = AF_INET; 
	add.sin_port = htons(this->Port); 
	add.sin_addr.s_addr = INADDR_ANY; // set the address to any local machine address
    sockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (!sockFd)
        throw std::runtime_error("Failed to creat Socket");
	if(setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == -1)
		throw(std::runtime_error("faild to set option on socket"));
	if (fcntl(sockFd, F_SETFL, O_NONBLOCK) == -1) 
		throw(std::runtime_error("faild to set option (O_NONBLOCK) on socket"));
	if (bind(sockFd, (struct sockaddr *)&add, sizeof(add)) == -1)
		throw(std::runtime_error("faild to bind socket"));
	if (listen(sockFd, SOMAXCONN) == -1)
		throw(std::runtime_error("listen() faild"));
	NewPoll.fd = sockFd;
	NewPoll.events = POLLIN;
	NewPoll.revents = 0;
	fds.push_back(NewPoll); 
    
}
void Server::SignalHandler(int signum)
{
    (void)signum;
    std::cout << std::endl << "Signale recived" << std::endl;
    Server::Signal = true;
}
void Server::CloseFds()
{
	for(size_t i = 0; i < this->clients.size(); i++)
	{ 
		std::cout << RED << "Client <" << clients[i].GetFd() << "> Disconnected" << WHI << std::endl;
		close(clients[i].GetFd());
	}
	if (sockFd != -1)
	{
		std::cout << RED << "Server <" << sockFd << "> Disconnected" << WHI << std::endl;
		close(sockFd);
	}
}

void Server::AcceptNewClient()
{
	Client cli;
	struct sockaddr_in cliadd;
	struct pollfd NewPoll;
	socklen_t len = sizeof(cliadd);
	int incofd = accept(sockFd, (sockaddr *)&(cliadd), &len);
	if (incofd == -1)
	{
		std::cout << "accept() failed" << std::endl;
		return;
	}
	if (fcntl(incofd, F_SETFL, O_NONBLOCK) == -1) // set the socket option (O_NONBLOCK) for non-blocking socket
	{
		std::cout << "fcntl() failed" << std::endl;
		return;
	}
	NewPoll.fd = incofd; // add the client socket to the pollfd
	NewPoll.events = POLLIN; // set the event to POLLIN for reading data
	NewPoll.revents = 0; // set the revents to 0
	cli.SetFd(incofd);
	cli.setIpAdd(inet_ntoa((cliadd.sin_addr)));
	clients.push_back(cli);
	fds.push_back(NewPoll); 
	std::cout << GRE << "Client <" << incofd << "> Connected" << WHI << std::endl;
}


void Server::ReceiveNewData(int fd)
{
	char buff[1024];
	memset(buff, 0, sizeof(buff));
	ssize_t bytes = recv(fd, buff, sizeof(buff) - 1 , 0);
	if(bytes <= 0)
	{ 
		std::cout << RED << "Client [[" << fd << "]] Disconnected" << WHI << std::endl;
		removeClient(fd);
		close(fd);
	}
	else
	{
		buff[bytes] = '\0';
		std::cout << YEL << "Client [[" << fd << "]] Data: " << WHI << buff;
	}
}

void Server::removeClient(int fd)
{
	for(size_t i = 0; i < fds.size(); i++)
	{
		if (fds[i].fd == fd)
			{
				fds.erase(fds.begin() + i); 
				break;
			}
	}
	for(size_t i = 0; i < clients.size(); i++)
	{
		if (clients[i].GetFd() == fd)
			{clients.erase(clients.begin() + i); 
			break;}
	}

}


////////////////////////////////////////////////////////
//channel function :

// void Server::addNewChannel(std::string name,char *pass, Client &client)
// {
//     Channel *channel = isChannelExisiting(name);
//     if (!channel)
//     {
//         channel = new Channel(name, pass,client, this);
//         this->channels.push_back(channel);
//         client.send_message(RPL_JOIN(client.nick, client.user, name, client.getIpAddress()));
//         client.send_message(RPL_MODEIS(name, this->getHostName(), channel->getMode()));
//         client.send_message(RPL_NAMREPLY(this->getHostName(), channel->getListClients(), channel->getName(), client.nick));
//         client.send_message(RPL_ENDOFNAMES(this->getHostName(), client.nick, name));
        
//     }
//     else
//     {
//         std::cout << "Channel existing" << std::endl;
//         try
//         {
//             channel->addClient(client, pass);
//             client.send_message(RPL_JOIN(client.nick, client.user, name, client.getIpAddress()));
//             client.send_message(RPL_MODEIS(name, this->getHostName(), channel->getMode()));
//             client.send_message(RPL_TOPIC(client.nick,this->getHostName(), name, channel->getTopic()));
//             client.send_message(RPL_TOPICTIME(client.nick , this->hostname, channel->getName(), channel->getTopicNickSetter(), channel->getTopicTimestamp()));
//             client.send_message(RPL_NAMREPLY(this->getHostName(), channel->getListClients(), channel->getName(), client.nick));
//             client.send_message(RPL_ENDOFNAMES(this->getHostName(), client.nick, name));
//             channel->send_message(client, RPL_JOIN(client.nick, client.user, channel->getName(), client.getIpAddress()));
//         }
//         catch(const ClientErrMsgException &e)
//         {
//             e._client.send_message(e.getMessage());
//         }
        
//     }
// }

// void IRCserv::removeChannel(std::string name)
// {
//     Channel *channel = isChannelExisiting(name);
//     if (!channel)
//         return;
//     std::vector<Channel *>::iterator it;
//     for (it = this->channels.begin(); it < this->channels.end(); it++)
//     {
//         if ((*it)->getName() == name)
//         {
//             if ((*it)->getClients().size() > 0)
//             {
//                 std::vector<Client>::iterator itc;
//                 for (itc = (*it)->getClients().begin(); itc < (*it)->getClients().end(); itc++)
//                 {
//                     (*it)->partClient(*itc, "");
//                 }
//             }
//             this->channels.erase(it);
//             delete channel;
//             return;
//         }
//     }
// }

// void IRCserv::partChannel(std::string name,char *_reason, Client &client)
// {
//     Channel *channel = isChannelExisiting(name);
//     if (!channel)
//         client.send_message(ERR_NOSUCHCHANNEL(hostname, name, client.nick));
//     if (!channel->is_member(client))
//         client.send_message(ERR_NOTONCHANNEL(hostname, name));
//     std::string reason("");
//     if (_reason)
//         reason = _reason;
//     channel->partClient(client, reason);
// }

///////////////////////////////////
//getters
std::string const Server::getHostName()
{
	return IrcServhostname;
}

Channel *Server::getChannelByName(std::string name)
{
    Channel *token;
	std::vector<Channel *>::iterator it;
	for (it = this->channels.begin(); it < this->channels.end(); it++)
	{
        token = *it;
		if (name.compare((*token).getName()) == 0)
			return token;
	}
	return NULL;
}