#include "server_aux.h"

/*****************************************************************************************************************************
 * This file is the server that controlls the communications between the different clients. It is managed by a thread a state 
 * machine with three different states:
 * Connect - The server received a Connect message and adds the new client to the list
 * Move_ball - The server broadcasts the ball movement to every player connected
 * Disconnect - The server receives a disconnect and removes that client from its list
 * ****************************************************************************************************************************/
 
/******Global variables shared by all threads******/
client_address *Head = NULL; //head of the client list
int number_players=0; //number of players in the list

/*****************************************************************************************************************************
* This thread is responsible for sending the messages Send_ball and Release_ball from the server to the clients as needed
* letting the each player play for 10 seconds at a time.
* - sock_fd -> socket of the server
* return: none
 ****************************************************************************************************************************/

void *send_message(void * arg){

    //Declaration of variables
	char message[BUFFER_SIZE], address[BUFFER_SIZE];
	int err=0, port=0;
    int sock_fd = *(int *) arg;

    //Declaration of client adress
	struct sockaddr_in client_addr;
	socklen_t client_addr_size = sizeof(struct sockaddr_in);

    while(1){

        //Initialization of message and selection of the message type to send to the client
        memset(message, '\0', BUFFER_SIZE);
        strcpy(message, "Send_ball \n");

        //if there are no players connected the ball won't be sent
        if(number_players == 0){
            continue;
        }else{
            //Choosing the client that will receive send ball message from the server
            choose_player(&Head, address, &port, number_players);

            if(inet_pton(AF_INET, address, &client_addr.sin_addr) < 1){
                printf("no valid address: \n");
                exit(-1);
            }
            client_addr.sin_port = htons(port);

            //Sending Send_ball message to client
            err=sendto(sock_fd, message, strlen(message)+1, 0, (struct sockaddr *) &client_addr, client_addr_size);
            if (err== -1){
                perror("sendto\n");
            }

            //let each user play for 10 seconds
            sleep(10);

            //Initialization of message and selection of the message type to send to the client
            memset(message, '\0', BUFFER_SIZE);
            strcpy(message, "Release_ball \n");

            //Sending Release_ball message to client
            err=sendto(sock_fd, message, strlen(message)+1, 0, (struct sockaddr *) &client_addr, client_addr_size);
            if (err== -1){
                perror("sendto\n");
            }
            sleep(1);
        }
    }
}

int main(){

	//Declaration of the server's states 
	enum
    {
        Connect,
		Move_ball,
		Disconnect
    } state;
	state=Connect;
	
	//Declaration of variables
	char message_type[BUFFER_SIZE], message[BUFFER_SIZE], address[BUFFER_SIZE];
	int err=0, port=0, n=0;
	client_address *New_address=NULL;

	//Initialization of message and message type
	memset(message_type, '\0', BUFFER_SIZE);
	memset(message, '\0', BUFFER_SIZE);

	//Creation of Socket
	int sock_fd = socket(AF_INET, SOCK_DGRAM, 0); 
    if (sock_fd == -1){
        perror("socket creation");
        exit(-1);
    }

	//Declaration of server adress 
	struct sockaddr_in local_addr;
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(SOCK_PORT);
	local_addr.sin_addr.s_addr = INADDR_ANY;

	//Binding socket
	n = bind(sock_fd, (struct sockaddr *) &local_addr, sizeof(local_addr));
    if (n == -1){
        perror("bind");
        exit(-1);
    }

	//Declaration of client adress
	struct sockaddr_in client_addr;
	socklen_t client_addr_size = sizeof(struct sockaddr_in);

    //create the thread responsible for managing the Send_ball and Release_ball messages
    pthread_t th_messages;
    int * th_socket = malloc(sizeof(int));
    *th_socket = sock_fd;
    pthread_create(&th_messages, NULL, send_message, th_socket);

    while(1){

        memset(message_type,'\0', BUFFER_SIZE);
        memset(message,'\0', BUFFER_SIZE);

        //Receiving information from client
        err = recvfrom(sock_fd, message, 100, 0, (struct sockaddr *)&client_addr, &client_addr_size);
        if (err == -1){
            perror("\rrcvfrom\n");
        }

        //Converting the client address and port 
        port = ntohs(client_addr.sin_port);
        inet_ntop(AF_INET, &client_addr.sin_addr, address, 100);

        //Analysing the first part of the message - identifies the message type
        for(n = 0; n < (int)strlen(message); n++){
            if(message[n] != ' '){
                message_type[n] = message[n];
            }   
        }

        //Choosing a state based on the message type received
        if(strcmp(message_type, "Connect")==0){
            state = Connect;
        }else if(strcmp(message_type, "Move_ball")==0){
            state = Move_ball;
        }else if(strcmp(message_type, "Disconnect")==0){
            state = Disconnect;
        }

        switch(state){

            case Connect: 
                //Storing client's information in a list of structures
                New_address = createNode(port, address);
                Head = insert_node(&Head, &New_address);
                number_players++;
                break;

            case Move_ball: 

                //Sending a move ball message to the clients that aren't playing
                send_move_ball(sock_fd, &Head, address, port, message);
                break;

            case Disconnect: 

                //Removing the client's information when a disconnect message is received from that client
                remove_node(&Head, port, address);
                number_players--; //The number of the players decreases after a disconnect message

                //Waiting to receive a connect message if there are no clients connected
                if (number_players == 0){
                    break;
                }
        }
}
exit(0);
}
