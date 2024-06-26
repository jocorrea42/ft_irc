NAME		=	ircserv
CPPFLAGS	=	-I $(INC_DIR) -Wall -Wextra -Werror -std=c++98 -MMD
SRC_DIR		=	src/
INC_DIR		=	./include/
OBJ_DIR		=	obj/
SRC			=	$(wildcard $(SRC_DIR)*.cpp)
OBJ			=	$(patsubst $(SRC_DIR)%.cpp,$(OBJ_DIR)%.o,$(SRC))
DEP			=	$(patsubst $(SRC_DIR)%.cpp,$(OBJ_DIR)%.d,$(SRC))
CC			=	c++
RM			=	rm -rf

################################# COLORS ########################################

DEL_LINE =		\033[2K
ITALIC =		\033[3m
BOLD =			\033[1m
DEF_COLOR =		\033[0;39m
GRAY =			\033[0;90m
RED =			\033[0;91m
GREEN =			\033[0;92m
YELLOW =		\033[0;93m
BLUE =			\033[0;94m
MAGENTA =		\033[0;95m
CYAN =			\033[0;96m
WHITE =			\033[0;97m
BLACK =			\033[0;99m
ORANGE =		\033[38;5;209m
BROWN =			\033[38;2;184;143;29m
DARK_GRAY =		\033[38;5;234m
MID_GRAY =		\033[38;5;245m
DARK_GREEN =	\033[38;2;75;179;82m
DARK_YELLOW =	\033[38;5;143m

################################################################################

all: $(NAME)

$(OBJ_DIR)%.o : $(SRC_DIR)%.cpp | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

-include $(DEP)

$(NAME): $(OBJ) Makefile
	$(CC) $(CPPFLAGS) $(OBJ) -o $(NAME)
	@echo "$(GREEN)\nCreated ${NAME} ✓$(DEF_COLOR)\n"

clean:
	$(RM) $(OBJ_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean $(NAME)

.PHONY: all clean fclean re