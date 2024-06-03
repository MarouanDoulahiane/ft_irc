#include "../../headers/channels.hpp"
#include "../../headers/Client.hpp"
#include <exception>


Channel::Channel(std::string name, std::string pass, Client &client, Server *srv)
{
    this->Ircserv = srv;
    this->srv_hostname = (*Ircserv).getHostName();
    if (!this->validChannelName(name))
        throw ClientErrMsgException(ERR_BADCHANNELNAME(client.nick, srv_hostname, name), client);
    this->name = name;
    if (pass.empty() == true)
    {
        this->isPasswordSet = false;
        this->add(client, pass);
        this->Operators.push_back(client.GetFd());
        this->TopicNicksetter = client.nick;
        this->TopicUsersetter = client.user;
        this->TopicTimestamp = time(NULL);
    }
    else
    { 
        this->TopicNicksetter = client.nick;
        this->TopicUsersetter = client.user;
        this->TopicTimestamp = time(NULL);
        this->isPasswordSet = true;
        this->password = pass;
        this->Operators.push_back(client.GetFd());
        this->add(client, pass);
        this->setMode("+k");// debug here double check 
    }
}

void Channel::updateNick(std::string oldNick, std::string newNick)
{
    std::vector<Client>::iterator it;
    for (it = this->clients.begin(); it != this->clients.end(); it++)
    {
        if (it->nick == oldNick)
        {
            it->nick = newNick;
            return;
        }
    }
}

void Channel::addInvited(Client &client)
{
    this->InvitedClients.push_back(client);
}

bool Channel::add(Client &client, std::string pass)
{
    if (checkClient(client))
        throw ClientErrMsgException(ERR_USERONCHANNEL(this->srv_hostname, this->name, client.nick), client);
    if (this->isPasswordSet == true)
    {
        if (pass.empty() == true || this->password != pass)
            throw ClientErrMsgException(ERR_BADCHANNELKEY(client.nick, this->srv_hostname, this->name), client);
    }
    if (this->getuserLimit() > 0 && this->clients.size() >= this->getuserLimit())
        throw ClientErrMsgException(ERR_CHANNELISFULL(client.nick, this->name), client);
    if (this->isInviteOnly() == true && !checkInvitedClient(client))
        throw ClientErrMsgException(ERR_INVITEONLY(client.nick, this->srv_hostname), client);
    else deleteInvitedClient(client);
    this->clients.push_back(client);
    client.getChannels().push_back(this);
    return true;
}

// Mode funcs -- adds funcs,, ....

void Channel::addOperator(const std::string& nickname, std::string hostName, Client &client)
{
    if (nickInChannel(nickname) == true)
    {
        std::vector<Client>::iterator it;
        for (it = this->clients.begin(); it != this->clients.end(); it++)
        {
            if (it->nick == nickname)
            {
                this->Operators.push_back(it->GetFd());
                this->sendMessageCh(RPL_MODEISOP(name, hostName, "+o", nickname));
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
    if (this->isOnOperatorList(client.GetFd()))
    {
        for (unsigned int i = 0; i < this->Operators.size(); i++)
        {
            if (this->Operators[i] == client.GetFd())
            {
                this->Operators.erase(this->Operators.begin() + i);
                this->sendMessageCh(RPL_MODEISOP(name, hostName, "-o", nickname));
                client.send_message(RPL_MODEISOP(name, hostName, "-o", nickname));
                return;
            }
        }
    }
    else
        client.send_message(ERR_CHANOPRIVSNEEDED(client.nick, hostName, this->getName()));
}

void Channel::deleteOperator(int fd)
{
    std::vector<int>::iterator it;
    for (it = this->Operators.begin(); it < this->Operators.end(); it++)
    {
        if (*it == fd)
        {
            this->Operators.erase(it);
            return;
        }
    }
}

void Channel::deleteClient(Client &client)
{
    std::vector<Client>::iterator it;
    for (it = this->clients.begin(); it != this->clients.end(); it++)
    {
        if (it->nick == client.nick)
        {
            this->clients.erase(it);
            client.eraseChannel(this->name);
            if (this->isOnOperatorList(client.GetFd()))
            {
                this->deleteOperator(client.GetFd());
                if (this->Operators.size() == 0 && this->clients.size() > 0)
                    this->setOperator(this->clients[0]);
            }
            if (checkInvitedClient(client))
                this->deleteInvitedClient(client);
            return;
        }
    }
}

void Channel::deleteInvitedClient(Client &client)
{
    std::vector<Client>::iterator it;
    for (it = this->InvitedClients.begin(); it != this->InvitedClients.end(); it++)
    {
        if (it->nick == client.nick)
        {
            this->InvitedClients.erase(it);
            return;
        }
    }
}

void Channel::sendMessageCh(std::string msg)
{
    std::vector<Client>::iterator it;
    for (it = this->clients.begin(); it != this->clients.end(); it++)
        it->send_message(msg);
}

void Channel::sendMessageChExclude(std::string msg, std::string nick)
{
    std::vector<Client>::iterator it;
    for (it = this->clients.begin(); it != this->clients.end(); it++)
    {
        if (it->nick != nick)
            it->send_message(msg);
    }
}


void Channel::sendMessageCh(Client &client, std::string msg)
{
    std::vector<Client>::iterator it;
    for (it = this->clients.begin(); it != this->clients.end(); it++)
        if (it->nick != client.nick)
            it->send_message(msg);
}


/////-------------------------------------
// setters :
void Channel::setTopic(std::string newTopic)
{
    Topic = newTopic;
}


void Channel::setKey(std::string newKey)
{
    Key = newKey;
    password = newKey;
}


void Channel::setUserLimit(int newLimit)
{
    AllowedUsers = newLimit;
}

void Channel::setTopicRestrictions(bool setFlag)
{
    isTopicSet = setFlag;
}

void Channel::setInviteOnly(bool setFlag)
{
    isInviteOnlySet = setFlag;
}

void Channel::setOperator(Client &client)
{
    if (this->isOnOperatorList(client.GetFd()))
        return;
    this->Operators.push_back(client.GetFd());
    this->sendMessageCh(RPL_MODEISOP(name, this->srv_hostname, "+o", client.nick));
    client.send_message(RPL_YOUREOPER(this->srv_hostname, client.nick));
    this->_isOperator = true;
}


void Channel::setMode(const std::string& newMode)
{
    Mode = newMode;
}


////////////////////////-------------------
// gettters

std::string Channel::getListClients()
{
    std::string str;
    std::vector<Client>::iterator it;
    for (it = this->clients.begin(); it != this->clients.end(); it++)
    {
        if (it != this->clients.begin())
            str += " ";
        if (this->isOnOperatorList((it)->GetFd()))
            str += "@";
        str += (it)->nick;
    }
    return str;
}

int Channel::getuserLimit()
{
   return AllowedUsers;
}


std::string Channel::getOp()
{
    return (Op);
}

std::string Channel::getKey()
{
    return Key;
}

std::string Channel::getMode()
{
    return Mode;
}

std::vector<Client> Channel::getClients()
{
    return (this->InvitedClients);
}


std::string Channel::getName()
{
    return name;
}

 
std::string Channel::getTopic()
{
    return Topic;
}

std::string Channel::getTopicNickSetter()
{
    return TopicNicksetter;
}

std::string Channel::getTopicUserSetter()
{
    return this->TopicUsersetter;
}

std::string Channel::getTopicTimestamp()
{
    std::stringstream ss;
    std::string str;

    ss << this->TopicTimestamp;
    ss >> str;
    return (str);
}


///////////////////////////////////////////////////////////////////////////
// boolen functions to check if mode is set
bool Channel::validChannelName(std::string name)
{
    for (unsigned int i = 1; i < name.length(); i++)
    {
        if ((name[i] == '#') || name[i] == ',' || name[i] == ' '
            || name[i] == '\r' || name[i] == '\n')
            return false;
    }
    return true;
}

bool Channel::isOnOperatorList(int fd)
{
    std::vector<int>::iterator it;
    for (it = this->Operators.begin(); it < this->Operators.end(); it++)
    {
        if (*it == fd)
            return true;
    }
    return false;
}

bool Channel::checkClient(Client &client)
{
    std::vector<Client>::iterator it;
    for (it = this->clients.begin(); it < this->clients.end(); it++)
        if (client.nick == it->nick)
            return true;
    return false;
}
bool Channel::checkInvitedClient(Client &client)
{
    std::vector<Client>::iterator it;
    for (it = this->InvitedClients.begin(); it < this->InvitedClients.end(); it++)
    {
        if (it->nick == client.nick)
            return true;
    }
    return false;

}

bool Channel::nickInChannel(std::string nickname)
{
    std::vector<Client>::iterator it;
    for (it = this->clients.begin(); it != this->clients.end(); it++)
    {
        if (it->nick == nickname)
            return true;
    }
    return false;
}

bool Channel::isKeySet()
{
    return isPasswordSet;
}

bool Channel::isUserLimitSet()
{
    return AllowedUsers;
}

bool Channel::isTopicRestrictionsSet()
{
    return isTopicSet;
}


bool Channel::isInviteOnly()
{
    return isInviteOnlySet;
}

bool Channel::isModeSet(std::string mode)
{
    return mode == this->Mode;
}
