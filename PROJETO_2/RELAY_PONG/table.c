#include "table.h"

WINDOW * message_win;
WINDOW * my_win;
int paddle_x=0, paddle_y=0, ball_x=0, ball_y=0;

/*******************************************************************************************************
 * Struct: ball_position_t - stores the coordinates of the ball and the character used to draw the ball
 * *****************************************************************************************************/

typedef struct ball_position_t{
    int x;
    int y;
    int up_hor_down; //  -1 up, 0 horizontal, 1 down
    int left_ver_right; //  -1 left, 0 vertical,1 right
    char c; 
}ball_position_t;

/*******************************************************************************************************
 * Struct: paddle_position_t - stores the coordinates of the paddle and it's length
 * *****************************************************************************************************/

typedef struct paddle_position_t{
    int x;
    int y;
    int length;
}paddle_position_t;

struct ball_position_t ball;
struct paddle_position_t paddle;

/*****************************************************************************
 * Validates the IPv4 only if it has the syntax: byte.byte.byte.byte
 * parameters:
 * - IPv4 -> IPv4 to be validated.
 * return: Bollean wheter the IPv4 is valid or not.
*******************************************************************************/
bool check_IP(char *IPv4)
{
    int a = -1, b = -1, c = -1, d = -1;
    sscanf(IPv4, "%d.%d.%d.%d", &a, &b, &c, &d);
    if (0 <= a && a <= 255 && 0 <= b && b <= 255 && 0 <= c && c <= 255 && 0 <= d && d <= 255)
        return true;

    fprintf(stderr, "IP: \"%s\" is not valid.\n", IPv4);
    return false;
}

/***************************************************************
 * Validates the port only if it in the range (4000:40000)
 * parameters:
 * - port -> port to be validated.
 * return: Bollean wheter the port is valid or not.
***************************************************************/
bool check_port(char *port)
{
    int port_int = atoi(port);
    if (4000 <= port_int && port_int <= 40000)
        return true;

    fprintf(stderr, "Port: \"%s\" is not valid.\n", port);
    return false;
}


/**************************************************************************************************************
 * Function: new_paddle
 * parameters: 
 * - int length - length of the paddle to be created
 * return: none
 * Porpuse: Creating a new paddle 
 * *************************************************************************************************************/

void new_paddle (int length){
    paddle.x = WINDOW_SIZE/2;
    paddle.y = WINDOW_SIZE-2;
    paddle.length = length;
}

/********************************************************************************************************
 * Function: draw_paddle
 * parameters: 
 * - int delete - true in order to draw paddle or false in order to erase paddle
 * - int previous - 1 if the change is to the previous coordinates 0 if it's to the updated ones
 * return: none
 * Porpuse: Drawing the paddle using the character '_'
 * Notes: Uses the structure paddle_position_t that stores the information about the paddle
 * ******************************************************************************************************/

void draw_paddle(int delete, int previous){
    int ch;
    int start_x=0, end_x=0, height=0;
    
    if(delete){
        ch = '_';
    }else{
        ch = ' ';
    }

    if(previous==0){
        start_x = paddle.x - paddle.length;
        end_x = paddle.x + paddle.length;
        height = paddle.y;
    }else if(previous==1){
        start_x = paddle_x - paddle.length;
        end_x = paddle_x + paddle.length;
        height = paddle_y;
    }

    for (int x = start_x; x <= end_x; x++){
        wmove(my_win, height, x);
        waddch(my_win,ch);
    }
    wrefresh(my_win);
}

/**************************************************************************************************************
 * Function: move_paddle
 * parameters: 
 *  - int direction - variable that stores the client's pressed key 
 * return: void
 * Porpuse: Allows for the paddle to move in the direction selected by the user
 * *************************************************************************************************************/

void moove_paddle (int direction){

    //atualizar variavel global
    paddle_x = paddle.x;
    paddle_y = paddle.y;

    int start_x = paddle.x - paddle.length;
    int end_x = paddle.x + paddle.length;

    if ((direction == KEY_UP) && (paddle.y != 1)){
        if ((ball.y == paddle.y - 1) && ((ball.x >= start_x) && (ball.x <= end_x))){
            mvwprintw(message_win, 2,1,"\rYou scored one point!\n");
            wrefresh(message_win);
        }else{
            paddle.y --; //subir
        }
    }
    if ((direction == KEY_DOWN) && (paddle.y != WINDOW_SIZE-2)){
        if ((ball.y == paddle.y + 1) && ((ball.x >= start_x) && (ball.x <= end_x))){
            mvwprintw(message_win, 2,1,"\rYou scored one point!\n");
            wrefresh(message_win);
        }else{
            paddle.y ++;
        }
    }
    if ((direction == KEY_LEFT) && (paddle.x - paddle.length != 1)){
        if ((ball.y == paddle.y) && ((ball.x >= start_x - 1) && (ball.x <= end_x - 1))){
            mvwprintw(message_win, 2,1,"\rYou scored one point!\n");
            wrefresh(message_win);
        }else{
            paddle.x --;
        }
    }
    if((direction == KEY_RIGHT) && (paddle.x + paddle.length != WINDOW_SIZE-2)){
        if ((ball.y == paddle.y) && ((ball.x >= start_x + 1) && (ball.x <= end_x + 1))){
            mvwprintw(message_win, 2,1,"\rYou scored one point!\n");
            wrefresh(message_win);
        }else{
            paddle.x ++;
        }
    }
}

/**************************************************************************************************************
 * Function: place_ball_random
 * parameters: none
 * return: none
 * Porpuse: Placing the ball in a random position in the game window
 * *************************************************************************************************************/

void place_ball_random(){

    int aux=-1;
    while(aux==-1){
        ball.x = rand() % WINDOW_SIZE;
        ball.y = rand() % WINDOW_SIZE;
        if(ball.x == 0 || ball.x == WINDOW_SIZE-1 || ball.y == 0 || ball.y == WINDOW_SIZE-1){
            aux = -1;
        }else aux = 1;
    }
    ball.c = 'o';
    ball.up_hor_down = rand() % 3 -1; //  -1 up, 1 - down
    ball.left_ver_right = rand() % 3 -1 ; // 0 vertical, -1 left, 1 right
}

/**************************************************************************************************************
 * Function: moove_ball
 * parameters: none 
 * return: none
 * Porpuse: Mooving the ball in a random pattern
 * Notes: The function accounts for the ball bouncing off the paddle and the walls
 * *************************************************************************************************************/

void moove_ball(){

    //atualizar variaveis globais
    ball_x = ball.x;
    ball_y = ball.y;
    
    int next_x = ball.x + ball.left_ver_right;
    int next_y = ball.y + ball.up_hor_down;
    int start_x = paddle.x - paddle.length;
    int end_x = paddle.x + paddle.length;
    
    if(next_x >= start_x - 1 && next_x <= end_x + 1){ 
        if(next_y == paddle.y + 1){
            ball.up_hor_down = 1;
            ball.left_ver_right = rand() % 3 -1;
            mvwprintw(message_win, 2,1,"\rYou scored one point!\n");
            wrefresh(message_win);
        }else if (next_y == paddle.y - 1){
            ball.up_hor_down = -1;
            ball.left_ver_right = rand() % 3 -1;
            mvwprintw(message_win, 2,1,"\rYou scored one point!\n");
            wrefresh(message_win);
        }
    }

    if( next_x == 0 || next_x == WINDOW_SIZE-1){
        ball.up_hor_down = rand() % 3 -1 ;
        ball.left_ver_right *= -1;
        mvwprintw(message_win, 2,1,"\rleft right window\n");
        wrefresh(message_win);
     }else{
        ball.x = next_x;
    }

    if( next_y == 0 || next_y == WINDOW_SIZE-1){
        ball.up_hor_down *= -1;
        ball.left_ver_right = rand() % 3 -1;
        mvwprintw(message_win, 2,1,"\rbottom top window\n");
        wrefresh(message_win);
    }else{
        ball.y = next_y;
    }

}

/**************************************************************************************************************
 * Function: draw_ball
 * parameters: 
 *  - int draw - true in order to draw the element and false in order to erase the element
 * return: none
 * Porpuse: Drawing the ball using a 'o' - the ball's character
 * *************************************************************************************************************/

void draw_ball(int delete, int previous){
    int ch, x = 0, y = 0;

    if(delete){
        ch = ball.c;
    }else{
        ch = ' ';
    }

    if(previous==0){
        x = ball.x;
        y = ball.y;    
    }else if(previous==1){
        x = ball_x;
        y = ball_y;
    }
    
    wmove(my_win, y, x);
    waddch(my_win,ch);
    wrefresh(my_win);
}

/**************************************************************************************************************
 * Function: create_window
 * parameters: none
 * return: none
 * Porpuse: Drawing the window of the game and a comments windom
 * Notes: Initializes curses mode allowing the use of functions from the lncurses library 
 * (wrefresh, newwin, wmvwprintw, ...)
 * *************************************************************************************************************/

void create_window(){

	initscr();		    	/* Start curses mode 		*/
	cbreak();				/* Line buffering disabled	*/
    keypad(stdscr, TRUE);   /* We get F1, F2 etc..		*/
	noecho();			    /* Don't echo() while we do getch */

    /* creates a window and draws a border */
    my_win = newwin(WINDOW_SIZE, WINDOW_SIZE, 0, 0);
    box(my_win, 0 , 0);	
	wrefresh(my_win);
    keypad(my_win, true);
    /* creates a window and draws a border */
    message_win = newwin(10, WINDOW_SIZE+20, WINDOW_SIZE, 0);
    box(message_win, 0 , 0);	
	wrefresh(message_win);
}

/**************************************************************************************************************
 * Function: get_key
 * parameters: none
 * return: 
 *  - key -> key pressed by the user
 * *************************************************************************************************************/

int get_key(){
    int key = wgetch(my_win);
    return key;
}
/**************************************************************************************************************
 * Function: decifer_message
 * parameters: 
 *  - char * message - message received from the server
 * return: none
 * Porpuse: Decifering the move ball message received from the server, storing the coordinates of the ball in the
 * respective structure
 * *************************************************************************************************************/

void decifer_message(char * message){
        sscanf(message, "%*s %d %d %c", &ball.x, &ball.y , &ball.c);
        draw_ball(true, 0);
}

/**************************************************************************************************************
 * Function: send_move_ball
 * parameters: 
 * - int sock_fd -> server socket
 * - char*SERVER_IP -> server IP adress
 * - int SERVER_PORT -> server port
 * return: none
 * Porpuse: Sends the new coordinates of the ball to the server
 * *************************************************************************************************************/

void send_move_ball(int sock_fd, char*SERVER_IP, int SERVER_PORT){

    char message[BUFFER_SIZE], message_type[BUFFER_SIZE];
	memset(message,'\0', BUFFER_SIZE);
    memset(message_type,'\0', BUFFER_SIZE);

	struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    if( inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) < 1){
		printf("no valid address: \n");
		exit(-1);
	}   
	int err=0;

    strcpy(message_type, "Move_ball");
    sprintf(message, "%s %d %d %c \n", message_type, ball.x, ball.y , ball.c);
    err = sendto(sock_fd, &message, sizeof(message), 0, (const struct sockaddr *) &server_addr, sizeof(server_addr));
    if (err == -1){
        perror("send to");
    }
}

/**************************************************************************************************************
 * Function: printing_messages
 * parameters: 
 *  - int n - 1 if the player is in play mode 2 if it's in watch
 * return: none
 * Porpuse: Printing messages to the message window clarifying the state in which the user is
 * *************************************************************************************************************/


void printing_messages(int n){
    if (n == 1){
        mvwprintw(message_win, 3,1,"\rYOU ARE IN A BALL CONTROLLING STATE\n");
    }else if (n == 2){
        mvwprintw(message_win, 3,1,"\rYOU ARE IN A WATCHING STATE\n");
    }
    wrefresh(message_win);
}

