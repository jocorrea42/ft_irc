NAME_BOUNUS = bot
NAME = ircserv
CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98
INCLUDE = -Iinclude/Server.hpp include/Channel.hpp include/Client.hpp
SRCS = src/Server.cpp src/Client.cpp src/Channel.cpp src/main.cpp 

SRCS_BONUS = 

OBJS = $(SRCS:.cpp=.o)
OBJS_BONUS = $(SRCS_BONUS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	@$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

bonus: $(OBJS_BONUS)
	@$(CC) $(CFLAGS) -o $(NAME_BOUNUS) $(OBJS_BONUS)

%.o: %.cpp $(INCLUDE)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS) $(OBJS_BONUS)

fclean: clean
	@rm -f $(NAME) $(NAME_BOUNUS)

re: fclean all

.PHONY: all clean fclean re