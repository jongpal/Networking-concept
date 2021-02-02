/*
  basic network subnetting implementation
  total 6 functionalities :

  1. getting broadcast value
  2. getting integral equivalent of ip address
  3. getting ip address out of integral equivalent
  4. getting network(subnet) id
  5. check the cardinality of that exact mask
  6. check the membership of check_ip 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PREFIX_LEN 16

void parse(char *s2, char *b_ip_add_[]){
  char *ret_ptr;
  char *next_ptr;
  char output_addr[PREFIX_LEN];

  ret_ptr = strtok_r(s2, ".", &next_ptr);
  int cur = 0;

  while(ret_ptr) {
    b_ip_add_[cur] = ret_ptr;
    ret_ptr = strtok_r(NULL, ".", &next_ptr);
    cur++;
  }
}

//broadcast mask value
void get_broadcast_address(char *ip_addr, char mask, char *output_buffer){

  int broad_mask = 0;
  int masks[4] = {0,};
  int count = 0;
  int k = 0;
  for(int i = 0 ; i < 32 - mask; i++) {
    broad_mask += 1 << (i - k);
    // for each 8 bits of total 4
    // ex) mask = 20 -> 32 - 20 = 12 for broadcasting
    // calculate every 8 bits
    if((i % 8) == 7) {
      masks[3 - count] = broad_mask;
      broad_mask = 0;
      count++;
      k = 8*count;
    }
  }
  //for the leftover 4 (if mask = 20 -> 12-8 = 4 : should be calculated as well)
  masks [3 - count] = broad_mask;

  // parse ip_addr 
  char *s1 = malloc(sizeof(char)*PREFIX_LEN); // make it to data segments 
  strcpy(s1, ip_addr);
  char *b_ip_add[4];
  parse(s1, b_ip_add);
  
  //OR operation for broadcast address
  for(int i = 0; i < 4; i++) {
    masks[i] = masks[i] | atoi(b_ip_add[i]);
  }
  //concat each elements of masks into one big string and set it to output_buffer
  sprintf(output_buffer,"%d.%d.%d.%d", masks[0],masks[1],masks[2],masks[3]);

  free(s1);
}

// ip format => integer value
unsigned int get_ip_integral_equivalent(char *ip_address){
  char *s1 = malloc(sizeof(char)*PREFIX_LEN);
  strcpy(s1, ip_address);
  char *b_ip_add[4];
  parse(s1, b_ip_add);

  int sum = 0;
  for(int i = 0; i < 4; i++) {
    sum += atoi(b_ip_add[i]) << (32 - 8*(i+1));
  }
  free(s1);
  return sum;
}

// integer value => A.B.C.D(ip format)
void get_abcd_ip_format(unsigned int integer_ip, char *output_buffer){
  int sum[4]= {0,};
  for(int i = 31; i >= 0; i--) {
    int where;
    if(i <= 31 && i >=24) where = 0;
    else if( i <=23 && i >= 16) where = 1;
    else if(i <= 15 && i >= 8) where = 2;
    else where = 3;
    unsigned int x = 1 << i;

    if(i == 31 && integer_ip >= x) {
      sum[0] += x >> (24 - where*8);
      integer_ip -= x;
    }
    else if(integer_ip >= x && integer_ip < (x << 1)){
      sum[where] += x >> (24 - where*8);
      integer_ip -= x;
    }
  }

  sprintf(output_buffer,"%d.%d.%d.%d", sum[0],sum[1],sum[2],sum[3]);
}

//network(subnet) id
void get_network_id(char *ip_addr, char mask, char *output_buffer){
  char *s1 = malloc(sizeof(char)*PREFIX_LEN);
  strcpy(s1, ip_addr);
  char *b_ip_add[4];
  parse(s1, b_ip_add);

  int mask_sum[4] = {0,};
  int cur = 0;
  int from = 7;

  for(int i = 32; i > 0; i--) { 
    if(i + mask - 1 <= 32) {
      from--;
      if(from < 0) {
        from = 7;
        cur += 1;
      }
    }
    else {
      mask_sum[cur] += 1 << from--;
      if(from < 0) {
      from = 7;
      cur += 1;
      }
    } 
  }

  for(int i = 0; i < 4; i++) {
    mask_sum[i] = mask_sum[i] & atoi(b_ip_add[i]);
  }

  sprintf(output_buffer,"%d.%d.%d.%d", mask_sum[0],mask_sum[1],mask_sum[2],mask_sum[3]);
  
  free(s1);
}

//max # of assignable ip addresses
unsigned int get_subnet_cardinality(char mask){
  int sum = 1;
  for(int i = 0 ; i < 32 - mask; i++) {
    sum *= 2;
  }
  
  return sum - 2; // - Network& Broadcast ids
};


int check_ip_subnet_membership(char *network_id, char mask, char *check_ip){
  char output_buf[PREFIX_LEN];
  memset(output_buf, 0, PREFIX_LEN);
  get_network_id(check_ip, mask, output_buf);

  if(strcmp(network_id, output_buf) == 0) return 0; // positive
  else return -1; // negative
};

int main(void) {
  char ipadd_buffer[PREFIX_LEN];
  memset(ipadd_buffer, 0, PREFIX_LEN);
  char *ip_add = "192.168.2.10";
  char mask = 24;

  //1
  get_broadcast_address(ip_add, mask, ipadd_buffer);
  printf("Broadcast address = %s\n", ipadd_buffer);

  //2
  unsigned int int_ip = get_ip_integral_equivalent(ip_add);
  printf("Integer equivalent for %s is %u \n", ip_add, int_ip);

  //3
  unsigned int integer_ip = 3232236042;
  memset(ipadd_buffer, 0, PREFIX_LEN);
  get_abcd_ip_format(integer_ip, ipadd_buffer);
  printf("Ip in A.B.C.D format is = %s\n", ipadd_buffer);

  //4
  memset(ipadd_buffer, 0, PREFIX_LEN);
  get_network_id(ip_add, mask, ipadd_buffer);
  printf("Network Id = %s\n", ipadd_buffer);

  //5
  printf("Subnet cardinality for Mask = %u is %u\n", mask, get_subnet_cardinality(mask));

  //6
  char *network_id = "192.168.0.0";
  char *check_ip = "192.168.0.13";
  int result = check_ip_subnet_membership(network_id, mask, check_ip);
  if(result == 0)
    printf("Ip address = %s is a member of subnet %s/%u\n", check_ip, network_id, mask);
  else
    printf("Ip address = %s is a not a member of subnet %s/%u\n", check_ip, network_id, mask);

  return 0;
}