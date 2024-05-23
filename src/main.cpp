#include "../headers/Server.hpp"
#include <iostream>

int main(int ac, char **av) {
	if (ac != 3)
	{
		std::cerr << "Usage: ./ircserv [port] [password]" << std::endl;
		return 1;
	}
		Server irc;
	try
	{
	    std::cout << "---- SERVER ----" << std::endl;
        signal(SIGINT, Server::SignalHandler);
        signal(SIGQUIT, Server::SignalHandler);
        irc.IrcServerInit(av[1], av[2]);
	}
	catch(const std::exception& e){
		irc.CloseFds(); //-> close the file descriptors
		std::cerr << e.what() << std::endl;
	}
	std::cout << "The Server Closed!" << std::endl;
	return 0;
}