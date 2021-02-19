#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "manage_fd.h"
#define MAX_CLIENT_SUPPORTED 32
#define SERVER_PORT 2000
#define BUF_SIZE 1024
#define NUM_POSSIBLE_QUERY 4
#define STUDENT_NUM 10

   /********************************************************************/
   /*Simple multiplexed tcp Web Server with simple get and post request*/
   /********************************************************************/


char data_buffer[BUF_SIZE];
const char *pri_search_key = "rollno";
int num_cur_student = 5;

// extract empty space at the start/end
void string_space_trim(char *string){
 
  if(!string) return;

  char *ptr = string;
  int len = strlen(ptr);
  
  if(!len) return;
  if(!isspace(ptr[0]) && !isspace(ptr[len-1])) return;
  
  while(len-1 > 0 && isspace(ptr[len-1])) {
    //ptr[--len];
    --len;
  }

  while(*ptr && isspace(*ptr)){
    ++ptr, --len;
  }

  memmove(string, ptr, len + 1); //+1 for NULL character 
}

//simple local student database
typedef struct student_{
  char name[32];
  unsigned int roll_no;
  char hobby[32];
  char dept[32];
} student_t; // sum of 100 bytes

student_t student[STUDENT_NUM] = {
    {"Abhishek", 10305042, "Programming", "CSE"},
    {"Nitin", 10305048, "Programming", "CSE"},
    {"Avinash", 10305041, "Cricket", "ECE"},
    {"Jack", 10305032, "Udemy Teaching", "Mechanical"},
    {"Cris", 10305030, "Programming", "Electrical"}
};

// get request : usage : ?<one of student_t field>=<value>
// find desired tuple using roll_no as primary key
static char *process_GET_request(char *URL, unsigned int *response_len){
  int key_loc = -1;
  char delimiter[2] = {'?', '\0'};
  char *token[NUM_POSSIBLE_QUERY+1]; // every 2(key, value) per query, + 1 for file location
  token[0] = strtok(URL, delimiter);
  delimiter[0] = '&';
  int i = 0;
  while(token[i] != NULL){
    token[++i] = strtok(NULL, delimiter);
    if(token[i] == NULL) break;
    if(strncmp(token[i], pri_search_key , strlen(pri_search_key)) == 0){
      key_loc = i;  	    
    }; 
  }
  if(key_loc == -1) {
    fprintf(stderr, "no given key to find proper data\n");
    exit(EXIT_FAILURE);
  }
  delimiter[0] = '=';
  char *rollno_key = strtok(token[key_loc],delimiter);
  char *rollno_value = strtok(NULL, delimiter);
  printf("roll_no_value = %s\n", rollno_value);
  unsigned int roll_no = atoi(rollno_value);
  int j = 0;
  for(; j < STUDENT_NUM; j++) {
    if(student[j].roll_no != roll_no) continue;
    else if(student[j].roll_no == 0) return NULL;
    break;
  } 
//  if(j == 5)
//    return NULL;

  //forming response body : simple HTML
  char *response = calloc(sizeof(char), 1024);

  strcpy(response, "<html><head>""<title>HTML Response</title>"
		  "<style>table, th, td { border: 1px solid black; }</style>"
		  "</head>""<body><table><tr><td>");
  strcat(response, student[j].name);
  strcat(response, "</td></tr>");
  strcat(response , "</table></body></html>");

  unsigned int content_len_str = strlen(response);
  char len_to_str[20];
  sprintf(len_to_str,"%d",content_len_str);
  
  //forming GET header
  char *header = calloc(1, 248 + content_len_str);
  strcpy(header, "HTTP/1.1 200 OK\n");
  strcat(header, "Server: My HTTP Server\n");
  strcat(header, "Content-Length: ");
  strcat(header, len_to_str);
  strcat(header, "\nConnection: close\n");
  strcat(header, "Content-Type: text/html; charset=UTF-8\n");
  strcat(header, "\n");
  strcat(header, response);
  content_len_str = strlen(header);
  *response_len = content_len_str;
  free(response);
  return header;
}

// post request : inserting new student url : /College/IIT/student/enroll
// provide body field with specified student_t field
static char *process_POST_request(char *URL, unsigned int *response_len,char *body){

  char *token[4];
  if((strncmp(URL, "/College/IIT/student/enroll", strlen("/College/IIT/student/enroll"))) == 0) {
    char *delimiter = "\n";
    token[0] = strtok(body, delimiter);
    token[1] = strtok(NULL, delimiter);
    token[2] = strtok(NULL, delimiter);
    token[3] = strtok(NULL, delimiter);
    char *del = "=";    
    strtok(token[0], del);
    strcpy(student[num_cur_student].name, strtok(NULL, del));
    strtok(token[1], del);
    student[num_cur_student].roll_no=atoi(strtok(NULL, del));
    strtok(token[2], del);
    strcpy(student[num_cur_student].hobby, strtok(NULL, del));
    strtok(token[3], del);
    strcpy(student[num_cur_student].dept,strtok(NULL, del));
    num_cur_student ++;
    
    // response : JSON
    char *response = (char *)calloc(1, 1024);
    char rollno_string[20];
    sprintf(rollno_string,"%d",student[num_cur_student-1].roll_no);
    strcpy(response, "{\n\"status\" : \"success\"\n\"rollno\" : ");
    strcat(response, rollno_string);
    strcat(response, "\n}");
   
    unsigned int content_len_str = strlen(response);
 
    char len_to_str[20];
    sprintf(len_to_str,"%d",content_len_str);
    char *header  = calloc(1, 248 + content_len_str);
    strcpy(header, "HTTP/1.1 201 CREATED\n");
    strcat(header, "Server: My HTTP Server\n");
    strcat(header, "Content-Length: ");
    strcat(header, len_to_str);
    strcat(header, "\nConnection: keep-alive\n");
    strcat(header, "Content-Type: application/json; charset=UTF-8\n");
    strcat(header, "\n");
    strcat(header, response);
    content_len_str = strlen(header);
    *response_len = content_len_str;
    free(response);
    return header;
  }			   
  return NULL;
}


void setup_tcp_multi_webserver() {
  int master_sock_tcp_fd = 0, sent_recv_bytes = 0, addr_len = 0;
  int comm_socket_fd = 0;
  int opt = 1;
  fd_set readfds;
  struct sockaddr_in server_addr, client_addr;

  init_monitored_fd_set();
  if((master_sock_tcp_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
    fprintf(stderr, "master socket creation failed\n");
    exit(1);
  }
  if((setsockopt(master_sock_tcp_fd, SOL_SOCKET, SO_REUSEADDR, (char *)& opt, sizeof(opt))) < 0){
    fprintf(stderr, "TCP socket creation failed for faster reusable purposes\n");
    exit(EXIT_FAILURE);  
  };
  
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(SERVER_PORT);
  server_addr.sin_addr.s_addr = INADDR_ANY;
  addr_len = sizeof(struct sockaddr);
  if(bind(master_sock_tcp_fd, (struct sockaddr *)&server_addr, addr_len) == -1){
    fprintf(stderr, "binding master fd and server failed\n");
    exit(EXIT_FAILURE);
  }
  if(listen(master_sock_tcp_fd, 5) == -1){
    fprintf(stderr, "master socket listen failed\n");
    exit(EXIT_FAILURE);
  }
  add_to_monitored_fd_set(master_sock_tcp_fd);

  while(1) {
    re_init_readfds(&readfds);
    printf("blocked on select system call...\n");
    if(select(get_max_fd()+1, &readfds, NULL, NULL, NULL) == -1){
      fprintf(stderr, "polling failed \n");
      exit(EXIT_FAILURE);
    };

    if(FD_ISSET(master_sock_tcp_fd, &readfds)){
      fprintf(stderr,"New Connection request, accept the connection !\n");

      if((comm_socket_fd = accept(master_sock_tcp_fd, (struct sockaddr*)&client_addr, &addr_len)) == -1){
        fprintf(stderr, "creating new communication file descriptor failed \n");
        exit(1);
      };
      add_to_monitored_fd_set(comm_socket_fd);
      printf("Connection accepted from client : %s: %u\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    }
    else {
      memset(data_buffer, 0, sizeof(data_buffer));
      for(int i = 0; i < MAX_CLIENT_SUPPORTED; i++) {
        if(!FD_ISSET(monitored_fd[i], &readfds)) continue;

	comm_socket_fd = monitored_fd[i];
        if((sent_recv_bytes = recvfrom(comm_socket_fd, data_buffer, sizeof(data_buffer), 0, (struct sockaddr *) &client_addr, &addr_len)) == -1){
	  fprintf(stderr, "error receiving bytes from client, try again\n");
          break;
	}
        printf("Server recvd %d bytes from client %s:%u\n", sent_recv_bytes, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        if(sent_recv_bytes == 0) {
          close(comm_socket_fd);
	  break;
	}

	printf("Message received : \n%s\n", data_buffer);
	char *request_line = NULL;
	char delimiter[2] = "\n";
        char *method = NULL, *URL = NULL;
	request_line = strtok(data_buffer, delimiter);
        // parse request body
	char *body = strtok(NULL, delimiter);
       
        while((body[0] != 13 || body[0] != 10)&&(body[1] != 0)){ 
          body = strtok(NULL, delimiter);
        }
        body = strtok(NULL, "\0");
	delimiter[0] = ' ';
	method = strtok(request_line, delimiter);
	URL = strtok(NULL, delimiter);

	printf("Body = %s\n", body);
//	printf("%d\n", data_buffer[286]);//data_buffer[286] 부터
       	
	printf("Method = %s\n", method);
	printf("URL = %s\n", URL);
	char *response = NULL;
	unsigned int response_length = 0;
	
	if(strncmp(method, "GET", strlen("GET")) == 0) {
	  response = process_GET_request(URL, &response_length);
	} else if(strncmp(method, "POST", strlen("POST")) == 0) {
	  //  /student/enroll route -> to enlist new student
	  response = process_POST_request(URL, &response_length, body);
	} else {
	  printf("Unsupported URL method request\n");
	  close(comm_socket_fd);
	  break;
	}

	if(response) {
	  printf("\nresponse to be sent to client = \n %s",response);
	  sent_recv_bytes = sendto(comm_socket_fd, response, response_length, 0, (struct sockaddr *)&client_addr, sizeof(struct sockaddr));
	  free(response);
	  printf("\nServer sent %d bytes in reply to client\n", sent_recv_bytes);
	}
      }
    }
  }
}

int main(int argc, char **argv) {
  setup_tcp_multi_webserver();
  return 0;
}
