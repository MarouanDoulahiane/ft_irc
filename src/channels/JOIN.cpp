#include "../../headers/Server.hpp"

//channel function :
Channel *Server::isChannelExisiting(std::string name)
{
	std::vector<Channel *>::iterator it;
	for (it = channels.begin(); it < channels.end(); it++)
	{
		Channel *c = *it;
		if (name.compare((*c).getName()) == 0)
			return c;
	}
	return NULL;
}


Channel *Server::getChannelByName(std::string name)
{
    Channel *token;
	std::vector<Channel *>::iterator it;
	for (it = this->channels.begin(); it < this->channels.end(); it++)
	{
        token = *it;
		if (name.compare((*token).getName()) == 0)
			return token;
	}
	return NULL;
}

void Server::addNewChannel(std::string name, std::string pass, Client &client)
{
    Channel *channel = isChannelExisiting(name);
    if (!channel)
    {
        channel = new Channel(name, pass,client, this);
        this->channels.push_back(channel);
        client.send_message(RPL_JOIN(client.nick, client.user, name, client.getIpadd()));
        client.send_message(RPL_MODEIS(name, this->getHostName(), channel->getMode()));
        client.send_message(RPL_NAMREPLY(this->getHostName(), channel->getListClients(), channel->getName(), client.nick));
        client.send_message(RPL_ENDOFNAMES(this->getHostName(), client.nick, name));
    }
    else
    {
        try
        {
            channel->add(client, pass);
            client.send_message(RPL_JOIN(client.nick, client.user, name, client.getIpadd()));
            client.send_message(RPL_MODEIS(name, this->getHostName(), channel->getMode()));
            client.send_message(RPL_TOPIC(client.nick, this->getHostName(), name, channel->getTopic()));
            client.send_message(RPL_TOPICTIME(client.nick , IrcServhostname, channel->getName(), channel->getTopicNickSetter(), channel->getTopicTimestamp()));
            client.send_message(RPL_NAMREPLY(this->getHostName(), channel->getListClients(), channel->getName(), client.nick));
            client.send_message(RPL_ENDOFNAMES(this->getHostName(), client.nick, name));
            channel->sendMessageCh(client, RPL_JOIN(client.nick, client.user, channel->getName(), client.getIpadd()));
        }
        catch(ClientErrMsgException &e)
        {
			std::cout << e.what() << std::endl;
			client.send_message(e.what());
        }
    }
}

void Server::handleJOIN(cmd &command, Client &cli)
{
	if (command.args.size() == 1)
		cli.send_message(ERR_NEEDMOREPARAMS(cli.nick, this->getHostName()));
    std::vector<std::string> channels;
    std::vector<std::string> temp_channels;
	std::vector<std::string> keys;
    for (size_t i = 1; i < command.args.size(); ++i)
    {

        size_t pos = command.args[i].find(",");
		if (pos != std::string::npos)
		{
			temp_channels = split(command.args[i],',');
			for(size_t i = 0; i < temp_channels.size(); i++)
			{
				size_t posH = temp_channels[i].find("#");
				if (posH || posH == std::string::npos)
				{
					cli.send_message(ERR_BADCHANNELNAME(cli.nick, this->getHostName(), temp_channels[i]));
					temp_channels.pop_back();
					return ;
				}
				else if (posH != std::string::npos)
				{
					std::string channelName = temp_channels[i].substr(0, temp_channels[i].size());
					channels.push_back(channelName);
				}
			}
		}
		else if (pos == std::string::npos && i == 1)
		{
			size_t posH = command.args[i].find("#");
			if (posH || posH == std::string::npos)
				cli.send_message(ERR_BADCHANNELNAME(cli.nick, this->getHostName(), command.args[i]));
			else if (posH != std::string::npos)
			{
				std::string channelName = command.args[i].substr(0, command.args[i].size());
				channels.push_back(channelName);
			}	
		}
		else if (i > 1)
		{
			size_t posK = command.args[i].find('\0');
			std::string _key  = command.args[i].substr(0, posK);
			keys.push_back(_key);
		}
    }
    std::vector<std::string>::iterator it = keys.begin();
	for(int i = 0; i < channels.size(); i++)
	{
		if (keys.size() == 0)
			addNewChannel(channels[i],"",cli);
		if (it != keys.end())
		{
			addNewChannel(channels[i], *it, cli);
            it++;
		}
	}
}