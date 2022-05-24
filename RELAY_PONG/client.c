#include "table.h"

/*****************************************************************************************************************************
 * This file is the client that is controlled by the user to play the game. It is managed by a state machine with
 * two different states. The states will be explained in detail ahead.
 * watch - The client is connect but doesn't have the ball, it's just watching the ball from the other clients game move
 * play - The client has the ball and is playing (moves the paddle)
 ****************************************************************************************************************************/

/******Global variables shared by all threads******/
int flag = 0; //controls the actions within the state machine
pthread_mutex_t mux_flag, mux_curses; //mutexes used in different threads to so there is no conflict when using ncurses or updating the flag
char SERVER_IP[BUFFER_SIZE]; //server IP entered in the terminal
int SERVER_PORT=0; //server port entered in the terminal

/*****************************************************************************************************************************
* This thread is responsible for continuously listening to the server socket and controling the following client's action
* according to the message received. This control is done by updating the value of the flag so the main thread can update
* the state machine status
* parameters: 
* - sock_fd -> socket of the server
* return: none
 ****************************************************************************************************************************/

void *rcv_message(void * arg){

    //Declaration of variables
    char message[BUFFER_SIZE], message_type[BUFFER_SIZE];
    int err = 0, n=0;
    int sock_fd = *(int *) arg;

    while(1){
        //receiving the incoming message
        memset(message,'\0', BUFFER_SIZE);
        memset(message_type,'\0', BUFFER_SIZE);
        err = recv(sock_fd, message, 100, 0);
        if (err == -1){
            perror("rcv");
        }
        //isolating the message type
        for(n = 0; n < (int)strlen(message); n++){
            if(message[n] != ' '){
                message_type[n] = message[n];
            }   
        }
        //in case the message is Move_ball
        if (strcmp(message_type, "Move_ball")==0){
            //delete the ball on the screen and draw it in the new position
            pthread_mutex_lock(&mux_curses);
            draw_ball(false, 0);
            decifer_message(message);
            pthread_mutex_unlock(&mux_curses);
        //in case the message is Send_ball
        }else if(strcmp(message_type, "Send_ball") == 0){
            //clear the window on the screen
            create_window();
            pthread_mutex_lock(&mux_flag);
            flag = 1; //tells the main thread that a Send_ball was received
            pthread_mutex_unlock(&mux_flag);
        //in case the message is Release_ball
        }else if(strcmp(message_type, "Release_ball") == 0){
            //clear the window on the screen
            create_window();
            pthread_mutex_lock(&mux_flag);
            flag = 3; //tells the main thread that a Release_ball was received
            pthread_mutex_unlock(&mux_flag);
        }
    }
}

/*****************************************************************************************************************************
* This thread is responsible for continuously listening to the keyboard socket to check if the user wants to quit
* the game and updating the value of the flag so the main thread can update the state machine status
* parameters: none
* return: none    
 ****************************************************************************************************************************/

void *disconnect(){

    char key;
    while(1){
        //reading the characters typed
        key = getchar();
        if(key == 'q' || key == 'Q'){
            pthread_mutex_lock(&mux_flag);
            flag = 2; //tells the main thread that a 'quit' was received
            pthread_mutex_unlock(&mux_flag);
        }
    }
}

/*****************************************************************************************************************************
* This thread is responsible for updating the position of the ball every second, dawing in on the screen of the client
* in playing state and sending the new position to the server
* parameters: 
* - sock_fd -> socket of the server
* return: no return 
 ****************************************************************************************************************************/

void *calculate_ball(void * arg){

    int sock_fd = *(int *) arg;
    
    while(1){
        moove_ball(); //calculate the new position
        pthread_mutex_lock(&mux_curses);
        draw_ball(false, 1); //delete the last ball from the screen
        draw_ball(true, 0); //draw the ball on the new coordinates
        pthread_mutex_unlock(&mux_curses);
        send_move_ball(sock_fd, SERVER_IP, SERVER_PORT); //send the new coordinated to the server
        sleep(1); //wait a second
    }
}

/*****************************************************************************************************************************
* This thread is responsible for keeping the active game running. It reads from the keyboard and moves the paddle accordingly
* parameters: none
* return: none    
 ****************************************************************************************************************************/

void *play_paddle(){

    pthread_mutex_lock(&mux_curses);
    draw_paddle(false, 0); //erase the previous paddle
    new_paddle(PADLE_SIZE); //set the coordinates for the new paddle
    draw_paddle(true, 0); //draw the new paddle
    pthread_mutex_unlock(&mux_curses);

    int key = -1;
    while(1){
        key = get_key(); //get the key pressed
        if (key == KEY_LEFT || key == KEY_RIGHT || key == KEY_UP || key == KEY_DOWN){
            moove_paddle(key); //move the paddle according to the key pressed
            pthread_mutex_lock(&mux_curses);
            draw_paddle(false, 1); //erase the previous paddle
            draw_paddle(true, 0); //draw the new paddle
            pthread_mutex_unlock(&mux_curses);
        }
        //if the user wants to quit
        if(key == 113){
            pthread_mutex_lock(&mux_flag);
            flag = 4; //tells the main thread that the user wants to quit the game
            pthread_mutex_unlock(&mux_flag);
        }
    }
}

int main(int argc, char *argv[]){

    if(argc !=3){
        printf("Three arguments expected - executable and server address and port\n");
        return 0;
    }
    
    //check if the choosen IP is valid and store it in the global variable
    if (check_IP(argv[1])){
        strcpy(SERVER_IP,argv[1]);
    }else{
        printf("IP is not valid\n");
        return 0;
    }

    //check if the choosen port is valid and store it in the global variable
    if (check_port(argv[2])){
        SERVER_PORT = atoi(argv[2]);
    }else{
        printf("Port is not valid\n");
        return 0;
    }

    //Declaration of the client's states 
    enum
    {
        watch,
        play
    } state;
    state = watch;

    //Declaration of variables
    char message[BUFFER_SIZE], message_type[BUFFER_SIZE];
    int err = 0;

    //Initialization of message and message type
    memset(message,'\0', BUFFER_SIZE);
    memset(message_type,'\0', BUFFER_SIZE);

    //Creating socket
	int sock_fd = socket(AF_INET, SOCK_DGRAM, 0); 
    if (sock_fd == -1){
        perror("socket creation");
        exit(-1);
    }

    //Declaration of server adress 
    struct sockaddr_in server_addr;
    socklen_t server_addr_size = sizeof(struct sockaddr_in);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT); 

    if(inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) < 1){
		printf("no valid address: \n");
		exit(-1);
	}
    
    //Sending connect message to server
    strcpy(message_type, "Connect \n");
    sprintf(message, "%s", message_type);
    err = sendto(sock_fd, message, strlen(message)+1, 0, (const struct sockaddr *) &server_addr, server_addr_size);
    if (err == -1){
        perror("sendto \n");
    }

    create_window();
    
    //initialize the mutexes
    pthread_t th_messages;
    pthread_t th_keyboard;
    pthread_mutex_init(&mux_flag, NULL);
    pthread_mutex_init(&mux_curses, NULL);

    //create the thread for receiving messages from the server
    int * th_socket = malloc(sizeof(int));
    *th_socket = sock_fd;
    pthread_create(&th_messages, NULL, rcv_message, th_socket);

    state = watch;

while(1){

    switch(state){
        case watch:
            printing_messages(2); //let the user know he is on watching state
            pthread_create(&th_keyboard, NULL, disconnect, NULL);
            while(1){
                if(flag == 1){ //received Send_ball
                    pthread_cancel(th_keyboard);
                    state = play; //goes to play state
                    break;
                }else if(flag == 2){ //received quit
                    pthread_cancel(th_keyboard);
                    pthread_cancel(th_messages);
                    //sending disconnect message to server
                    memset(message, '\0', BUFFER_SIZE);
                    strcpy(message, "Disconnect \n");
                    err = sendto(sock_fd, message, strlen(message)+1, 0, (const struct sockaddr *) &server_addr, server_addr_size);
                    if (err == -1){
                        perror("sendto \n");
                    }
                    //close the server socket
                    if(close(sock_fd) == -1){
                        perror("close");
                    }
                    return 0;
                }
            }

        case play: 

            printing_messages(1); //let the user know he is on playing state
            draw_paddle(false, 0); //erase the paddle
            draw_ball(false, 0); //erase the ball
            place_ball_random(); //create new coordinates for the ball

            //creating the ball and paddle thread
            pthread_t th_ball;
            pthread_create(&th_ball, NULL, calculate_ball, th_socket);
            pthread_t th_paddle;
            pthread_create(&th_paddle, NULL, play_paddle, NULL);

            while(1){
                if(flag == 4){ //received quit
                    //cancel threads
                    pthread_cancel(th_messages);
                    pthread_cancel(th_ball);
                    pthread_cancel(th_paddle);

                    //Sending disconnect message to server
                    memset(message, '\0', BUFFER_SIZE);
                    strcpy(message, "Disconnect \n");
                    err = sendto(sock_fd, message, strlen(message)+1, 0, (const struct sockaddr *) &server_addr, server_addr_size);
                    if (err == -1){
                        perror("sendto \n");
                    }
                    //close the server socket
                    if(close(sock_fd) == -1){
                        perror("close");
                    }
                    return 0;
                }else if(flag == 3){ //Received Release_ball
                    //cancel threads
                    pthread_cancel(th_ball);
                    pthread_cancel(th_paddle);
                    //go back to watch state
                    state = watch;
                    break;
                }
            }

    }
}

return(0);
}