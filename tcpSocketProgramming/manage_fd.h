
#define MAX_CLIENT_SUPPORTED 32

int monitored_fd[MAX_CLIENT_SUPPORTED];
void init_monitored_fd_set();
void add_to_monitored_fd_set(int skt_fd);
void remove_from_monitored_fd_set(int skt_fd);
void re_init_readfds(fd_set *fd_set_ptr);
int get_max_fd();
