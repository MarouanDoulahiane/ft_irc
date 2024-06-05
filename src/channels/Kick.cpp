#include "../../headers/Server.hpp"


void Server::handleKICK(cmd &command, Client &cli)
{
	if (command.args.size() < 3)
	{
		cli.send_message(ERR_NEEDMOREPARAMS(cli.nick, getHostName()));
		return;
	}
	Channel *chan = getChannelByName(command.args[1]);
	if (chan == NULL)
	{
		cli.send_message(ERR_NOSUCHCHANNEL(getHostName(), command.args[1], cli.nick));
		return;
	}
	if (chan->nickInChannel(cli.nick) == false)
	{
		cli.send_message(ERR_NOTONCHANNEL(getHostName(), command.args[1]));
		return;
	}
	if (chan->isOnOperatorList(cli.GetFd()) == false)
	{
		cli.send_message(ERR_CHANOPRIVSNEEDED(cli.nick, getHostName(), command.args[1]));
		return;
	}
	std::vector<std::string> targets = split(command.args[2], ',');
	for (size_t i = 0; i < targets.size(); i++)
	{
		Client *client = isClientBef(targets[i]);
		if (client == NULL)
		{
			cli.send_message(ERR_NOSUCHNICK(this->getHostName(), chan->getName(), targets[i]));
			continue;
		}
		if (chan->nickInChannel(client->nick) == false)
		{
			cli.send_message(ERR_USERNOTINCHANNEL(cli.nick, chan->getName()));
			continue;
		}
		chan->sendMessageCh(RPL_KICK(cli.nick, cli.user, getHostName(), command.args[1], targets[i], command.buff));
		chan->deleteClient(*client);
	}
}
