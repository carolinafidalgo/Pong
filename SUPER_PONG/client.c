#include "table_client.h"

/*****************************************************************************************************************************
 * This file is the client that is controlled by the user to play the game. It contains one thread - messages - that is 
 * responsible for reading messages - board updates - from the server, decifering them and drawing them on the game window
 * ****************************************************************************************************************************/

void *messages(void * arg){

    //Declaration of variables
    int err=0;
    int sock_fd = *(int*) arg;
    char message[BUFFER_SIZE];

    while(1){

        //Reading Board Update message from server
        memset(message, '\0', BUFFER_SIZE); 
        err = read(sock_fd, message, BUFFER_SIZE);

        //Closing the socket if the client isn't able to read from the server
        if(err == -1){
            printf("The server is unavailable\n"); 
            if (close(sock_fd) == -1){
                perror("close\n");
            }
            exit(1);
        }

        //Erasing all elements, decifering Board Update message and drawing all elements
        decifer_board(message, 1); 
    }
}

int main(int argc, char *argv[]){

    //Declaration of variables
    char message[BUFFER_SIZE], SERVER_IP[BUFFER_SIZE];
    int err=0, q=0, SERVER_PORT=0;

    //Checking if all of the arguments are supplied in the command line
    if(argc !=3){
        printf("Three arguments expected - executable and server address and port\n");
        return 0;
    }
    
    //Checking if the IP supplied is valid
    if (check_IP(argv[1])){
        strcpy(SERVER_IP,argv[1]);
    }else{
        printf("IP is not valid\n");
        return 0;
    }

    //Checking if the port supplied is valid
    if (check_port(argv[2])){
        SERVER_PORT = atoi(argv[2]);
    }else{
        printf("Port is not valid\n");
        return 0;
    }

    //Creating socket
	int sock_fd = socket(AF_INET, SOCK_STREAM, 0); 
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
		perror("no valid address: \n");
		exit(-1);
	}
    
    //Connecting to server
    err = connect(sock_fd, (struct sockaddr*)&server_addr, server_addr_size);
    if(err == -1){
        perror("connect\n");
        exit(1);
    }

    //Creating a window for the game
    create_window(0);

    //Receiving first Board update message - guaranteeing that there is a paddle and ball registered before entering play state
    memset(message, '\0', BUFFER_SIZE); 
    err = read(sock_fd, message, BUFFER_SIZE);
    if(err == -1){
        perror("read");
        exit(1);
    }

    //Erasing all elements, decifering the first Board Update message and drawing all elements
    decifer_board(message, 1); 

    //Declaration and dynamic allocation of memory in order to pass the socket as an argument in a thread
    pthread_t th_server; 
    int * th_socket = malloc(sizeof(int));
    *th_socket = sock_fd;

    //Creating the thread to receive and process messages
    pthread_create(&th_server, NULL, messages, &sock_fd);

    //Freeing the memory dynamically allocated 
    free(th_socket);

    //Playing - user is allowed to make a move
    q = play_state(sock_fd);

    //Disconnecting from the server when the client presses 'q' in order to stop playing
    if(q == 1){
        pthread_cancel(th_server); //The thread that processes the server's messages is canceled
        sleep(1);
        if (close(sock_fd) == -1){ //The client's socket is closed
            perror("close\n");
        }
        printf("\rThank you for playing!\n");
        exit(0);
    }
}
