//STEP 1 : creating a socket file descriptor
//STEP 2 : binding the socket with port and IP
//STEP 3 : listening to the client requests
//STEP 4 : accept the TCP conenction from the client side
//STEP 5 : recv/send for data communication
//STEP 6 : closing the file descriptor



#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

#define SERVER_PORT 9000
#define LISTEN_BACKLOG 5

#define MAX_BUFFER_SIZE 1024


//Create a listening socket for server
int server_create_socket(int *listen_fd);
int server_new_client_handle(int listen_fd, int *new_socket_fd);
int server_recv_from_client(int socket_client, char *recv_msg);
int server_send_to_client(int socket_client, char *send_msg);
int server_create_new_process(int listen_fd, int);

int main() {

    int listen_fd = 0;
    int new_socket = 0;
    
    printf("Server Started !!!\n");

    if(server_create_socket(&listen_fd) != 0) {
        perror("socket failed");
        exit(0);
    }
    
    while(1) {
        server_new_client_handle(listen_fd,&new_socket);
        server_create_new_process(listen_fd,new_socket);
    }

    close(listen_fd);
    printf("Bye From server\n");

    return 0;
}



int server_create_socket(int *listen_fd) {
    struct sockaddr_in server_addr;

    if((*listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("ERROR : socket creation failed");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT); 
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if( 0!=bind(*listen_fd,(struct sockaddr*)&server_addr,sizeof(struct sockaddr))){
         perror("ERROR : socket bind failed");
         return -1;
    }

    if(0!=listen(*listen_fd, LISTEN_BACKLOG)) {
         perror("ERROR : socket listen failed");
         return -1;
    }
    return 0;

}

int server_new_client_handle(int listen_fd, int *new_socket_fd) {
        struct sockaddr_in client_addr;
        int len = sizeof(struct sockaddr);
        bzero(&client_addr,sizeof(client_addr));
        if((*new_socket_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &len)) < 0) {
            perror("ERROR :accept");
            return -1;
        }

    return 0;
}

int server_recv_from_client(int client_socket, char *recv_msg) {
     int read_bytes = 0;
     
     if((read_bytes = recv(client_socket, recv_msg, MAX_BUFFER_SIZE, 0)) > 0) {
            printf("\n[CLIENT_ID : %d] || Read [%d] number of bytes || BYTES = [%s]\n",client_socket,read_bytes, recv_msg);
    }
    else if(read_bytes == 0) {
            printf("Client Disconnected\n");
            fflush((FILE*)&client_socket);
            close(client_socket);
            exit(0);
    }
    else {
            printf("ERROR: recv failed\n");
            exit(0);
        }
    return 0;
}

int server_send_to_client(int client_socket, char *send_msg) {
    int write_bytes = 0;
       if((write_bytes = send(client_socket, send_msg, strlen(send_msg), 0)) > 0) {
            printf("\n[CLIENT : %d] || Wrote [%d] number of bytes || BYTES = [%s]\n",client_socket,write_bytes, send_msg);
        }
        else {
            perror("send failed");
            return -1;
        }

    return write_bytes;
}


int server_create_new_process(int listen_socket, int new_socket) {
    int status = 0;
    char buffer[MAX_BUFFER_SIZE];

    int ret  = fork();
    if(ret == 0) {
        printf("In Child process\n");
        printf("new socket = %d\n",new_socket);
        close(listen_socket);

        while(1) {
            server_recv_from_client(new_socket,buffer);
        }
    }
    else if(ret >0) {
        printf("In Parent process\n");
    }
    else {
        printf("ERROR : fork() failed\n");
        exit(0);
    }

    return 0;
}