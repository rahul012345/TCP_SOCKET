//STEP 1 : creating a socket file descriptor
//STEP 2 : binding the socket with port and IP
//STEP 3 : listening to the client requests
//STEP 4 : accept the TCP conenction from the client side
//STEP 5 : recv/send for data communication
//STEP 6 : closing the file descriptor


//TASK-1 --> maintain the connected clients details in server data structure
//TASK-2 --> remove clients details on client exit 
//TASK-3 --> respond to command like LIST and CONNECT from clients
//TASK-4 --> communication between clients through server 


#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/select.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>

//Macros

#define SERVER_PORT 9000
#define LISTEN_BACKLOG 5
#define MAX_NAME_SZE 20
#define NO_OF_CLIENTS 10
#define MAX_BUFFER_SIZE 1024
#define CONNECT_CLIENT_TO_CHAT "Please connect to the client to chat"
#define CONNECTED "Connected ....."

static int listen_fd = 0;

//Data structure
struct client {
    char cname[MAX_NAME_SZE];
    char chatwith[MAX_NAME_SZE];
    int chatwith_fd;
    int file_des;
    int port;
    char ip[INET_ADDRSTRLEN];
};
struct server_data {
    int total_client;
    struct client client_list[NO_OF_CLIENTS];
};

struct server_data server;

//Function declarations
int server_create_socket(int *listen_fd);
int server_new_client_handle(int listen_fd, int *new_socket_fd);
int server_recv_from_client(int socket_client, char *recv_msg);
int server_send_to_client(int socket_client, char *send_msg);

int server_build_fdsets(int listenfd, fd_set *readfds, fd_set *writefds, fd_set *exceptfds);
int server_select(int max_fd, int listenfd, fd_set *readfds, fd_set *writefds);
void server_delete_client(int socket_fd_del);
void server_add_new_client(struct sockaddr_in client_info, int new_socket_fd);
int process_recv_data(int socket,char*buffer);
int find_the_client_index_list(int socket);
int find_the_client_index_by_name(char*name);
void cleanup(void);


int main() {
    int new_socket = 0;
    fd_set readfds;
    fd_set writefds;
    fd_set exceptfds;
    int max_fd= 0;

    memset(&server,0,sizeof(struct server_data));
    printf("Server Started !!!\n");

    if(server_create_socket(&listen_fd) != 0) {
        perror("ERROR : creation socket failed");
        exit(0);
    }
    max_fd = listen_fd;

    while(1) {
        max_fd = server_build_fdsets(listen_fd, &readfds, &writefds, &exceptfds);
        server_select(max_fd,listen_fd, &readfds, &writefds);
    }

    cleanup();
    printf("Bye From server\n");

    return 0;
}


//create the server socket
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

//Accept the connection to server
int server_new_client_handle(int listen_fd, int *new_socket_fd) {
        struct sockaddr_in client_addr;
        int len = sizeof(struct sockaddr);
        bzero(&client_addr,sizeof(client_addr));
        if((*new_socket_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &len)) < 0) {
            perror("ERROR :accept failed");
            return -1;
        }
        server_add_new_client(client_addr ,*new_socket_fd);

    return 0;
}

//Receiving socket data from clients
int server_recv_from_client(int client_socket, char *recv_msg) {
    int read_bytes = 0;
    memset(recv_msg,0,strlen(recv_msg));
    
    if((read_bytes = recv(client_socket, recv_msg, MAX_BUFFER_SIZE, 0)) > 0) {
            process_recv_data(client_socket, recv_msg);
    }
    else if(read_bytes == 0) {
           printf("Client Disconnected\n");
           server_delete_client(client_socket);  
    }
    else {
            printf("ERROR: recv failed\n");
        }
    return 0;
}

int server_send_to_client(int client_socket, char *send_msg) {
    int write_bytes = 0;
    int len  =strlen(send_msg);
    if((write_bytes = send(client_socket, send_msg, len, 0)) > 0) {
            printf("\n[CLIENT : %d] || Wrote [%d] number of bytes || BYTES = [%s]\n",client_socket,write_bytes, send_msg);
    }
    else {
            perror("Error : send failed");
            return -1;
    }

    return write_bytes;
}

int server_build_fdsets(int listenfd, fd_set *readfds, fd_set *writefds, fd_set *exceptfds) {
    int max_fd = listenfd;

    FD_ZERO(readfds);
    FD_SET(listenfd, readfds);
    FD_SET(STDIN_FILENO,readfds);
    fcntl(STDIN_FILENO,F_SETFL,O_NONBLOCK);

    for(int i = 0; i<server.total_client; i++) {
        FD_SET(server.client_list[i].file_des,readfds);
        max_fd++;
    }
    return max_fd;
}

//select based processing 
int server_select(int max_fd,int listen_fd, fd_set *readfds, fd_set *writefds) {
    int new_socket_fd = 0;
    char recv_msgg[MAX_BUFFER_SIZE] ;
    char send_buff[MAX_BUFFER_SIZE] ;
    memset(recv_msgg, 0 ,sizeof(recv_msgg));
    memset(send_buff, 0 ,sizeof(send_buff));

    int action = select(max_fd+1,readfds,writefds,NULL,NULL);

    if(action == -1 || action == 0) {
        perror("ERROR: select");
        exit(0);
    }

    //check the server listenfd
     if(FD_ISSET(listen_fd,readfds)) {
        server_new_client_handle(listen_fd,&new_socket_fd);
        printf("New socket created = %d\n",new_socket_fd);              
    }

    //check the data from stdin and send message to all connected clients
    if(FD_ISSET(STDIN_FILENO,readfds)) {
         if(read(0,send_buff,sizeof(send_buff))>0) {
            for(int i = 0;i<server.total_client;i++)
                server_send_to_client(server.client_list[i].file_des,send_buff);
        }             
    }
     
    for(int i = 0; i<server.total_client; i++) {    
           if(FD_ISSET(server.client_list[i].file_des,readfds)) {
               server_recv_from_client(server.client_list[i].file_des, recv_msgg); 
    } 
    } 
    return 0;
}


//Detete the client data on client exit
void server_delete_client(int socket_fd_del) {
    int i = 0;
    int index = 0;

    for(i=0;i<NO_OF_CLIENTS;i++) {
        if(server.client_list[i].file_des == socket_fd_del) {
            for(index = i; index<NO_OF_CLIENTS;index++ ) {
                server.client_list[index] = server.client_list[index+1];
            }
        }
    }

    server.total_client--;
    printf("Socket deleted  = [%d]\n",socket_fd_del);
    close(socket_fd_del);
}

//Adding a new client to the server
void server_add_new_client(struct sockaddr_in client_info, int new_socket_fd) {
    char ip[INET_ADDRSTRLEN] = {0};
    char buffer[MAX_BUFFER_SIZE] = {0};
//get extra server details
    server_recv_from_client(new_socket_fd,buffer);
   
//get the IP and Port client details
    int port = ntohs(client_info.sin_port);
    inet_ntop(AF_INET, &(client_info.sin_addr), ip, INET_ADDRSTRLEN);
    printf("[CLIENT-INFO] : [port = %d , ip = %s]\n",port, ip);
   
    if(server.total_client >=NO_OF_CLIENTS) {
        perror("ERROR : no more space for client to save");
    }
//populate the new client data 
    strncpy(server.client_list[server.total_client].cname ,buffer,strlen(buffer));
    server.client_list[server.total_client].port = port;
    strcpy(server.client_list[server.total_client].ip, ip);
    server.client_list[server.total_client].file_des=new_socket_fd;
    
    server.total_client++;
    
}

//processing the received data from clients
int process_recv_data(int socket,char*buffer) {
    char chat_c[MAX_BUFFER_SIZE];
    char buffer_send[MAX_BUFFER_SIZE] = {0};
    int index_sender = 0;
    int index_receiver = 0;
    int len = 0;
    index_sender = find_the_client_index_list(socket);

    if(strncmp(buffer, "LIST",4) ==0) {
         memset(buffer,0,sizeof(buffer));
         for(int i=0;i<server.total_client;i++) {
             strcat(buffer,server.client_list[i].cname);
             strcat(buffer,";");
         }
        server_send_to_client(socket,buffer);
        goto out;
    }
    if(strncmp(buffer, "CONNECT",7) == 0) {
        
        sscanf(buffer,"%*[^:]:%s",chat_c);
        strcpy(server.client_list[index_sender].chatwith, chat_c);
       
        index_receiver = find_the_client_index_by_name(server.client_list[index_sender].chatwith);
        server.client_list[index_sender].chatwith_fd = server.client_list[index_receiver].file_des;
        server_send_to_client(server.client_list[index_sender].file_des,CONNECTED);
        goto out;
    }

    if(strlen(server.client_list[index_sender].chatwith) != 0){
        snprintf(buffer_send,sizeof(buffer_send),"[%s] : %s",server.client_list[index_sender].cname,buffer);
        printf("Buffer  =%s\n",buffer_send);
        server_send_to_client(server.client_list[index_sender].chatwith_fd,buffer_send);
    }

out:
    return 0;
}

//find index of the client data structure from client socket
int find_the_client_index_list(int socket) {
    int index = 0;
    for(int i = 0; i<server.total_client; i++) {
           if(server.client_list[i].file_des == socket) {
               index =i;
           }
    }
    return index;
}

//find index of the client data structure from client name
int find_the_client_index_by_name(char*name) {
    int index = 0;
    for(int i = 0; i<server.total_client; i++) {
           if(strcmp(server.client_list[i].cname,name) == 0) {
               index =i;
           }
    }
    return index;
}

void cleanup() {

    close(listen_fd);
    for(int i = 0; i<server.total_client; i++) {
           close(server.client_list[i].file_des);
    }
}