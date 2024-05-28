#include "../../headers/Server.hpp"

void Server::handleMode(cmd &command, Client &cli)
{
    if (command.args.size() < 2)
    {
        cli.send_message(ERR_NEEDMOREPARAMS(cli.nick, this->getHostName()));
        return;
    }
    std::string target = command.args[1];
    std::cout << RED << "START MODE DEBUG" << command.args.size()<< WHI << std::endl;
    if (command.args.size() < 3)
    {
        Channel *channel = isChannelExisiting(target);
        cli.send_message(RPL_CHANNELMODES(this->getHostName(), target, cli.nick, channel->getMode()));
        return ;
    }
    std::string flags = command.args[2];
    std::string addParams = "";
    int i;
    for(i = 3; i < command.args.size(); i++)
    {
        addParams += command.args[i] + " ";
        applyModeFlags(target, flags, addParams, cli);
    }
    if (i == 3)
        applyModeFlags(target, flags, addParams, cli);
    
}

void FInviteOnly(Channel* channel, bool setFlag,  std::string& additionalParams, Client& client, std::string hostName)
{
    channel->setInviteOnly(setFlag);
}

void FKey(Channel* channel, bool setFlag,  std::string& additionalParams, Client& client, std::string hostName)
{
    if (setFlag)
    {
    
        if (additionalParams.empty() == true)
            client.send_message(ERR_NEEDMOREPARAMS(client.nick, hostName));
        else
        {
            channel->setKey(additionalParams);
            channel->isPasswordSet = true;
        }
    }
    else
        channel->isPasswordSet = false;
}


void FUserLimit(Channel* channel, bool setFlag,  std::string& additionalParams, Client &client, std::string hostName)
{
    int limit = 0;
    if (setFlag)
    {
        if (additionalParams.empty())
            client.send_message(ERR_NEEDMOREPARAMS(client.nick, hostName));
        else
        {
            limit = atoi(additionalParams.c_str());
            if (limit && (limit > 0 && limit < 1000))
            {
                channel->setUserLimit(limit);
                channel->sendMessageCh(RPL_MODEISLIMIT(channel->getName(), hostName, "+l", additionalParams));// 3ndak
            }
            else
            {
                channel->setUserLimit(0);
                channel->sendMessageCh(RPL_MODEISLIMIT(channel->getName(), hostName, "-l", "0"));// 3ndak

            }
        }
    }
    else
    {
        channel->setUserLimit(0);
        channel->sendMessageCh(RPL_MODEISLIMIT(channel->getName(), hostName, "-l", "0"));
    }
}
void FOperator(Channel* channel, bool setFlag,  std::string& additionalParams, Client& client, std::string hostName)
{

    if (setFlag)
    {
        if (additionalParams.empty())
            client.send_message(ERR_NEEDMOREPARAMS(client.nick, hostName));
        else
        {
            channel->addOperator(additionalParams, hostName, client);
        }

    }
    else
    {
        if (additionalParams.empty())
            client.send_message(ERR_NEEDMOREPARAMS(client.nick, hostName));
        else
        {
            channel->removeOperator(additionalParams, hostName, client);// fix it
        }
    }
}


void FTopicRestrictions(Channel* channel, bool setFlag,  std::string& additionalParams, Client& client, std::string hostName)
{
    if (setFlag)
    {
        channel->isTopicSet = setFlag;
        channel->sendMessageCh(RPL_MODEIS(channel->getName(), hostName, "+t"));
    }
    else
    {
        channel->isTopicSet = setFlag;
        channel->sendMessageCh(RPL_MODEIS(channel->getName(), hostName, "-t"));
    }
}

void Server::applyModeFlags(std::string channelName, std::string modeFlags, std::string addParams, Client &client)
{
    std::vector<std::string> splitParams;
    if (addParams.size() != 0){
        splitParams = split(addParams, ' ');
    }
    else
        splitParams.push_back(addParams);
    std::string mode = "";
    Channel *channel = isChannelExisiting(channelName);
    if (!channel)
    {
        client.send_message(ERR_NOSUCHCHANNEL(this->getHostName(), channelName, client.nick));
        return;
    }
    if (channel->isOnOperatorList(client.sock) == false)
    {
        client.send_message(ERR_CHANOPRIVSNEEDED(client.nick, this->getHostName(), channel->getName()));
        return ;
    }
    bool setFlag = true;
    std::map<char, void (*)(Channel*, bool,  std::string&, Client&, std::string)> modeActions;
    modeActions['i'] = &FInviteOnly;
    modeActions['t'] = &FTopicRestrictions;
    modeActions['k'] = &FKey;
    modeActions['o'] = &FOperator;
    modeActions['l'] = &FUserLimit;
    size_t i = 0;
    for (std::string::iterator it = modeFlags.begin(); it != modeFlags.end(); ++it)
    {
        char flag = *it;
        if (flag == '+')
        {
            setFlag = true;
            continue;
        }
        else if (flag == '-')
        {
            setFlag = false;
            continue;
        }
        std::map<char, void (*)(Channel*,  bool, std::string&, Client&, std::string)>::iterator actionIt = modeActions.find(flag);
        if (actionIt != modeActions.end())
        {
            if (splitParams.size() == 0 && flag != 'i' && flag != 't' && setFlag == true)
            {
                client.send_message(ERR_NEEDMOREPARAMS(client.nick, this->getHostName()));
                return;
            }
            if (i < splitParams.size())
            {
                actionIt->second(channel, setFlag, splitParams[i], client, this->getHostName());
                if (flag != 'i' && flag != 't')
                    i++;
            }
            else if (flag != 'i' && flag != 't')
            {
                client.send_message(ERR_NEEDMOREPARAMS(client.nick, this->getHostName()));
            }
            if (setFlag)
                mode += "+";
            else
                mode += "-";
            mode += flag;
            channel->setMode(mode);
        }
        else
        {
            if (flag != 's' && flag != 'n')
                client.send_message(ERR_UNKNOWNMODE(client.nick, this->IrcServhostname, channelName, flag));
        }
    }
    
}
