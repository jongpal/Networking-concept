/*
serializing TLV structured data
*/
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <stdint.h>
#define SERIALIZED_BUF_SIZE 1024
#define SEND_BUFFER_SIZE 1024

char *buffer;
char ip_address[4];

typedef struct serialized_data{
  char *d;
  int size;
  int next;
}ser_buf_t;

void init_serial_data (ser_buf_t **s) {
  *s = (ser_buf_t*)calloc(1, sizeof(ser_buf_t));
  (*s)-> d = (char *)calloc(1, SERIALIZED_BUF_SIZE);
  (*s)->size = SERIALIZED_BUF_SIZE;
  (*s)->next = 0;
}

void serialize_data(ser_buf_t *buf, char *data, int nbytes){
  
  int available_size = buf->size - buf->next;
  char isResize = 0;
  while(available_size < nbytes) {
    buf->size *= 2; 
    available_size = buf->size - buf->next;
    isResize = 1;
  }
  if(isResize) {
    buf->d = realloc(buf->d, buf->size);
  }
  memcpy(buf->d + buf->next, data, nbytes);
  buf->next += nbytes;
  return;
}


// using char(invariant over different machine)
void rcvTLV(char *tlv_ptr){
  char tlv_type = tlv_ptr[0];
  unsigned int ip;
  // memcpy(&ip, tlv_ptr+2, 4);
  // printf("%d\n", ip);
  tlv_ptr++;
  char data_length = *tlv_ptr++;
  char mask;
  switch(tlv_type){
    case 2: 
      for(int i = 0 ; i < data_length / 5; i++){
        memcpy(ip_address, tlv_ptr, 4);
        tlv_ptr+=4;
        mask = *tlv_ptr;
        tlv_ptr++;
      }
    break;
    default: 
      // if this machine is not configured with this TLV, then escape using LENGTH field
      tlv_ptr += data_length;
      break;
  }
}


int main(void) {
  ser_buf_t *stream;
  init_serial_data(&stream);
  //TLV type 2 : data pattern : 4bytes of ip address, 1byte of mask value 
  char data = 2;
  serialize_data(stream, &data, 1);
  //TLV length : 10 bytes
  data = 10;
  serialize_data(stream, &data, 1);
  //TLV Value
  unsigned int ip = 234947817;
  char copied[4];
  memcpy(copied, &ip, 4);
  serialize_data(stream, copied, 4);
  char mask = 24;
  serialize_data(stream, &mask, 1);
  
  unsigned int ip2 = 218169601;
  memcpy(copied, &ip2, 4);
  serialize_data(stream, copied, 4);
  mask = 24;
  serialize_data(stream, &mask, 1);
  
  // suppose we send this data through socket
  buffer = stream->d; 
  rcvTLV(buffer);
  //last ip_address received
  memcpy(&ip2, ip_address, 4);
  printf("%d\n", ip2);
  free(stream);
  return 0;
}