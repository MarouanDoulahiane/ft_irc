void IRCserv::handleTopic(char *msg, Client &client)
{
	char *part = strtok(msg, " ");
	bool clear = false;
	if (strcmp("TOPIC", part))
		return;
	char *channel = strtok(NULL, " ");
	char *topic = strtok(NULL, "");
	if (topic && topic[0] == ':')
	{
		clear = true;
		topic++;
	}
	else if (topic && topic[0] != ':')
		topic = strtok(topic, " ");
	if (!channel)
	{
		client.send_message(ERR_NEEDMOREPARAMS(client.nick, this->getHostName()));
		return;
	}
	Channel *ch = isChannelExisiting(channel);
	if (!ch)
	{
		client.send_message(ERR_NOSUCHCHANNEL(this->hostname, client.nick, channel));
		return;
	}
	if (!ch->isClientOnChannel(client))
	{
		client.send_message(ERR_NOTONCHANNEL(this->getHostName(), channel));
		return;
	}
	if (!topic && clear)
	{
		if (ch->isTopicSet && ch->isFdOperator(client.sock))
		{
			ch->setTopic("");
			client.send_message(RPL_TOPIC(this->hostname, client.nick, channel, ch->getTopic()));
		}
		else if (!ch->isTopicSet)
		{
			ch->setTopic("");
			client.send_message(RPL_TOPIC(this->getHostName(), client.nick, channel, ch->getTopic()));
		}
		else
			client.send_message(ERR_CHANOPRIVSNEEDED(client.nick, this->getHostName(), channel));
	}
	else if (!topic && !clear)
	{
		if (ch->getTopic().size() > 0)
			client.send_message(RPL_TOPIC(client.nick, this->hostname, channel, ch->getTopic()));
		else
			client.send_message(RPL_NOTOPIC(client.nick, this->hostname, channel));
	}
	else
	{
		if (ch->isTopicSet && ch->isFdOperator(client.sock))
		{
			ch->setTopic(topic);
			client.send_message(RPL_TOPIC(client.nick, this->hostname, channel ,ch->getTopic()));
		}
		else if (!ch->isTopicSet)
		{
			ch->setTopic(topic);
			client.send_message(RPL_TOPIC(client.nick, this->hostname, ch->getName(), ch->getTopic()));
		}
		else
			client.send_message(ERR_CHANOPRIVSNEEDED(client.nick, this->getHostName(), channel));
	}
}