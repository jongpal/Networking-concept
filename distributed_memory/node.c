#include "hash_db.h"
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#define _ef EXIT_FAILURE
#define BUF_SIZE 1024
#define NODE_NUM 4

// message types

#define PUT_FORWARD 1
#define GET_FORWARD 2
#define FETCH_VALUE 3
#define PUT_REPLY 4
#define GET_REPLY 5

/*
  making conceptual ring shaped distributed memory:
  different node share their own memory by networking 
  client will not know about what's going on behind the scene
  2GB memory * (3 Node) => client would think that he/she has total 6GB of memory
  - each node communicates with their successive node by UDP protocol
  - each node has their own memory (implemented by hash table(double hashing used here))
  - each node will check whether that key belongs to local memory and if not, it will forward the packet to successive node by UDP
  - if that successive node checks the key again and if it matches, it will request PUT_REPLY message to the original node that received PUT command from client 
  - this data transmission would use TCP, so inside of packet needs
    appl_layer_message structure which is defined below
  
*/



uint8_t buffer[BUF_SIZE];

uint16_t temp_value;

//simple hash function for memory distribution
int hash(int el){
  return el % NODE_NUM;   
}

//message structure
typedef struct appl_layer_message{
  uint8_t msg_id; 
  uint16_t key; 
  uint16_t value;
  uint16_t origin_port; // tcp port #
  uint32_t origin_ip_addr;
}msg_t;


//encode to Big Endian (Network way)
uint8_t* pack8(uint8_t *buf, uint8_t i){
  *buf++ = i;
  return buf;
}

uint8_t* pack16(uint8_t *buf, uint16_t i){
  *buf++ = i >> 8;
  *buf++ = i;
  return buf;
}

uint8_t* pack32(uint8_t *buf, uint32_t i){
  *buf++ = i >> 24;
  *buf++ = i >> 16;
  *buf++ = i >> 8;
  *buf++ = i;
  return buf;
}

//unpack
uint8_t unpacku8(uint8_t *buf){
  uint8_t i = *buf++;
  return i;
}
uint16_t unpacku16(uint8_t *buf){
  return ((unsigned int)buf[0]<<8 | buf[1]);  
}

uint32_t unpacku32(uint8_t *buf){
  return ((uint32_t)buf[0]<<24) |
         ((uint32_t)buf[1]<<16) |
	 ((uint32_t)buf[2]<<8)  |
	  buf[3];
}

void string_space_trim(char *string){
 
  if(!string) return;

  char *ptr = string;
  int len = strlen(ptr);
  
  if(!len) return;
  if(!isspace(ptr[0]) && !isspace(ptr[len-1])) return;
  
  while(len-1 > 0 && isspace(ptr[len-1])) {
    --len;
  }

  while(*ptr && isspace(*ptr)){
    ++ptr, --len;
  }

  memmove(string, ptr, len + 1); //+1 for NULL character 
}

static void parse(char input[], char *token[]){
 string_space_trim(input);
 token[0] = strtok(input, " ");
 if(strncmp(token[0], "PUT", strlen("PUT")) == 0){
   token[1] = strtok(NULL, ",");
   token[2] = strtok(NULL, "\0");
//   printf("\nparsing start: \n%s\n",token[0]);
//   printf("%s\n",token[1]);
//   printf("%s\n",token[2]);
 } else if(strncmp(token[0], "GET", strlen("GET"))==0) {
   token[1] = strtok(NULL, "\0");
//   printf("\nparsing start : \n%s\n", token[1]);
 }
}

int main(int argc, char **argv){
  if(argc != 5) {
    fprintf(stderr, "Usage: %s this_ip_addr this_udp_port_num successor_udp_port_num this_tcp_port_num", argv[0]);
    exit(_ef);
  }
  int tcp_master_sock_fd, udp_master_sock_fd,sockfd, comm_socket_fd,addr_len;
  struct sockaddr_in udp_addr, tcp_addr, next_node_addr, incoming_addr, tcp_data_addr;
  fd_set readfds;
  int opt = 1;
  int sent_recv_bytes = 0;
  addr_len = sizeof(struct sockaddr);
  
  char get_input[30];
  char *token[3];
  //initialize local db
  memset(hash_table, -1 , sizeof(hash_table));
  
  //creating socket
  if((tcp_master_sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))== -1) {

    fprintf(stderr, "TCP master socket creation failed\n");
    exit(_ef);
  }
  if((setsockopt(tcp_master_sock_fd, SOL_SOCKET, SO_REUSEADDR, (char *)& opt, sizeof(opt))) < 0){
    fprintf(stderr, "TCP socket creation failed for faster reusable purposes\n");
    exit(_ef);  
  };
  tcp_addr.sin_family = AF_INET;
  tcp_addr.sin_port = htons(atoi(argv[4]));
  tcp_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  
  if((udp_master_sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
    fprintf(stderr, "UDP master socket creation failed\n");
    exit(_ef);
  }
  if((setsockopt(udp_master_sock_fd, SOL_SOCKET, SO_REUSEADDR, (char *)& opt, sizeof(opt))) < 0){
    fprintf(stderr, "UDP socket creation failed for faster reusable purposes\n");
    exit(_ef);
  };
  
  udp_addr.sin_family = AF_INET;
  udp_addr.sin_port = htons(atoi(argv[2]));
  udp_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  
  // making a Ring shaped distributed memory
  // specifying next node 
  next_node_addr.sin_family = AF_INET;
  next_node_addr.sin_port = htons(atoi(argv[3]));
  next_node_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  
  // total 3 master fd : tcp, udp, stdin
  if(bind(tcp_master_sock_fd, (struct sockaddr *)&tcp_addr, addr_len) == -1) {
    fprintf(stderr, "binding tcp master fd with server failed \n");
    exit(_ef);
  }
  if(bind(udp_master_sock_fd, (struct sockaddr *)&udp_addr, addr_len) == -1) {
    fprintf(stderr, "binding udp master fd with server failed \n");
    exit(_ef);
  }
  if(listen(tcp_master_sock_fd, 5) == -1) {
    fprintf(stderr, "tcp master socket listen failed\n");
    exit(_ef);
  }
starting_point:  
  while(1){
    FD_ZERO(&readfds);
    FD_SET(tcp_master_sock_fd, &readfds);
    FD_SET(udp_master_sock_fd, &readfds);
    FD_SET(0, &readfds);
    memset(buffer, 0, sizeof(buffer));
    int max = tcp_master_sock_fd > udp_master_sock_fd ? tcp_master_sock_fd : udp_master_sock_fd;
  


    printf("input key and value(both are integers), usage :<Method> <key,value> ex) PUT <key>,<value> or GET <key>");
    printf("\nMethod available : PUT(save data), GET(fetch data)\n");
    printf("type right after block message : \n");
    
    printf("blocked on select System call...\n");
  
    if(select(max+1, &readfds, NULL, NULL, NULL) == -1){
      fprintf(stderr, "Polling failed..\n");
      exit(_ef);    
    };
  
    if(FD_ISSET(tcp_master_sock_fd, &readfds)){
      if((comm_socket_fd = accept(tcp_master_sock_fd, (struct sockaddr*)&tcp_data_addr, &addr_len)) == -1) {
	fprintf(stderr, "creating new communication file descriptor failed \n");
	exit(_ef);
      }
      printf("Connection accepted from node: %s : %u\n", inet_ntoa(tcp_data_addr.sin_addr), ntohs(tcp_data_addr.sin_port));
      while(1){
        memset(buffer, 0, sizeof(msg_t));
        if((sent_recv_bytes = recvfrom(comm_socket_fd, buffer, sizeof(msg_t), 0, (struct sockaddr*) &tcp_data_addr, &addr_len)) == -1) {
	  fprintf(stderr, "receiving message at tcp failed\n");
          break;	
        }
        uint8_t msg_id = unpacku8(buffer);

	//FETCH_VALUE message : get back with value that was temporarily saved here
        if(msg_id == 3){
	  //respond with 4(PUT_REPLY) message
	  pack8(buffer, 4);
	  uint8_t* ptr = buffer+3;
	  ptr = pack16(ptr, temp_value);
	  if((sent_recv_bytes = sendto(comm_socket_fd, buffer, ptr - buffer, 0, (struct sockaddr*) &tcp_data_addr, sizeof(struct sockaddr))) == -1){
	    fprintf(stderr, "sending GET_REPLY message at tcp failed\n");
	    exit(_ef);
	  }
	  // here you should receive success message and let client know the value was saved 
	  if((sent_recv_bytes = recvfrom(comm_socket_fd, buffer, 1, 0, (struct sockaddr*)&tcp_data_addr, &addr_len)) == -1){
	    fprintf(stderr, "receiving confirmation message failed\n");
  	    exit(_ef);	    
	  };
	  if(buffer[0] == 1) {
	    printf("Successfully saved value %d\n", temp_value); 
	    close(comm_socket_fd);
	  }
	  
	  goto starting_point;
        }

	// GET_REPLY message
        else if(msg_id == 5) {
	// printf the value
	  uint16_t got_val = unpacku16(buffer+3);
  	  if(!got_val) printf("No result with that key !\n");	  
	  else printf("value you were looking for : %d\n", got_val);
	  close(comm_socket_fd);
	  goto starting_point;	
        }
      }
    }
    else if(FD_ISSET(udp_master_sock_fd, &readfds)){
      while((sent_recv_bytes = recvfrom(udp_master_sock_fd, buffer,sizeof(buffer),0, (struct sockaddr*)&incoming_addr, &addr_len)) == -1);
      // PUT or GET ?
      // unpack the message 
      uint8_t msg_id = unpacku8(buffer);
      uint16_t key = unpacku16(buffer+1);
      uint16_t ori_port = unpacku16(buffer+5);
      uint32_t ori_ip_addr = unpacku32(buffer+7);

      // if hash value is same, the data will be consumed here
      if(hash(key) == hash(atoi(argv[2]+3)/2)){
	tcp_data_addr.sin_port = ori_port;
	tcp_data_addr.sin_family = AF_INET;
	tcp_data_addr.sin_addr.s_addr = ori_ip_addr;
//	printf("\nport : %d\n ip_addr : %d\n", htons(ori_port), ori_ip_addr);
        if((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
  	  fprintf(stderr, "error:%d setting socket \n", errno);
  	  exit(1);
 	};	
	if(connect(sockfd, (struct sockaddr*)&tcp_data_addr, addr_len) == -1) {
          fprintf(stderr, "error making tcp connection to original node %d ",ori_port);
	  exit(_ef);
	};
	
	// separate : get or put
	if(msg_id == 1){
	  //make FETCH_VALUE message
	  msg_id = 3;
          uint8_t* ptr = buffer;
	  ptr = pack8(buffer, msg_id);
	  ptr = pack16(ptr, key);

	  if((sent_recv_bytes = sendto(sockfd, buffer, ptr-buffer, 0, (struct sockaddr*)&tcp_data_addr, addr_len)) == -1) {
	    fprintf(stderr, "FETCH_VALUE to node %d failed\n", ori_port);
	    exit(_ef);
	  }
	  if((sent_recv_bytes = recvfrom(sockfd, buffer, ptr + 2 - buffer, 0, (struct sockaddr*)&tcp_data_addr, &addr_len)) == -1){
	    fprintf(stderr, "receiving value from %s to original node %d failed\n", argv[2], ori_port);
  	    exit(_ef);	    
	  };
	  //unpack value and save it
	  //printf("unpacked buffer : %d, value : %d\n", unpacku8(buffer), unpacku16(buffer+3));
	  if(sent_recv_bytes && (unpacku8(buffer) == 4)) {
	    uint16_t val = unpacku16(buffer+3);
	    kv_t kv = { key, val };
	    insert(kv);
	    buffer[0] = 1;
	    if((sent_recv_bytes = sendto(sockfd, buffer, 1, 0, (struct sockaddr*)&tcp_data_addr, addr_len)) == -1) {
	      fprintf(stderr, "sending conf message failed\b");
	      exit(_ef);
	    } 
	    // server should close connection if value affirmed
	  }	  
	} else if(msg_id == 2){
	    uint16_t val = search_val(key);
	    msg_id = 5;
	    uint8_t* ptr = buffer;
	    ptr = pack8(buffer, msg_id);
	    ptr = pack16(ptr, key);
	    ptr = pack16(ptr, val);
	    if((sent_recv_bytes = sendto(sockfd, buffer, ptr-buffer ,0, (struct sockaddr*)&tcp_data_addr, addr_len)) == -1){
	      fprintf(stderr, "Sending value to from %s to node %d failed\n",argv[2], ori_port);
	      exit(_ef);
	    } 
	}
      } else {
	//if hash value is different
	//send to next node
	  if((sent_recv_bytes = sendto(udp_master_sock_fd, buffer, sent_recv_bytes, 0, (struct sockaddr*)&next_node_addr, addr_len)) == -1){
            fprintf(stderr, "udp forwarding failed \n");
            exit(_ef);
        };
      }
    }
    else if(FD_ISSET(0, &readfds)){
      fflush(stdin);
         
      //scanf("%[^\n]s", get_input);
      fgets(get_input, sizeof(get_input), stdin);
      parse(get_input, token);
      // argv[2][3] => ex) 4 of port no 2004 
      // if same , data will be consumed here
      int key = atoi(token[1]);
      int value;   
    
      //consume data in local db if the hash value of the key user has put is same with hash value of this node's udp port  
      if(hash(key) == hash(atoi(argv[2]+3)/2)){
        //save (key, value) in local db double hashing
	if(strncmp(token[0],"PUT", strlen("PUT")) == 0){
	  value = atoi(token[2]);
	  kv_t to_save = {key, value};
	  insert(to_save);
	  printf("\n successfully saved the value\n");
	}
	else if(strncmp(token[0], "GET",strlen("GET")) == 0) {
	  printf("\nvalue of %d : %d\n", key, search_val(atoi(token[1])));
	}
	goto starting_point;
      } else {
        //not here, so should send this key, this ip_addr, this tcp_port to next node : need ip_addr, tcp_port of original one who got this data for future data transmission
	
	//make a packet      
	temp_value = atoi(token[2]);
   	int msg_type;
	if(strncmp(token[0],"PUT", strlen("PUT")) == 0) msg_type = 1; // PUT_FORWARD
	else if(strncmp(token[0], "GET",strlen("GET")) == 0) msg_type = 2; // GET_FORWARD
	uint16_t key = atoi(token[1]);
	//printf("\n fir port: %d\n", atoi(argv[4]));
	msg_t msg_forward = {msg_type, key, 0, htons(atoi(argv[4])), inet_addr(argv[1])};
	//printf("htons: %d, ip : %d\n",msg_forward.origin_port, msg_forward.origin_ip_addr);
	// encode, serialize
	uint8_t* ptr = buffer; 
	ptr = pack8(buffer, msg_forward.msg_id);
	ptr = pack16(ptr, msg_forward.key);
	ptr = pack16(ptr, msg_forward.value);
	ptr = pack16(ptr, msg_forward.origin_port);
        ptr = pack32(ptr, msg_forward.origin_ip_addr);
        
	//create udp socket
  	if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
    	  fprintf(stderr, "UDP socket creation failed\n");
    	  exit(_ef);
  	}	
	if((sent_recv_bytes = sendto(sockfd, buffer, ptr - buffer, 0, (struct sockaddr*)&next_node_addr, sizeof(struct sockaddr))) == -1){
	  fprintf(stderr, "udp forwarding b failed \n");
          exit(_ef);    
	};   

      }
    }
  }

  exit(EXIT_SUCCESS);
};
