#include "fm.h"
#include <set>

void FM::Parser(ifstream &fin) {
    // Balance factor
    fin >> ratio_;

    string token;
    int pmax = 0;

    while(fin >> token) {
        if(token=="NET") {
            string net_name;
            fin >> net_name;

            // Create a new net
            int nid = stoi(net_name.substr(1));
            Net *new_net = new Net(nid, net_name);
            net_count_++;

            int cid;

            while(fin >> token && token!=";") {
                string cell_name = token;
                cid = stoi(cell_name.substr(1));


                if(cell_array_.size()<=cid) {
                    cell_array_.resize(cid+1, nullptr);
                }

                if(cell_array_[cid]==nullptr) {
                    // The cell is a new cell
                    Cell *new_cell = new Cell(cid, cell_name);
                    cell_array_[cid] = new_cell;
                    new_net->addCell(cid, new_cell);
                    cell_count_++;
                }
                else {
                    // The cell already exists, thus directly add the cell to the net
                    new_net->addCell(cid, cell_array_[cid]);
                }

                cell_array_[cid]->addNet(nid, new_net);
                if(cell_array_[cid]->getPins() > pmax) {
                    pmax = cell_array_[cid]->getPins();
                }
            }
            net_array_.push_back(new_net);
            // if(cell_array_[cid]->getPins() > pmax) {
            //     pmax = cell_array_[cid]->getPins();
            // }
            
        }
        
    }

    setPmax(pmax);

}

void FM::Initialization() {
    bool toG1;
    int counter = 0;
    cut_size_ = 0;

    // Assign random partition

    for(auto c : cell_array_) {
        
        toG1 = (counter++ % 3 == 0);
        
        if(c != nullptr) {
            if(toG1) {
                c->movePartition(1);
                incrementNumInPartition(1);
            }
            else {
                c->movePartition(2);
                incrementNumInPartition(2);
            }
        }
    }

    // Check cut state of each net and update where the cell is located in the partition
    for(auto n : net_array_) {
        if(n != nullptr) {
            bool inG1 = false, inG2 = false;
            for(auto c : n->getCells()) {
                if(c->cell_ptr_->getPartition() == 1) {
                    inG1 = true;
                    n->incrementNumInPartition(1);
                }
                else if(c->cell_ptr_->getPartition() == 2) {
                    inG2 = true;
                    n->incrementNumInPartition(2);
                }
                if(inG1 && inG2) {
                    if(n->isCut()==false) {
                        n->updateCut(true);
                        cut_size_++;
                    }
                }
            }
        }
    }

    // Calculate initial gain of each cell
    /*
    
    
    FS(i): number of nets that have cell i as the only cell in From Block
    TE(i): number of nets that contain cell i and are entirely located in From Block
    Gain(i) = FS(i) - TE(i)


    */

    int inG1, inG2;

    for(auto n : net_array_) {
        inG1 = 0;
        inG2 = 0;
        if(n!=nullptr) {
            for(auto c : n->getCells()) {
                if(c->cell_ptr_->getPartition()==1) inG1++;
                else if(c->cell_ptr_->getPartition()==2) inG2++;
            }
            // cout << n->getName() << " inG1: " << inG1 << " inG2: " << inG2 << endl;
            // case I : inG1 == 0 or inG2 == 0 : TE of the cell in the net = 1
            if(inG1 == 0 || inG2 == 0) {
                for(auto c : n->getCells()) {
                    // cout << "cell" << c->cell_ptr_->getId() << "'s TE is increased" << endl;
                    c->cell_ptr_->incrementTE();
                }
            }

                // case II : inG1==1 : FS of the cell in G1 = 1, TE of that = 0. FS TE of the other cells are both 0
            if(inG1 == 1) {
                for(auto c : n->getCells()) {
                    if(c->cell_ptr_->getPartition()==1) {
                        // cout << "cell" << c->cell_ptr_->getId() << "'s FS is increased" << endl;
                        c->cell_ptr_->incrementFS();
                        // break;
                    }
                }
            }
            // case III : inG2==1 : FS of the cell in G2 = 1, TE  of that = 0. FS TE of the other cells are both 0
            if(inG2 == 1) {
                for(auto c : n->getCells()) {
                    if(c->cell_ptr_->getPartition()==2) {
                        c->cell_ptr_->incrementFS();
                        // cout << "cell" << c->cell_ptr_->getId() << "'s FS is increased" << endl;
                        // break;
                    }
                }
            }

            
            
            // case IV : default
            
        }
        // cout << "------------------------------------" << endl;

        
        
    }

    /* 
    
    Set initial gain of each cell
    And maintain summation of each gain for calculate cut size:

                     N = cutsize + total_gain

    */

    setTotalGain(0);
    for(auto c : cell_array_) {
        if(c != nullptr) {
            c->setInitGain();
            addTotalGain(c->getGain());
        }
    }

    // for(auto c : cell_array_) {
    //     if(c != nullptr) {
    //         cout << c->getName() << " gain: " << c->getGain() << endl;
    //     }
    // }

    // cout << "Pmax is " << getPmax() << endl;
    
    // Add each cell to bucket list according to its gain
    for(int i=0; i<3; i++) {
        blist_.push_back(new BList(pmax_));
    }
    
    for(auto c : cell_array_) {
        if(c != nullptr) {
            if(c->getPartition() == 1) {
                blist_[0]->addNode(c);
            }
            else if(c->getPartition() == 2) {
                blist_[1]->addNode(c);
            }
        }
    }
    cout << "num of cell: " << cell_count_ << endl;
    cout << "num of net: " << net_count_ << endl;
    cout << "initial cut size: " << cut_size_ << endl;
    // blist_[0]->printList();

    // GPT
    // Cell *c = cell_array_[15679];
    // cout << "[AFTER INIT] cell 15679 stored gain = " << c->getGain()
    //     << ", true gain = " << recomputeGainFromScratch(c) << endl;
    // debugIncidentNetCounts(c);
    // debugCellGainBreakdown(c);

}

void FM::Solve() {
    int cur_step = 0;
    // Do 1:2 partition
    BList *left = blist_[0];
    BList *right = blist_[1];
    // cout << "+-------------+" << endl;
    // cout << "| Let's solve |" << endl;
    // cout << "+-------------+" << endl;


    // Maintain that the left partition remains 1/3 of total number of cells
    int min_available_cells = (1-ratio_) * cell_array_.size() / 3;
    int max_available_cells = (1+ratio_) * cell_array_.size() / 3;

    // int total_num_cell = cell_array_.size();
    Log log(cut_size_);

    // GPT
    // int running_cut = cut_size_;


    // Create a log to store moving history
    // blist_[0]->printList();
    // blist_[1]->printList();
    // cout << "================ initial blist ==================" << endl;
    for(int i=0; i<cell_count_; i++) {
        // cout << "iter: " << i << endl;
        // if(i==7086) {
        //     cout << "<< LEFT PARTITON >>" << endl;
        //     blist_[0]->printList();
        //     cout << "<< RIGHT PARTITON >>" << endl;
        //     blist_[1]->printList();
        // }
        
        


        Page *page = new Page();
        page->setStep(cur_step++);


        Node *left_node = left->getMaxGainNode();
        Node *right_node = right->getMaxGainNode();

        // GPT
        // if (left_node) {
        //     int g_stored = left_node->getCell()->getGain();
        //     int g_true   = recomputeGainFromScratch(left_node->getCell());
        //     if (g_stored != g_true) {
        //         cout << "[LEFT GAIN MISMATCH] step " << i
        //             << " cell " << left_node->getCell()->getId()
        //             << " stored " << g_stored
        //             << " true " << g_true << endl;
        //             debugCellGainBreakdown(left_node->getCell());
        //         break;
        //     }
        // }

        // if (right_node) {
        //         int g_stored = right_node->getCell()->getGain();
        //         int g_true   = recomputeGainFromScratch(right_node->getCell());
        //         if (g_stored != g_true) {
        //             cout << "[RIGHT GAIN MISMATCH] step " << i
        //                 << " cell " << right_node->getCell()->getId()
        //                 << " stored " << g_stored
        //                 << " true " << g_true << endl;
        //                 debugIncidentNetCounts(right_node->getCell());
        //                 debugCellGainBreakdown(right_node->getCell());
        //             break;
        //         }
        //     }



        // if(i==7086) {
        //     cout << "left node: " << left_node->getCell()->getId() << " right node: " << right_node->getCell()->getId() << endl;
        //     cout << "=====================================" << endl;
        //     cout << "num node in left: " << left->getNumCells() << " num node in right: " << right->getNumCells() << endl;
        // }
        // Find the valid move
        if(left_node == nullptr && right_node == nullptr) {
            cout << "No more valid move! because all cells are in the same partition" << endl;
            break;
        }
        else if(left_node != nullptr && right_node == nullptr) {
            // cout << "case 2" << endl;
            if(getNumInPartition(1)-1<min_available_cells) {
                cout << "No more valid move! because the left partion can not lose any cell" << endl;
                break;
            }
            else {
                // move from left to right
                decrementNumInPartition(1);
                incrementNumInPartition(2);
                // updateGain(left_node);
                // left_node->getCell()->movePartition(2);

                // page->setCellMoved(left_node->getCell());
                // page->setGain(left_node->getCell()->getGain());
                int selected_gian = left_node->getCell()->getGain();
                Cell *moved = left_node->getCell();
                updateGain(left_node);
                moved->movePartition(2);
                page->setCellMoved(moved);
                page->setGain(selected_gian);



                
                // left->getBL()[left_node->getCell()->getGain() + getPmax()].remove(left_node);
            }
        }
        else if(left_node == nullptr && right_node != nullptr) {

            // if(i==7086)cout << "here==" << endl;
            if(getNumInPartition(1)+1>max_available_cells) {
                cout << "No more valid move! because the left partion can not gain any cell" << endl;
                break;
            }
            else {
                // move from right to left
                incrementNumInPartition(1);
                decrementNumInPartition(2);
                

                // updateGain(right_node);
                // right_node->getCell()->movePartition(1);

                // page->setCellMoved(right_node->getCell());
                // page->setGain(right_node->getCell()->getGain());

                int selected_gian = right_node->getCell()->getGain();
                Cell *moved = right_node->getCell();
                updateGain(right_node);
                moved->movePartition(1);
                page->setCellMoved(moved);
                page->setGain(selected_gian);
                


                // right->getBL()[right_node->getCell()->getGain() + getPmax()].remove(right_node);
                
            }
        }
        else {
            // if(!left_node || !right_node) break;
            // cout << "case 4" << endl;
            // cout << "---------------------------------------------" << endl;
            // cout << "left:  " << left_node->getCell()->getId() << " gain: " << left_node->getCell()->getGain() << endl;
            // cout << "right: " << right_node->getCell()->getId() << " gain: " << right_node->getCell()->getGain() << endl;
            // cout << "---------------------------------------------" << endl;
            
            if(left_node->getCell()->getGain() >= right_node->getCell()->getGain()) {
                
                if(getNumInPartition(1)-1<min_available_cells) {
                    // move from right to left
                    incrementNumInPartition(1);
                    decrementNumInPartition(2);
                    // updateGain(right_node);
                    // right_node->getCell()->movePartition(1);

                    // page->setCellMoved(right_node->getCell());
                    // page->setGain(right_node->getCell()->getGain());

                    int selected_gian = right_node->getCell()->getGain();
                    Cell *moved = right_node->getCell();
                    updateGain(right_node);
                    moved->movePartition(1);
                    page->setCellMoved(moved);
                    page->setGain(selected_gian);

                    // right->getBL()[right_node->getCell()->getGain() + getPmax()].remove(right_node);
                }
                else {
                    decrementNumInPartition(1);
                    incrementNumInPartition(2);
                    // updateGain(left_node);
                    // left_node->getCell()->movePartition(2);

                    // page->setCellMoved(left_node->getCell());
                    // page->setGain(left_node->getCell()->getGain());
                    int selected_gian = left_node->getCell()->getGain();
                    Cell *moved = left_node->getCell();
                    updateGain(left_node);
                    moved->movePartition(2);
                    page->setCellMoved(moved);
                    page->setGain(selected_gian);
                    // TODO: Remove operation should be Encapsulated
                    // left->getBL()[left_node->getCell()->getGain() + getPmax()].remove(left_node);
                }
            }
            else {
                
                if(getNumInPartition(1)+1>max_available_cells) {
                    
                    decrementNumInPartition(1);
                    incrementNumInPartition(2);
                    // updateGain(left_node);
                    // left_node->getCell()->movePartition(2);

                    // page->setCellMoved(left_node->getCell());
                    // page->setGain(left_node->getCell()->getGain());

                    int selected_gian = left_node->getCell()->getGain();
                    Cell *moved = left_node->getCell();
                    updateGain(left_node);
                    moved->movePartition(2);
                    page->setCellMoved(moved);
                    page->setGain(selected_gian);

                    // left->getBL()[left_node->getCell()->getGain() + getPmax()].remove(left_node);
                }
                else {
                    
                    incrementNumInPartition(1);
                    decrementNumInPartition(2);
                    // updateGain(right_node);
                    // right_node->getCell()->movePartition(1);
                    
                    // page->setCellMoved(right_node->getCell());
                    // page->setGain(right_node->getCell()->getGain());

                    int selected_gian = right_node->getCell()->getGain();
                    Cell *moved = right_node->getCell();
                    updateGain(right_node);
                    moved->movePartition(1);
                    page->setCellMoved(moved);
                    page->setGain(selected_gian);


                    // blist_[0]->printList();
                    // right->getBL()[right_node->getCell()->getGain() + getPmax()].remove(right_node);
                }
            }
        }
        // blist_[0]->printList();
        // blist_[1]->printList();
        // cout << "================ end of iter " << i << " ===================" << endl;

        


        log.addPage(page);

        // // GPT
        // int actual_cut = recomputeCutSizeFromPartitions();
        // int expected_cut = running_cut - page->getGain();

        // if (actual_cut != expected_cut) {
        //     cout << "Mismatch at step " << page->getStep()
        //         << ", cell " << page->getCellMoved()->getId()
        //         << ", gain " << page->getGain()
        //         << ", expected " << expected_cut
        //         << ", actual " << actual_cut << endl;
        //     break;
        // }

        // running_cut = actual_cut;
        // if(i>=7085)cout << "finish iter " << i << endl;
    }
    
    // log.printLog();
    log.printMinCutSize();
    // cout << "total net: " << net_count_ << endl;
    // cout << "total gain: " << getTotalGain() << endl;
    restoreBestSolution(&log, 1);

    int temp_cut = 0;
    for(auto n : net_array_) {
        n->intraUpdateCut();
        if(n->isCut()==true) temp_cut++;
    }
    cout << "after restore, cut size is: " << temp_cut << endl;

}

inline void FM::updateGain(Node *node) {
    node->getCell()->Lock();
    
    int partition = node->getCell()->getPartition();
    
    blist_[partition-1]->removeNode(node->getCell());

    // cout << "updating gain of cell " << node->getCell()->getId() << " to " << node->getCell()->getGain() << endl;
    // cout << "updating gain of cell " << node->getCell()->getId() << " to " << node->getCell()->getGain() << endl;
    // cout << "updating gain of cell " << node->getCell()->getId() << " to " << node->getCell()->getGain() << endl;
    // cout << "updating gain of cell " << node->getCell()->getId() << " to " << node->getCell()->getGain() << endl;
    // cout << "processing node " << node->getId() << node->getCell()->isLocked() << endl;

    int from, to;

    // Create the Page and add it to the log

    // for each net n on the base cell do
    for(auto net_info : node->getCell()->getNets()) {

        Net *net = net_info->net_ptr_;
        int victim_cell_partition = node->getCell()->getPartition();
        // cout << "--- iter " << net->getName() << " --- " << endl;

        


        
        if(victim_cell_partition == 1) {
            from = net->getNumInPartition(1);
            to = net->getNumInPartition(2);
        }
        else {
            from = net->getNumInPartition(2);
            to = net->getNumInPartition(1);
        }

        // GPT
        // if (net->getId() == 12905) {
        //     cout << "[WATCH12905] base cell " << node->getCell()->getId()
        //         << " basePart " << victim_cell_partition
        //         << " before: from=" << from
        //         << " to=" << to << endl;
        // }



        
        // if T(n) = 0 then increment gains of all free cells on n (case 4)
        if(to == 0) {
            for(auto cell_info : net->getCells()) {
                Cell *cell = cell_info->cell_ptr_;
                if(!cell->isLocked()) {
                    int partition = cell->getPartition();
                    blist_[partition-1]->removeNode(cell);
                    cell->incrementGain();
                    blist_[partition-1]->addNode(cell);

                    // removeNode(cell);
                    // cell->incrementGain();
                    // addNode(cell);
                    
                }
            }
        }
        //else if T(n)=1 then decrement gain of the only T cell on n, if it is free
        else if(to == 1) {
            int to_blk_partition = (node->getCell()->getPartition() == 1) ? 2 : 1;
            for(auto cell_info : net->getCells()) {
                Cell *cell = cell_info->cell_ptr_;
                // cout << "cell id: " << cell->getId() << endl;
                if(cell->getPartition() == to_blk_partition && !cell->isLocked()) {
                    blist_[to_blk_partition-1]->removeNode(cell);
                    cell->decrementGain();
                    blist_[to_blk_partition-1]->addNode(cell);
                    // removeNode(cell);
                    // cell->decrementGain();
                    // addNode(cell);
                }
            }
        }
        // change F(n) and T(n) to reflect the move of the base cell
        from --; to++;
        if(victim_cell_partition == 1) {
            net->decrementNumInPartition(1);
            net->incrementNumInPartition(2);
        }
        else {
            net->decrementNumInPartition(2);
            net->incrementNumInPartition(1);
        }
        

        // if F(n)=0 then decrement gains of all free cells on n (case 1)
        if(from == 0) {
            for(auto cell_info : net->getCells()) {
                
                Cell *cell = cell_info->cell_ptr_;

                // cout << cell->getId() <<"here!!" << endl;

                if(!cell->isLocked()) {
                    // cout << "cell id: " << cell->getId() << endl;
                    int partition = cell->getPartition();
                    blist_[partition-1]->removeNode(cell);
                    cell->decrementGain();
                    blist_[partition-1]->addNode(cell);
                    // removeNode(cell);
                    // cell->decrementGain();
                    // addNode(cell);
                }
            }
        }
        // else if F(n) = 1 then increment gain of the only F cell on n, if it is free (case 3, 4)
        else if(from == 1) {
            int from_blk_partition = node->getCell()->getPartition();
            for(auto cell_info : net->getCells()) {
                Cell *cell = cell_info->cell_ptr_;
                if(cell->getPartition() == from_blk_partition && !cell->isLocked()) {

                    // GPT
                    // if (cell->getId() == 15679) {
                    //     cout << "  [FROM==1] updating cell 15679 on net " << net->getId()
                    //         << " old gain=" << cell->getGain() << endl;
                    // }


                    blist_[from_blk_partition-1]->removeNode(cell);
                    cell->incrementGain();
                    blist_[from_blk_partition-1]->addNode(cell);


                    // // GPT
                    // if (cell->getId() == 15679) {
                    //     cout << "  [FROM==1] new gain=" << cell->getGain() << endl;
                    // }
                    




                    // removeNode(cell);
                    // cell->incrementGain();
                    // addNode(cell);
                    // blist_[from_blk_partition-1]->removeNode(cell);
                    // cell->incrementGain();
                    // blist_[from_blk_partition-1]->addNode(cell);
                }
            }
        }

    }


}

void FM::restoreBestSolution(Log *log, int phase) {
    int last_step = log->getLog().size() - 1;
    int best_step = log->getBestStep() + 1;
    // restore to the next step of best step: remain the best cell unmoved
    cout << "best step is: " << best_step << endl;
    cout << "last step is: " << last_step << endl;
    for(int i=last_step; i>=best_step; i--) {
        Cell *to_be_restored_cell = log->getLog()[i]->getCellMoved();
        int moved_partition = to_be_restored_cell->getPartition();
        if(phase==1) {
            // cout << "restoring cell " << to_be_restored_cell->getId() << " from partition " << moved_partition << " to " << (moved_partition==1? 2 : 1) << endl;
            if(moved_partition == 1) to_be_restored_cell->movePartition(2);
            else to_be_restored_cell->movePartition(1);
        }
        else {

        }
    }
    

}


void FM::Print_Cell_array() {
    for (auto c : cell_array_) {
        if (c != nullptr) {
            cout << c->getName() << "|" << c->getPartition() << "-> ";
            for(auto n : c->getNets()) {
                cout << n->id_ << " ";
            }
            cout << endl;

        }
    }
}

void FM::Print_Net_array() {
    for (auto n : net_array_) {
        if (n != nullptr) {
            cout << n->getName() << "-> " << endl;
            for(auto c : n->getCells()) {
                cout << c->id_ << " gain: ";
                c->cell_ptr_->showGain();
                cout << endl;
            }
            cout << endl;
        }
    }
}