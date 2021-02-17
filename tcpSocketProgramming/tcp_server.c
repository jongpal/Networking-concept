#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#define MAX_CLIENT_SUPPORTED 32
#define SERVER_PORT 2000
#define BUF_SIZE 20

int monitored_fd[MAX_CLIENT_SUPPORTED];
char data_buffer[BUF_SIZE];
FILE *f;

static void init_monitored_fd_set(){
 for(int i = 0; i < MAX_CLIENT_SUPPORTED; i++){
  monitored_fd[i] = -1;
 }
}

static void add_to_monitored_fd_set(int skt_fd) {
 for(int i = 0 ; i < MAX_CLIENT_SUPPORTED; i++){
  if(monitored_fd[i] != -1) continue;
  monitored_fd[i] = skt_fd;
  break; 
 }
}

static void remove_from_monitored_fd_set(int skt_fd){
 for(int i = 0 ; i < MAX_CLIENT_SUPPORTED; i++){
  if(monitored_fd[i] != skt_fd) continue;
  monitored_fd[i] = -1;
  break;
 }
}
// re-set fd_set
static void re_init_readfds(fd_set *fd_set_ptr){
 FD_ZERO(fd_set_ptr);

 for(int i = 0 ; i < MAX_CLIENT_SUPPORTED; i++){
  if(monitored_fd[i] == -1) continue;
  FD_SET(monitored_fd[i], fd_set_ptr);
 }
}
// get maximum file descriptor : for efficient select
static int get_max_fd(){
 int max = monitored_fd[0];
 for(int i = 1 ; i < MAX_CLIENT_SUPPORTED; i++)
  if(monitored_fd[i] > max) max = monitored_fd[i];
 
 return max;
}


void setup_tcp_multiplex_server() {
 int master_sock_tcp_fd = 0, sent_recv_bytes = 0, addr_len = 0;
 int comm_socket_fd = 0;
 fd_set readfds;
 struct sockaddr_in server_addr, client_addr;

 init_monitored_fd_set();
 if((master_sock_tcp_fd = socket(AF_INET, SOCK_STREAM,IPPROTO_TCP)) == -1){
  fprintf(stderr, "master socket creation failed\n");
  exit(1);
 }

 server_addr.sin_family = AF_INET;
 server_addr.sin_port = SERVER_PORT;
 server_addr.sin_addr.s_addr = INADDR_ANY;
 addr_len = sizeof(struct sockaddr);

 if(bind(master_sock_tcp_fd, (struct sockaddr *)&server_addr, addr_len) == -1){
  fprintf(stderr, "binding master fd and server failed\n");
  exit(1);
 }

 if(listen(master_sock_tcp_fd, 5) == -1){
  fprintf(stderr, "master socket listen failed\n");
  exit(1);
 }
 add_to_monitored_fd_set(master_sock_tcp_fd);

 
 while(1) {

  re_init_readfds(&readfds);

  printf("blocked on select System call... \n");

  if(select(get_max_fd()+1, &readfds, NULL, NULL, NULL) == -1){
   fprintf(stderr, "polling failed \n");
   exit(1);
  };

  // new connection request arrived
  if(FD_ISSET(master_sock_tcp_fd, &readfds)){
   fprintf(stderr,"New Connection request, accept the connection !\n");

   if((comm_socket_fd = accept(master_sock_tcp_fd, (struct sockaddr*)&client_addr, &addr_len)) == -1){
    fprintf(stderr, "creating new communication file descriptor failed \n");
    exit(1);
  };
     
   add_to_monitored_fd_set(comm_socket_fd);
   printf("Connection accepted from client : %s: %u\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
  } 
  // data communication
  else {
   memset(data_buffer, 0, sizeof(data_buffer));
   for(int i = 0; i< MAX_CLIENT_SUPPORTED; i++){
    if(!FD_ISSET(monitored_fd[i], &readfds)) continue;
    
    comm_socket_fd = monitored_fd[i];
    int t_rcvd = 0;// t_rcvd : the buffer size of connected client
    
    while((sent_recv_bytes = recvfrom(comm_socket_fd, data_buffer+t_rcvd, sizeof(data_buffer)-t_rcvd, 0, (struct sockaddr *)&client_addr, &addr_len))){
     if(sent_recv_bytes == -1) {
      fprintf(stderr, "error receiving bytes from client, try again\n");
      break;
     }
     printf("Server recvd %d bytes from client %s:%u\n", sent_recv_bytes, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
     t_rcvd+= sent_recv_bytes;
     if(data_buffer[0] <= t_rcvd) break;
    }
    if(!strcmp(data_buffer+1,"/quit")) { // send "end" if client wants to terminate the connection
     fprintf(stderr, "terminating connection with client %s:%u\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port)); 
     close(comm_socket_fd);
     remove_from_monitored_fd_set(comm_socket_fd);
     break;
    }
    // get filename from data_buffer data_buffer[0] is for buffer size info
    // of client side buffer
    strcat(data_buffer+1, ".txt"); 
    f = fopen(data_buffer+1, "r");
    if(f == NULL){
     fprintf(stderr,"error : opening file, terminating connection on fd : %d \n", comm_socket_fd);
     close(comm_socket_fd);
     remove_from_monitored_fd_set(comm_socket_fd);
     break;
    }
    // calculating file size and send it to client
    fseek(f, 0, SEEK_END);
    int file_size = ftell(f);
    int total_count = file_size / sizeof(data_buffer) + 1;
    fseek(f, 0, SEEK_SET);
    int count = 0;
    int total_sent_bytes = 0;
    int read_bytes = 0;
    memset(data_buffer, 0, sizeof(data_buffer));
    sprintf(data_buffer, "%d", file_size);
    if((sent_recv_bytes = sendto(comm_socket_fd, (char *)&data_buffer, t_rcvd, 0, (struct sockaddr *)&client_addr, sizeof(struct sockaddr))) == -1){
     fprintf(stderr, "sending file size info to client failed \n");
     break;     
    };
    memset(data_buffer, 0, sizeof(data_buffer));
       
    while((read_bytes = fread(data_buffer, sizeof(char), t_rcvd, f)) > 0){
     sent_recv_bytes = sendto(comm_socket_fd,(char *)&data_buffer, read_bytes, 0, (struct sockaddr *)&client_addr, sizeof(struct sockaddr));
     count ++;
     total_sent_bytes += sent_recv_bytes;
	//for(int i = 0 ; i < t_rcvd; i++)
	// printf("sent : %c\n", data_buffer[i]);
     printf("Server sent %d bytes, %d/%d Byte(s) [%d%%]\n", sent_recv_bytes, total_sent_bytes, file_size, (count * 100 / total_count));
    }
    fclose(f);
    printf("file sending finished \n");
    }
   } 	   
  }
 }

int main(int argc, char **argv){
  setup_tcp_multiplex_server();
  return 0;
}
