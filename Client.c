#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<signal.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<pthread.h>


#define LENGTH 2048
#define MAX_CLIENTS 10

/* Global variables */
volatile sig_atomic_t flag = 0;
int sockfd = 0;
char name[32];

/* Function for overwriting standard output */
void str_overwrite_stdout(){
        printf("%s", "-> ");
        fflush(stdout);
}

/* Function: If new line entered send message */
void str_new_line(char *arr, int length)
{
  int i;
  for(i=0;i<length;i++){
    if(arr[i] == '\n'){
      arr[i] = '\0';
      break;
    }
  }
}

void catch_and_exit(int sig) {
  flag = 1;
}

/* Function for sending messages */
void send_msg_handler(){
  char message[LENGTH] = {};
    char buffer[LENGTH + 32] = {};

  while(1) {
        str_overwrite_stdout();
    fgets(message, LENGTH, stdin);
    str_new_line(message, LENGTH);

    if(strcmp(message,"exit") == 0)
    {
        break;
    }
    else{
        sprintf(buffer, "%s: %s\n",name,message);
        send(sockfd, buffer, strlen(buffer), 0);
    }
        bzero(message, LENGTH);
        bzero(buffer, LENGTH + 32);
  }
  catch_and_exit(2);
}

/* Function for recieving messages */
void recv_msg_handler(){
        char message[LENGTH] = {};
  while(1) {
                int recieve = recv(sockfd, message, LENGTH, 0); // Recieve message
        if(recieve > 0)
        {
                printf("%s", message);
                str_overwrite_stdout();
        } else if (recieve == 0)
        {
                break;
        }
        bzero(message, LENGTH);
  }
}

int main(int argc, char **argv)
{
        if(argc != 2)
        {
                printf("Usage: ./Client <port number>\n");
                return EXIT_FAILURE;
        }

        char *ip = "127.0.0.1"; // IP Address
        int port = atoi(argv[1]);

        signal(SIGINT, catch_and_exit);

        printf("Please enter your name: ");
        fgets(name, 32, stdin);
        str_new_line(name, strlen(name));


        if(strlen(name) > 32 || strlen(name) <2){
                printf("Name must be more than 2 and less than 32 characters.\n");
                return EXIT_FAILURE;
        }

        struct sockaddr_in server_addr;

/* Socket settings */
          sockfd = socket(AF_INET, SOCK_STREAM, 0);
          server_addr.sin_family = AF_INET;
          server_addr.sin_addr.s_addr = inet_addr(ip);
          server_addr.sin_port = htons(port);


/* Connect to the Server */
        int err = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
        if(err == -1) {
                printf("ERROR: Connection Failed, Kindly check the port number you've entered\n");
                return EXIT_FAILURE;
            }
/* Send name */
    send(sockfd, name, 32, 0);
    printf("\n*******************************************\n");
    printf("\n === WELCOME TO CHAT CLIENT SERVER === \n");
    printf("\n*******************************************\n");

    pthread_t send_msg_thread;

  if(pthread_create(&send_msg_thread, NULL, (void *) send_msg_handler, NULL) != 0) {
                printf("ERROR: pthread\n");
        return EXIT_FAILURE;
  }

  pthread_t recv_msg_thread;
  if(pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0) {
        printf("ERROR: pthread\n");
        return EXIT_FAILURE;
  }

  while(1){
        if(flag){
                printf("\nBye, I'm leaving.\n");
                break;
        }
  }

  close(sockfd);

  return EXIT_SUCCESS;
}
