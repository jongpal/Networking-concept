#pragma once

#include <iostream>
#include <vector>
#include "node.h"

class Heap_funcs{
public:
  void upheap(Node el, std::vector <Node> &h2);
  void downheap(int top, std::vector <Node> &heap);
  Node extract_min(std::vector <Node> &heap);
  void build_min_heap(std::vector<Node> from, std::vector <Node> &h);
};