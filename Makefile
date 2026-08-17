NAME = ft_irc
CXX = c++

CXXFLAGS = -Wall -Wextra -Werror -std=c++98
SRCS = main.cpp server.cpp client.cpp channel.cpp irc_utils.cpp

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