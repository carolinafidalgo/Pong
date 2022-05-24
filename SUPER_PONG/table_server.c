#include "table_server.h"

//Declaration of global variables
WINDOW * message_win;
WINDOW * my_win;

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

struct ball_position_t ball;

/***********************************************************************************************************
 * Struct: ClientAddress - stores client's information - used in order to create a list of connected clients
 * - The client's id and score
 * - The client's adress and port
 * - The client's paddle coordinates
 * - The client's paddle length
 * *********************************************************************************************************/

struct ClientAddress{
    int id;
    char addr[BUFFER_SIZE];
	int port;
    int socket; 
	int paddle_x;
    int paddle_y;
    int length;
    int score;
    struct ClientAddress *next; 
};

/**************************************************************************************************************
 * Function: createNode
 * Input: 
 *  - int remote_port - port of the client that connected to the server
 *  - int remote_addr_str - address of the client that connected to the server
 * Output: client_address structure that stores information of the client that has just connected
 * Porpuse: Creating a client_address structure to store the information of the clients that connected
 * to the server in a list
 * *************************************************************************************************************/

client_address* createNode(int remote_port, char *remote_addr_str, int id, int client_fd){

	client_address* newNode;
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
    newNode->id = id;
    newNode->score = 0;
    newNode->socket = client_fd;
    new_paddle(newNode);

    return newNode;
}

/***********************************************************************************************************
 * Function: insert_node
 * Input: 
 *  - client_address Head - the head of the connected client information list
 *  - client_address newNode - the new node that has just been created and needs to be inserted in the list
 * Output: client_address structure that contains the head of the client's information list
 * Porpuse: Inserting a client_address structure in a list of connected clients
 * Note: After the insertion of the created node, the client's information is properly stored
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
 * Notes: By removing a client_address structure from this list, the client is disconnected from the server
 * *************************************************************************************************************/

void remove_node(client_address** head_ref, int port, char* addr)
{
 
    client_address *temp = *head_ref, *prev;

    if (temp != NULL && strcmp(temp->addr, addr)==0 && (temp->port == port)){

        *head_ref = temp->next; 
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
 * Function: new_paddle
 * Input: 
 * - client_address node - a node from the connected client information list that refers to client that has 
 * just connected to the server
 * Output: void
 * Porpuse: Creating a new paddle for the client that just connected to the server
 * *************************************************************************************************************/

void new_paddle (client_address* node){
    node->paddle_x = WINDOW_SIZE/2;
    node->paddle_y = WINDOW_SIZE-2;
    node->length = PADDLE_SIZE;
}

/**************************************************************************************************************
 * Function: place_ball_random
 * Input: No input
 * Output: void
 * Porpuse: Placing the ball in a random position in the game window
 * *************************************************************************************************************/

void place_ball_random(){
    ball.x = rand() % WINDOW_SIZE;
    ball.y = rand() % WINDOW_SIZE;
    ball.c = 'o';
    ball.up_hor_down = rand() % 3 -1; //  -1 up, 1 - down
    ball.left_ver_right = rand() % 3 -1 ; // 0 vertical, -1 left, 1 right
}

/**************************************************************************************************************
 * Function: moove_ball
 * Input: 
 *  - client_address head - the head of the connected client information list 
 * Output: void
 * Porpuse: Mooving the ball in a random pattern
 * Notes: The function accounts for the ball bouncing off the paddle and the walls
 * *************************************************************************************************************/

void moove_ball(client_address** head){

    client_address* Aux;
    
    int next_x = ball.x + ball.left_ver_right;
    int next_y = ball.y + ball.up_hor_down;

    for(Aux = *head; Aux != NULL; Aux = Aux->next){

        int start_x = Aux->paddle_x - Aux->length;
        int end_x = Aux->paddle_x + Aux->length;
        
        if((next_x >= start_x - 1) && (next_x <= end_x + 1)){ 
            if(next_y == Aux->paddle_y + 1){
                Aux->score++;
                ball.up_hor_down = 1;
                ball.left_ver_right = rand() % 3 -1;
            }
             if (next_y == Aux->paddle_y - 1){
                Aux->score++;
                ball.up_hor_down = -1;
                ball.left_ver_right = rand() % 3 -1;
            }
        }
    }

    if( next_x == 0 || next_x == WINDOW_SIZE-1){
        ball.up_hor_down = rand() % 3 -1 ;
        ball.left_ver_right *= -1;
    }else{
        ball.x = next_x;
    }

    if( next_y == 0 || next_y == WINDOW_SIZE-1){
        ball.up_hor_down *= -1;
        ball.left_ver_right = rand() % 3 -1;
    }else{
        ball.y = next_y;
    }

}

/*****************************************************************************************************************************+
 * Function: board_update
 * Input: 
 *  - client_address **head - the head of the connected client information list  
 *  - char * message - string that stores the board update message to send to the client
 * Output: void
 * Porpuse: Creating and sending a board update message with the information required to all clients
 *  - ball coordinates
 *  - ball character
 *  - own client's id and score
 *  - own client's paddle coordinates
 *  - other clients' id and score
 *  - other clients' paddle coordinates
 * Notes: In order for the client in question to know their own paddle, the first paddle coordinates, id and 
 * score are referent to themselves. The message is created in each thread for each client and, therefore, sent to all clients
 * ****************************************************************************************************************************/

void board_update(client_address** head, char* message){

    //Declaration of variables
    client_address* Aux_1=NULL;
    client_address* Aux_2=NULL;
    char message_aux[BUFFER_SIZE], ch='\n';
    int err=0;

    //Contructing the board update message with clients' paddle coordinates, id and scores
    for(Aux_1 = *head; Aux_1 != NULL; Aux_1 = Aux_1->next){
        memset(message,'\0', BUFFER_SIZE);
        memset(message_aux,'\0', BUFFER_SIZE);
        //Constructing the board update message with the ball coordinates and character, identifying the message type 
        sprintf(message, "%s %d %d %c ", "Board_update", ball.x, ball.y , ball.c);
        sprintf(message_aux, "%d %d %d %d %d ", Aux_1->id, Aux_1->score, Aux_1->paddle_x, Aux_1->paddle_y, Aux_1->length);
        strcat(message, message_aux);
        for(Aux_2 = *head; Aux_2 != NULL; Aux_2 = Aux_2->next){
            if((strcmp(Aux_1->addr, Aux_2->addr)!=0) || (Aux_1->port != Aux_2->port)){
                memset(message_aux,'\0', BUFFER_SIZE);
                sprintf(message_aux, "%d %d %d %d %d ", Aux_2->id, Aux_2->score, Aux_2->paddle_x, Aux_2->paddle_y, Aux_2->length);
                strcat(message, message_aux);
            }
        }
        strcat(message, &ch); 

        err = write(Aux_1->socket, message, strlen(message));
        if (err== -1){
            printf("\rPlayer %d is not available\n", Aux_1->id);
            continue;
        }
    }
}

/************************************************************************************************************************
 * Function: move_update
 * Input: 
 *  - client_address **Head - the head of the connected client information list  
 *  - char * message - string that stores the paddle move message sent from the client
 *  - char * client_addr_str - string that stores the address of the client 
 *  - int client_port - variable that stores the client's port number
 * Output: void
 * Porpuse: Updating the client's information with the new coordinates of the client's paddle and its score. The client 
 * in question is found by going through the hole list and identifying the node with a corresponding addres (IP and port)
 * Notes: This is necessary in order keep all of the current clients' paddle positions stored in the designated list. 
 * This function accounts for the collision of two paddles. If a paddle wishes to move to a position occupied by another 
 * paddle, the position is not updated and that paddle stays in the same place
 * ***********************************************************************************************************************/

void move_update(client_address** Head, char *client_addr_str, int client_port, char *message){

    client_address* Aux_1=NULL;
    int begin=0, end=0, paddle_x=0, paddle_y=0, length=0, paddle_score=0;
    
    sscanf(message, "%*s %d %d %d %d", &paddle_x, &paddle_y, &length, &paddle_score);

    //Comparing the client's paddle position with all of the other client's paddles' positions
    for(Aux_1 = *Head; Aux_1 != NULL; Aux_1 = Aux_1->next){
        begin = Aux_1->paddle_x - Aux_1->length;
        end = Aux_1->paddle_x + Aux_1->length;
        if((((paddle_x + length >= begin) && (paddle_x - length <= end)) && (paddle_y == Aux_1->paddle_y)) && 
            ((strcmp(Aux_1->addr, client_addr_str) != 0) || (Aux_1->port != client_port))){
            return; 
        }
    }

    //Updating the client's paddle coordinates on connected client information list
    for(Aux_1 = *Head; Aux_1 != NULL; Aux_1 = Aux_1->next){
        if((strcmp(Aux_1->addr, client_addr_str) == 0) && (Aux_1->port == client_port)){
            Aux_1->paddle_x = paddle_x;
            Aux_1->paddle_y = paddle_y;
            Aux_1->length = length;
            Aux_1->score = paddle_score;
            break;
        }
    }
}

/************************************************************************************************************************
 * Function: search_node
 * Input: 
 *  - client_address **Head - the head of the connected client information list  
 *  - char * client_addr_str - string that stores the address of the client 
 *  - int client_port - variable that stores the client's port number
 *  - int client_fd - client's socket
 * Output: void
 * Porpuse: Searching for the node on the client information list that corresponds to the client in question by comparing
 * the client's socket - client_fd - to the variable that contains the client's socket, stored in the list
 * Notes: This allows for the client thread to know the address and port of the client in question
 * ***********************************************************************************************************************/

void search_node(client_address** Head, char *client_addr_str, int *client_port, int client_fd){

    client_address* Aux=NULL;
    
    for(Aux = *Head; Aux != NULL; Aux = Aux->next){
        if(Aux->socket == client_fd){
            strcpy(client_addr_str, Aux->addr);
            *client_port = Aux->port;
            break;
        }
    }
}

