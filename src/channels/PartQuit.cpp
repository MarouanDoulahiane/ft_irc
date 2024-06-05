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
	channel->sendMessageCh(RPL_PART(this->getHostName(), cli.nick, cli.user, command.args[1], command.buff));
	cli.send_message(RPL_PART(this->getHostName(), cli.nick, cli.user, command.args[1], command.buff));
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
			this->channels[i]->sendMessageCh(RPL_QUIT(this->getHostName(), cli.nick, cli.user, command.buff));
		}
	}
	std::cout << RED << "Client: " << cli.GetFd() << " Disconnected" << WHI << std::endl;
	removeClient(cli.GetFd());
}