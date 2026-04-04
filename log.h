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
private:
    int step_;
    Cell *cell_moved_;
};

class Log {
public:
    Log(int max_gain) : max_gain_(max_gain), max_step_(0) {}
    ~Log() {}
    void addPage(Page *page) { move_log_.push_back(page);
    }
    void printLog() {
        cout << "<<  Log >>" << endl;
        for(auto page : move_log_) {
            cout << "| step: " << page->getStep();
            cout << " cell moved: " << page->getCellMoved()->getId() << " |" << endl;
        }
        cout << "<< End of Log >>" << endl;
    }

private:
    int max_gain_;
    int max_step_;
    vector<Page*> move_log_;
};