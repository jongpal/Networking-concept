
#include <stdlib.h>
#include <sys/time.h>
#define MAX_CLIENT_SUPPORTED 32

int monitored_fd[MAX_CLIENT_SUPPORTED];

void init_monitored_fd_set(){
 for(int i = 0; i < MAX_CLIENT_SUPPORTED; i++){
  monitored_fd[i] = -1;
 }
}

void add_to_monitored_fd_set(int skt_fd) {
 for(int i = 0 ; i < MAX_CLIENT_SUPPORTED; i++){
  if(monitored_fd[i] != -1) continue;
  monitored_fd[i] = skt_fd;
  break;
 }
}

void remove_from_monitored_fd_set(int skt_fd){
 for(int i = 0 ; i < MAX_CLIENT_SUPPORTED; i++){
  if(monitored_fd[i] != skt_fd) continue;
  monitored_fd[i] = -1;
  break;
 }
}

void re_init_readfds(fd_set *fd_set_ptr){
 FD_ZERO(fd_set_ptr);

 for(int i = 0 ; i < MAX_CLIENT_SUPPORTED; i++){
  if(monitored_fd[i] == -1) continue;
  FD_SET(monitored_fd[i], fd_set_ptr);
 }
}

int get_max_fd(){
 int max = monitored_fd[0];
 for(int i = 1 ; i < MAX_CLIENT_SUPPORTED; i++)
  if(monitored_fd[i] > max) max = monitored_fd[i];

 return max;
}
