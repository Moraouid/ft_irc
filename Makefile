NAME = ircserv
CXX = c++

CXXFLAGS = -Werror -Wextra -Wall -std=c++98
SRCS = main.cpp server.cpp client.cpp channel.cpp irc_utils.cpp Bot.cpp

OBJS = $(SRCS:.cpp=.o)
RM = rm -f

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

all: $(NAME)

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all
