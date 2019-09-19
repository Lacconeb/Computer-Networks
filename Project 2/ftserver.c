
/*****************************************************************
* Brian Laccone
* CS 372 Project 2
* ftserver.c
* 
* Description: This program is designed to be a server in which 
* clients can connect to. The clients can request to either have
* the server send a list of it's directory or the client can 
* request a specific file from the server and the server 
* will send the contents of the file to the client
*
* References for both ftserver.c and ftclient.py:
* https://beej.us/guide/bgnet/html/multi/index.html
* http://www.linuxhowtos.org/C_C++/socket.htm
* http://www.gnu.org/savannah-checkouts/gnu/libc/manual/html_node/Simple-Directory-Lister.html
* https://github.com/cirosantilli/cpp-cheat/blob/53196c683990d639fac97fbc416cb86782279739/posix/socket/inet/send_file_client.c
* ftp://gaia.cs.umass.edu/pub/kurose/ftpserver.c
* Kurose and Ross, Computer Networking: A Top-Down Approach, 7th Edition, Pearson
* http://www.bogotobogo.com/cplusplus/sockets_server_client.php
* http://www.bogotobogo.com/python/python_network_programming_server_client_file_transfer.php
* https://docs.python.org/2/library/
* http://stackoverflow.com/questions/17667903/python-socket-receive-large-amount-of-data
******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>

/*****************************************************************
* Simple function to show an error message and exit the program
******************************************************************/
void error(char *msg) 
{
    perror(msg);
    exit(1);
}


/*****************************************************************
* A function for sending a message from the server to the client
******************************************************************/
void sendMessage(int sock, char* buffer) 
{
    size_t total = 0;
    ssize_t n;
    size_t size = strlen(buffer) + 1;
    
    //loop until all the data is sent
    while (total < size) 
    {
        n = write(sock, buffer, size - total);

        total += n;

        //check is there was an error in sending
        if (n < 0) 
        {
            error("Error sending message");
            exit(1);
        }
        else if (n == 0) 
        {
            total = size - total;
        }
    }
}


/*****************************************************************
* A function for sending a number from the server to the client
******************************************************************/
void sendNumber(int sock, int num) {
    ssize_t n = 0;

    n = write(sock, &num, sizeof(int));
    if (n < 0) {
        error("Unable to send number");
    }
}


/*****************************************************************
* A function for sending a file, that was requested by the client, 
* to the client.
******************************************************************/
void sendFile(int sock, char* fileName) {
    
    char* msg;

    //open the file
    FILE* fp = fopen(fileName, "r");

    //check if file was actually opened
    if (fp == NULL) {
        error("Unable to open file");
    }

    if (fp != NULL) {
        if (fseek(fp, 0L, SEEK_END) == 0) {
            long size = ftell(fp);
            if (size == -1) {
                error("Invalid file");
                exit(1);
            }

            //allocate the variable used to hold content
            msg = malloc(sizeof(char) * (size + 1));

            //reset at beginning
            if (fseek(fp, 0L, SEEK_SET) != 0) {
                error("Unable to read file");
            }

            //read file to memory
            size_t newLen = fread(msg, sizeof(char), size, fp);
            if ( ferror( fp ) != 0 ) {
                fputs("Error reading file", stderr);
            } else {
                msg[newLen++] = '\0'; /* Just to be safe. */
            }
        }
    }
    fclose(fp);


    sendNumber(sock, strlen(msg));
    sendMessage(sock, msg);
}


/*****************************************************************
* A function for receiving a file from the client
******************************************************************/
void receiveMessage(int sock, char* buffer, size_t size) {
    char tmp[size + 1];
    ssize_t n;
    size_t total = 0;

    //loop until all data is read
    while (total < size) {
        n = read(sock, tmp + total, size - total);
        total += n;

        //check if there is an error while reading the message
        if (n < 0){
            error("Error receiving message");
            exit(1);
        }
    }

    strncpy(buffer, tmp, size);
}


/*****************************************************************
* A function for receiving a number from the client
******************************************************************/
int receiveNumber(int sock) 
{
    int num;
    ssize_t n = 0;
    n = read(sock, &num, sizeof(int));

    //check if there was an error in receiving the message
    if (n < 0) 
    {
        error("Unable to receive number");
    }
    return (num);
}


/*****************************************************************
* A function for finding the contents of the directory in which
* the server is located in.
* Influenced heavily by: http://www.gnu.org/savannah-checkouts/gnu/
* libc/manual/html_node/Simple-Directory-Lister.html
******************************************************************/
int getDirectory(char* path[]) 
{
    DIR *d;
    struct dirent *dir;
    int size = 0;
    int num_of_files = 0;

    d = opendir(".");
    if (d) {
        int i = 0;
        while ((dir = readdir(d)) != NULL) {

            if (dir->d_type == DT_REG) {
                path[i] = dir->d_name;
                size += strlen(path[i]);
                i++;
            }
        }
        num_of_files = i - 1;
    }
    closedir(d);
    return size + num_of_files;
}


/*****************************************************************
* A function to start up the server
******************************************************************/
int startUp(int portno) {
    
    int sockfd;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        return -1;
    }

    //variables for the server
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(portno);
    server.sin_addr.s_addr = INADDR_ANY;

    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

    //bind the server
    if(bind(sockfd, (struct sockaddr *) &server, sizeof(server)) < 0){
        error("Bind failed");
    }

    //set the server to listen
    if(listen(sockfd, 10) < 0){
        error("Listen failed");
    }

    return sockfd;
}


/*****************************************************************
* main function used to control the flow of the server. It will 
* set up the server and then wait for a client to connect and 
* ask for either a list of the directory or send a file to the
* client. This program must be exited by pressing CTRL + C
******************************************************************/
int main(int argc, char *argv[]) {

    int sockfd, newsockfd, datasockfd, portno, pid;

    //if this program is called with any argc amount besides 1 it will exit 
    //with an error
    if (argc == 1) {
        error("Usage: ftserver [portNumber]\n");
        exit(1);
    }

    portno = atoi(argv[1]);    

    //if the portnumber is not within range an error will occur
    if (portno < 1024 || portno > 65535) {
        error("Invalid port number. Range = 1024 - 65535\n");
    }

    //Set up initial socket
    sockfd = startUp(portno);
    printf("Server open on %d\n\n", portno);

    //keep the server running until CTRL + c is pressed
    while(1) {
        newsockfd = accept(sockfd, NULL, NULL);
        if(newsockfd < 0) {
            error("Error on accept\n");
        }
        pid = fork();
        if (pid < 0) {
            error("Error on fork\n");
        }

        if (pid == 0) {
            close(sockfd);
            int command = 0;
            int dataPort;
            int newsock;

            printf("Connection started on port %d.\n", portno);

            char tmp[3] = "\0";

            receiveMessage(newsockfd, tmp, 3);
            dataPort = receiveNumber(newsockfd);

            //if the "command" arg was set to -l then set command to 1
            if (strcmp(tmp, "-l") == 0) {
                command = 1;
            }

            //if the "command" arg was set to -g then set command to 2
            if (strcmp(tmp, "-g") == 0) {
                command = 2;
            }

            //if the command arg does not equal -l or -g it will send an error
            if (command == 0) {
                error("Did not receive -l or -g");
            }

            //if command == 1 "-l" then find the directory and send it's contents to the client
            if (command == 1) {
                char* path[100];
                int i = 0;
                int length = 0;
                printf("Client requested the directory list on port %d. Sending...\n\n", dataPort);
                length = getDirectory(path);

                newsock = startUp(dataPort);
                datasockfd = accept(newsock, NULL, NULL);

                if (datasockfd < 0) {
                    error("Unable to open data socket");
                }

                sendNumber(datasockfd, length);
                while (path[i] != NULL) {
                    sendMessage(datasockfd, path[i]);
                    i++;
                }

                close(newsock);
                close(datasockfd);
                exit(0);
            }

            //if command == 2 "-g" then find the file requested and send it to the client
            if (command == 2) {
                int i = receiveNumber(newsockfd);
                char fileName[255] = "\0";
                receiveMessage(newsockfd, fileName, i);
                printf("Client had requested \"%s\" on port %d \n", fileName, dataPort);

                //check if file is found
                //if file is found, send file's contents to the client
                if (access(fileName, F_OK) == -1) 
                {
                    printf("The file that was requested was not found\n\n");
                    char errorMessage[] = "no file";
                    sendNumber(newsockfd, strlen(errorMessage));
                    sendMessage(newsockfd, errorMessage);
                    close(newsock);
                    close(datasockfd);
                    exit(1);
                }
                else 
                {
                    char message[] = "yes";
                    sendNumber(newsockfd, strlen(message));
                    sendMessage(newsockfd, message);
                }
                printf("Sending \"%s\" on port %d\n\n", fileName, dataPort);

                newsock = startUp(dataPort);
                datasockfd = accept(newsock, NULL, NULL);
                if (datasockfd < 0) {
                    error("Unable to open data socket");
                }
                sendFile(datasockfd, fileName);
                close(newsock);
                close(datasockfd);
                exit(0);
            }
            exit(0);
        }

    }
}