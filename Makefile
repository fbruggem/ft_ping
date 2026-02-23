CC = clang

SRC = main.c parse.c send.c icmp.c print.c
OBJ = $(SRC:.c=.o)

NAME = ft_ping

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(OBJ) -o $(NAME)

install:
	mkdir -p  $(DESTDIR)/bin
	cp $(NAME) $(DESTDIR)/bin

%.o: %.c
	$(CC) -c $<


clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)
