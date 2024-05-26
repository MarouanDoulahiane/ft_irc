#pragma once

#include <iostream>
#include <cstring>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <map>
#include <fcntl.h>
#include <sstream>
#include <unistd.h>
#include "Client.hpp"
#include "channels.hpp"
#include "parsedReplies.hpp"
