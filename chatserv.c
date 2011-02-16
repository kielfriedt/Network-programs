/*
 *	Copyright (C) 2011  Kiel Friedt
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/***********************************************************
 Project 1
 Filename:chatserv.c
 Name: Kiel Friedt
 Class: CS372
 Sources: TCP/IP Sockets in c by michael j. donahoo and kenneth l. calvert
 ***********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

//prototypes
void error(char *msg);
int sendMSG(int clntSocket, char *handle);
int recvMSG(int servsock);

/************************************************************
 // Function Name: error
 // Parameters: char *msg
 // Description: Is called when a error ocurrs displays 
 // message and exits.
 ************************************************************/
void error(char *msg)
{
    perror(msg);
    exit(1);
}

/************************************************************
 // Function Name: SendMSG
 // Parameters: int sockfd, char *handle
 // Description: Reads in user input and writes it to the socket.
 ************************************************************/
int sendMSG(int sockfd, char *handle)
{
	char buffer[512];
	char message[501];
	int n = 0;
	bzero(message, 501);
	bzero(buffer,512);
	strcat(buffer,handle);
	strcat(buffer, " > ");
	printf("%s > ", handle);
	fgets(message, sizeof(message), stdin);
	message[ strlen ( message ) - 1 ] = '\0';
	if (strstr(message,"quit") != NULL)
	{
		n = write(sockfd,message,strlen(message));
		if (n < 0) 
			error("ERROR writing to socket");
		return 1;
	}
	else{
		strcat(buffer, message);
		n = write(sockfd,buffer,strlen(buffer));
		//printf("%s\n",buffer);
		if (n < 0) 
			error("ERROR writing to socket");
	}
}

/************************************************************
 // Function Name: RecvMSG
 // Parameters: int servsock
 // Description: Reads from the socket and if "quit" is received 
 // it quits the chat session.
 ************************************************************/
int recvMSG(int servsock)
{
	int n = 0;
	char buffer[512];
	bzero(buffer,512);
	n = read(servsock,buffer,512);
	if (n < 0) 
		error("ERROR reading from socket");
	if(strcmp(buffer, "quit") == 0)
		return 1;
	else {
		printf("%s\n", buffer);	
		return 0;
	}
}

/************************************************************
 // Function Name: main
 // Parameters: int argc, char *argv[]
 // Description: Gets handle from user check if arguments 
 // match and sets up socket. 
 ************************************************************/
int main(int argc, char *argv[])
{
	int newsockfd, sockserv, clilen, portno, quit;
	struct sockaddr_in serv_addr, cli_addr;
	char handle[256];
	//checks if arguments match up
	if (argc < 2) {
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}
	
	printf("What is the servers handle?\n");
	fgets(handle,sizeof(handle),stdin);
	handle[ strlen ( handle ) - 1 ] = '\0';
	
	//gets reference for socket
	sockserv = socket(AF_INET, SOCK_STREAM, 0);
	if (sockserv < 0) 
        error("ERROR opening socket");
	
	//assigning port number from argument
	portno = atoi(argv[1]);
	//zeros out struct and fills it in
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	
	//bind socket to port
	if (bind(sockserv, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) 
		error("ERROR on binding");
	
	//listen for socket connections and limit the queue of incoming connections
	if (listen(sockserv,10) < 0)
        error("listen() failed");
	
	/* Set the size of the in-out parameter */
	clilen = sizeof(cli_addr);
	
	while(1)
	{
		/* Wait for a client to connect */
		if ((newsockfd = accept(sockserv, &cli_addr, &clilen)) < 0)
			error("accept() failed");
		else{
			/* clntSock is connected to a client! */
			printf("Now connected\n");
			printf("Please enter the message: please type \"quit\" to exit chat.\n");
			quit = 0;
			while (quit != 1){
				quit = recvMSG(newsockfd);
				if(quit != 1)
					quit = sendMSG(newsockfd, handle);
			}
		}
		printf("Client has disconnected\n");
	}
	/* never get here */
	close(newsockfd);
	return 0; 
}
