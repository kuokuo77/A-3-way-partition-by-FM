// blist.h
/*
bucket list
mapping relation:

+Pmax | 0
      |
      |
      |
-Pmax | 2*Pmax+1

*/

#ifndef BLIST_H
#define BLIST_H

#include "node.h"
#include <vector>
#include <list>

using namespace std;


class BList {
public:
    BList(int pmax) : pmax_(pmax), max_gain_(0), num_cells_(0) { bl_.resize(2*pmax+1); }
    ~BList() {}
    
    void addNode(Cell* cell);
    void printList() const ;
    Node* getMaxGainNode() ;
    int getNumCells() const { return num_cells_; };
    vector<list<Node*>>& getBL() { return bl_; }
    void removeNode(Cell* cell);
    void resetBlist() { bl_.clear(); num_cells_=0;}

private:
    int pmax_;
    int max_gain_;
    int num_cells_;
    vector<list<Node*>> bl_;
};

inline Node* BList::getMaxGainNode() {
    // Find the max gain node
    while (max_gain_ + pmax_>= 0 && bl_[max_gain_ + pmax_].empty()) {
        max_gain_--;
    }
    if (max_gain_ < -pmax_) {
        return nullptr;
    }
    Node* target = bl_[max_gain_ + pmax_].front();
    
    return target;
}

inline void BList::printList() const {
    for(int i=bl_.size()-1; i>=0; i--) {
        cout << "|" << i - pmax_ << "|-> ";
        for(auto node : bl_[i]) {
            cout << node->getId() << "->";
        }
        cout << endl;
    }
}

inline void BList::addNode(Cell* cell) {
    int gain = cell->getGain();
    if(gain > max_gain_) max_gain_ = gain;
    Node* newNode = new Node(cell->getId(), cell);
    bl_[gain + pmax_].push_front(newNode);

    newNode->setIt(bl_[gain + pmax_].begin());
    cell->setNode(bl_[gain + pmax_].front());
    num_cells_++;
}

inline void BList::removeNode(Cell* cell) {
    // 1. 取得該 Cell 對應的 Node 指標
    Node* target_node = cell->getNode();
    if(!target_node) return;
    // if (target_node == nullptr) return nullptr;

    // 2. 根據 Cell 目前的 Gain 找到在哪個 list (索引 = gain + pmax)
    int idx = cell->getGain() + pmax_;

    // 3. 透過存好的 iterator 從 list 中移除該節點
    // 注意：這只是從 list 容器中移除指標，Node 物件本身還在記憶體中
    bl_[idx].erase(target_node->getIt());

    // 4. 更新 Bucket List 的總數
    num_cells_--;

    // 5. 回傳該 Node，以便後續重新插入或是搬移
    // return target_node;
}


// TODO: updateGain implementation


#endif