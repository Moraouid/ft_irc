NAME = ircserv
CXX = c++
CXXFLAGS = -Werror -Wextra -Wall -std=c++98 -Iinclude
SRCS = src/main.cpp \
	   src/server.cpp \
	   src/client.cpp \
	   src/channel.cpp \
	   src/command.cpp \
	   src/irc_utils.cpp \
	   src/Bot.cpp

OBJS = $(SRCS:.cpp=.o)
RM = rm -f


$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $(NAME) $(OBJS)

all: $(NAME)

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re