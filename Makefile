SRC		=		src/server/Server.cpp	\
				src/server/Client.cpp	\
				src/main.cpp \


OBJ		=		$(SRC:.cpp=.o)

NAME	=		ircserv

CXX		=		c++

CXXFLAGS	=		-Wall  -std=c++98 -I./include -g3 -fsanitize=address

HEADER	=		headers/server.hpp \
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