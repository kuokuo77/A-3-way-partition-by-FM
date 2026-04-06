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
    void restoreBestSolution(Log *log, int phase);

    // GPT
    int recomputeCutSizeFromPartitions() {
        int cut = 0;
        for (auto n : net_array_) {
            if (n == nullptr) continue;
            bool inG1 = false, inG2 = false;
            for (auto c : n->getCells()) {
                int p = c->cell_ptr_->getPartition();
                if (p == 1) inG1 = true;
                else if (p == 2) inG2 = true;
            }
            if (inG1 && inG2) cut++;
        }
        return cut;
    }
    int recomputeGainFromScratch(Cell *target) {
        int gain = 0;
        int from_p = target->getPartition();
        int to_p = (from_p == 1) ? 2 : 1;

        for (auto net_info : target->getNets()) {
            Net *net = net_info->net_ptr_;

            int from = 0, to = 0;
            for (auto cell_info : net->getCells()) {
                int p = cell_info->cell_ptr_->getPartition();
                if (p == from_p) from++;
                else if (p == to_p) to++;
            }

            if (from == 1) gain++;
            if (to == 0) gain--;
        }
        return gain;
    }
    void debugCellGainBreakdown(Cell *target) {
        int from_p = target->getPartition();
        int to_p   = (from_p == 1) ? 2 : 1;

        cout << "=== debug cell " << target->getId()
            << " partition " << from_p << " ===" << endl;

        int total = 0;
        for (auto net_info : target->getNets()) {
            Net *net = net_info->net_ptr_;
            int from = 0, to = 0;

            for (auto cell_info : net->getCells()) {
                int p = cell_info->cell_ptr_->getPartition();
                if (p == from_p) from++;
                else if (p == to_p) to++;
                // else to++;
            }

            int delta = 0;
            if (from == 1) delta++;
            if (to == 0) delta--;

            total += delta;

            cout << "net " << net->getId()
                << " from=" << from
                << " to=" << to
                << " contrib=" << delta << endl;
        }

        cout << "true gain total = " << total
            << ", stored gain = " << target->getGain() << endl;
    }
    void debugIncidentNetCounts(Cell *target) {
        cout << "=== incident net counts for cell " << target->getId() << " ===" << endl;
        for (auto net_info : target->getNets()) {
            Net *net = net_info->net_ptr_;
            int actual1 = 0, actual2 = 0;

            for (auto cell_info : net->getCells()) {
                int p = cell_info->cell_ptr_->getPartition();
                if (p == 1) actual1++;
                else if (p == 2) actual2++;
            }

            cout << "net " << net->getId()
                << " cached=(" << net->getNumInPartition(1)
                << "," << net->getNumInPartition(2) << ")"
                << " actual=(" << actual1
                << "," << actual2 << ")" << endl;
        }
    }



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

