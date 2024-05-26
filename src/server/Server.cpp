#include "../../headers/Server.hpp"
#include "../../headers/Client.hpp"
#include "../../headers/channels.hpp"
#include "../../headers/parsedReplies.hpp"

Server::Server()
{
	sockFd = -1;
	this->IrcServhostname = "IRC-localhost";
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

void	Server::Registration(Client &cli, cmd &command)
{
	int initState = cli.registerState;
	if (!(cli.registerState & HAVE_PASS) && command.args[0] == "PASS")
		 handlePass(cli, command);
	else if (!(cli.registerState & HAVE_NICK) && command.args[0] == "NICK")
		handleNick(cli, command);
	else if (command.args[0] == "USER")
		handleUser(cli, command);
	if (initState == cli.registerState)
		cli.send_message(ERR_ALREADYREGISTERED(cli.nick, std::string("hostname")));
	if (cli.registerState == HAVE_REGISTERD)
	{
		cli.send_message(RPL_WELCOME(cli.nick, std::string("hostname")));
		cli.send_message(RPL_YOURHOST(cli.nick, std::string("hostname")));
		cli.send_message(RPL_CREATED(cli.nick, std::string("hostname")));
		cli.send_message(RPL_MYINFO(cli.nick, std::string("hostname")));
	}
}

void	printVectorCmd(std::vector<cmd> commands)
{
	for (size_t i = 0; i < commands.size(); i++)
	{
		std::cout << "buff: " << "|" << commands[i].buff << "|" << std::endl;
		for (size_t j = 0; j < commands[i].args.size(); j++)
			std::cout << "args[" << j << "]: |" << commands[i].args[j] << "|" << std::endl;
	}
}

void Server::ReceiveNewData(int fd)
{
	char buff[1024];
	memset(buff, 0, sizeof(buff));
	ssize_t bytes = recv(fd, buff, sizeof(buff) - 1 , 0);
	if(bytes <= 0)
	{ 
		std::cout << RED << "Client " << fd << " Disconnected" << WHI << std::endl;
		removeClient(fd);
		close(fd);
	}
	else
	{
		buff[bytes] = '\0';
		std::vector<cmd>	commands = this->parseBuffer(buff);
		printVectorCmd(commands);
		Client &cli = findClient(fd);
		// register----?
		for (size_t i = 0; i < commands.size(); i++)
		{
			// PING PONG
			if (commands[i].args.size() > 0 && commands[i].args[0] == "PING")
				cli.send_message(RPL_PONG(cli.nick, commands[i].buff));
			else if (commands[i].args.size() > 0 && commands[i].args[0] == "PONG")
				cli.send_message(RPL_PING(cli.nick, commands[i].buff));
			else if (cli.registerState == HAVE_REGISTERD)
			{
				// commands
				if  (commands[i].args.size() > 0 && commands[i].args[0] == "JOIN")
					handleJOIN(commands[i], cli);
				if  (commands[i].args.size() > 0 && commands[i].args[0] == "USER")
					cli.send_message(ERR_ALREADYREGISTERED(cli.nick, std::string("hostname")));
			}
			else if (commands[i].args.size() > 0 && (commands[i].args[0] == "PASS" || commands[i].args[0] == "NICK" || commands[i].args[0] == "USER"))
				Registration(cli, commands[i]);
			else
				cli.send_message(ERR_NOTREGISTERED(cli.nick, std::string("hostname")));
		}
	}
}

void Server::handleJOIN(cmd &command, Client &cli)
{
    if (command.args.size() <= 0)
        return;
	if (command.args.size() == 1)
		std::cout << "ERR_NEEDMOREPARAMS"<< std::endl;
    std::vector<std::string> channels;
	std::vector<std::string> keys;

    for (size_t i = 1; i < command.args.size(); ++i)
    {

        size_t pos = command.args[i].find(",");
		if (pos != std::string::npos)
		{
			std::cout << RED << "MORE THAN ONE CH" << std::endl;
			channels = split(command.args[i],',');
			for(size_t i = 0; i < channels.size(); i++)
			{
				size_t posH = channels[i].find("#");
				if (posH || posH == std::string::npos)
				{
					std::cout << RED << "ERR_BADCHANNELNAME" << std::endl;
					channels.pop_back();
					return ;
				}
				else if (posH != std::string::npos)
				{
					std::string channelName = channels[i].substr(1, channels[i].size() - 1);
					std::cout << YEL << channelName << std::endl;
				}
			}
		}
		else if (pos == std::string::npos && i == 1)
		{
			size_t posH = command.args[i].find("#");
			if (posH || posH == std::string::npos)
				std::cout << RED << "ERR_BADCHANNELNAME" << std::endl;
			else if (posH != std::string::npos)
			{
				std::string channelName = command.args[i].substr(1, command.args[i].size() - 1);
				std::cout << YEL << channelName << std::endl;
				channels.push_back(channelName);
			}	
		}
		else if (i > 1)
		{
			size_t posK = command.args[i].find('\0');
			std::string _key  = command.args[i].substr(0, posK);
			std::cout << RED << "KEY ::" << _key << std::endl;
			keys.push_back(_key);
		}
    }
	for(int i = 0; i < channels.size(); i++)
	{
		if (keys.empty() == true)
			addNewChannel(channels[i],"xxx",cli);
	}
}





std::vector<cmd> Server::parseBuffer(std::string buff)
{
	std::vector<cmd>	commands;

	// struct	cmd {
	// 	std::vector<std::string> args;
	// 	std::string	buff;
	// };

	std::string::size_type start = 0;
	std::string::size_type end = 0;

	while (end != std::string::npos)
	{
		cmd command;
		end = buff.find("\r\n", start);
		command.buff = buff.substr(start, end - start);
		std::string::size_type start_arg = 0;
		std::string::size_type end_arg = 0;
		while (end_arg != std::string::npos)
		{
			end_arg = command.buff.find(" ", start_arg);
			command.args.push_back(command.buff.substr(start_arg, end_arg - start_arg));
			start_arg = end_arg + 1;
		}
		start = command.buff.find(":", start);
		if (start != std::string::npos)
			command.buff = command.buff.substr(start + 1);
		else
			command.buff = "";

		while (command.buff.length() && ((command.buff[command.buff.length() - 1] == '\r') \
			|| command.buff[command.buff.length() - 1] == '\n'))
			command.buff = command.buff.substr(0, command.buff.length() - 1);

		while (command.args.size() && command.args[command.args.size() - 1].size() &&( command.args[command.args.size() - 1][command.args[command.args.size() - 1].size() - 1] == '\r' \
			|| command.args[command.args.size() - 1][command.args[command.args.size() - 1].size() - 1] == '\n'))
			command.args[command.args.size() - 1] = command.args[command.args.size() - 1].substr(0, command.args[command.args.size() - 1].size() - 1);
		while (command.args.size() && command.args[command.args.size() - 1].size() == 0)
			command.args.pop_back();
		
		if (command.args.size() > 0 && command.args[0] != "")
			commands.push_back(command);
		start = end + 2;
	}

	return commands;
}


void	Server::handleUser(Client &cli, cmd &command)
{
	if (command.args.size() != 5)
	{
		cli.send_message(ERR_NEEDMOREPARAMS(cli.nick, std::string("hostname")));
		return;
	}
	cli.user = command.args[1];
	cli.hostname = command.args[2];
	cli.realname = command.buff;
	cli.registerState |= HAVE_USER;
}

void	Server::handleNick(Client &cli, cmd &command)
{
	if (command.args.size() != 2)
	{
		if (command.args.size() == 1)
			cli.send_message(ERR_NONICKNAMEGIVEN(cli.nick, std::string("hostname")));
		else
			cli.send_message(ERR_NEEDMOREPARAMS(cli.nick, std::string("hostname")));
		return;
	}
	for (size_t i = 0; i < clients.size(); i++)
	{
		if ((command.args[1][0] == ':' && clients[i].nick == command.buff) || clients[i].nick == command.args[1])
		{
			cli.send_message(ERR_NICKNAMEINUSE(cli.nick, std::string("hostname")));
			return;
		}
	}
	for (size_t i = 0; command.args[1][0] != ':' && i < command.args[1].size(); i++)
	{
		if (isspace(command.args[1][i]) || command.args[1][i] == ':' || command.args[1][i] == '#')
		{
			cli.send_message(ERR_ERRONEUSNICKNAME(cli.nick, std::string("hostname")));
			return;
		}
	}
	for (size_t i = 0; command.args[1][0] == ':' && i < command.buff.size(); i++)
	{
		if (isspace(command.buff[i]) || command.buff[i] == ':' || command.buff[i] == '#')
		{
			cli.send_message(ERR_ERRONEUSNICKNAME(cli.nick, std::string("hostname")));
			return;
		}
	}
	if (command.args[1][0] == ':')
		cli.nick = command.buff;
	else
		cli.nick = command.args[1];
	cli.registerState |= HAVE_NICK;
}

void	Server::handlePass(Client &cli, cmd &command)
{
	if (command.args.size() != 2 && command.buff.size() == 0)
	{
		cli.send_message(ERR_NEEDMOREPARAMS(cli.nick, std::string("hostname")));
		return;
	}
	else if (command.args.size() == 2 && command.args[1][0] != ':' && command.args[1] == pass)
	{
		cli.registerState |= HAVE_PASS;
		return;
	}
	else if (command.args.size() >= 2 && command.args[1][0] == ':' && command.buff == pass)
	{
		cli.registerState |= HAVE_PASS;
		return;
	}
	cli.send_message(ERR_PASSWDMISMATCH(cli.nick, std::string("hostname")));
}

Client	&Server::findClient(int fd)
{
	for (unsigned int i = 0; i < clients.size(); i++)
	{
		if (clients[i].sock == fd)
			return clients[i];
	}
	CloseFds();
	throw std::runtime_error("Client not found");// need to double ckeck
}
//lol
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
Channel *Server::isChannelExisiting(std::string name)
{
	std::vector<Channel *>::iterator it;
	for (it = channels.begin(); it < channels.end(); it++)
	{
		Channel *c = *it;
		if (name.compare((*c).getName()) == 0)
			return c;
	}
	return NULL;
}

void Server::addNewChannel(std::string name, std::string pass, Client &client)
{
	std::cout << RED << "DEBUG HERE" << std::endl;
    Channel *channel = isChannelExisiting(name);
    if (!channel)
    {
        channel = new Channel(name, pass,client, this);
        this->channels.push_back(channel);
        client.send_message(RPL_JOIN(client.nick, client.user, name, client.getIpadd()));
        client.send_message(RPL_MODEIS(name, this->getHostName(), channel->getMode()));
        client.send_message(RPL_NAMREPLY(this->getHostName(), channel->getListClients(), channel->getName(), client.nick));
        client.send_message(RPL_ENDOFNAMES(this->getHostName(), client.nick, name));
    }
    else
    {
        std::cout << "Channel existing" << std::endl;
        try
        {
            channel->add(client, pass);
            client.send_message(RPL_JOIN(client.nick, client.user, name, client.getIpadd()));
            client.send_message(RPL_MODEIS(name, this->getHostName(), channel->getMode()));
            client.send_message(RPL_TOPIC(client.nick,this->getHostName(), name, channel->getTopic()));
            // client.send_message(RPL_TOPICTIME(client.nick , IrcServhostname, channel->getName(), channel->getTopicNickSetter(), channel->getTopicTimestamp()));
            client.send_message(RPL_NAMREPLY(this->getHostName(), channel->getListClients(), channel->getName(), client.nick));
            client.send_message(RPL_ENDOFNAMES(this->getHostName(), client.nick, name));
            channel->sendMessageCh(client, RPL_JOIN(client.nick, client.user, channel->getName(), client.getIpadd()));
        }
        catch(std::exception &e)
        {
            e.what();
        }
    }
}

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

Server::~Server()
{

}