#ifndef TABLE_SERVER_H
#define TABLE_SERVER_H

#define WINDOW_SIZE 20
#define PADDLE_SIZE 2
#define BUFFER_SIZE 500
#define SERVER_ADDR "127.0.0.1\0"
#define SOCK_PORT 5001

#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "table_server.h"

typedef struct ClientAddress client_address; 

client_address* createNode(int remote_port, char *remote_addr_str, int id, int client_fd);

client_address *insert_node(client_address **Head, client_address **newNode);

void remove_node(client_address** head_ref, int port, char* addr);

void new_paddle (client_address* node);

void place_ball_random();

void moove_ball(client_address** node);

void board_update(client_address** head, char* message);

void update_struct(int sock_fd, client_address* Head, char *client_addr_str, int client_port, char *message);

void move_update(client_address** Head, char *client_addr_str, int client_port, char *message);

void search_node(client_address** Head, char *client_addr_str, int *client_port, int client_fd);



#endif