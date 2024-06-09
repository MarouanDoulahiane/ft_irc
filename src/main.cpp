#include "../headers/Server.hpp"
#include <iostream>


int main(int ac, char **av) 
{
	if (ac != 3)
	{
		std::cerr << "Usage: ./ircserv [port] [password]" << std::endl;
		return 1;
	}
	Server irc;
	try
	{
	    std::cout << GRE << ">>>> PIPE-SERVER IS LOADING <<<<" << WHI << std::endl;
        signal(SIGINT, Server::SignalHandler);
        signal(SIGQUIT, Server::SignalHandler);
        irc.IrcServerInit(av[1], av[2]);
	}
	catch(const std::exception& e)
	{
		irc.CloseFds();
		std::cerr << e.what() << std::endl;
	}
	std::cout<< RED << "The Server Closed!" << WHI << std::endl;
	return 0;
}