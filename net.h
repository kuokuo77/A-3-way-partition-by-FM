// net.h

#ifndef NET_H
#define NET_H

#include <vector>
// #include "cell.h"
#include <string>

class Cell;

/*
The data type of the element in net array and the net itself
+-------+
| NET   |
+-------+
| id    | net number in integer
| name  | net number in string. eg. "n100"
| cells | vector <Cell><Cell>.....<Cell> stored the Cell class represented the cell connected to this net
| count | number of cells in the partition 0, 1, 2
+-------+

*/

class Cell_info {
public:
    Cell_info(int id, Cell *cell_ptr) : id_(id), cell_ptr_(cell_ptr) {}
    ~Cell_info() {}


    int id_;
    Cell *cell_ptr_;
};

class Net {
public:
    Net(int id, string name) : id_(id), name_(name), is_cut_(false) {
		num_in_partition_.resize(3, 0);
	}
	~Net() {}

    int getId() const { return id_; }
	string getName() const { return name_; }
	vector<Cell_info*> getCells() const { return cells_; }
	void addCell(int id, Cell *cell) {cells_.push_back(new Cell_info(id, cell));}
	void updateCut(bool is_cut) { is_cut_ = is_cut; }
	bool isCut() const { return is_cut_; }
	void intraUpdateCut();

	void incrementNumInPartition(int p) { num_in_partition_[p-1]++; }
	void decrementNumInPartition(int p) { num_in_partition_[p-1]--; }
	int getNumInPartition(int p) const { return num_in_partition_[p-1]; }


private:
    int const id_;
	string const name_;
	vector<Cell_info*> cells_;       
	vector<int> num_in_partition_;
	bool is_cut_;    
};



#endif