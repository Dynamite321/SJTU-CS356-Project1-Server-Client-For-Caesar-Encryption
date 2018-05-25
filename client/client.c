
/* Projet 1, problem 4 */
/* client.c programmed by Yanjun Fu, 516030910354 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int main(int argc, char *argv[])
{
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer[256];
	portno = 2050;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
        printf("ERROR opening socket");
        exit(1);
	}
	server = gethostbyname("127.0.0.1");
	if (server == NULL)
	{
        printf("ERROR, no such host");
        exit(0);
	}
	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);

	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
        printf("ERROR connecting");
        exit(1);
	}
	printf("PLease enter the message:\n");

	////////////////////////////////////////////////////////

	int flag = 1; //
	while(flag)
	{
        bzero(buffer,256);
        fgets(buffer,255,stdin); // get input
        n = write(sockfd, buffer, strlen(buffer)); //write the input into the socket
        if (n < 0)
        {
            printf("Error writing to socket");
            exit(1);
        }
        if (buffer[0] == ':' && buffer[1] == 'q') // judge of the client need to quit
        {
        	flag = 0;
        	printf("Client closing...\n");
        	break;
        }
        bzero(buffer,256);
        n = read(sockfd, buffer, 255); //read the output from socket
        if (n < 0)
        {
            printf("Error writing to socket");
            exit(1);
        }
        printf("%s",buffer);
	}
	close(sockfd);

	/////////////////////////////////////////////////////////

	return 0;
}
