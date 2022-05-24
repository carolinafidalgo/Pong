#compiler used
CC = gcc

CFLAGS = -Wall -Wextra -Wshadow -Wformat=2 -g

#executable name
EXECUTABLE_1 = server
EXECUTABLE_2 = client

SOURCES_SERVER = server_aux.c server.c
SOURCES_CLIENT = table.c client.c

OBJ_SERVER	=	$(SOURCES_SERVER:.c=.o)
OBJ_CLIENT	=	$(SOURCES_CLIENT:.c=.o)

$(EXECUTABLE_1):
	@clear
	$(CC) $(CFLAGS) $(SOURCES_SERVER) -lpthread -lncurses -o $(EXECUTABLE_1)	

$(EXECUTABLE_2):
	@clear
	$(CC) $(CFLAGS) $(SOURCES_CLIENT) -lpthread -lncurses -o $(EXECUTABLE_2)

clean_s:
	@clear
	rm $(EXECUTABLE_1)

clean_c:
	@clear
	rm $(EXECUTABLE_2)

#./client 127.0.0.1 5000
