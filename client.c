//STEP 1 : creating a socket file descriptor
//STEP 2 : connect to the server
//STEP 3 : recv/send for data communication
//STEP 4 : closing the file descriptor



#include<stdio.h>
#include<sys/socket.h>
#include<sys/select.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>

#define SERVER_PORT 9000
#define MAX_NAME_SZE 20
#define MAX_BUFFER_SIZE 1024

char client_name[MAX_NAME_SZE] ={0};

//Function declaration
int client_create_socket(int *listen_fd);
int client_recv_from_server(int socket_client, char *recv_msg);
int client_send_to_server(int socket_client, char *send_msg);

int client_build_fdsets(int listenfd, fd_set *readfds, fd_set *writefds, fd_set *exceptfds);
int client_select(int max_fd, int listenfd, fd_set *readfds, fd_set *writefds);

int main(int argc, char *argv[]) {

    int listen_fd = 0;
    int new_socket = 0;
    int max_fd = 0;
    fd_set readfds;
    fd_set writefds;
    fd_set exceptfds;
    
    if(argc > 2) {
        perror("ERROR : Parameters error");
        exit(0);
    }
    
    strcpy(client_name,argv[1]);


    if(client_create_socket(&listen_fd) != 0) {
        perror("ERROR : socket creation failed");
        exit(0);
    }
    max_fd = listen_fd;
  
    
    while(1) {
         max_fd = client_build_fdsets(listen_fd, &readfds, &writefds, &exceptfds);
        client_select(max_fd,listen_fd, &readfds, &writefds);
        
    }

    close(listen_fd);

    return 0;
}


//Create a client socket
int client_create_socket(int *listen_fd) {
    struct sockaddr_in server_addr;

    if((*listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("ERROR : socket creation failed");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT); 
    server_addr.sin_addr.s_addr = INADDR_ANY;

    
    if(0!=connect(*listen_fd,(struct sockaddr *)&server_addr,sizeof(struct sockaddr))) {
        perror("ERROR : connect failed");
        return -1;
    }

    client_send_to_server(*listen_fd,client_name);

    return 0;

}

//Receive data from server
int client_recv_from_server(int client_socket, char *recv_msg) {
     int read_bytes = 0;
     
     if((read_bytes = recv(client_socket, recv_msg, MAX_BUFFER_SIZE, 0)) > 0) {
            printf("%s\n",recv_msg);
    }
    else if(read_bytes == 0) {
            printf("Client Disconnected\n");
            close(client_socket);
    }
    else {
            printf("ERROR: recv failed\n");
        }
    return 0;
}

//Send data to server
int client_send_to_server(int client_socket, char *send_msg) {
    int write_bytes = 0;
    int len =strlen(send_msg);

       if((write_bytes = send(client_socket, send_msg, len, 0)) <=0) {
            perror("ERROR : send failed");
            return -1;
        }

    return write_bytes;
}

//Refresh set Fds
int client_build_fdsets(int listenfd, fd_set *readfds, fd_set *writefds, fd_set *exceptfds) {
    int max_fd = listenfd;

    FD_ZERO(readfds);
    FD_SET(listenfd, readfds);
    FD_SET(listenfd, writefds);
    FD_SET(STDIN_FILENO,readfds);
    fcntl(STDIN_FILENO,F_SETFL,O_NONBLOCK);

    return max_fd;
}

//Client select call
int client_select(int max_fd,int listen_fd, fd_set *readfds, fd_set *writefds) {
    char recv_msgg[MAX_BUFFER_SIZE] ;
    char send_buff[MAX_BUFFER_SIZE] ;
    memset(recv_msgg, 0 ,sizeof(recv_msgg));
    memset(send_buff, 0 ,sizeof(send_buff));
    
    int action = select(max_fd+1,readfds,writefds,NULL,NULL);

    if(action == -1 || action == 0) {
        perror("ERROR: select");
        exit(0);
    }

     if(FD_ISSET(listen_fd,readfds)) {
        client_recv_from_server(listen_fd,recv_msgg);             
    }

     if(FD_ISSET(STDIN_FILENO,readfds)) {
         if(read(0,send_buff,sizeof(send_buff))>0) {
                client_send_to_server(listen_fd,send_buff);
        }
                   
    }
     
    return 0;
}