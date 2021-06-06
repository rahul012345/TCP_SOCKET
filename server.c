//STEP 1 : creating a socket file descriptor
//STEP 2 : binding the socket with port and IP
//STEP 3 : listening to the client requests
//STEP 4 : accept the TCP conenction from the client side
//STEP 5 : recv/send for data communication
//STEP 6 : closing the file descriptor



#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

#define SERVER_PORT 9000
#define LISTEN_BACKLOG 5


//Create a listening socket for server
int server_create_socket(int *listen_fd);
//new connection handler
int server_new_client_handle(int listen_fd, int *new_socket_fd);
//receive message from client
int server_recv_from_client(int socket_client, char *recv_msg);
//send message to client
int server_send_to_client(int socket_client, char *send_msg);
//create a new process of server
int server_create_new_process(int listen_fd, int);


int main() {

    int socket_fd = 0;
    int new_socket_fd = 0;
    char buffer[1024] = {0};
    int read_bytes = 0;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
   

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT); 
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(socket_fd,(struct sockaddr*)&server_addr,sizeof(struct sockaddr));

    listen(socket_fd, LISTEN_BACKLOG);
    int len = sizeof(struct sockaddr);
 while(1) {
        new_socket_fd = accept(socket_fd, (struct sockaddr*)&client_addr, &len);
  
        if((read_bytes = recv(new_socket_fd,buffer, sizeof(buffer), 0)) > 0) {
            printf("Read [%d] number of bytes BYTES = [%s]\n",read_bytes, buffer);
        }
    }

    close(socket_fd);
    close(new_socket_fd);

    printf("Bye From server");

    return 0;
}