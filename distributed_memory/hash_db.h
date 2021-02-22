#include <stdio.h>
#include <memory.h>
#include <stdbool.h>
#define TABLE_SIZE 17

int curr_size;

typedef struct k_v {
  int key;
  int value;
}kv_t;

kv_t hash_table[TABLE_SIZE];
int h1(int k);
int h2(int k);
int preHash (int i, int k);
bool isFull();

int insert(kv_t kv); 
int search_val(int key);

int deletion (int key);
void displayTable();
