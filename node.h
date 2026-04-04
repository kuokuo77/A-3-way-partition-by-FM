// node.h
#ifndef NODE_H
#define NODE_H
/*
the node structure is used for bucket list.
a pointer from each element in cell array will point to it.
*/

#include <list>

using namespace std;

class Cell;

class Node {
    // Constructor and Destructor
public:
    Node(int id, Cell *cell_ptr) : id_(id), cell_ptr_(cell_ptr) {}
    ~Node() {}
    int getId()   const {return id_;}
    Cell *getCell() const {return cell_ptr_;}

    void setIt(std::list<Node*>::iterator it) { it_ = it; }
    list<Node*>::iterator getIt() { return it_; }
    
private:
    int id_;
    Cell *cell_ptr_;
    list<Node*>::iterator it_;    
};

#endif