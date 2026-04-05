#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>

#include "cell.h"
#include "net.h"
#include "blist.h"
#include "log.h"

using namespace std;

class BList;

class FM {
public:
    // Constructor and Destructor
    FM(ifstream &fin)  {
        cell_count_ = 0;
        net_count_  = 0;
        cut_size_ = 0;
        num_in_partition_.resize(3, 0);        
        Parser(fin);
        Initialization();

        // Print_Cell_array();
        // Print_Net_array();
    } 
    ~FM() {
        for (auto c : cell_array_) delete c;
        for (auto n : net_array_) delete n;
    }

    // data process
    void Parser(ifstream &fin); 
    void Initialization();
    void Print_Cell_array();
    void Print_Net_array();
    void setPmax(int pmax) { pmax_ = pmax; }
    int getPmax() const { return pmax_; }
    void Solve();
    void setTotalGain(int gain) { total_gain_ = gain; }
    void addTotalGain(int gain) { total_gain_ += gain; }
    int getTotalGain() const { return total_gain_; }
    void incrementNumInPartition(int p) { num_in_partition_[p-1]++; }
    void decrementNumInPartition(int p) { num_in_partition_[p-1]--; }
    int getNumInPartition(int p) const { return num_in_partition_[p-1]; }
    void updateGain(Node *node);
    void restoreBestSolution(Log *log);


private:
    // parameters
    int cell_count_;
    int net_count_;
    double ratio_;
    vector <int> num_in_partition_;
    vector <Cell*> cell_array_;
    vector <Net*> net_array_;
    unordered_map<int, Cell*> cell_map_;
    vector<BList*> blist_;
    int pmax_;
    int total_gain_;
    int cut_size_;

};

