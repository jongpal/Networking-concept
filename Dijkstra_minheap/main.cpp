#include <iostream>
#include <vector>
#include <fstream>
#include "heap.h"
#include "node.h"

std::ifstream fin("graph.txt");
std::vector<Node> *head;
int *cost;
std::vector<std::unordered_map<int,std::string>> *nexthop_info; // next hop info
std::vector<int> check; //to check if visited
std::vector<int> *parents;

Heap_funcs func;
// source node(starting point 0 -> A, 1 -> B ...)
int src_node = 1;

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
  std::string oif; //outgoing interface
  std::string iif; //interface
  fin >> *V >> *E;
  head = new std::vector<Node>[*V];
  cost = new int[*V];
  nexthop_info = new std::vector<std::unordered_map<int,std::string>>[*V];
  parents = new std::vector<int>[*V];

  //init : set check to 0
  for(int i = 0 ; i < *V; i++) {
    check.push_back(0);
  }

  for (int i = 0; i < *E; i++) {
    fin >> vertex >> w >> oif >> iif;
    int from = name2int(vertex[0]);
    int to = name2int(vertex[1]);
    Node node(to, w, oif);
    Node node_mirror(from, w, iif);

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
      // parent[i].push_back(-1);
  }
}

  //recursive
void dijkstra(std::vector<Node> &heap, int s, int V){
  std:: vector<Node>:: iterator it;
  check[s] = 1; //check visited node
  for(it = head[s].begin(); it != head[s].end(); it++){
  //if it is yet to visited
    if(check[(*it).get_val()]) continue;
    
    int d = cost[s] + (*it).get_weight();
    if( d < cost[(*it).get_val()]) {
        cost[(*it).get_val()] = d;
        Node update((*it).get_val(), d, (*it).get_oif());
  //inherit the next_hop from parent
        nexthop_info[(*it).get_val()] = nexthop_info[s];

        func.upheap(update, heap);
        if(parents[(*it).get_val()].size() >=1) parents[(*it).get_val()].back() = s;
        else parents[(*it).get_val()].push_back(s);
    }
  //ECMP : if d is same , then append 
    else if(d == cost[(*it).get_val()]) {
        cost[(*it).get_val()] = d;
        Node update((*it).get_val(), d, (*it).get_oif());
  //push_back because it is ECMP case
        nexthop_info[(*it).get_val()].push_back((nexthop_info[s][0]));

        func.upheap(update, heap);
        parents[(*it).get_val()].push_back(s);
    }
  }
  //check if heap is empty (size 1) 
  if(heap.size() <= 1) return;
  Node min = func.extract_min(heap);
  
  //if parent of min is src_node : this min would be one of next hops
  if(parents[min.get_val()].back() == src_node) {
    std::unordered_map<int, std::string> x;
    x[min.get_val()] = min.get_oif();
    nexthop_info[min.get_val()].push_back(x);
  } 
  //recursive
  dijkstra(heap, min.get_val(), V);
}

// 2) derive next hop from parent info
// void set_nexthop(int s, int V){
//   int j;
//   for(int i = 0 ; i < V; i++) {
//     if(parents[i].size() == 1){
//       if((i == s) || (parents[i][0] == s)) {
//         next_hop[i].push_back(i);
//         continue;
//       }
//       j = i;
//       while(parents[j].back() != s) {
//         j = parents[j].back();
//       }
//       next_hop[i].push_back(j);
//     } 
//     //for ECMP
//      else if(parents[i].size() >= 2) {
//       std::vector <int>:: iterator it;
//       for(it = parents[i].begin(); it != parents[i].end(); it++){
//         j = *it;
//         while(parents[j].back() != s) {
//           j = parents[j].back();
//         }
//         next_hop[i].push_back(j);
//       }
//     } 
//     //source node
//     else {
//       next_hop[i].push_back(i);
//     }
//   }
// }

int main() {
  int V,E;
  
  std::vector <Node> heap;
  std::vector <std::unordered_map<int, std::string>>:: iterator it;
  //heap init (dummy heap[0])
  Node empty(30,10000, "empty");
  heap.insert(heap.begin(), empty);

  if(!fin){
    std::cout << "Error : file open \n" << std::endl;
    exit(1);
  }

  adjGraph(&V, &E);
  printGraph(V);
  cost_init(src_node, V);
  dijkstra(heap, src_node, V);
  // set_nexthop(src_node, V);

  std::cout << "\n from Source node " <<  int2name(src_node) << '\n';

  for(int i = 0 ; i < V; i++){
    std::cout <<"\n if you want to go : " << int2name(i) <<", next hop : ";
    for(it = nexthop_info[i].begin(); it != nexthop_info[i].end(); it++){
       for(auto& i : *it)
          std::cout << int2name(i.first) << ", " << i.second << "   ";
    }
    std::cout << " cost :" << cost[i] <<'\n';
  }
   
  delete [] parents;
  delete [] head;
  delete [] cost;
  delete [] nexthop_info;
}
