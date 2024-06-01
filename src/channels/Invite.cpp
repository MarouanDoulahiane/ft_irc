#include "../../headers/Server.hpp"

class Server;

void	Server::handleInvite(cmd &command, Client &cli)
{
	if (command.args.size() < 3)
	{
		cli.send_message(ERR_NEEDMOREPARAMS(cli.nick, this->getHostName()));
		return;
	}
	std::string  nick = command.args[1];
	std::string channel = command.args[2];
	if ((nick.empty() || !command.buff.empty()) && channel.empty())
	{
		cli.send_message(RPL_INVITELIST(cli.nick, this->getHostName(), cli.getInvitedChannels()));
		cli.send_message(RPL_ENDOFINVITELIST(cli.nick, this->getHostName()));
		return;
	}
	std::cout << RED << "DEBUG INVITE " << channel << std::endl;
	Channel *ch = isChannelExisiting(channel);
	if (!ch)
	{
		cli.send_message(ERR_NOSUCHCHANNEL(cli.nick, this->getHostName(), channel));
		return;
	}
	if (!ch->checkClient(cli))
	{
		cli.send_message(ERR_NOTONCHANNEL(this->getHostName(), channel));
		return;
	}
	if (ch->isInviteOnly() && !ch->isOnOperatorList(cli.GetFd()))
	{
		cli.send_message(ERR_CHANOPRIVSNEEDED(cli.nick, this->getHostName(), channel));
		return;
	}
	if (ch->nickInChannel(nick))
	{
		cli.send_message(ERR_USERONCHANNEL(cli.nick, this->getHostName(), nick));
		return;
	}
	bool invited = false;
	for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); it++)
	{
		if (it->nick == nick)
		{
			invited = true;
			inviteClinetToChannel(*it, *ch, cli);// need to double check
			break;
		}
	}
	if (!invited)
	{
		cli.send_message(ERR_NOSUCHNICK(cli.nick, this->getHostName(), nick));
		return;
	}
}

void Server::inviteClinetToChannel(Client &invitedClient, Channel &channel, Client &client)
{
	// invitedClient.invitedChannels.push_back(channel.getName());
	// check if client is already invited
	std::vector<std::string>::iterator it = std::find(invitedClient.invitedChannels.begin(), invitedClient.invitedChannels.end(), channel.getName());
	if (it == invitedClient.invitedChannels.end())
		invitedClient.invitedChannels.push_back(channel.getName());
	bool isAreadyInvited = false;
	for (size_t i = 0; i < channel.InvitedClients.size(); i++)
	{
		if (channel.InvitedClients[i].nick == invitedClient.nick)
		{
			isAreadyInvited = true;
			break;
		}
	}
	if (!isAreadyInvited)
		channel.InvitedClients.push_back(invitedClient);
	invitedClient.send_message(RPL_INVITING(client.nick, this->getHostName(), invitedClient.nick, channel.getName()));
}