#include "../../headers/Server.hpp"
#include "../../headers/channels.hpp"


void	Server::handleTOPIC(cmd &command, Client &cli)
{
	if (command.args.size() < 2 ||( command.args[1].size() >= 1 && command.args[1][0] != '#'))
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
	if (command.args.size() == 2)
	{
		cli.send_message(RPL_TOPIC(cli.nick, getHostName(), channel->getName(), channel->getTopic()));
		return;
	}
	if (channel->isTopicRestrictionsSet() == true && channel->isOnOperatorList(cli.GetFd()) == false)
	{
		cli.send_message(ERR_CHANOPRIVSNEEDED(cli.nick, getHostName(), channel->getName()));
		return;
	}
	if (command.useBuffer(2))
	{
		channel->setTopic(command.buff);

	}
	else
		channel->setTopic(command.args[2]);
	channel->sendMessageCh(RPL_SETTOPIC(cli.nick, getHostName(), channel->getName(), channel->getTopic()));
}


bool	cmd::useBuffer(int n)
{
	if (args.size() > n && args[n].size() > 0 && args[n][0] == ':')
		return true;
	return false;
}