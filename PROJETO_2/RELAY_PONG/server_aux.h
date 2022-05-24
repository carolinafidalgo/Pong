#ifndef SERVER_AUX
#define SERVER_AUX

#define WINDOW_SIZE 20
#define PADLE_SIZE 2
#define BUFFER_SIZE 1024
#define SERVER_ADDR "127.0.0.1\0"
#define SOCK_PORT 5005

#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include "server_aux.h"

typedef struct ClientAddress client_address;

client_address *createNode(int remote_port, char *remote_addr_str);

client_address *insert_node(client_address **Head, client_address **newNode);

void remove_node(client_address** Head, int port, char* addr);

void choose_player(client_address** Head, char* address, int* port, int number_players);

void send_move_ball(int sock_fd, client_address **Head, char* address, int port, char* message);

#endif