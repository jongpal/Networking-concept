
#include "hash_db.h"

int h1(int k) {
  return k % TABLE_SIZE;
}
int h2(int k) {
  return 1 + ( k % (TABLE_SIZE - 1));
}
int preHash (int i, int k) {
  return (h1(k)+ i*h2(k)) % TABLE_SIZE;
}
bool isFull() {
  return curr_size == TABLE_SIZE;
}

int insert(kv_t kv) {
  if(isFull()) {
    fprintf(stderr, "the table is full\n");
    return -1;
  };
  int cur = 0;
  int a = preHash(cur, kv.key);
  if(hash_table[a].key == -1) {
    hash_table[a] = kv;
    printf("insert key :%d, value:%d success", kv.key, kv.value);
  } else {
    while (1) {
      a = preHash(++cur, kv.key);
      if(hash_table[a].key == -1) {
        hash_table[a] = kv;
        printf("insert key :%d, value:%d success\n", kv.key, kv.value);
        break;
      };
    };
  };
  curr_size++;
  //repetition number
  return cur;
} 
int search_val(int key){

  int trial = 0;
  int phash;
  while (1) {
    phash = preHash(trial, key);
    if(hash_table[phash].key == -1){
      printf("no result\n");
      return 0;
    } else if(hash_table[phash].key == key) {
      int value = hash_table[phash].value;
      printf("for key %d found value : %d\n", key, hash_table[phash].value);
      return value;
    };
    trial ++;
  };
};

int deletion (int key) {
  int trial = 0;
  int phash;
  while(1) {
    phash = preHash(trial, key);
    if(hash_table[phash].key == key){
      int k = hash_table[phash].key;
      //delete flag : -2
      hash_table[phash].key = -2;
      return k;
    } else if(hash_table[phash].key < 0) {
      printf("no item to be deleted\n");
      return -1;
    }
    trial++;
  }
}

void displayTable() 
{ 
    printf("\n");
    for (int i = 0; i < TABLE_SIZE; i++) { 
        if (hash_table[i].key != -1) 
            printf("%d --> %d\n", i, hash_table[i].key); 
        else
            printf("%d --> \n", i);
    } 
};
