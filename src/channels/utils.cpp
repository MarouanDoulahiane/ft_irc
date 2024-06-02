#include "../../headers/channels.hpp"
#include "../../headers/Server.hpp"
#include <string>
#include <cctype>

std::vector<std::string> split(const std::string &s, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(s);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

void       trim(std::string& str) {
    size_t begin = 0;
    size_t end = str.size() -1;
    while (begin < end)
    {
        if (isspace(str[begin]))
            begin++;
        else if (isspace(str[end]))
            end--;
        else
            break;
    }
    if (begin != end)
        str = str.substr(begin, end - begin + 1);
    else
        str = "";
}

std::vector<cmd> Server::parseBuffer(std::string buff)
{
	std::vector<cmd>	commands;
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
Client *Server::isClientBef(std::string name)
{
	for(int i =0; i < clients.size(); i++)
	{
		if (clients[i].nick == name)
			return &clients[i];
	}
	return NULL;
}

Client	&Server::findClient(int fd)
{
	for (unsigned int i = 0; i < clients.size(); i++)
	{
		if (clients[i].GetFd() == fd)
			return clients[i];
	}
	CloseFds();
	throw std::runtime_error("Client not found");// need to double ckeck
}
//lol
void Server::removeClient(int fd)
{
	// remove client from all channels he is in
	Client &cli = findClient(fd);
	for (unsigned int i = 0; i < this->channels.size(); i++)
	{
		if (channels[i]->nickInChannel(cli.nick))
			channels[i]->deleteClient(cli);
	}
	// remove client from clients list
	for (unsigned int i = 0; i < clients.size(); i++)
	{
		if (clients[i].GetFd() == fd)
		{
			clients.erase(clients.begin() + i);
			break;
		}
	}
	// remove client from fds list
	for (unsigned int i = 0; i < fds.size(); i++)
	{
		if (fds[i].fd == fd)
		{
			fds.erase(fds.begin() + i);
			break;
		}
	}
	close(fd);
}
