#include "../../headers/Server.hpp"
#include "../../headers/channels.hpp"

void Server::sendMSG(std::string &target, std::string &text, Client &cli)
{
    std::vector<std::string> spTargets = split(target, ',');
    for (size_t i = 0; i < spTargets.size() ; i++)
    {
        if (spTargets[i][0] == '#')
        {
            Channel *ch = isChannelExisiting(spTargets[i]);
            if (ch) 
            {
                if (ch->checkClient(cli))
                    ch->sendMessageCh(cli, PRIVMSG_FORMATCH(cli.nick, cli.user, this->getHostName(), ch->getName(), text));
                else
                    cli.send_message(ERR_CANNOTSENDTOCHAN(cli.nick, spTargets[i], this->getHostName()));
            }
            else
                cli.send_message(ERR_NOSUCHCHANNEL(this->getHostName(), spTargets[i], cli.nick));
        }
        else
        {
            Client *receiver = isClientBef(spTargets[i]);
            if (receiver)
                receiver->send_message(PRIVMSG_FORMATUSER(cli.nick, spTargets[i], text));
            else
                cli.send_message(ERR_NOSUCHNICK(this->getHostName(), spTargets[i], cli.nick));
        }
    }
}


void Server::handlePRIVMSG(cmd &command, Client &cli)
{
    if (!command.buff.empty())
    {
        if (command.args.size() < 3)
        {
            cli.send_message(ERR_NEEDMOREPARAMS(cli.nick, this->getHostName()));
            return ;
        }
        std::string target = command.args[1];
        sendMSG(target, command.buff, cli);
    }
    else
    {
        if (command.args.size() < 3)
        {
            cli.send_message(ERR_NEEDMOREPARAMS(cli.nick, this->getHostName()));
            return ;
        }
        std::string target = command.args[1];
        std::string text;
        for(int i = 2; i < command.args.size(); i++)
        {
            text += command.args[i];
            if (i + 1 != command.args.size())
                text += " ";
        }
        sendMSG(target, text, cli);
    }
}