#include "table_server.h"
 
/*****************************************************************************************************************************
 * This file is the server that controlls the communications between the different clients. It contains two threads. The first
 * thread - client - is responsible for managing each client, by reading the messages sent from each one and processing them.
 * The second thread - calculate ball - is responsible for calculating the movement of the ball every second and sending the 
 * respective board update to all clients
 * ****************************************************************************************************************************/

//Declaration of global variables
int number_players = 0;
client_address *Head = NULL;
pthread_t th_ball;
pthread_mutex_t mux_players, mux_list;

void *client(void * arg){

    //Declaration of the server's states 
	enum
    {
		Paddle_move,
		Disconnect
    } state;
	state=Paddle_move;

    //Declaration of variables
    char message_type[BUFFER_SIZE], message[BUFFER_SIZE], client_addr_str[BUFFER_SIZE];
    int err=0, client_port=0, n=0;
    int client_fd = *(int*) arg;

    //Initialization of the message
    memset(message_type,'\0', BUFFER_SIZE);
    memset(message,'\0', BUFFER_SIZE);

    //Finding the client for the right client socket - exctrating its IP and port
    search_node(&Head, client_addr_str, &client_port, client_fd);    
    
    while(1){

        //Receiving information from client
        err = read(client_fd, message, BUFFER_SIZE);
        if (err == -1){
            state = Disconnect;
        }

        //Analysing the first part of the message - identifies the message type
        for(n = 0; n < (int)strlen(message); n++){
            if(message[n] != ' '){
                message_type[n] = message[n];
            }   
        }

        //Choosing a state - only paddle move is received by the server
        if(strcmp(message_type, "Paddle_move")==0){
            state = Paddle_move;
        }

        switch(state){
            case Paddle_move: 
                move_update(&Head, client_addr_str, client_port, message);  //Updates the client's paddle information
                board_update(&Head, message); //Sending a board update message to all connected clients
                printf("paddle_board_update: %s\n", message);
                break;

            case Disconnect: 
        
                //Disconnecting the client connected to this particular thread 
                pthread_mutex_lock(&mux_players);
                number_players--; //Reducing number of players
                pthread_mutex_unlock(&mux_players);
                if (number_players == 0){ //Canceling the ball thread if there are no more connected clients
                    pthread_cancel(th_ball);
                    printf("\rWaiting for players to connect\n");
                }
                pthread_mutex_lock(&mux_list);
                remove_node(&Head, client_port, client_addr_str); //Removing the client's node from the list
                pthread_mutex_lock(&mux_list);
                if(close(client_fd) == -1){ //Closing the client's socket
                    perror("close\n");
                }
                pthread_exit(NULL); //Exiting the thread
        }
    }
}

void *calculate_ball(){

    //Declaration of variables
    char message[BUFFER_SIZE];

    place_ball_random(); //Placing ball in a random position

    //Moving the ball every second and sending a board update message to all connected clients
    while(1){
        moove_ball(&Head);
        board_update(&Head, message);
        printf("ball_board_update: %s\n", message);
        sleep(1);
    }
}

int main(){

	//Declaration of variables
	char message_type[BUFFER_SIZE], message[BUFFER_SIZE], client_addr_str[BUFFER_SIZE];
	int err=0, client_port=0, id=0, client_fd=0;
	client_address *New_address=NULL;
    pthread_mutex_init(&mux_players, NULL);
    pthread_mutex_init(&mux_list, NULL);

    //Initialization of the message
	memset(message_type, '\0', BUFFER_SIZE);
	memset(message, '\0', BUFFER_SIZE);

	//Creating socket
	int sock_fd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sock_fd == -1){
        perror("socket creation");
        exit(-1);
    }

	//Declaration of server address 
	struct sockaddr_in local_addr;
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(SOCK_PORT);
	local_addr.sin_addr.s_addr = INADDR_ANY;

    //Declaration of client address
    struct sockaddr_in client_addr; 
    socklen_t client_addr_size = sizeof(struct sockaddr_in);

	//Binding socket
	err = bind(sock_fd, (struct sockaddr *) &local_addr, sizeof(local_addr));
    if (err == -1){
        perror("bind");
        exit(-1);
    }

    while(1){
        
        //Listening to the connect sent from the clients
        if(listen(sock_fd,5) == -1){
            perror("listen");
            exit(1);
        }
        
        //Accepting the connect sent from the client
        if((client_fd = accept(sock_fd,(struct sockaddr*)&client_addr, &client_addr_size))==-1){
            perror("accept");
            exit(1);
        }           
        id++; 
        pthread_mutex_lock(&mux_players);
        number_players++; //Number of players in the game
        pthread_mutex_unlock(&mux_players);

        //The number of players never exceeds 10 players 
        if(number_players < 11){ 

            //Converting the client address and port 
            inet_ntop(AF_INET, &client_addr.sin_addr, client_addr_str, 100);
            client_port = ntohs(client_addr.sin_port);

            //Creating and inserting a new node - client - into the list
            New_address = createNode(client_port, client_addr_str, id, client_fd); 
            pthread_mutex_lock(&mux_list);
            Head = insert_node(&Head, &New_address);
            pthread_mutex_lock(&mux_list);

            //Declaration and dynamic allocation of memory in order to pass the socket as an argument in a thread
            pthread_t th_client;    
            int * th_socket = malloc(sizeof(int));
            *th_socket = client_fd;

            //Creating the thread for each client
            pthread_create(&th_client, NULL, client, th_socket); 

            /*Creating the thread for the ball if there is only one player (the ball is created only once and only if 
            previously there were no players and, therefore, no ball)*/
            if(number_players==1){
                pthread_create(&th_ball, NULL, calculate_ball, NULL);  
            }

            //Freeing the memory dynamically allocated 
            free(th_socket);
            continue;

        }else if(number_players >= 11){

            //The number of player exceeds 10 players - the client that tried to connect is dismissed
            if (close(client_fd) == -1){ //The client's socket is closed
                perror("close\n");
            }
            pthread_mutex_lock(&mux_players);
            number_players--; //Reducing number of players
            pthread_mutex_unlock(&mux_players);
            printf("\rClient didn't connect: The game has reached the maximum number of players.\n");
        }
    }
    exit(0);
}