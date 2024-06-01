#pragma once

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include "Server.hpp"
#include "channels.hpp"
#include "header.hpp"
#include "parsedReplies.hpp"
#include "Client.hpp"

class Client;
class Server;


class Channel
{
    private:
        std::string srv_hostname;
        std::string name;
        std::string password;
        std::string ChannelName;
        
        //mode variables:
        std::string Mode;
        std::string Key;
        std::string Topic;
        std::string Op;
        std::string TopicUsersetter;
        std::string TopicNicksetter;
        int         AllowedUsers;
        int         TopicTimestamp;


        //boolen vars
        Server      *Ircserv;

    public:
        Channel();
        ~Channel();
        Channel(std::string name, std::string pass, Client &client, Server *srv);

        //more vars
        bool        isPasswordSet;
        bool        isInviteOnlySet;
        bool        _isOperator;
        bool        isTopicSet;
        std::vector<int> Operators;
        std::vector<Client> InvitedClients;
        std::vector<Client> clients;

        
        //boolen functs
        bool isModeSet(std::string mode);
        bool isInviteOnly();
        bool isTopicRestrictionsSet();
        bool isUserLimitSet();
        bool isKeySet();
        bool nickInChannel(std::string nickname);
        bool checkInvitedClient(Client &client);
        bool checkClient(Client &client);
        bool isOnOperatorList(int fd);
        bool validChannelName(std::string name);

        // check and update ^_^
        bool add(Client &client, std::string pass);
        void addInvited(Client &client);
        // bool part(Client &client, std::string issue);
        // bool quit(Client &client, std::string issue);
        
        //Modes functions
        void addOperator(const std::string& nickname, std::string hostName, Client &client);
        void removeOperator(const std::string& nickname, std::string hostName, Client &client);
        void deleteOperator(int fd);
        void deleteClient(Client &client);
        void deleteInvitedClient(Client &client);
        void sendMessageCh(Client &c, std::string message);
        void sendMessageCh(std::string message);

        //setters
        void setName(std::string name);
        void setTopic(std::string newTopic);
        void setKey(std::string key);
        void setUserLimit(int limit);
        void setTopicRestrictions(bool setFlag);
        void setInviteOnly(bool setFlag);
        void setOperator(Client &client);
        void setMode(const std::string& newMode);


        //getters
        std::vector<Client> getClients();
        std::string getName();
        std::string getKey();
        std::string getListClients();
        std::string getOp();
        std::string getMode();
        int         getuserLimit();
        std::string getTopic();
        std::string getTopicNickSetter();
        std::string getTopicTimestamp();
        std::string getTopicUserSetter();

};

void FInviteOnly(Channel* channel, bool setFlag,  std::string& additionalParams, Client& client, std::string hostName);
void FKey(Channel* channel, bool setFlag,  std::string& additionalParams, Client& client, std::string hostName);
void FUserLimit(Channel* channel, bool setFlag,  std::string& additionalParams, Client &client, std::string hostName);
void FOperator(Channel* channel, bool setFlag,  std::string& additionalParams, Client& client, std::string hostName);


//         std::vector<Client> getClients();
//         std::string getListClients();

//         void eraseInvitedClient(Client &client);

//         void eraseClient(Client &client);
//         void eraseChannel(std::string channelName);
//         void eraseOp(int fd);

//         bool partClient(Client &c, std::string reason);
//         bool quitClient(Client &c, std::string reason);
