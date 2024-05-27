#include "../../headres/Server.hpp"
////////////////////////////////////////////////////////


// void IRCserv::removeChannel(std::string name)
// {
//     Channel *channel = isChannelExisiting(name);
//     if (!channel)
//         return;
//     std::vector<Channel *>::iterator it;
//     for (it = this->channels.begin(); it < this->channels.end(); it++)
//     {
//         if ((*it)->getName() == name)
//         {
//             if ((*it)->getClients().size() > 0)
//             {
//                 std::vector<Client>::iterator itc;
//                 for (itc = (*it)->getClients().begin(); itc < (*it)->getClients().end(); itc++)
//                 {
//                     (*it)->partClient(*itc, "");
//                 }
//             }
//             this->channels.erase(it);
//             delete channel;
//             return;
//         }
//     }
// }

// void IRCserv::partChannel(std::string name,char *_reason, Client &client)
// {
//     Channel *channel = isChannelExisiting(name);
//     if (!channel)
//         client.send_message(ERR_NOSUCHCHANNEL(hostname, name, client.nick));
//     if (!channel->is_member(client))
//         client.send_message(ERR_NOTONCHANNEL(hostname, name));
//     std::string reason("");
//     if (_reason)
//         reason = _reason;
//     channel->partClient(client, reason);
// }