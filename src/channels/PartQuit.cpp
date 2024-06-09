#include "../../headers/Server.hpp"

void	Server::handlePART(cmd &command, Client &cli)
{
	if (command.args.size() < 2)
	{
		cli.send_message(ERR_NEEDMOREPARAMS(cli.nick, getHostName()));
		return;
	}
	Channel *channel = getChannelByName(command.args[1]);
	if (!channel)
	{
		cli.send_message(ERR_NOSUCHCHANNEL(getHostName(), command.args[1], cli.nick));
		return;
	}
	if (channel->nickInChannel(cli.nick) == false)
	{
		cli.send_message(ERR_NOTONCHANNEL(getHostName(), command.args[1]));
		return;
	}
	channel->deleteClient(cli);
	if (channel->clients.size())
		channel->sendMessageCh(RPL_PART(this->getHostName(), cli.nick, cli.user, command.args[1], command.buff));
	else
		removeChannel(channel->getName());
	cli.send_message(RPL_PART(this->getHostName(), cli.nick, cli.user, command.args[1], command.buff));
}


void Server::removeChannel(std::string name)
{
	for (size_t i = 0; i < channels.size(); i++)
	{
		if (channels[i]->getName() == name)
		{
			delete channels[i];
			channels.erase(channels.begin() + i);
			return;
		}
	}
}


void	Server::handleQUIT(cmd &command, Client &cli)
{
	if (command.args.size() < 2)
	{
		cli.send_message(ERR_NEEDMOREPARAMS(cli.nick, getHostName()));
		return;
	}
	cli.send_message(RPL_QUIT(this->getHostName(), cli.nick, cli.user, command.buff));
	for (unsigned int i = 0; i < this->channels.size(); i++)
	{
		if (this->channels[i]->nickInChannel(cli.nick))
		{
			this->channels[i]->deleteClient(cli);
			if (this->channels[i]->clients.size())
				this->channels[i]->sendMessageCh(RPL_QUIT(this->getHostName(), cli.nick, cli.user, command.buff));
			else
				removeChannel(this->channels[i]->getName());
		}
	}
	std::cout << RED << "Client is Disconnected" << WHI << std::endl;
	removeClient(cli.GetFd());
}