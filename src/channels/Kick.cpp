void IRCserv::handleKick(char *msg, Client &client)
{
	char *part = strtok(msg, " ");
	if (strcmp("KICK", part))
		return;
	char *channel = strtok(NULL, " ");
	char *nick = strtok(NULL, " ");
	char *reason = strtok(NULL, "");
	if (!channel || !nick)
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
	if (!ch->isNickInChannel(nick))
	{
		client.send_message(ERR_USERNOTINCHANNEL(this->getHostName(), channel));
		return;
	}
	if (!ch->isFdOperator(client.sock))
	{
		client.send_message(ERR_NOTOP(this->getHostName(), channel));
		return;
	}
	Client *kicked = NULL;
	for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); it++)
	{
		if (it->second.nick == nick)
		{
			kicked = &it->second;
			break;
		}
	}
	if (!kicked)
	{
		client.send_message(ERR_NOSUCHNICK(this->getHostName(), channel, nick));
		return;
	}
	std::string reasonStr = (!reason? "" : std::string(reason)); // the error was here, because it get aborded if reason is `null`
	if (reasonStr.size() > 0 && reasonStr[0] == ':')
		reasonStr = reasonStr.substr(1);
	ch->send_message(RPL_KICK(client.nick, client.user, this->getHostName(), channel, kicked->nick, reasonStr));
	kicked->eraseChannel(ch->getName());
	ch->deleteClient(*kicked);
}