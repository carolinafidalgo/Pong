#ifndef TABLE_CLIENT_H
#define TABLE_CLIENT_H

#define WINDOW_SIZE 20
#define PADDLE_SIZE 2
#define BUFFER_SIZE 500

#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "table_client.h"

bool check_IP(char *IPv4);

bool check_port(char *port);

void draw_my_paddle(int delete);

void draw_others_paddle(int delete);

void moove_paddle(int direction);

void draw_ball(int draw);

void create_window(int window);

int play_state(int sock_fd);

void decifer_board(char * message, int window);

void notice(int n);

#endif