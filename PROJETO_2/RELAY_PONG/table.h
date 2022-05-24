#ifndef TABLE_H
#define TABLE_H
#define WINDOW_SIZE 20
#define PADLE_SIZE 2
#define BUFFER_SIZE 1024

#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <time.h>
#include <pthread.h>
#include "table.h"

bool check_IP(char *IPv4);

bool check_port(char *port);

void new_paddle(int length);

void draw_paddle(int delete, int previous);

void moove_paddle(int direction);

void place_ball_random();

void moove_ball();

void draw_ball(int delete, int previous);

void create_window();

int get_key();

void decifer_message(char * message);

void send_move_ball(int sock_fd, char *SERVER_IP, int SERVER_PORT);

void printing_messages(int n);

#endif