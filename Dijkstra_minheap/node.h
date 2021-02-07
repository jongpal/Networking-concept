#pragma once

#include <iostream>

class Node {
  int val; 
  int weight;
public:
  Node(int val, int weight) : val(val), weight(weight){};
  int get_val();
  int get_weight();
  Node& operator=(const Node& c);
  bool operator==(Node& cmp);
};
