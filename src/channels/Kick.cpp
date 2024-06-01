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
		cli.send_message(ERR_NOSUCHCHANNEL(cli.nick, getHostName(), command.args[1]));
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
	Client *client = isClientBef(command.args[2]);
	if (client == NULL)
	{
		cli.send_message(ERR_NOSUCHNICK(cli.nick, getHostName(), command.args[2]));
		return;
	}
	if (chan->nickInChannel(client->nick) == false)
	{
		cli.send_message(ERR_USERNOTINCHANNEL(cli.nick, command.args[2]));
		return;
	}
	chan->deleteClient(*client);
	chan->sendMessageCh(RPL_KICK(cli.nick, cli.user, getHostName(), command.args[1], command.args[2], command.buff));
}
