#include "../../headers/Server.hpp"


void Server::handleMode(cmd &command, Client &cli)
{
    if (command.args.size() < 2)
    {
        cli.send_message(ERR_NEEDMOREPARAMS(cli.nick, this->getHostName()));
        return;
    }
    std::string target = command.args[1];
    if (command.args.size() < 3)
    {
        Channel *channel = isChannelExisiting(target);
        if (!channel)
        {
            cli.send_message(ERR_NOSUCHCHANNEL(this->getHostName(), target, cli.nick));
            return;
        }
        if (channel->getMode().empty())
            cli.send_message(RPL_CHANNELMODES(this->getHostName(), target, cli.nick, "No modes set"));
        else
            cli.send_message(RPL_CHANNELMODES(this->getHostName(), target, cli.nick, "+" + channel->getMode()));
        return ;
    }
    std::string flags;
    std::string addParams;
    parseFlags(command, cli, flags, addParams);
    applyModeFlags(target, flags, addParams, cli);
}
void  Server::parseFlags(cmd &command, Client &cli, std::string &flagsHolder, std::string &addParams)
{
    std::vector<std::string> flags;
    std::vector<std::string> params;
    std::string flagsList = "+-oltik";
    for(size_t i = 2; i < command.args.size();i++)
    {
        if (command.args[i][0] == '+' || command.args[i][0] == '-')
            flags.push_back(command.args[i]);
        else
        {
            addParams += command.args[i];
            if (i != command.args.size() - 1)
                addParams += " ";
        }
    }
    int numberOfFlags = 0;
    std::string singleFlag;
    for(std::vector<std::string>::iterator it = flags.begin(); it != flags.end(); it++)
    {
        singleFlag = *it;
        for (size_t i = 0; i < singleFlag.size(); i++)
        {
            if (flagsList.find(singleFlag[i]) != std::string::npos)
            {
                if (singleFlag[i] != '+' && singleFlag[i] != '-')
                    numberOfFlags++;
                flagsHolder += singleFlag[i];
            }
            else
                cli.send_message(ERR_UNKNOWNMODE(cli.nick, cli.getHostname(), command.args[1], singleFlag[i]));
        }
    }
}


void FInviteOnly(Channel* channel, bool setFlag,  std::string& additionalParams, Client& client, std::string hostName)
{
    (void)additionalParams;
    (void)client;
    channel->setInviteOnly(setFlag);
    if (setFlag == true)
    {
        channel->sendMessageCh(RPL_MODEIS(channel->getName(), hostName, "+i"));
    }
    else
        channel->sendMessageCh(RPL_MODEIS(channel->getName(), hostName, "-i"));

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
            channel->sendMessageCh(RPL_MODEIS(channel->getName(), hostName, "+k"));
        }
    }
    else
    {
        channel->isPasswordSet = false;
        channel->setKey("");
        channel->sendMessageCh(RPL_MODEIS(channel->getName(), hostName, "-k"));
    }
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
                channel->sendMessageCh(RPL_MODEISLIMIT(channel->getName(), hostName, "+l", additionalParams));
            }
            else
            {
                channel->setUserLimit(0);
                channel->sendMessageCh(RPL_MODEISLIMIT(channel->getName(), hostName, "-l", "0"));
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
            channel->addOperator(additionalParams, hostName, client);
    }
    else
    {
        if (additionalParams.empty())
            client.send_message(ERR_NEEDMOREPARAMS(client.nick, hostName));
        else
            channel->removeOperator(additionalParams, hostName, client);
    }
}


void FTopicRestrictions(Channel* channel, bool setFlag,  std::string& additionalParams, Client& client, std::string hostName)
{
    (void)additionalParams;
    (void)client;

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


void Server::storeMode(Channel *channel, char mode, bool setFlag)
{
    (void)mode;
    std::string modeList = channel->getMode();
    if (setFlag == true)
    {
        if ((channel->isInviteOnlySet == true && modeList.find('i') == std::string::npos ) || modeList.empty() == true)
            modeList += 'i';
        if ((channel->isTopicSet == true && modeList.find('t') == std::string::npos) || modeList.empty() == true)
            modeList += 't';
        if ((channel->isPasswordSet == true && modeList.find('k') == std::string::npos) || modeList.empty() == true)
            modeList += 'k';
        if ((channel->getuserLimit() > 0 && modeList.find('l') == std::string::npos) || modeList.empty() == true)
            modeList += 'l';
        if ((channel->Operators.size() > 0 && modeList.find('o') == std::string::npos) || modeList.empty() == true)
            modeList += 'o';
    }
    else
    {
        if (channel->isInviteOnlySet == false)
        {
            if (modeList.find('i') != std::string::npos)
                modeList.erase(modeList.find('i'), 1);
        }
        if (channel->isTopicSet == false)
        {
            if (modeList.find('t') != std::string::npos)
                modeList.erase(modeList.find('t'), 1);
        }
        if (channel->isPasswordSet == false)
        {
            if (modeList.find('k') != std::string::npos)
                modeList.erase(modeList.find('k'), 1);
        }
        if (channel->getuserLimit() == 0)
        {
            if (modeList.find('l') != std::string::npos)
                modeList.erase(modeList.find('l'), 1);
        }
        if (channel->Operators.size() == 0)
        {
            if (modeList.find('o') != std::string::npos)
                modeList.erase(modeList.find('o'), 1);
        }
    }
    channel->setMode(modeList);
}

void Server::applyModeFlags(std::string channelName, std::string modeFlags, std::string addParams, Client &client)
{
    std::vector<std::string> splitParams;
    if (addParams.size() != 0)
        splitParams = split(addParams, ' ');
    else
        splitParams.push_back(addParams);
    std::string mode = "";
    Channel *channel = isChannelExisiting(channelName);
    if (!channel)
    {
        client.send_message(ERR_NOSUCHCHANNEL(this->getHostName(), channelName, client.nick));
        return;
    }
    if (channel->isOnOperatorList(client.GetFd()) == false)
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
    for (std::string::iterator it = modeFlags.begin(); it != modeFlags.end(); it++)
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
        std::string empty = "";
        if (actionIt != modeActions.end())
        {
            if ((i < splitParams.size() && flag != 'i' && flag != 't' && setFlag == true))
            {
                actionIt->second(channel, setFlag, splitParams[i], client, this->getHostName());
                i++;
            }
            else if (flag == 'i' || flag == 't')
            {
                actionIt->second(channel, setFlag, empty, client, this->getHostName());
            }
            else if(setFlag == false && flag == 'o')
            {
                if (i < splitParams.size())
                {
                    actionIt->second(channel, setFlag, splitParams[i], client, this->getHostName());
                    i++;
                }
            }
            else if(setFlag == false)
            {
                actionIt->second(channel, setFlag, empty, client, this->getHostName());
            }
            else
                client.send_message(ERR_NEEDMOREPARAMS(client.nick, this->getHostName()));
        }
        else
        {
            if (flag != 's' && flag != 'n')
                client.send_message(ERR_UNKNOWNMODE(client.nick, this->IrcServhostname, channelName, flag));
        }
        storeMode(channel, flag, setFlag);
    }
}
