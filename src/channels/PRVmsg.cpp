#include "../../headers/Server.hpp"
#include "../../headers/channels.hpp"

void Server::sendMSG(std::string &target, std::string &text, Client &cli)
{
    std::vector<std::string> spTargets = split(target, ',');
    for (size_t i = 0; i < spTargets.size() ; i++)
    {
        Channel *ch = isChannelExisiting(spTargets[i]);
        if (ch) 
        {
            if (ch->checkClient(cli))
                ch->sendMessageChExclude(PRIVMSG_FORMCH(cli.nick, spTargets[i], getHostName(), ch->getName(), text), cli.nick);
            else
                cli.send_message(ERR_CANNOTSENDTOCHAN(cli.nick, spTargets[i], this->getHostName()));
        }
        else
        {
            Client *receiver = isClientBef(spTargets[i]);
            if (receiver)
                receiver->send_message(PRIVMSG_FORMATUSER(cli.nick, cli.user, getHostName(), receiver->nick, text));
            else
                cli.send_message(ERR_NOSUCHNICK(this->getHostName(), spTargets[i], cli.nick));
        }
    }
}


void Server::handlePRIVMSG(cmd &command, Client &cli)
{
    if (command.args.size() < 3)
    {
        cli.send_message(ERR_NEEDMOREPARAMS(cli.nick, this->getHostName()));
        return;
    }
    std::string target = command.args[1];
    std::string text = command.useBuffer(2) ? command.buff : command.args[2];
    sendMSG(target, text, cli);
}