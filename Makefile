CC = clang

SRC = main.c parse.c send.c icmp.c print.c loop.c calc.c recv.c
OBJ = $(SRC:.c=.o)

NAME = ft_ping

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(OBJ) -fsanitize=undefined -O0 -o $(NAME)

install:
	mkdir -p  $(DESTDIR)/bin
	cp $(NAME) $(DESTDIR)/bin

%.o: %.c
	$(CC) -c $<


clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)
