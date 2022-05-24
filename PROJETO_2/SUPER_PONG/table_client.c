#include "table_client.h"

//Declaration of global variables 
WINDOW * message_win;
WINDOW * my_win;
int id_paddle=0, id_score=0, len_paddle=0, x_paddle=0, y_paddle=0, x_ball=0, y_ball=0;
char c_ball;

/*************************************************************
 * Struct: paddle_position_t - stores client's information
 * - The client's own paddle coordinates 
 * - The length of the paddle
 * - The client's id and score
 * ************************************************************/

typedef struct paddle_position_t{
    int x;
    int y;
    int length;
    int id;
    int score;
}paddle_position_t;

struct paddle_position_t paddle;

/*****************************************************************************
 * Validates the IPv4 only if it has the syntax: byte.byte.byte.byte
 * Input: char *IPv4 - IP to be validated.
 * Output: Bollean wheather the IPv4 is valid or not.
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

/***************************************************************************
 * Validates the port only if it in the range (4000:40000)
 * Input: char *port - Port to be validated.
 * Output: Bollean wheather the port is valid or not.
*****************************************************************************/
bool check_port(char *port)
{
    int port_int = atoi(port);
    if (4000 <= port_int && port_int <= 40000)
        return true;

    fprintf(stderr, "Port: \"%s\" is not valid.\n", port);
    return false;
}

/********************************************************************************************************
 * Function: draw_my_paddle
 * Input: int delete - true in order to draw paddle or false in order to erase paddle
 * Output: void
 * Porpuse: Drawing the client's own paddle using a '=' 
 * Notes: Uses the structure paddle_position_t that stores the information about the client's own paddle
 * ******************************************************************************************************/

void draw_my_paddle(int delete){

    int ch;
    if(delete){
        ch = '=';
    }else{
        ch = ' ';
    }
    int start_x = paddle.x - paddle.length;
    int end_x = paddle.x + paddle.length;
    for (int x = start_x; x <= end_x; x++){
        wmove(my_win, paddle.y, x);
        waddch(my_win,ch);
    }
    wrefresh(my_win);
}

/***************************************************************************************************************
 * Function: draw_others_paddle
 * Input: int delete - true in order to draw paddle or false in order to erase paddle
 * Output: void
 * Porpuse: Drawing the other clients' paddle using a '_' 
 * Notes: Uses variables that contains the coordinates of the paddle of other clients, decifered from the board 
 * update message
 * *************************************************************************************************************/

void draw_others_paddle(int delete){

    int ch;
    if(delete){
        ch = '_';
    }else{
        ch = ' ';
    }
    int start_x = x_paddle - len_paddle;
    int end_x = x_paddle + len_paddle;
    for (int x = start_x; x <= end_x; x++){
        wmove(my_win, y_paddle, x);
        waddch(my_win,ch);
    }
    wrefresh(my_win);
}

/******************************************************************************************************************
 * Function: moove_paddle
 * Input: int direction - variable that stores the client's pressed key 
 * Output: void
 * Porpuse: Allows for the paddle to move in the direction selected by the used
 * Notes: Accounts for blocking the paddle's movement if the ball is the in the position of the intended movement
 * ****************************************************************************************************************/


void moove_paddle (int direction){

    int start_x = paddle.x - paddle.length;
    int end_x = paddle.x + paddle.length;

    if ((direction == KEY_UP) && (paddle.y != 1)){
        if ((y_ball == paddle.y - 1) && ((x_ball >= start_x) && (x_ball <= end_x))){
            paddle.score++;
        }else{
            paddle.y --; 
        }
    }
    if ((direction == KEY_DOWN) && (paddle.y != WINDOW_SIZE-2)){
        if ((y_ball == paddle.y + 1) && ((x_ball >= start_x) && (x_ball <= end_x))){
            paddle.score++;
        }else{
            paddle.y ++;
        }
    }
    if ((direction == KEY_LEFT) && (paddle.x - paddle.length != 1)){
        if ((y_ball == paddle.y) && ((x_ball >= start_x - 1) && (x_ball <= end_x - 1))){
            paddle.score++;
        }else{
            paddle.x --;
        }
    }
    if((direction == KEY_RIGHT) && (paddle.x + paddle.length != WINDOW_SIZE-2)){
        if ((y_ball == paddle.y) && ((x_ball >= start_x + 1) && (x_ball <= end_x + 1))){
            paddle.score++;
        }else{
            paddle.x ++;
        }
    }
}


/**************************************************************************************************************
 * Function: draw_ball
 * Input: int draw - true in order to draw the element and false in order to erase the element
 * Output: void
 * Porpuse: Drawing the ball using a 'o' 
 * *************************************************************************************************************/

void draw_ball(int draw){
    
    int ch;
    if(draw){
        ch = c_ball;
    }else{
        ch = ' ';
    }
    wmove(my_win, y_ball, x_ball);
    waddch(my_win,ch);
    wrefresh(my_win);
}

/**************************************************************************************************************
 * Function: create_window
 * Input: int window - 0 in order to draw 2 windows(game window and comments window) or 1 in order to draw 1 
 * window (game window)
 * Output: void
 * Porpuse: Drawing the window of the game and a comments windom (when necessary)
 * Notes: Initializes curses mode allowing the use of functions from the lncurses library 
 * (wrefresh, newwin, wmvwprintw, ...)
 * *************************************************************************************************************/

void create_window(int window){

	initscr();		    	/* Start curses mode 		*/
	cbreak();				/* Line buffering disabled	*/
    keypad(stdscr, TRUE);   /* We get F1, F2 etc..		*/
	noecho();			    /* Don't echo() while we do getch */

    // creates a game window and draws a border
    my_win = newwin(WINDOW_SIZE, WINDOW_SIZE, 0, 0);
    box(my_win, 0 , 0);	
	wrefresh(my_win);
    keypad(my_win, true);

    if(window == 0){
    /* creates a comments window and draws a border  
     * only drawn once at the start of the program after connecting to the server*/
        message_win = newwin(10, WINDOW_SIZE+10, WINDOW_SIZE, 0);
        box(message_win, 0 , 0);	
        wrefresh(message_win);
    }
}

/**************************************************************************************************************
 * Function: play_state
 * Input: int sock_fd - refers to the used socket
 * Output: int q - returns 1 if the client wants to disconnect (clicks 'q')
 * Porpuse: Allows for the client to press a arrow key in order to move its paddle and play the game
 * Notes: The client keeps playing until the client presses 'q' to disconnect
 * *************************************************************************************************************/

int play_state(int sock_fd){ 

    //Declaration of variables
    char message[BUFFER_SIZE], message_type[BUFFER_SIZE];
    int err=0, key=-1;

    while(1){
        //The user inputs a arrow key throught the keyboard
        key = wgetch(my_win);		
        if(key == KEY_LEFT || key == KEY_RIGHT || key == KEY_UP || key == KEY_DOWN){
            /*Erasing, moving and drawing client's own paddle after the coordinates change accordingly to key 
            pressed by the user*/
            moove_paddle(key);
            //Creating a paddle move message with the paddle coordinates and its length
            memset(message,'\0', BUFFER_SIZE);
            memset(message_type,'\0', BUFFER_SIZE);
            strcpy(message_type, "Paddle_move");
            sprintf(message, "%s %d %d %d %d \n", message_type, paddle.x, paddle.y, paddle.length, paddle.score);
            err = write(sock_fd, message, strlen(message));
            if (err == -1){
                perror("write paddle\n");
                exit(1);
            }
        } else if (key == 113){ //If the pressed key is a 'q' - 113 key in the ASCII table - user wants to be disconnected
            return 1;
        }
    }
    return 0;
}

/**************************************************************************************************************
 * Function: decifer_board
 * Input: 
 *  - char * message - message received from the server
 *  - int window - input variable of create_window function that allows the creation of the game window
 * Output: void
 * Porpuse: Decifering the board update message received from the server, receiving the coordinates of the ball, 
 * the client's own paddle coordinates, id and score as well as for the other clients
 * *************************************************************************************************************/

void decifer_board(char * message, int window){

    int num=1; //variable used in order to print the clients' scores on the comments window

    werase(my_win); //Erasing all elements in the game window
    create_window(window); //Creating a game window 

    //Extracting the ball's coordinates from the board update message
    sscanf(message, "%*s %d %d %c%[^\n]", &x_ball, &y_ball, &c_ball, message);
    draw_ball(true); //Drawing ball on the game window

    //Extracting the client's own paddle coordinates, id and score from the board update message
    sscanf(message, "%d %d %d %d %d%[^\n]", &paddle.id, &paddle.score, &paddle.x, &paddle.y, &paddle.length, message);
    draw_my_paddle(true); //Drawing paddle on the game window

    //Printing the client's own id and score
    mvwprintw(message_win, 1,1,"\rP%d: %d <----\n", paddle.id, paddle.score);

    while(strlen(message)>2){
        num++;
        //Extracting other clients' paddle coordinates, id and score from the board update message
        sscanf(message, "%d %d %d %d %d%[^\n]", &id_paddle, &id_score, &x_paddle, &y_paddle, &len_paddle, message);
        draw_others_paddle(true); //Drawing paddles on the game window

        //Printing the other clients' id and score
        mvwprintw(message_win, num,1,"\rP%d: %d\n", id_paddle, id_score);	
    }
    wrefresh(message_win);	

}
