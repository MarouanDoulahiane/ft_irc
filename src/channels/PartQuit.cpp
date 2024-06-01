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
		cli.send_message(ERR_NOSUCHCHANNEL(cli.nick, command.args[1], getHostName()));
		return;
	}
	if (channel->nickInChannel(cli.nick) == false)
	{
		cli.send_message(ERR_NOTONCHANNEL(cli.nick, getHostName()));
		return;
	}
	channel->deleteClient(cli);
	cli.eraseChannel(channel->getName());
	if (channel->isOnOperatorList(cli.GetFd()) == true)
	{
		channel->removeOperator(cli.nick, this->getHostName(), cli);
		if (channel->Operators.size() == 0 && channel->clients.size() > 0)
		{
			channel->setOperator(channel->clients[0]);
			channel->sendMessageCh(RPL_MODEISOP(channel->getName(), this->getHostName(), "+o", channel->clients[0].nick));
			channel->clients[0].send_message(RPL_YOUREOPER(this->getHostName(), channel->clients[0].nick));
		}
	
	}
	channel->sendMessageCh(RPL_PART(this->getHostName(), cli.nick, cli.user, command.args[1], command.buff));
	cli.send_message(RPL_PART(this->getHostName(), cli.nick, cli.user, command.args[1], command.buff));
}
