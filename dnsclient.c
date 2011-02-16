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
/*
 * Name: Kiel Friedt
 *
 * class: CS372
 *
 * Description: DNS client asks for a URL and creates a DNS header and sends that
 * formatted header to NS1.oregonstate.edu Once it receives the information back it 
 * displays it in hex and ascii characters. Then Translates the information and 
 * finds one IP address of a webserver and displays it.
 *
 * Sources: 
 * TCP/IP Sockets in c by michael j. donahoo and kenneth l. calvert
 * http://www.binarytides.com/blog/dns-query-code-in-c-with-winsock-and-linux-sockets/
 * http://www.google.com/codesearch
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

//prototypes
void error(char *msg);
int returnIP(char* buf,int n);
int parseDNSresponse(unsigned char *buffer);
char *createDNSquery(char *query,char params[]);
void connecttoDNSserver(struct sockaddr_in *serveradd);
void sendDNSquery(int *socksd,char *buffer,struct sockaddr_in serveradd);
void recvDNSrespond(int *socksd, char *buffer,struct sockaddr_in serveradd);

/************************************************************
// Function Name: error
// Parameters: char *msg
// Description: Is called when a error ocurrs, displays 
// message and exits.
************************************************************/
void error(char *msg)
{
    perror(msg);
    exit(1);
}

/************************************************************
 // Function Name: connecttoDNSserver
 // Parameters: struct sockaddr_in *serveradd
 // Description: Creates structure and fills it for socket.
 ************************************************************/
void connecttoDNSserver(struct sockaddr_in *serveradd)
{
	struct hostent *Server; //creates structure    
    if ((Server = gethostbyname("NS1.oregonstate.edu")) == NULL)//gets NS1.oregonstate.edu address using gethostbyname function
		error("ERROR, no such host\n");
	serveradd->sin_family = AF_INET;
	serveradd->sin_port = htons(53);//puts port 53 into structure 
	serveradd->sin_addr = *((struct in_addr *)Server->h_addr);//address into structure
}

/************************************************************
 // Function Name: *createDNSquery
 // Parameters: char *query, char params[]
 // Description: Takes the URL and turns it into DNS query
 // and prints out the inital DNS query.
 ************************************************************/
char *createDNSquery(char *query,char params[])
{
	int i,j,x = 0,count[10]; 
	int domainName= 12; 
	char temp[16];
	for (i=0;i<10;i++) 
		count[i] = 0;  
	for (i=0;i<strlen(params);i++)//Parses through URL
	{ 
		if (params[i] == '.')
			x++;
		else
			count[x] += 1;
	}
	query[domainName++] = (char)count[0]; 
	x=1; 
	for (i=0;i<strlen(params);i++)//converts URL to HEX
	{ 		
		if (params[i] == '.')
			query[domainName++] = (char)count[x++]; 
		else
			query[domainName++] = params[i]; 
	}
	
	query[0] = '\00'; 					
	query[1] = '\02';
	query[2] = '\01';
	query[3] = '\00';
	query[4] = '\00';
	query[5] = '\01';
	int k;
	for (k = 6; k < 12; k++) 
		query[k] = '\00';	
	query[domainName++] = '\00';
	query[domainName++] = '\00';
	query[domainName++] = '\01';
	query[domainName++] = '\00';
	query[domainName++] = '\01';	
	
	for (i=0; i<domainName; i++)// prints the query in hEX form
	{ 
		if(i % 16 == 0 && i != 0)
		{
			printf(" %s\n",temp);
			memset(temp,'\0',16);
		}
		printf(" ");
		printf("%02hhX", query[i]);//Prints out in HEX
		if(query[i] > 32)//If Ascii is greater then 33 prints it out
			temp[i%16] = query[i];
		else
			temp[i%16] = '.';//if ascii value is less then 33 
	}
	
	while(i % 16 != 0){
		printf("   ");
		i++;
	}
	printf(" %s\n",temp);
	return query;
}

/************************************************************
 // Function Name: parseDNSresponse
 // Parameters: unsigned char *buffer
 // Description: Searches through the response and finds the 
 // pointers to the IP in the response and then returns the 
 // IP addess.
 ************************************************************/
int parseDNSresponse(unsigned char *buffer)
{
	int i;
	char * tmp = buffer; 
	for (i = 0;i<300;i++)  
		if (tmp[i] == '\00') 
			if (tmp[i+1] == '\01')
				if (tmp[i+8] == '\00')
					if (tmp[i+9] == '\04') 
						return (i+10);
	printf("\n");
	return -1; 
}

/************************************************************
 // Function Name: sendDNSquery
 // Parameters: int *socksd, char *buffer, struct sockaddr_in serveradd
 // Description: Takes the information and sends it over the socket.
 ************************************************************/
void sendDNSquery(int *socksd, char *buffer, struct sockaddr_in serveradd)
{
	int n; 
	n = sendto(*socksd, buffer,(size_t) 512,0,(struct sockaddr *)&serveradd,sizeof serveradd); 
	if (n < 0) 
		error("ERROR writing to socket\n");
}

/************************************************************
 // Function Name: recvDNSrespond
 // Parameters: int *socksd, char *buffer, struct sockaddr_in serveradd
 // Description: After receiving the response it prints out the response
 // in HEX and ascii
 ************************************************************/
void recvDNSrespond(int *socksd, char *buffer,struct sockaddr_in serveradd)
{
	int n, i, addlen;
	char temp[16];
	int buff = 512;
	//clears buffer 
	bzero(buffer,sizeof(buffer));
	addlen = sizeof(struct sockaddr *); 
	//Receives response from socket
	n = recvfrom(*socksd, buffer,(size_t) buff, 0,(struct sockaddr *)&serveradd,&addlen); 
	if (n < 0)//if read fails
	{
		error("Couldn't read data on socket\n"); 
	}
	else
    {
		for (i = 0;i<300;i++){
			if(i % 16 == 0 && i != 0){
				printf(" %s\n",temp);
				bzero(temp, sizeof(temp));
			}
			printf(" ");
			printf("%02hhX",buffer[i]);//prints string of response in HEX
			if((int)(buffer[i]) > 32)
				temp[i%16] = buffer[i];//If Ascii is greater then 33 prints it out
			else
				temp[i%16] = '.';//if ascii value is less then 33 
		}
		//prints out three spaces when the hex dump stops so it displays corrently
		while(i % 16 != 0){
			printf("   ");
			i++;
		}
		printf(" %s\n",temp);
    }
}

/************************************************************
// Function Name: returnIP
// Parameters: char *buf, int i
// Description: Receives the buffer and finds the octet and 
// returns position.
************************************************************/
int returnIP(char *buf, int i)
{
	int x = 0;
	if (buf[i] < 0) 
		x = buf[i] + 256;
	else
		x = buf[i];
	return x;
}

/************************************************************
// Function Name: main
// Parameters: none
// Description: Gets URL from user, sets up socket and calls 
// fucntions and displays an ip address.
************************************************************/
int main ()
{
	char buffer[512];
	char Url[512];
	int socksd;
	int IPfound = 0;
	//creates structure
	struct sockaddr_in server;
	printf("\nPlease enter in the URL: ");
	fgets(Url,sizeof(Url),stdin);
	Url[ strlen ( Url ) - 1 ] = '\0';
	printf("\nURL: %s\n", Url);
	//creates socket with structure
	socksd = socket(AF_INET,SOCK_DGRAM,0); 
	if (socksd < 0) 
		error("ERROR opening socket");  
	connecttoDNSserver(&server);//connects to DNS server
	printf("\nDNS Query:\n");//send DNS query to server
	sendDNSquery(&socksd, createDNSquery(buffer, Url), server);//sends information to DNS server
	printf("\nDNS Response:\n");
	recvDNSrespond(&socksd,buffer,server);//recieve DNS respond from server
	IPfound = parseDNSresponse(buffer);//Find server IP and prints it
	//displays the IP if found
	if (IPfound != -1) 
		printf("\nIP address for %s : %d.%d.%d.%d\n\n",Url,returnIP(buffer,IPfound),returnIP(buffer,IPfound+1),returnIP(buffer,IPfound+2),returnIP(buffer,IPfound+3));
	else
		error("No DNS reponse\n");
	close(socksd); //closes socket descriptor
	return 1;	
}
