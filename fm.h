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
        // cell_count_ = 0;
        // net_count_  = 0;
        // cut_size_ = 0;
        // num_in_partition_.resize(3, 0);        
        // Parser(fin);
        // Initialization();
        // Solve();
        // InitPhaseTwo();
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
    void InitPhaseTwo();
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
    // void restoreBestSolution(Log *log, int phase);
    void updateCutsize(int size) { cut_size_ = size; }
    void InitGainPhaseTwo();




    // GPT
    void OutputResult(const string &output_file);
    void Run() {
        Solve();              // phase 1
        InitPhaseTwo();
        InitGainPhaseTwo();
        BuildBucketPhaseTwo();
        SolvePhaseTwo();
    }
    void ResetBuckets() {
        for (auto b : blist_) delete b;
        blist_.clear();
    }

    void BuildBucketPhaseTwo() {
        ResetBuckets();
        blist_.push_back(new BList(pmax_));   // for partition 2
        blist_.push_back(new BList(pmax_));   // for partition 3

        for (auto c : cell_array_) {
            if (c == nullptr) continue;

            if (c->getPartition() == 2) {
                blist_[0]->addNode(c);
            } else if (c->getPartition() == 3) {
                blist_[1]->addNode(c);
            }
        }
    }
    int CountPhaseTwoCells() const {
        int cnt = 0;
        for (auto c : cell_array_) {
            if (c != nullptr && (c->getPartition() == 2 || c->getPartition() == 3)) {
                cnt++;
            }
        }
        return cnt;
    }
    void UpdateGainPhaseTwo(Node *node) {
        Cell *base = node->getCell();
        base->Lock();

        int base_partition = base->getPartition();   // only 2 or 3
        int bucket_idx = (base_partition == 2) ? 0 : 1;

        blist_[bucket_idx]->removeNode(base);

        for (auto net_info : base->getNets()) {
            Net *net = net_info->net_ptr_;

            // Nets touching set 1 do not affect phase-2 objective
            if (net->getNumInPartition(1) > 0) continue;

            int from, to;
            if (base_partition == 2) {
                from = net->getNumInPartition(2);
                to   = net->getNumInPartition(3);
            } else {
                from = net->getNumInPartition(3);
                to   = net->getNumInPartition(2);
            }

            // If T(n) == 0, increment gains of all free cells on n
            if (to == 0) {
                for (auto cell_info : net->getCells()) {
                    Cell *cell = cell_info->cell_ptr_;
                    if ((cell->getPartition() == 2 || cell->getPartition() == 3) && !cell->isLocked()) {
                        int bi = (cell->getPartition() == 2) ? 0 : 1;
                        blist_[bi]->removeNode(cell);
                        cell->incrementGain();
                        blist_[bi]->addNode(cell);
                    }
                }
            }
            // Else if T(n) == 1, decrement gain of the only T cell if free
            else if (to == 1) {
                int to_partition = (base_partition == 2) ? 3 : 2;
                for (auto cell_info : net->getCells()) {
                    Cell *cell = cell_info->cell_ptr_;
                    if (cell->getPartition() == to_partition && !cell->isLocked()) {
                        int bi = (to_partition == 2) ? 0 : 1;
                        blist_[bi]->removeNode(cell);
                        cell->decrementGain();
                        blist_[bi]->addNode(cell);
                    }
                }
            }

            // Reflect move of base cell
            from--;
            to++;

            if (base_partition == 2) {
                net->decrementNumInPartition(2);
                net->incrementNumInPartition(3);
            } else {
                net->decrementNumInPartition(3);
                net->incrementNumInPartition(2);
            }

            // If F(n) == 0, decrement gains of all free cells on n
            if (from == 0) {
                for (auto cell_info : net->getCells()) {
                    Cell *cell = cell_info->cell_ptr_;
                    if ((cell->getPartition() == 2 || cell->getPartition() == 3) && !cell->isLocked()) {
                        int bi = (cell->getPartition() == 2) ? 0 : 1;
                        blist_[bi]->removeNode(cell);
                        cell->decrementGain();
                        blist_[bi]->addNode(cell);
                    }
                }
            }
            // Else if F(n) == 1, increment gain of the only F cell if free
            else if (from == 1) {
                int from_partition = base_partition;
                for (auto cell_info : net->getCells()) {
                    Cell *cell = cell_info->cell_ptr_;
                    if (cell->getPartition() == from_partition && !cell->isLocked()) {
                        int bi = (from_partition == 2) ? 0 : 1;
                        blist_[bi]->removeNode(cell);
                        cell->incrementGain();
                        blist_[bi]->addNode(cell);
                    }
                }
            }
        }
    }
    void restoreBestSolution(Log *log, int phase) {
        int last_step = (int)log->getLog().size() - 1;
        int best_step = log->getBestStep();

        for (int i = last_step; i > best_step; --i) {
            Cell *cell = log->getLog()[i]->getCellMoved();
            int p = cell->getPartition();

            if (phase == 1) {
                if (p == 1) cell->movePartition(2);
                else if (p == 2) cell->movePartition(1);
            } else if (phase == 2) {
                if (p == 2) cell->movePartition(3);
                else if (p == 3) cell->movePartition(2);
            }
        }
    }
    void SolvePhaseTwo() {
        // InitPhaseTwo();
        // InitGainPhaseTwo();
        // BuildBucketPhaseTwo();

        int cur_step = 0;
        BList *left = blist_[0];   // partition 2
        BList *right = blist_[1];  // partition 3

        int phase2_cells = CountPhaseTwoCells();
        int min_available_cells = ceil((1-ratio_)  * cell_count_ / 3);
        int max_available_cells = floor((1+ratio_) * cell_count_ / 3);
        cout << "min available cells: " << min_available_cells << " max available cells: " << max_available_cells << endl;

        Log log(cut_size_);

        for (int i = 0; i < phase2_cells; i++) {
            Page *page = new Page();
            page->setStep(cur_step++);

            Node *left_node = left->getMaxGainNode();    // from partition 2
            Node *right_node = right->getMaxGainNode();  // from partition 3

            if (left_node == nullptr && right_node == nullptr) {
                cout << "No more valid move in phase 2." << endl;
                break;
            }
            else if (left_node != nullptr && right_node == nullptr) {
                if (getNumInPartition(2) - 1 < min_available_cells || getNumInPartition(3) + 1 > max_available_cells) {
                    break;
                } else {
                    decrementNumInPartition(2);
                    incrementNumInPartition(3);

                    int selected_gain = left_node->getCell()->getGain();
                    Cell *moved = left_node->getCell();

                    UpdateGainPhaseTwo(left_node);
                    moved->movePartition(3);

                    page->setCellMoved(moved);
                    page->setGain(selected_gain);
                }
            }
            else if (left_node == nullptr && right_node != nullptr) {
                if (getNumInPartition(2) + 1 > max_available_cells || getNumInPartition(3) - 1 < min_available_cells) {
                    break;
                } else {
                    incrementNumInPartition(2);
                    decrementNumInPartition(3);

                    int selected_gain = right_node->getCell()->getGain();
                    Cell *moved = right_node->getCell();

                    UpdateGainPhaseTwo(right_node);
                    moved->movePartition(2);

                    page->setCellMoved(moved);
                    page->setGain(selected_gain);
                }
            }
            else {
                if (left_node->getCell()->getGain() >= right_node->getCell()->getGain()) {
                    if (getNumInPartition(2) - 1 < min_available_cells || getNumInPartition(3) + 1 > max_available_cells) {
                        // right to left
                        incrementNumInPartition(2);
                        decrementNumInPartition(3);

                        int selected_gain = right_node->getCell()->getGain();
                        Cell *moved = right_node->getCell();

                        UpdateGainPhaseTwo(right_node);
                        moved->movePartition(2);

                        page->setCellMoved(moved);
                        page->setGain(selected_gain);
                    } else {
                        // left to right
                        decrementNumInPartition(2);
                        incrementNumInPartition(3);

                        int selected_gain = left_node->getCell()->getGain();
                        Cell *moved = left_node->getCell();

                        UpdateGainPhaseTwo(left_node);
                        moved->movePartition(3);

                        page->setCellMoved(moved);
                        page->setGain(selected_gain);
                    }
                }
                else {
                    if (getNumInPartition(2) + 1 > max_available_cells || getNumInPartition(3) - 1 < min_available_cells) {
                        // left to right
                        decrementNumInPartition(2);
                        incrementNumInPartition(3);

                        int selected_gain = left_node->getCell()->getGain();
                        Cell *moved = left_node->getCell();

                        UpdateGainPhaseTwo(left_node);
                        moved->movePartition(3);

                        page->setCellMoved(moved);
                        page->setGain(selected_gain);
                    } else {
                        // right to left
                        incrementNumInPartition(2);
                        decrementNumInPartition(3);

                        int selected_gain = right_node->getCell()->getGain();
                        Cell *moved = right_node->getCell();

                        UpdateGainPhaseTwo(right_node);
                        moved->movePartition(2);

                        page->setCellMoved(moved);
                        page->setGain(selected_gain);
                    }
                }
            }

            log.addPage(page);
            // cout << "step: " << cur_step-1 << " gain: " << page->getGain() << " cell: " << page->getCellMoved()->getId() << " partition: " << page->getCellMoved()->getPartition() << endl;
        }

        log.printMinCutSize();

        restoreBestSolution(&log, 2);

        // Rebuild all net counts / cut state after restoring best phase-2 solution
        cut_size_ = 0;
        for (auto n : net_array_) {
            if (n == nullptr) continue;

            n->resetNumInPartition();
            bool inG1 = false, inG2 = false, inG3 = false;

            for (auto c : n->getCells()) {
                int p = c->cell_ptr_->getPartition();
                if (p == 1) {
                    inG1 = true;
                    n->incrementNumInPartition(1);
                } else if (p == 2) {
                    inG2 = true;
                    n->incrementNumInPartition(2);
                } else if (p == 3) {
                    inG3 = true;
                    n->incrementNumInPartition(3);
                }
            }

            bool is_cut = ((int)inG1 + (int)inG2 + (int)inG3 > 1);
            n->updateCut(is_cut);
            if (is_cut) cut_size_++;
        }

        cout << "after phase 2 restore, cut size is: " << cut_size_ << endl;
        cout << "min cut size recorded in phase 2 log: " << log.getMinCutSize() << endl;
    }

    void RebuildNetStateAndCutsize() {
        cut_size_ = 0;
        num_in_partition_[0] = 0;
        num_in_partition_[1] = 0;
        num_in_partition_[2] = 0;

        for (auto c : cell_array_) {
            if (c == nullptr) continue;
            int p = c->getPartition();
            if (p >= 1 && p <= 3) {
                incrementNumInPartition(p);
            }
        }

        for (auto n : net_array_) {
            if (n == nullptr) continue;

            n->resetNumInPartition();
            bool inG1 = false, inG2 = false, inG3 = false;

            for (auto c : n->getCells()) {
                int p = c->cell_ptr_->getPartition();
                if (p == 1) {
                    inG1 = true;
                    n->incrementNumInPartition(1);
                } else if (p == 2) {
                    inG2 = true;
                    n->incrementNumInPartition(2);
                } else if (p == 3) {
                    inG3 = true;
                    n->incrementNumInPartition(3);
                }
            }

            bool is_cut = ((int)inG1 + (int)inG2 + (int)inG3 > 1);
            n->updateCut(is_cut);
            if (is_cut) cut_size_++;
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

