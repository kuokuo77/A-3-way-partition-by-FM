// // cell.h

#ifndef CELL_H
#define CELL_H

#include <vector>
#include <string>
#include "node.h"
#include "net.h"

using namespace std;

class Net;

/*
The data type of the element in cell array

+---------------------------------------+
|CELL ARRAY| cid <-> net_0 <-> net_1 ...|
+---------------------------------------+
                       |
                       v
+-----------------------------------------+
|NET ARRAY | nid <-> cell_0 <-> cell_1 ...|
+-----------------------------------------+
                       |
                       v
       +-----------+
       |Bucket List|  Cell_0  
       +-----------+

+------+
| CELL |
+------+
| id   | cell number in integer
| name | cell number in string. eg. "c100"
| node | point to the cell in the bucket list which is a node class
| nets | vector <Net><Net>.....<Net> stored the Net class represented the net connected to this cell
+------+

*/
class Net_info {
public:
    Net_info(int id, Net *net_ptr) : id_(id), net_ptr_(net_ptr) {}
    ~Net_info() {}


    int id_;
    Net *net_ptr_;
};

class Cell {
public:
    // Constructor and Destructor
    Cell(int id, string name) : id_(id), name_(name), fs_(0), te_(0), num_pins_(0), isLocked_(false) {}
    ~Cell() {}

    int getId() const { return id_; }
    string getName() const { return name_; }
    const vector<Net_info*>& getNets() const { return nets_; }
    void addNet(int id, Net *net) {nets_.push_back(new Net_info(id, net)); num_pins_++; }
    void movePartition(int p) { partition_ = p; }
    int getPartition() const { return partition_; }
    void incrementFS() { fs_++; }
    void decrementFS() { fs_--; }
    void incrementTE() { te_++; }
    void decrementTE() { te_--; }
    void resetGain() { fs_ = 0; te_ = 0; }
    void showGain() const { cout << "fs: " << fs_ << " te: " << te_ << " gain: " << fs_ - te_;}
    void setInitGain() { gain_ = fs_ - te_; }
    int getGain() const { return gain_; }
    void incrementGain() { gain_++; }
    void decrementGain() { gain_--; }
    int getPins() const { return num_pins_; }
    void setNode(Node *node) { node_ = node; }
    Node* getNode() const { return node_; }
    void Lock() { isLocked_ = true; }
    void Unlock() { isLocked_ = false; }
    bool isLocked() const { return isLocked_; }

private:
    int const id_;
    string const name_;
    int partition_;
    vector<Net_info*> nets_;
    Node *node_;
    int fs_;
    int te_;
    int gain_;
    int num_pins_;
    bool isLocked_;
};

inline void Net::intraUpdateCut() {
    int inG1 = 0, inG2 = 0, inG3 = 0;
    for (auto cell_info : cells_) {
        if (cell_info->cell_ptr_->getPartition()==1) inG1=1;
        else if (cell_info->cell_ptr_->getPartition()==2) inG2=1;
        else if (cell_info->cell_ptr_->getPartition()==3) inG3=1;
    }
    // cout << "net " << getId() << " inG1: " << inG1 << " inG2: " << inG2 << " inG3: " << inG3 << endl;
    is_cut_ =  (inG1+inG2+inG3>1);

}

#endif
