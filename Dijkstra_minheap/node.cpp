#include "node.h"

int Node:: get_val() {
    return val;
}

int Node:: get_weight() {
    return weight;
}

// void Node:: set_oif(std::string oif){
//   this->oif = oif;
// }

std::string Node:: get_oif() {
  return oif;
}
Node& Node::operator=(const Node& c){
  val = c.val;
  weight = c.weight;
  oif = c.oif;
  return *this;
}

bool Node::operator==(Node &cmp){
  if(cmp.get_val() == val && cmp.get_weight() == weight) return 1;
  else return 0;
}
