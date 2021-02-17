#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <errno.h>


#define DEST_PORT 2000
#define SERVER_IP_ADDRESS "127.0.0.1"
#define BUF_SIZE 20

FILE *f;
char data_buf[BUF_SIZE];

void setup_tcp_communication(char client_file_name[]){

 int sockfd = 0, sent_recv_bytes = 0;
 int addr_len = 0;	
 struct sockaddr_in server_addr;

 server_addr.sin_port = DEST_PORT;
 server_addr.sin_family = AF_INET;
 struct hostent *host  = (struct hostent *)gethostbyname(SERVER_IP_ADDRESS); 
 // set first address as sin_addr
 server_addr.sin_addr = *((struct in_addr *)host->h_addr);
 
 addr_len = sizeof(struct sockaddr);
 if((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
  fprintf(stderr, "error:%d setting socket \n", errno);
  exit(1);
 };
 
 if(connect(sockfd,(struct sockaddr*)&server_addr, sizeof(struct sockaddr)) == -1){
  fprintf(stderr,"error connecting to the server %s ",SERVER_IP_ADDRESS);
  exit(1);
 }

 while(1){

  printf("which file you want to download ? : \n");
  scanf("%s", data_buf+1);
  data_buf[0] = sizeof(data_buf);
  printf("%s\n", data_buf+1);
  if(!strcmp(data_buf+1,"/quit")){
   fprintf(stderr,"terminating connection ... \n");
   return;	  
  };
  if((sent_recv_bytes = sendto(sockfd, data_buf, sizeof(data_buf), 0, (struct sockaddr*)&server_addr, sizeof(struct sockaddr))) == -1){
   fprintf(stderr,"error sending information to server %s\n", SERVER_IP_ADDRESS);
   exit(1);
  };
  
  printf("No of bytes sent = %d\n", sent_recv_bytes);
  memset(data_buf, 0, sizeof(data_buf));
  if((sent_recv_bytes = recvfrom(sockfd, (char *)data_buf, sizeof(data_buf), 0,(struct sockaddr *)&server_addr, &addr_len)) == -1){
   fprintf(stderr, "receiving file size from server failed \n");
   exit(1);   
  };
  
  strcat(client_file_name,".txt");
  f = fopen(client_file_name, "w");
  if(f == NULL) {
    fprintf(stderr,"error opening file\n");
    exit(1);
  };
  long file_size = atol(data_buf);
  int total_buf_count = file_size / sizeof(data_buf) + 1;
  printf("file size: %ld\n",file_size);  
  int buf_count = 0;
  long total_read_bytes = 0;
  memset(data_buf, 0, sizeof(data_buf));

  while(buf_count != total_buf_count){
    sent_recv_bytes = recvfrom(sockfd, &data_buf, sizeof(data_buf), 0,(struct sockaddr *)&server_addr, &addr_len);
    buf_count++;
    total_read_bytes += sent_recv_bytes;
    printf("downloading ... %ld/%ldByte(s) [%d%%]\n", total_read_bytes, file_size, ((buf_count*100) / total_buf_count)); 
    //printf("size : %ld\n", sizeof(data_buf));
    //printf("set : %d \n", sent_recv_bytes);
    /* for(int i = 0; i < set_recv_bytes; i++){
     fputc((char)read_buffer[i],f);
     byte_count++;
      printf("%c\n ", data_buf[i]); 
    }*/

    fwrite(data_buf, sizeof(char) , sent_recv_bytes , f);
    
  }
   fclose(f);
 }
}

int main(int argc, char **argv){
 if(argc != 2) {
  fprintf(stderr, "Usage: %s your_file_name\n", argv[0]);
  return 1; 
 }
 setup_tcp_communication(argv[1]);
 printf("Application quits\n");
 return 0;
}
