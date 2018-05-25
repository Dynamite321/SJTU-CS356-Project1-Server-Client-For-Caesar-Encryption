
/* Projet 1, problem 4 */
/* server.c programmed by Yanjun Fu, 516030910354 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int counter = 0; //record the current number of clients
int sockfd_array[100000]; //this array is used to store the newsockfd in different threads

/*NOTICE: I just make sockfd_array have space of 100000, 
which means the number of clients can't be larger than 100000*/

pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER; //this semaphore is used to protect counter
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER; //this semaphore is used to make sure only two clients can get served

void *retval; //just a void* used in pthread_exit(), don't have sepecific meaning

void work_stuff(int *sockfd);
void *serve(void *newsockfd);

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno, clilen, n;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("ERROR opening socket\n");
        exit(1);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;

    portno = 2050;
    serv_addr.sin_port = htons(portno);

    serv_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("ERROR on binding\n");
        exit(1);
    }
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    printf("Server initiating...\n");

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    int i = 0; //the index of sockfd_array

    while(1)
    {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);  //get new sockfd
        sockfd_array[i] = newsockfd;
        if (newsockfd < 0)
        {
            printf("ERROR On accept\n");
        }
        work_stuff(&sockfd_array[i]); //work_stuff is a function packing all the details of create thread and do Caesar cipher
        ++i;
    }
    return 0;
}

void work_stuff(int *sockfd)
{
    pthread_t thread1;

    pthread_create(&thread1, NULL, serve, sockfd); //create a thread
}

void *serve(void *sockfd)
{
    int n, cur; //n record the return value of read() and write()
    char buffer[256], tmp[300] = "From server: "; //buffer store the message in socket
    int newsockfd = (int)(*((int*)sockfd));

    pthread_mutex_lock(&count_mutex); //protect client counter(mutual exclusion)
    ++counter;
    pthread_mutex_unlock(&count_mutex);

    pthread_mutex_lock(&count_mutex); //protect client counter(mutual exclusion)
    if (counter >= 3)
    {
        bzero(buffer,256); //initialize the buffer before reading
        n = read(newsockfd,buffer,255);
        printf("Server thread closing...\n");
        strcat(tmp, "Please wait...\n"); //just to fulfill the demand of output style
        n = write(newsockfd, tmp, strlen(tmp));
        if (n < 0)
        {
            printf("Error on writing to socket\n");
            exit(1);
        }
        pthread_cond_wait(&condition_var, &count_mutex); 
        /* after telling the client that it needs to wait, we need to put it in the waiting list, 
        this is the funtion of the phtread_cond_wait(), 
        this function will be called when the number of clients more than 2 */
    }
    pthread_mutex_unlock(&count_mutex);

    while(1)
    {

        int i = 0;
        bzero(buffer,256);
        strcpy(tmp, "From server: ");
        n = read(newsockfd,buffer,255);
        if (n < 0)
        {
            printf("Error on reading from socket\n");
            exit(1);
        }
        if (!(buffer[0] == ':' && buffer[1] == 'q'))
        {
            printf("Receiving message: %s",buffer);
            /* following lines is to do the Caesar cipher */
            for (i = 0; i < strlen(buffer); ++i)
            {
                if ((buffer[i] >= 'a' && buffer[i] <= 'w') || (buffer[i] >= 'A' && buffer[i] <= 'W'))
                {
                    buffer[i] = buffer[i] + 3;
                }
                else if ((buffer[i] >= 'x' && buffer[i] <= 'z') || (buffer[i] >= 'X' && buffer[i] <= 'Z')) //x, y, z need to be considered specially
                    buffer[i] = buffer[i] - 23;
            }
            strcat(tmp, buffer);
            n = write(newsockfd, tmp, strlen(tmp));
            if (n < 0)
            {
                printf("Error on writing to socket\n");
                exit(1);
            }
        }
        else
        {
            break; // call ":q", quit
        }

    }

    pthread_mutex_lock(&count_mutex);
    --counter;
    cur = counter;
    pthread_mutex_unlock(&count_mutex);

    pthread_cond_signal(&condition_var); 
    /* when a client calls ":q" to exit, then we can let one more client to get served,
    if ther exist a client waiting, pthread_cond_signal() let the client to be served */

    pthread_exit(retval); //exit the thread
}
