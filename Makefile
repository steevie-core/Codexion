NAME = codexion

SRCS =	coder_journey.c\
		get_dongle.c\
		let_dongle.c\
		inits.c\
		monitor_journey.c\
		parser.c\
		heap_op.c\
		schedulers.c\
		numeric_parser.c\
		main.c\

OBJ = $(SRCS:.c=.o)

CC = cc
FLAGS = -Wall -Werror -Wextra -pthread

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(FLAGS) -o $@ $^

%.o: %.c
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re