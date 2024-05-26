#pragma once

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include "Server.hpp"
#include "channels.hpp"
#include "header.hpp"
#include "parsedReplies.hpp"

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
        bool        isPasswordSet;
        bool        isInviteOnlySet;
        bool        _isOperator;
        bool        isTopicSet;
        Server      *Ircserv;

    public:
        std::vector<int> Operators;
        std::vector<Client> InvitedClients;
        std::vector<Client> clients;
        Channel();
        ~Channel();
        Channel(std::string name, char *pass, Client &client, Server *srv);

        
        //boolen functs
        bool isModeSet(std::string mode);
        bool isInviteOnly();
        bool isOperator(std::string nickname);
        bool isTopicRestrictionsSet();
        bool isUserLimitSet();
        bool isKeySet();
        bool nickInChannel(std::string nickname);
        bool checkInvitedClient(Client &client);
        bool checkClient(Client &client);
        bool isOnOperatorList(int fd);
        bool validChannelName(std::string name);

        // check and update ^_^
        bool add(Client &client, char *pass);
        bool addClient(Client &client, char *pass);
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
        void setOperator(Client &client, bool setFlag);
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
        std::string getTopicTimestamp();
        std::string getTopicUserSetter();

};

// class Channel{
   

//         std::string srv_hostname;
//         std::string name;
//         std::string pass;
//         std::string op;
//         std::vector<Client> clients;

//         std::string topic;
//         std::string topic_usersetter;
//         std::string topic_nicksetter;

//         int topic_set_timestamp;
//         std::string key;
//         int userLimit;
//         std::string mode;

//         IRCserv *serv;

//         bool _isChannelNameValid(std::string name);

//     public:
//         std::vector<int> fdOps;
//         bool        isPasswordSet;
//         bool        isInviteOnlySet;
//         bool        _isOperator;
//         bool        isTopicSet;
//         std::vector<Client> clientsInvited;
//         std::string getName();
//         std::string getTopicNickSetter();
//         std::string getTopicUserSetter();
//         std::string getTopicTimestamp();
//         std::string getTopic();
//         std::string getKey();
//         std::string getMode();
//         std::string getOp();
//         int         getuserLimit();

//         std::vector<Client> getClients();
//         std::string getListClients();

//         void eraseInvitedClient(Client &client);

//         void eraseClient(Client &client);
//         void eraseChannel(std::string channelName);
//         void eraseOp(int fd);



//         bool isFdOperator(int fd);
//         bool isClientOnChannel(Client client);
//         bool isInviteOnly();
//         bool isNickInChannel(std::string nickname);
//         bool isKeySet();
//         bool isUserLimitSet();
//         bool isTopicRestrictionsSet();
//         bool isOperator(std::string nickname);
//         bool isModeSet(std::string mode);



//         void setName(std::string name);
//         void setTopic(std::string topic);
        
        
//         bool addClient(Client &c, char *pass);
//         bool partClient(Client &c, std::string reason);
//         bool quitClient(Client &c, std::string reason);
//         bool is_member(Client &c);


        

//         void addOperator(const std::string& nickname, std::string hostname, Client &client);
//         void removeOperator(const std::string& nickname, std::string hostName, Client &client);

//         void addInvited(Client &client);
//         bool isClientInvited(Client &client);
//         // Client& getClientByNick(std::string nickname);

//         // static void parseChannelMessage(char *msg, Client client);

//         Channel(std::string name, char *pass, Client &client, IRCserv *serv);
//         ~Channel();
// };