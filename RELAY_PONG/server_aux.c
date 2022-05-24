#include "server_aux.h"

/***********************************************************************************************************
 * Struct: ClientAddress - stores client's information - used in order to create a list of connected clients
 * - The client's adress
 * - The client's port
 * *********************************************************************************************************/

struct ClientAddress{
    char addr[BUFFER_SIZE];
	int port; //this is the value the node stores
    struct ClientAddress *next; //this is the node the current node points to. this is how the nodes link
};

/**************************************************************************************************************
 * Function: createNode
 * Input: 
 *  - int remote_port - port of the client that sent a Connect message to the server
 *  - int remote_addr_str - address of the client that sent a Connect message to the server
 * Output: client_address structure that stores information of the client that has just connected
 * Porpuse: Creating a client_address structure to store the information of the clients that send Connect 
 * messages in a list, in order to connect them to the server
 * *************************************************************************************************************/

client_address *createNode(int remote_port, char *remote_addr_str){

	client_address *newNode;
    newNode = (client_address*)calloc(1, sizeof(client_address));
	if (newNode == NULL)
    {
        fprintf(stderr, "Error allocating memory\n");
        exit(1);
    }
	
	memset(newNode->addr, '\0', BUFFER_SIZE);
    strcpy(newNode->addr, remote_addr_str);
	newNode->port = remote_port;
    newNode->next = NULL;
    return newNode;
}

/***********************************************************************************************************
 * Function: insert_node
 * Input: 
 *  - client_address Head - the head of the connected client information list
 *  - client_address newNode - the new node that has just been created and needs to be inserted in the list
 * Output: client_address structure that contains the head of the client's information list
 * Porpuse: Inserting a client_address structure in a list of connected clients
 * Note: After the insertion of the created node, the client is connected to the server
 * **********************************************************************************************************/

client_address *insert_node(client_address **Head, client_address **newNode){
	if (*Head == NULL){
		*Head = *newNode;
	}else{
		(*newNode)->next = *Head;
		*Head = *newNode;
	}
	return *Head;
}

/**************************************************************************************************************
 * Function: remove_node
 * Input: 
 *  - client_address head_ref - the head of the connected client information list 
 *  - int port - port number of the client that wants to be disconnected
 *  - char * addr - address of the client that wants to be disconnected
 * Output: void
 * Porpuse: Removing a client_address structure from the list that stores the information of connected clients
 * Notes: By removing a client_address structure form this list, the client is disconnected from the server
 * *************************************************************************************************************/

void remove_node(client_address** Head, int port, char* addr){
 
    client_address *temp = *Head, *prev;

    if (temp != NULL && strcmp(temp->addr, addr)==0 && (temp->port == port)){

        *Head = temp->next; 
        free(temp);  
         
        return;
    }else{
		while (temp != NULL && (strcmp(temp->addr, addr)!=0 || (temp->port != port))){
			prev = temp;
			temp = temp->next;
		}
    if (temp == NULL)
        return;

    prev->next = temp->next;
    free(temp);
    }
}

/**************************************************************************************************************
 * Function: choose_player
 * Input: 
 *  - client_address Head - the head of the connected client information list 
 *  - int port - port number of the client that wants to be disconnected
 *  - char * address - address of the client that wants to be disconnected
 *  - int number_players - number of players currently in the game
 * Output: void
 * Porpuse: Choosing the player that will receive the send ball message next
 * *************************************************************************************************************/

void choose_player(client_address **Head, char* address, int* port, int number_players){

	client_address *Aux = *Head;

	if(number_players == 1){
		strcpy(address, (*Head)->addr);
		*port = (*Head)->port;
	}else{
		for(Aux = *Head; Aux != NULL; Aux = Aux->next){
			if(strcmp(Aux->addr, address) != 0 || Aux->port != *port){
				strcpy(address, Aux->addr);
				*port = Aux->port;
				break;
			}
		}
	}
}

/**************************************************************************************************************
 * Function: send_move_ball
 * Input: 
 *  - int sock_fd - refers to the used socket
 *  - client_address Head - the head of the connected client information list 
 *  - int port - port number of the client that wants to be disconnected
 *  - char * address - address of the client that wants to be disconnected
 *  - char * message - string that holds the message to be sent
 * Output: void
 * Porpuse: Sending move ball message to all other clients that are not playing in order to update their screen
 * *************************************************************************************************************/

void send_move_ball(int sock_fd, client_address **Head, char* address, int port, char* message){

	client_address *Aux=NULL;
	struct sockaddr_in client_addr; 
	client_addr.sin_family = AF_INET;
	socklen_t client_addr_size = sizeof(struct sockaddr_in);
	int err=0;

	for(Aux = *Head; Aux != NULL; Aux = Aux->next){
		if(strcmp(Aux->addr, address) != 0 || Aux->port != port){
			if(inet_pton(AF_INET, Aux->addr, &client_addr.sin_addr) < 1){
				printf("no valid address: \n");
				exit(-1);
			}
			client_addr.sin_port = htons(Aux->port);
			err = sendto(sock_fd, message, strlen(message)+1, 0, (struct sockaddr *) &client_addr, client_addr_size);
			if (err== -1){
				perror("sendto\n");
			}
		}
	}
}