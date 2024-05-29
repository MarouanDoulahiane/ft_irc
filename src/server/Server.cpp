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
	this->IrcServhostname = "IRC-localhost";
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
		std::cout << RED << "Client: " << clients[i].GetFd() << " Disconnected" << WHI << std::endl;
		close(clients[i].GetFd());
	}
	if (sockFd != -1)
	{
		std::cout << RED << "Server: " << sockFd << " Disconnected" << WHI << std::endl;
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
	cli.setHostname(this->getHostName());
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
		cli.send_message(ERR_ALREADYREGISTERED(cli.nick, this->getHostName()));
	if (cli.registerState == HAVE_REGISTERD)
	{
		cli.send_message(RPL_WELCOME(cli.nick, getHostName()));
		cli.send_message(RPL_YOURHOST(cli.nick, getHostName()));
		cli.send_message(RPL_CREATED(cli.nick, getHostName()));
		cli.send_message(RPL_MYINFO(cli.nick, getHostName()));
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
	Client &cli = findClient(fd);
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
		for (size_t i = 0; i < commands.size(); i++)
		{
			// PING PONG
			if (commands[i].args.size() > 0 && commands[i].args[0] == "PING")
				cli.send_message(RPL_PONG(getHostName(), cli.nick));
			else if (commands[i].args.size() > 0 && commands[i].args[0] == "PONG")
				cli.send_message(RPL_PING(getHostName(), cli.nick));
			else if (cli.registerState == HAVE_REGISTERD)
			{
				// commands
				if  (commands[i].args.size() > 0 && commands[i].args[0] == "USER")
					cli.send_message(ERR_ALREADYREGISTERED(cli.nick, getHostName()));
				else if  (commands[i].args.size() >= 1 && commands[i].args[0] == "JOIN")
					handleJOIN(commands[i], cli);
				else if  (commands[i].args.size() >= 1 && commands[i].args[0] == "INVITE")
					handleInvite(commands[i], cli);
				else if  (commands[i].args.size() >= 1 && commands[i].args[0] == "MODE")
					handleMode(commands[i], cli);
				else if ((commands[i].args.size() >= 1 && commands[i].args[0] == "PRIVMSG"))
					handlePRIVMSG(commands[i], cli);
			}
			else if (commands[i].args.size() > 0 && (commands[i].args[0] == "PASS" || commands[i].args[0] == "NICK" || commands[i].args[0] == "USER"))
				Registration(cli, commands[i]);
			else
				cli.send_message(ERR_NOTREGISTERED(cli.nick, getHostName()));
		}
	}
}


void	Server::handleUser(Client &cli, cmd &command)
{
	if (command.args.size() != 5)
	{
		cli.send_message(ERR_NEEDMOREPARAMS(cli.nick, getHostName()));
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
			cli.send_message(ERR_NONICKNAMEGIVEN(cli.nick, getHostName()));
		else
			cli.send_message(ERR_NEEDMOREPARAMS(cli.nick, getHostName()));
		return;
	}
	for (size_t i = 0; i < clients.size(); i++)
	{
		if ((command.args[1][0] == ':' && clients[i].nick == command.buff) || clients[i].nick == command.args[1])
		{
			cli.send_message(ERR_NICKNAMEINUSE(cli.nick, getHostName()));
			return;
		}
	}
	for (size_t i = 0; command.args[1][0] != ':' && i < command.args[1].size(); i++)
	{
		if (isspace(command.args[1][i]) || command.args[1][i] == ':' || command.args[1][i] == '#')
		{
			cli.send_message(ERR_ERRONEUSNICKNAME(cli.nick, getHostName()));
			return;
		}
	}
	for (size_t i = 0; command.args[1][0] == ':' && i < command.buff.size(); i++)
	{
		if (isspace(command.buff[i]) || command.buff[i] == ':' || command.buff[i] == '#')
		{
			cli.send_message(ERR_ERRONEUSNICKNAME(cli.nick, getHostName()));
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
		cli.send_message(ERR_NEEDMOREPARAMS(cli.nick, getHostName()));
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
	cli.send_message(ERR_PASSWDMISMATCH(cli.nick, getHostName()));
}

///////////////////////////////////
//getters
std::string const Server::getHostName()
{
	return IrcServhostname;
}

Server::~Server()
{

}