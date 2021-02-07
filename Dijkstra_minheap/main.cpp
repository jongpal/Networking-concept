#include <iostream>
#include <vector>
#include <fstream>
#include "heap.h"
#include "node.h"

std::ifstream fin("graph.txt");
std::vector<Node> *head;
int *cost;
int *next_hop; //next hop info
std::vector<int> check; //to check if visited
std::vector<int> parent;
Heap_funcs func;

int name2int(char c)
{
    return c - 'A';
}
char int2name(int i)
{
    return i + 'A';
}

void adjGraph(int* V, int* E){
  char vertex[3];
  int w;
  fin >> *V >> *E;
  head = new std::vector<Node>[*V];
  cost = new int[*V];
  next_hop = new int[*V];

  //init : set check to 0
  for(int i = 0 ; i < *V; i++) {
    check.push_back(0);
  }

  for (int i = 0; i < *E; i++) {
    fin >> vertex >> w;
    int from = name2int(vertex[0]);
    int to = name2int(vertex[1]);
    Node node(to, w);
    Node node_mirror(from, w);

    head[from].push_back(node);
    head[to].push_back(node_mirror);
  }
}

void printGraph(int V){
  for (int i = 0; i < V; i++) {
    std::cout << '\n' <<int2name(i) << " --> ";
    std::vector<Node>::iterator it;

    for(it = head[i].begin(); it!=head[i].end(); it++) {
      std::cout << int2name((*it).get_val()) << " --> ";
    }
  }
};

//only starting point is cost 0, else : infinite(here : 1000)
void cost_init(int s, int V) {
  for(int i = 0 ; i < V; i++) {
    if(i == s) cost[i] = 0;
    else cost[i] = 1000;
    parent.push_back(-1);
  }
}

//recursive
void dijkstra(std::vector<Node> &heap, int s, int V){
  std:: vector<Node>:: iterator it;
  check[s] = 1;
  for(it = head[s].begin(); it != head[s].end(); it++){
    if(check[(*it).get_val()]) continue;
    //if it is yet to visited
    int d = cost[s] + (*it).get_weight();
    if( d < cost[(*it).get_val()]) {
        cost[(*it).get_val()] = d;
        Node update((*it).get_val(), d);
        func.upheap(update, heap);
        parent[(*it).get_val()] = s;
    } 
  }
  //check heap is empty (size 1) 
  if(heap.size() <= 1) return;
  Node min = func.extract_min(heap);
  dijkstra(heap, min.get_val(), V);
}

void set_nexthop(int s, int V){
  int j;
  for(int i = 0 ; i < V; i++) {
    if((i == s) || (parent[i] == s)) {
      next_hop[i] = i;
      continue;
    }
    j = i;
    while(parent[j] != s) {
      j = parent[j];
    }
    next_hop[i] = j;
  }
}

int main() {
  int V,E;
  std::vector <Node> heap;
  //heap init (dummy heap[0])
  Node empty(30,10000);
  heap.insert(heap.begin(), empty);

  if(!fin){
    std::cout << "Error : file open \n" << std::endl;
    exit(1);
  }

  adjGraph(&V, &E);
  printGraph(V);
  cost_init(0, V);
  dijkstra(heap, 0, V);
  set_nexthop(0, V);

  for(int i = 0 ; i < V; i++){
    std::cout <<"\n if you want to go : " << int2name(i) <<", next hop : "<< int2name(next_hop[i]) << ", cost : " << cost[i] << '\n';
  }
   

  delete [] head;
  delete [] cost;
  delete [] next_hop;
}