#pragma once

#include <iostream>

class Node {
  int val; 
  int weight;
  std::string oif;
public:
  Node(int val, int weight, std::string oif) : val(val), weight(weight), oif(oif) {};
  int get_val();
  int get_weight();
  std::string get_oif();
  // void set_oif(std::string oif);
  Node& operator=(const Node& c);
  bool operator==(Node& cmp);
};
