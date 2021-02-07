#include "heap.h"

void Heap_funcs::upheap(Node el, std::vector <Node> &h2) {
    h2.push_back(el);
    if(h2.size() > 2) {
        int ind = h2.size()-1;
        while((ind/2 >=1) && (h2[ind/2].get_weight() > h2[ind].get_weight())){
            Node temp = h2[ind];
            h2[ind] = h2[ind/2];
            h2[ind/2] = temp;
            ind = ind / 2;
        }
    }
}

void Heap_funcs::downheap(int top, std::vector <Node> &heap) {
  int ind = top;
  while((ind <= (heap.size()-1)/2) && ((heap[ind].get_weight() > (heap[2*ind].get_weight())) || (heap[ind].get_weight() > heap[2*ind+1].get_weight()))) {
    Node smaller = heap[2*ind].get_weight() <= heap[2*ind+1].get_weight() ? heap[2*ind] : heap[2*ind+1];
    if(heap[ind].get_weight() >= smaller.get_weight()) {
      if(smaller == heap[2*ind]) {
        Node temp = heap[ind];
        heap[ind] = heap[2*ind];
        heap[2*ind] = temp;
        ind *=2;
      } else {
        Node temp = heap[ind];
        heap[ind] = heap[2*ind+1];
        heap[2*ind+1] = temp;
        ind = 2*ind +1;
      }
    }
  }
}

Node Heap_funcs:: extract_min(std::vector <Node> &heap) {
    Node min = heap[1];
    heap[1] = heap[heap.size()-1];
    heap.pop_back();
    downheap(1, heap);
    return min;
}

void Heap_funcs:: build_min_heap(std::vector<Node> from, std::vector <Node> &h) {
    for(int i = 0; i < from.size(); i++) {
    upheap(from[i], h);
  }
}