//STEP 1 : creating a socket file descriptor
//STEP 2 : connect to the TCP server
//STEP 3 : recv/send for data communication
//STEP 4 : closing the file descriptor


#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

#define SERVER_PORT 9000

int main() {
    int socket_fd = 0;
    char buffer[1024] = {0};
    int write_bytes = 0;
    struct sockaddr_in server_addr;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    strcpy(buffer, "Hello : SERVER");
    connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));

    while(1) {
        write_bytes = send(socket_fd,buffer,sizeof(buffer) ,0);
        printf("Write [%d] bytes , Bytes : [%s]\n",write_bytes, buffer);
        sleep(3);
    }
    close(socket_fd);


    return 0;
}