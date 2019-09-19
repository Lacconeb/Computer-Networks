/*****************************************************************
* Brian Laccone
* CS 372 Project 1
* chatclient.c
* 
* References:
* 1. http://www.linuxhowtos.org/C_C++/socket.htm (heavily used)
* 2. https://beej.us/guide/bgnet/html/multi/index.html
* 
* Description: This program is designed to be a client that
* connects to a server. The connection is made through API
* sockets through a TCP connection. The client connects to 
* the server and sends a message. Both the server and client
* will alternate between sending messages until either the
* server or client send "\quit". This program will end after 
* the connectin is lost
******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


/*****************************************************************
* Simple function to show and error message and exit the program
* 
******************************************************************/
void error(const char *msg) 
{
    perror(msg);
    exit(0);
}


/*****************************************************************
* A function to set up and initialize a connection to the server.
* returns an int for the sockfd file descriptor so it can be used
* by other functions
******************************************************************/
int connection(char *hostname,int portno) 
{
	//file descriptor for socket
    int sockfd;

    //a struct sockaddr_in variable to hold the address of the server
	struct sockaddr_in serv_addr;

	//a struct hostent variable to hold information on the host
	struct hostent *server;

    //creates a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    //make sure the socket was created
    if (sockfd < 0) {
        error("ERROR opening socket");
    }

    //gets information about the host
    server = gethostbyname(hostname);

    //make sure the server is not null
    if (server == NULL) 
    {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    //clears serv_addr
    bzero((char *) &serv_addr, sizeof(serv_addr));

    //sets the fields in serv_addr
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
            (char *)&serv_addr.sin_addr.s_addr,
            server->h_length);

    // converts port number to network byte order
    serv_addr.sin_port = htons(portno);

    // connects to the server and checks if it was successful
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
    {
        error("ERROR connecting");
    }

    return sockfd;
}



/*****************************************************************
* A function for sending messages to the server. The function will
* wait until the user inputs a message. It then proceeds to send it
* the server. If the message being sent contains a "\quit", it will
* set stay_alive to 0 to exit the connection.
******************************************************************/
void sending(int sockfd, char *username, int *stay_alive) 
{
	//variable to hold read and write returned values
	int n;

	//variable to hold the message being sent
	char sendMsg[501];

	//variable to hold client input
	char user_input[489];

    //displays the username and then prompts the user to create a message
    printf("%s: ",username);
    bzero(user_input,489);
    fgets(user_input,488,stdin);
    
    //simple for loop to make sure the a new line command isn't present.
    //the program won't work properly without this.
    int i;
    for (i = 0; i < 489; i++) 
    {
        if (user_input[i] == '\n') 
        {
            user_input[i] = '\0';
        }
    }

    //checks if the user typed "\quit" and then will set stay_alive to 0
    if (strcmp(user_input, "\\quit") == 0)
    {
            //change stay_alive to 0 to end the program
            *stay_alive = 0;

            //sends "\quit" to the server so it can disconnect
            n = write(sockfd,user_input,strlen(user_input));

            //make sure there wasn't an error when sending "\quit" to server
            if (n < 0) {
                error("ERROR writing to socket");
            }

            //close connection
            close(sockfd);
    }
    else 
    {
    	
        //clears the sendMSG array
        bzero(sendMsg,501);

        //add the username, a semicolon, and the user inputted message to the message variable
        strcat(sendMsg,username);
        strcat(sendMsg,": ");
        strcat(sendMsg,user_input);

        //send message to the server
        n = write(sockfd,sendMsg,strlen(sendMsg));

        //make sure there wasn't an error when sending sendMsg to server
        if (n < 0) 
        {
            error("ERROR writing to socket");
        }
    }
}



/*****************************************************************
* A function for receiving messages from the server. The function 
* will wait until it received a message form the server. If the 
* message being received contains a "\quit", it will set stay_alive
* to 0 to exit the connection.
******************************************************************/
void receiving(int sockfd, char *username, int *stay_alive) 
{
	//variable to hold read and write returned values
	int n;

	//variable to hold the message that was received
	char receivedMSG[501];

    //clear the message array
    bzero(receivedMSG,501);

    //receive message from the server
    n = read(sockfd,receivedMSG,500);

    //make sure there wasn't an error in receiving from the server
    if (n < 0) 
    {
        error("ERROR reading from socket");
    }

    //checks if a "\quit" message was received from the server and then set stay_alive to 0
    if (strcmp(receivedMSG, "server: \\quit") == 0)
    {

            //change stay_alive to 0 to end the program
            *stay_alive = 0;

            //close connection
            close(sockfd);
    }
    else 
    {

        //if "\quit" was not received, display the received message
        printf("%s\n",receivedMSG);
    }
}



/*****************************************************************
* Main function responsible for calling the connection function and
* controlling the flow of sending and receiving messages. When the 
* stay_alive variable is set to 0 the program will exit.
******************************************************************/
int main(int argc, char *argv[]) 
{
	//file descriptor for socket
	int sockfd;

	//holds username (handle)
	char username[11];

	//variable to determine whether to keep the connection alive
	int stay_alive = 1;

    //checks if the user input the correct number of arguments
    if (argc != 3) 
    {
       fprintf(stderr,"Invalid number of arguments");
       exit(0);
    }

    //prompts the user to input a username (handle)
    bzero(username,10);
    printf("Enter a username: ");
    fgets(username,10,stdin);

    //simple for loop to make sure the a new line command isn't present.
    //the program won't work properly without this.
    int i;
    for (i = 0; i < 11; i++) 
    {
        if (username[i] == '\n') 
        {
            username[i] = '\0';
        }
    }

    //creates a connection with the server
    sockfd = connection(argv[1],atoi(argv[2]));

    //prints a header
    printf("Chat\n");

    //the program will stay in a loop of sending messages
    //and receiving messages until either side sends a "\quit".
    while (stay_alive == 1) 
    {
        if (stay_alive == 1) 
        {
            //send a message to the server
            sending(sockfd, username, &stay_alive);
        }

        if (stay_alive == 1) 
        {
            //receive a message from the server
            receiving(sockfd, username, &stay_alive);
        }
    }
    return 0;
}