void	IRCserv::handleInvite(char *msg, Client &client)
{
	char *tmp;
	tmp = strtok(msg, " ");
	if (strcmp("INVITE", tmp))
		return;
	char *nick = strtok(NULL, " ");
	char *channel = strtok(NULL, " ");
	if ((!nick || !strcmp(nick, ":"))&& !channel)
	{
		client.send_message(RPL_INVITELIST(client.nick, this->getHostName(), client.getInvitedChannels()));
		client.send_message(RPL_ENDOFINVITELIST(client.nick, this->getHostName()));
		return;
	}
	if (!nick || !channel)
	{
		client.send_message(ERR_NEEDMOREPARAMS(client.nick, this->getHostName()));
		return;
	}
	Channel *ch = isChannelExisiting(channel);
	if (!ch)
	{
		client.send_message(ERR_NOSUCHCHANNEL(client.nick, this->getHostName(), channel));
		return;
	}
	if (!ch->isClientOnChannel(client))
	{
		client.send_message(ERR_NOTONCHANNEL(this->getHostName(), channel));
		return;
	}
	if (ch->isInviteOnly() && !ch->isFdOperator(client.sock))
	{
		client.send_message(ERR_CHANOPRIVSNEEDED(client.nick, this->getHostName(), channel));
		return;
	}
	if (ch->isNickInChannel(nick))
	{
		client.send_message(ERR_USERONCHANNEL(client.nick, this->getHostName(), nick));
		return;
	}
	Client *invited = NULL;
	for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); it++)
	{
		if (it->second.nick == nick)
		{
			invited = &it->second;
			break;
		}
	}
	if (!invited)
	{
		client.send_message(ERR_NOSUCHNICK(client.nick, this->getHostName(), nick));
		return;
	}
	inviteClinetToChannel(*invited, *ch, client);
}

void IRCserv::inviteClinetToChannel(Client &invitedClient, Channel &channel, Client &client)
{
	// invitedClient.invitedChannels.push_back(channel.getName());
	// check if client is already invited
	std::vector<std::string>::iterator it = std::find(invitedClient.invitedChannels.begin(), invitedClient.invitedChannels.end(), channel.getName());
	if (it == invitedClient.invitedChannels.end())
		invitedClient.invitedChannels.push_back(channel.getName());
	bool isAreadyInvited = false;
	for (size_t i = 0; i < channel.clientsInvited.size(); i++)
	{
		if (channel.clientsInvited[i].nick == invitedClient.nick)
		{
			isAreadyInvited = true;
			break;
		}
	}
	if (!isAreadyInvited)
		channel.clientsInvited.push_back(invitedClient);
	invitedClient.send_message(RPL_INVITING(client.nick, this->getHostName(), invitedClient.nick, channel.getName()));
}