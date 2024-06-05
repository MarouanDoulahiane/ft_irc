SRC		=		src/server/Server.cpp	\
				src/server/Client.cpp	\
				src/channels/utils.cpp \
				src/channels/channels.cpp \
				src/channels/Invite.cpp \
				src/channels/Topic.cpp \
				src/channels/JOIN.cpp \
				src/channels/modeChannels.cpp \
				src/channels/PRVmsg.cpp \
				src/channels/Kick.cpp \
				src/channels/PartQuit.cpp \
				src/main.cpp \


OBJ		=		$(SRC:.cpp=.o)

NAME	=		ircserv

CXX		=		c++

CXXFLAGS	=		-Wall -Wextra -Werror  -std=c++98 -I./include -g3 -fsanitize=address

HEADER	=		headers/Server.hpp \
				headers/Client.hpp\
				headers/channels.hpp\

all:		$(NAME)

$(NAME):	$(OBJ)
		$(CXX) -o $(NAME) $(OBJ) $(CXXFLAGS)

%.o:		%.cpp $(HEADER)
		$(CXX) -c -o $@ $< $(CXXFLAGS)

clean:
		rm -f $(OBJ)

fclean:		clean
		rm -f $(NAME)
	
re:		fclean all

run: all
	./$(NAME) 8000 xxx

.PHONY:		all clean fclean re