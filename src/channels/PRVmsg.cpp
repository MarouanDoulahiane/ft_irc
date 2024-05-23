#include "../headers/IRCserv.hpp"
#include "../headers/replies.hpp"


void IRCserv::parsePRIVMSG(char *msg, Client &client)
{  
    char *prefix = strtok(msg, " ");
    char *target = strtok(NULL, " ");
    char *text = strtok(NULL, "");
    if  (!prefix || strcmp(prefix, "PRIVMSG"))
        return ;
    if (text && text[0] == ':')
        text++;
    else if (text && text[0] != ':')
		text = strtok(text, " ");
    if (!target || !text)
    {
        client.send_message(ERR_NEEDMOREPARAMS(client.nick, this->getHostName()));
        return ;
    }
    std::vector<std::string> spTargets = split(target, ',');
    for (size_t i = 0; i < spTargets.size() ; i++)
    {
        if (spTargets[i][0] == '#')
        {
            Channel *ch = isChannelExisiting(spTargets[i]);
            if (ch) 
            {
                if (ch->is_member(client))
                    ch->send_message(client, PRIVMSG_FORMATCH(client.nick, client.user, this->getHostName(), ch->getName(), text));
                else
                    client.send_message(ERR_CANNOTSENDTOCHAN(client.nick, spTargets[i], this->getHostName()));
            }
            else
                client.send_message(ERR_NOSUCHCHANNEL(this->getHostName(), spTargets[i], client.nick));
        }
        else
        {
            Client *receiver = isClientExisiting(spTargets[i]);
            if (receiver)
                receiver->send_message(PRIVMSG_FORMATUSER(client.nick, spTargets[i], text));
            else
                client.send_message(ERR_NOSUCHNICK(this->getHostName(), spTargets[i], client.nick));
        }
    }
}