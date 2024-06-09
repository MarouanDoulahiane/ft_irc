#include "../../headers/Server.hpp"
#include "../../headers/Client.hpp"
#include "../../headers/channels.hpp"
#include "../../headers/parsedReplies.hpp"

Server::Server()
{
	sockFd = -1;
	this->IrcServhostname = "IRC-localhost";
	this->jokes.push_back("Why did the computer go to the doctor? Because it had a virus!");
	this->jokes.push_back("Why was the math book sad? Because it had too many problems.");
	this->jokes.push_back("Why did the tomato turn red? Because it saw the salad dressing!");
	this->jokes.push_back("Why did the scarecrow win an award? Because he was outstanding in his field!");
	this->jokes.push_back("Why did the golfer bring two pairs of pants? In case he got a hole in one!");
}

bool cmd::isValidNick()
{
	if (args.size() < 2)
		return false;
	std::string nick = useBuffer(1) ? buff : args[1];

	if (nick.size() < 1 || nick.size() > 9)
		return false;
	for (size_t i = 0; i < nick.size(); i++)
	{
		if (i == 0 && !std::isalpha(nick[i]))
			return false;
		if (!std::isalnum(nick[i]) && nick[i] != '_')
			return false;
	}
	if (nick == "USER" || nick == "NICK" || nick == "QUIT" || nick == "JOIN" || nick == "PART" || nick == "TOPIC" || nick == "MODE" || nick == "PRIVMSG" || nick == "KICK" || nick == "INVITE" || nick == "PASS" || nick == "BOT")
		return false;
	return true;
}

bool Server::Signal = false;

void Server::IrcServerInit(std::string port, std::string password)
{
    this->Port = atoi(port.c_str());
    if (Port >= 0 && Port <= 1023)
        throw std::out_of_range("Invalid Port");
    this->pass = password;
    SerSocket();
	std::cout << GRE << "PIPE-SERVER has been Connected" << WHI << std::endl;
	std::cout << YEL << "Waiting to accept a connection.." << WHI << std::endl;
	while (Server::Signal == false)
	{
		if((poll(&fds[0],fds.size(),-1) == -1) && Server::Signal == false)
			throw(std::runtime_error("poll() faild"));
		for (size_t i = 0; i < fds.size(); i++)
		{
			if (fds[i].revents & POLLIN)
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
	add.sin_addr.s_addr = INADDR_ANY;
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
    std::cout << std::endl << "SIGNAL CAUGHT !" << std::endl;
    Server::Signal = true;
}
void Server::CloseFds()
{
	for(size_t i = 0; i < this->clients.size(); i++)
	{ 
		std::cout << RED << "Client is Disconnected" << WHI << std::endl;
		close(clients[i].GetFd());
	}
	if (sockFd != -1)
	{
		std::cout << RED << "PIPE-SERVER has been Disconnected" << WHI << std::endl;
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
	if (fcntl(incofd, F_SETFL, O_NONBLOCK) == -1)
	{
		std::cout << "fcntl() failed" << std::endl;
		return;
	}
	NewPoll.fd = incofd;
	NewPoll.events = POLLIN;
	NewPoll.revents = 0;
	cli.SetFd(incofd);
	cli.setIpAdd(inet_ntoa((cliadd.sin_addr)));
	cli.setHostname(this->getHostName());
	clients.push_back(cli);
	fds.push_back(NewPoll); 
	std::cout << GRE << "Client is Connected" << WHI << std::endl;
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

void Server::handleBOT(Client &cli)
{
	int random = rand() % jokes.size();

	cli.send_message(PRIVMSG_FORMATUSER(std::string("BOT"), "BOT", this->getHostName(), cli.nick, jokes[random]));
}

void Server::ReceiveNewData(int fd)
{
	char buff[1024];
	memset(buff, 0, sizeof(buff));
	Client &cli = findClient(fd);
	ssize_t bytes = recv(fd, buff, sizeof(buff) - 1 , 0);
	if(bytes <= 0 || fd == -1)
	{ 
		std::cout << RED << "Client is Disconnected" << WHI << std::endl;
		removeClient(fd);
	}
	else
	{
		buff[bytes] = '\0';
		std::vector<cmd>	commands = this->parseBuffer(buff);
		// printVectorCmd(commands);
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
				if (commands[i].args.size() > 0 && commands[i].args[0] == "NICK")
					handleNick(cli, commands[i]);
				else if  (commands[i].args.size() >= 1 && commands[i].args[0] == "JOIN")
					handleJOIN(commands[i], cli);
				else if  (commands[i].args.size() >= 1 && commands[i].args[0] == "INVITE")
					handleInvite(commands[i], cli);
				else if  (commands[i].args.size() >= 1 && commands[i].args[0] == "MODE")
					handleMode(commands[i], cli);
				else if ((commands[i].args.size() >= 1 && commands[i].args[0] == "PRIVMSG"))
					handlePRIVMSG(commands[i], cli);
				else if ((commands[i].args.size() >= 1 && commands[i].args[0] == "TOPIC"))
					handleTOPIC(commands[i], cli);
				else if ((commands[i].args.size() >= 1 && commands[i].args[0] == "KICK"))
					handleKICK(commands[i], cli);
				else if ((commands[i].args.size() >= 1 && commands[i].args[0] == "PART"))
					handlePART(commands[i], cli);
				else if ((commands[i].args.size() >= 1 && commands[i].args[0] == "QUIT"))
					handleQUIT(commands[i], cli);
				else if ((commands[i].args.size() >= 1 && commands[i].args[0] == "BOT"))
					handleBOT(cli);		
				else 
					cli.send_message(ERR_UNKNOWNCOMMAND(cli.nick, getHostName(), commands[i].args[0]));
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

std::string returnNickORGhost(std::string nick)
{
	if (nick.empty())
		return "GHOST";
	return nick;
}

void	Server::handleNick(Client &cli, cmd &command)
{
	if (command.args.size() != 2)
	{
		if (command.args.size() == 1)
			cli.send_message(ERR_NONICKNAMEGIVEN(returnNickORGhost(cli.nick), getHostName()));
		else
			cli.send_message(ERR_ERRONEUSNICKNAME(returnNickORGhost(cli.nick), getHostName()));
		return;
	}
	if (!command.isValidNick())
	{
		cli.send_message(ERR_ERRONEUSNICKNAME(returnNickORGhost(cli.nick), getHostName()));
		return;
	}
	std::string newNick = command.useBuffer(1) ? command.buff : command.args[1];
	if (isClientBef(newNick) != NULL)
	{
		cli.send_message(ERR_NICKNAMEINUSE(returnNickORGhost(cli.nick), getHostName()));
		return;
	}
	if (cli.registerState == HAVE_REGISTERD)
	{
		for (size_t i = 0; i < channels.size(); i++)
		{
			channels[i]->updateNick(cli.nick, newNick);
		}
		cli.send_to_all_channels(RPL_NICK(cli.nick ,cli.user, newNick, cli.hostname));
		cli.send_message(RPL_NICK(cli.nick, cli.user, newNick, cli.hostname));
	}
	cli.nick = newNick;
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

std::string const Server::getHostName()
{
	return IrcServhostname;
}

Server::~Server()
{
	for (size_t i = 0; i < channels.size(); i++)
		delete channels[i];
}