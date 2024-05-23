#include "../headers/Channel.hpp"

Channel::Channel(std::string name, char *pass, Client &client, IRCserv *srv)
{
    this->serv = srv;
    this->srv_hostname = this->serv->getHostName();
    if (!this->_isChannelNameValid(name))
        throw ClientErrMsgException(ERR_BADCHANNELNAME(client.nick, srv_hostname, name), client);
    this->name = name;
    if (pass)
    {
        this->isPasswordSet = true;
        this->pass = pass;
        this->setMode("+k");
    }
    else this->isPasswordSet = false;
    this->addClient(client, pass);
    this->fdOps.push_back(client.sock);
    this->topic_nicksetter = client.nick;
    this->topic_usersetter = client.user;
    this->topic_set_timestamp = time(NULL);
}

bool Channel::_isChannelNameValid(std::string name)
{
    for (size_t i = 0; i < name.length(); i++)
    {
        if ((i != 0 && name[i] == '#') || name[i] == ',' || name[i] == ' '
            || name[i] == '\r' || name[i] == '\n')
            return false;
    }
    return true;    
}
Channel *IRCserv::isChannelExisiting(std::string name)
{
	std::vector<Channel *>::iterator it;
	for (it = this->channels.begin(); it < this->channels.end(); it++)
	{
		Channel *c = *it;
		if (name.compare((*c).getName()) == 0)
			return c;
	}
	return NULL;
}


void Channel::eraseClient(Client &client)
{
    std::vector<Client>::iterator it;
    for (it = this->clients.begin(); it < this->clients.end(); it++)
    {
        if (it->nick == client.nick)
        {
            this->clients.erase(it);
            return;
        }
    }
}

void Channel::eraseOp(int fd)
{
    std::vector<int>::iterator it;
    for (it = this->fdOps.begin(); it < this->fdOps.end(); it++)
    {
        if (*it == fd)
            this->fdOps.erase(it);
    }
}


// getters :
std::vector<Client> Channel::getClients()
{
    return (this->clients);
}


std::string Channel::getName()
{
    return name;
}

 
std::string Channel::getTopic()
{
    return this->topic;
}

std::string Channel::getTopicTimestamp()
{
    std::stringstream ss;
    std::string str;

    ss << this->topic_set_timestamp;
    ss >> str;
    return (str);
}

void Channel::setName(std::string name)
{
    this->name = name;
}


bool Channel::isFdOperator(int fd)
{
    std::vector<int>::iterator it;
    for (it = this->fdOps.begin(); it < this->fdOps.end(); it++)
    {
        if (*it == fd)
            return true;
    }
    return false;
}


void Channel::addInvited(Client &client)
{
    this->clientsInvited.push_back(client);
}

std::string Channel::getListClients()
{
    std::string str;
    std::vector<Client>::iterator it;
    for (it = this->clients.begin();it < this->clients.end();it++)
    {
        if (it != this->clients.begin())
            str += " ";
        if (this->isFdOperator((it)->sock))
            str += "@";
        str += (it)->nick;
    }
    // std::cout << "Clients list of " << this->name << "  : " << str << std::endl;
    return str;
}

bool Channel::isClientOnChannel(Client client)
{
    std::vector<Client>::iterator it;

    for (it = this->clients.begin(); it < this->clients.end(); it++)
        if (client.nick == it->nick)
            return true;
    return false;
}
bool Channel::isClientInvited(Client &client)
{
    std::vector<Client>::iterator it;
    for (it = this->clientsInvited.begin(); it < this->clientsInvited.end(); it++)
    {
        if (it->nick == client.nick)
            return true;
    }
    return false;

}


bool Channel::addClient(Client &client, char *pass)
{
    if (isClientOnChannel(client))
        throw ClientErrMsgException(ERR_USERONCHANNEL(this->srv_hostname, this->name, client.nick), client);
    if (this->isPasswordSet)
    {
        if (!pass || this->pass != pass)
            throw ClientErrMsgException(ERR_BADCHANNELKEY(client.nick, this->srv_hostname, this->name), client);
    }
    if (this->getuserLimit() > 0 && this->clients.size() >= this->getuserLimit())
        throw ClientErrMsgException(ERR_CHANNELISFULL(client.nick, this->name), client);
    if (this->isInviteOnly() == true && !isClientInvited(client))
        throw ClientErrMsgException(ERR_INVITEONLY(client.nick, this->srv_hostname), client);
    else eraseInvitedClient(client);
    this->clients.push_back(client);
    client._channels.push_back(this);
    return true;
}

bool Channel::partClient(Client &client, std::string reason)
{
    this->send_message(client, RPL_PART(srv_hostname, client.nick, client.user, name, reason));
    client.send_message(RPL_PART(srv_hostname, client.nick, client.user, name, reason));
    std::cout << "Client " << client.nick << " removed from channel " << this->name << std::endl;
    std::vector<Client>::iterator it;
    for (it = this->clients.begin(); it < this->clients.end(); it++)
    {
        if (it->nick == client.nick)
        {
            this->clients.erase(it);
            if (this->isFdOperator(client.sock))
            {
                this->eraseOp(client.sock);
                if (this->fdOps.size() == 0)
                {
                    this->_isOperator = false;
                    if (this->clients.size() == 0)
                        serv->removeChannel(this->name);
                    else 
                        this->addOperator(this->clients[0].nick, this->srv_hostname, this->clients[0]);
                }
            }
            break;
        }
    }
    return true;
}

bool Channel::quitClient(Client &client, std::string reason)
{
    this->send_message(client, RPL_QUIT(srv_hostname, client.nick, client.user, reason));
    std::vector<Client>::iterator it;
    for (it = this->clients.begin(); it < this->clients.end(); it++)
    {
        if (it->nick == client.nick)
        {
            this->clients.erase(it);
            if (this->isFdOperator(client.sock))
            {
                this->eraseOp(client.sock);
                if (this->fdOps.size() == 0)
                {
                    this->_isOperator = false;
                    if (this->clients.size() == 0)
                        serv->removeChannel(this->name);
                    else 
                        this->addOperator(this->clients[0].nick, this->srv_hostname, this->clients[0]);
                }
            }
            break;
        }
    }
    return true;
}


void Channel::eraseInvitedClient(Client &client)
{
    std::vector<Client>::iterator it;
    for (it = this->clientsInvited.begin(); it < this->clientsInvited.end(); it++)
    {
        if (it->nick == client.nick)
        {
            this->clientsInvited.erase(it);
            client.eraseInvitedChannel(this->name);
            return;
        }
    }
}



void Channel::send_message(std::string msg)
{
    std::vector<Client>::iterator it;
    for (it = this->clients.begin();it < this->clients.end();it++)
        it->send_message(msg);
}


void Channel::send_message(Client &client, std::string msg)
{
    std::vector<Client>::iterator it;
    for (it = this->clients.begin();it < this->clients.end();it++)
        if (it->nick != client.nick)
            it->send_message(msg);
}


// std::

void IRCserv::addNewChannel(std::string name,char *pass, Client &client)
{
    Channel *channel = isChannelExisiting(name);
    if (!channel)
    {
        channel = new Channel(name, pass,client, this);
        this->channels.push_back(channel);
        client.send_message(RPL_JOIN(client.nick, client.user, name, client.getIpAddress()));
        client.send_message(RPL_MODEIS(name, this->getHostName(), channel->getMode()));
        client.send_message(RPL_NAMREPLY(this->getHostName(), channel->getListClients(), channel->getName(), client.nick));
        client.send_message(RPL_ENDOFNAMES(this->getHostName(), client.nick, name));
        
    }
    else
    {
        std::cout << "Channel existing" << std::endl;
        try
        {
            channel->addClient(client, pass);
            client.send_message(RPL_JOIN(client.nick, client.user, name, client.getIpAddress()));
            client.send_message(RPL_MODEIS(name, this->getHostName(), channel->getMode()));
            client.send_message(RPL_TOPIC(client.nick,this->getHostName(), name, channel->getTopic()));
            client.send_message(RPL_TOPICTIME(client.nick , this->hostname, channel->getName(), channel->getTopicNickSetter(), channel->getTopicTimestamp()));
            client.send_message(RPL_NAMREPLY(this->getHostName(), channel->getListClients(), channel->getName(), client.nick));
            client.send_message(RPL_ENDOFNAMES(this->getHostName(), client.nick, name));
            channel->send_message(client, RPL_JOIN(client.nick, client.user, channel->getName(), client.getIpAddress()));
        }
        catch(const ClientErrMsgException &e)
        {
            e._client.send_message(e.getMessage());
        }
        
    }
}

void IRCserv::removeChannel(std::string name)
{
    Channel *channel = isChannelExisiting(name);
    if (!channel)
        return;
    std::vector<Channel *>::iterator it;
    for (it = this->channels.begin(); it < this->channels.end(); it++)
    {
        if ((*it)->getName() == name)
        {
            if ((*it)->getClients().size() > 0)
            {
                std::vector<Client>::iterator itc;
                for (itc = (*it)->getClients().begin(); itc < (*it)->getClients().end(); itc++)
                {
                    (*it)->partClient(*itc, "");
                }
            }
            this->channels.erase(it);
            delete channel;
            return;
        }
    }
}

void IRCserv::partChannel(std::string name,char *_reason, Client &client)
{
    Channel *channel = isChannelExisiting(name);
    if (!channel)
        client.send_message(ERR_NOSUCHCHANNEL(hostname, name, client.nick));
    if (!channel->is_member(client))
        client.send_message(ERR_NOTONCHANNEL(hostname, name));
    std::string reason("");
    if (_reason)
        reason = _reason;
    channel->partClient(client, reason);
}


std::string Channel::getTopicNickSetter()
{
    return this->topic_nicksetter;
}

std::string Channel::getTopicUserSetter()
{
    return this->topic_usersetter;
}

bool Channel::is_member(Client &client)
{
    std::vector<Client>::iterator it;
    for (it = this->clients.begin(); it != this->clients.end(); it++)
    {
        if (client.nick == it->nick)
            return true;
    }
    return false;
}


// setters and adds funcs,, omar....

void Channel::addOperator(const std::string& nickname, std::string hostName, Client &client)
{
    if (isNickInChannel(nickname) == true)
    {
        std::vector<Client>::iterator it;
        for (it = this->clients.begin(); it < this->clients.end(); it++)
        {
            if (it->nick == nickname)
            {
                this->fdOps.push_back(it->sock);
                std::cout << "Operator added " << std::endl;
                this->send_message(RPL_MODEISOP(name, hostName, "+o", nickname));
                client.send_message(RPL_YOUREOPER(hostName, client.nick));
                this->_isOperator = true;
                return;
            }
        }
    } 
    else
        client.send_message(ERR_USERNOTINCHANNEL(hostName,  this->getName()));
}

void Channel::removeOperator(const std::string& nickname, std::string hostName, Client &client)
{
    if (isNickInChannel(nickname) == true)
    {

        std::vector<Client>::iterator it;
        for (it = this->clients.begin(); it < this->clients.end(); it++)
        {
            if (it->nick == nickname)
            {
                eraseOp(it->sock);
                this->send_message(RPL_MODEISOP(name, hostName, "-o", nickname));
                // client.send_message(RPL_YOUREOPER(hostName, client.nick));
                // it->send_message(ERR_CHANOPRIVSNEEDED(it->nick, hostName, this->getName()));
                this->_isOperator = false;
                return;
            }
        }
    }
    else
        client.send_message(ERR_USERNOTINCHANNEL(hostName,  this->getName()));
}


void Channel::setTopic(std::string newTopic)
{
    topic = newTopic;
}

void Channel::setInviteOnly(bool setFlag)
{
    isInviteOnlySet = setFlag;
}

void Channel::setKey(std::string newKey)
{
    key = newKey;
}


void Channel::setUserLimit(int newLimit)
{
    userLimit = newLimit;
}

void Channel::setTopicRestrictions(bool setFlag)
{
    isTopicSet = setFlag;
}

void Channel::setOperator(Client &client, bool setFlag)
{
    this->fdOps.push_back(client.sock);
}


void Channel::setMode(const std::string& newMode)
{
    mode = newMode;
}


// gettters
bool Channel::isNickInChannel(std::string nickname)
{
    std::vector<Client>::iterator it;
    for (it = this->clients.begin(); it < this->clients.end(); it++)
    {
        if (it->nick == nickname)
            return true;
    }
    return false;
}
 int Channel::getuserLimit()
 {
    return userLimit;
 }


std::string Channel::getOp()
{
    return (this->op);
}

std::string Channel::getKey()
{
    return key;
}

std::string Channel::getMode()
{
    return mode;
}



// boolen functions to check if mode is set
bool Channel::isKeySet()
{
    return isPasswordSet;
}

bool Channel::isUserLimitSet()
{
    return userLimit;
}

bool Channel::isTopicRestrictionsSet()
{
    return isTopicSet;
}


bool Channel::isOperator(std::string nickname)
{
    return _isOperator;
}

bool Channel::isInviteOnly()
{
    return isInviteOnlySet;
}

bool Channel::isModeSet(std::string mode)
{
    return mode == this->mode;
}
