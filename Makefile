# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/08/01 11:53:48 by damachad          #+#    #+#              #
#    Updated: 2024/09/17 08:35:53 by damachad         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# /\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\ COLORS /\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\ #

RESET	= \033[0m
BLACK 	= \033[1;30m
RED 	= \033[1;31m
GREEN 	= \033[1;32m
YELLOW 	= \033[1;33m
BLUE	= \033[1;34m
PURPLE 	= \033[1;35m
CYAN 	= \033[1;36m
WHITE 	= \033[1;37m
SBLINK 	= \033[5m

# /\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\ COMMANDS /\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\ #
CXX = c++
RM = rm -rf

# /\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_ FLAGS _/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\ #

CXXFLAGS 	= -Wall -Werror -Wextra -Wshadow -pedantic -std=c++98 -g #-fsanitize=address

# /\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_ PATHS _/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\ #

INC			= includes
SRC_DIR		= srcs
OBJ_DIR		= objs

# /\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_ FILES _/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\ #

NAME 		= webserv
FILES 		= main Helpers ConfigParser Exceptions Cluster utils \
            LocationContext Server HTTPRequestParser AResponse \
			GetResponse RequestErrorResponse
SRC 		= $(addprefix $(SRC_DIR)/, $(addsuffix .cpp, $(FILES)))
OBJ 		= $(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(FILES)))

# /\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_ RULES _/\_/\_/\_/\_/\_/\_/\_/\_/\_/\_/\ #

all: $(NAME)

$(NAME): $(OBJ_DIR) $(OBJ)
	@$(CXX) $(CXXFLAGS) $(SRC) -o $(NAME) -I $(INC)
	@echo "[$(NAME) created]"

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp 
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) -c $< -o $@ -I $(INC)

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)
	@echo "[objs directory created]"

clean:
	@$(RM) $(OBJ_DIR)
	@echo "[objs directory removed]"

fclean: clean
	@$(RM) $(NAME)
	@echo "[$(NAME) removed]"

re: fclean all

test: all
	@./$(NAME)

client:
	c++ srcs/client.cpp -o client

reclient:
	rm client
	c++ srcs/client.cpp -o client

.PHONY: all clean fclean re
