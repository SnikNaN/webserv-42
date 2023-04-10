NAME = webserv
CPP = c++
FLAGS = -Wall -Wextra -Werror -std=c++98
FLAGS += -MMD -MP
INC = network parsers parsing process utils
INC_PARAMS = $(addprefix -I,$(INC))
SRCS = main.cpp network/Server.cpp	network/SocketHolder.cpp	\
				parsers/ConfigParser.cpp	parsers/HttpReqHeader.cpp	\
				parsing/CfgCtx.cpp	process/Handler.cpp
OBJS = $(SRCS:.cpp=.o)

%.o:	%.cpp	Makefile
	$(CPP) $(FLAGS) $(INC_PARAMS) -c $< -o $@

all:	$(NAME)

$(NAME):	$(OBJS)
	$(CPP) $(FLAGS) $(OBJS) -o $(NAME)
	@$(MAKE) -C www/

clean:
	@rm -rf $(OBJS)
	@rm -rf $(SRCS:.cpp=.d)

fclean:	clean
	@rm -f $(NAME)

re:	fclean all

val:	all
	valgrind --leak-check=full \
	--suppressions=.valgrind-supression\
	--show-leak-kinds=all \
	--track-origins=yes \
	--verbose \
	--log-file=valgrind-out.txt \
	./$(NAME)

-include $(SRCS:.cpp=.d)

.PHONY: all clean fclean val re