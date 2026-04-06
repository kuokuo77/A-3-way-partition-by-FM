// log.h

/*

this class Log is used to store the cell status of each move

+--------------------+
| max_gain, max_step |
+--------------------+

+-------+-------------------+
| index | STEP | CELL MOVED |
+-------+-------------------+

*/

#include <vector>
#include <iostream>


using namespace std;

class Cell;

class Page {
public:
    Page() {}
    Page(int step, Cell *cell_moved) : step_(step), cell_moved_(cell_moved) {}
    ~Page() {}
    int getStep() const { return step_; }
    void setStep(int step) { step_ = step; }
    void setCellMoved(Cell *cell) { cell_moved_ = cell; }
    Cell *getCellMoved() const { return cell_moved_; }
    int getGain() const { return gain_; }
    void setGain(int gain) { gain_ = gain; }
private:
    int step_;
    Cell *cell_moved_;
    int gain_;
};

class Log {
public:
    Log(int init_cut_size) : init_cut_size_(init_cut_size), best_step_(-1) {
        min_cut_size_ = init_cut_size;
        cur_cut_size_ = init_cut_size;
    }
    ~Log() {}
    void addPage(Page *page) {
        move_log_.push_back(page);
        cur_cut_size_ = cur_cut_size_ - page->getGain();
        if(cur_cut_size_ < min_cut_size_) {
            best_step_ = page->getStep();
            min_cut_size_ = cur_cut_size_;
        }
    }
    void printLog() {
        cout << "<<  Log >>" << endl;
        for(auto page : move_log_) {
            cout << "| step: " << page->getStep();
            cout << " cell moved: " << page->getCellMoved()->getId() << " |";
            cout << " gain: " << page->getGain() << " |" << endl;
        }
    }
    vector<Page*> getLog() const { return move_log_; }

    void printMinCutSize() {
        cout << "+-<<  Cut Info >>" << endl;
        cout << "| max step: " << best_step_ << endl;
        cout << "| init cut size: " << init_cut_size_ << endl;
        cout << "| min cut size: " << min_cut_size_ << endl;
        cout << "+=<< End of Cut info >>" << endl;
    }
    int getBestStep() const { return best_step_; }
    int getMinCutSize() { return min_cut_size_; }

private:
    int init_cut_size_;
    int min_cut_size_;
    int cur_cut_size_;
    int best_step_;
    vector<Page*> move_log_;
};