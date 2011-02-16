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
 Filename:chatclient.c
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
#include <time.h>

//prototypes
void error(char *msg);
int sendMSG(int sockfd, char *handle);
int recvMSG(int clntsock);

/************************************************************
 // Function Name: error
 // Parameters: char *msg
 // Description: Is called when a error ocurrs displays 
 // message and exits.
 ************************************************************/
void error(char *msg)
{
    perror(msg);
    exit(0);
}

/************************************************************
 // Function Name: SendMSG
 // Parameters: int sockfd, char *handle
 // Description: Reads in user input and writes it to the socket.
 ************************************************************/
int sendMSG(int sockfd, char *handle)
{
	time_t start,end;
	char message[501];
	char buffer[512];
	double dif;
	int n = 0;
	bzero(buffer,512);
	bzero(message, 501);
	strcat(buffer,handle);
	strcat(buffer, " > ");
	time (&start);
	printf("%s > ", handle);
	fgets(message,sizeof(message),stdin);
	message[ strlen ( message ) - 1 ] = '\0';
	time (&end);
	dif = difftime (end,start);
	if(dif > 180.0)
	{
		bzero(message, 501);
		strcpy(message,"quit");
		n = write(sockfd,message,strlen(message));
		if (n < 0) 
			error("ERROR writing to socket");
		return 1;
	}
	else if (strstr(message,"quit") != NULL)
	{
		n = write(sockfd,message,strlen(message));
		if (n < 0) 
			error("ERROR writing to socket");
		return 1;
	}
	else{
		strcat(buffer, message);
		n = write(sockfd,buffer,strlen(buffer));
		if (n < 0) 
			error("ERROR writing to socket");
	}
	return 0;
}

/************************************************************
 // Function Name: RecvMSG
 // Parameters: int clntsock
 // Description: Reads from the socket and if "quit" is received 
 // it quits the chat session.
 ************************************************************/
int recvMSG(int clntsock)
{
	int n = 0;
	char buffer[512];
	bzero(buffer,512);
	n = read(clntsock,buffer,512);
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
 // Description: Gets handle and sets up connection to server,
 // runs chat until timeout or user types "quit".
 ************************************************************/
int main(int argc, char *argv[])
{
    int sockfd, portno, quit;
    struct sockaddr_in serv_addr;
    struct hostent *server;
	char handle[256];
	
	//checks if arguments match up
    if (argc < 3) {
		fprintf(stderr,"usage %s error requires 3 arguments\n", argv[0]);
		exit(0);
    }
	
	printf("What is your handle?\n");
	fgets(handle,sizeof(handle),stdin);
	handle[ strlen ( handle ) - 1 ] = '\0';
	
	//get reference for socket using port number from argument
	portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");
	
	//checks if servername is valid
	server = gethostbyname(argv[1]);
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}
	//zeros out struct and fills it in
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, 
		  (char *)&serv_addr.sin_addr.s_addr,
		  server->h_length);
	serv_addr.sin_port = htons(portno);
	
	//connecting to socket
	if (connect(sockfd,&serv_addr,sizeof(serv_addr)) < 0) 
		error("ERROR connecting");  
	
	printf("Now connected\n");
	printf("Please enter the message: please type \"quit\" to exit chat.\n");
	quit = 0;
	while(quit != 1)
	{
		quit = sendMSG(sockfd,handle);
		if(quit != 1)
			quit = recvMSG(sockfd);	
	}
	printf("Connection closed\n");
	close(sockfd);
	return 0;
}
