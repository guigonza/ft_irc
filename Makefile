# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: Guille <Guille@student.42.fr>              +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/02/08 07:53:27 by Guille            #+#    #+#              #
#    Updated: 2026/03/23 12:00:39 by Guille           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = 	ircserv
BONUS_NAME = ircserv_bonus
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 
DEPFLAGS = -MMD -MP
SRCS =  main.cpp Server.cpp Client.cpp Channel.cpp
BONUS_SRC = $(SRCS) Bot.cpp
OBJDIR = obj
OBJS = $(addprefix $(OBJDIR)/, $(SRCS:.cpp=.o))
BONUS_OBJ = $(addprefix $(OBJDIR)/, $(BONUS_SRC:.cpp=.o))
DEPS = $(OBJS:.o=.d) $(BONUS_OBJ:.o=.d)

all: $(NAME)

$(OBJDIR):
	mkdir -p $(OBJDIR)
$(NAME): $(OBJDIR) $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

bonus: $(OBJDIR) $(BONUS_OBJ)
	$(CXX) $(CXXFLAGS) -o $(BONUS_NAME) $(BONUS_OBJ)
	
$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

-include $(DEPS)
clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME) $(BONUS_NAME)

re: fclean all

.PHONY: all clean fclean re bonus
