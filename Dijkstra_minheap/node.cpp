#include "node.h"

int Node:: get_val() {
    return val;
}

 int Node:: get_weight() {
    return weight;
}

Node& Node::operator=(const Node& c){
  val = c.val;
  weight = c.weight;
  return *this;
}

bool Node::operator==(Node &cmp){
  if(cmp.get_val() == val && cmp.get_weight() == weight) return 1;
  else return 0;
}