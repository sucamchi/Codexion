NAME = codexion
CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread

SRCDIR = src
OBJDIR = obj

SRCS = $(SRCDIR)/main.c \
       $(SRCDIR)/simulation.c \
       $(SRCDIR)/coder.c \
       $(SRCDIR)/dongle.c \
       $(SRCDIR)/heap.c \
       $(SRCDIR)/monitor.c \
       $(SRCDIR)/utils.c

OBJS = $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

all: $(NAME)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(SRCDIR)/codexion.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
