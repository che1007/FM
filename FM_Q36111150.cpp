#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <random>
#include <chrono>
#include <set>
#include <unordered_map>
#include <list>
#include <ctime>
using namespace std;


struct Node {
    string name;
    int size = 0;
    int gain = 0;
    int set = 0;
    int num_pin = 0;
    vector<int> netlist;
    Node* next = nullptr;
    Node* prev = nullptr;
    Node* cell_to_bucketnode = nullptr;
    Node* bucketnode_to_cell = nullptr;
    bool locked = false;
};

struct Net {
    int num_cell = 0;
    string net_id;
    int num_set1 = 0;
    int num_set2 = 0;
    vector<Node*> cells;
    
};

struct net_NumSet {
    string name;
    int num_set1 = 0;
    int num_set2 = 0;
};

class DoublyLinkedList {
public:
    
    DoublyLinkedList() : head(nullptr), tail(nullptr) ,max(nullptr){}

    void init_head() {
        head->next = nullptr;
    }

    void initial() {
        Node* empty = new Node;
        head = empty;
        empty->prev = nullptr;
        empty->next = nullptr;
    }

    void push_back(Node* node) {
        Node* newCell = new Node;
        if (!head->next) {
            head->next = newCell;
            newCell->prev = head;
            newCell->next = nullptr;
            tail = newCell;
            newCell->bucketnode_to_cell = node;
            node->cell_to_bucketnode = newCell;
        }
        else {
            newCell->prev = tail;
            newCell->next = nullptr;
            tail->next = newCell;
            tail = newCell;
            node->cell_to_bucketnode = newCell;
            newCell->bucketnode_to_cell = node;
        }
    }

    void print() {
        Node* current = head->next;
        while (current) {
            cout << "Cell ID: " << current->bucketnode_to_cell->name << " Gain: " << current->bucketnode_to_cell->gain << " ";
            current = current->next;
        }
        cout << endl;
    }

    void select_basecell_fromA(float& lower, float& upper, int& size_a, int& set1maxgain, int& set2maxgain, int& max_pin) {
        max = head->next;
        while (max) {
            if ((size_a - max->bucketnode_to_cell->size <= upper) && (size_a - max->bucketnode_to_cell->size >= lower) && (!max->bucketnode_to_cell->locked)) {
                break;
            }
            else {
                max = max->next;
            }   
        }
    }

    void select_basecell_fromB(float& lower, float& upper, int& size_a, int& set1maxgain, int& set2maxgain, int& max_pin) {
        max = head->next;
        while (max) {
            if ((size_a + max->bucketnode_to_cell->size <= upper) && (size_a + max->bucketnode_to_cell->size >= lower) && (!max->bucketnode_to_cell->locked)) {
                break;
            }
            else {
                max = max->next;
            }   
        }
    }



    void delete_cell(Node* mark) {
        if (mark == head->next) {
            if (mark->next==nullptr) {
                // cout << "1st and only one" << endl;
                mark->prev = nullptr;
                head->next = nullptr;
            }
            else {
                // cout << "1st and more" << endl;
                head->next = mark->next;
                mark->next->prev = head;
                mark->prev = nullptr;
                mark->next = nullptr;
            }
        }
        else {
            if (mark->next==nullptr) {
                // cout << "last one" << endl;
                mark->prev->next = nullptr;
                mark->next = nullptr;
                mark->prev = nullptr;
            }
            else {
                // cout << "in the middle" << endl;
                mark->prev->next = mark->next;
                mark->next->prev = mark->prev;
                mark->next = nullptr;
                mark->prev = nullptr;
            }
        }    
    }

    void insert(Node* mark) {
        if (!head->next) {
            head->next = mark;
            mark->prev = head;
            mark->next = nullptr;
        }
        else {
            mark->next = head->next;
            mark->prev = head;
            head->next->prev = mark;
            head->next = mark;
        }
    }

    Node* getHead() const {
        return head;
    }

    Node* getTail() const {
        return tail;
    }

    Node* getMax() const {
        return max;
    }

private:
    Node* head;
    Node* tail;   
    Node* max; 
};

vector<Net> nets;
vector<Node> nodes;
vector<DoublyLinkedList> bucketListSetA;
vector<DoublyLinkedList> bucketListSetB;
vector<net_NumSet> net_numset;
Node* maxptr;


void read_node(ifstream& nodefile, int& total, int& max) {
    string line;
    bool readingdata = false;
    while (getline(nodefile, line)) {
        if (line.find("NumTerminals") != string::npos) {
            readingdata = true;
            continue;
        }

        if (readingdata && (line.find("terminal") != string::npos)) {
            break;
        }

        if (readingdata) {
            Node node;
            istringstream iss(line);
            iss >> std::ws;
            int temp1, temp2;
            if (iss >> node.name >> temp1 >> temp2) {
                node.size = temp1 * temp2;
                node.gain = 0;
                total += node.size;
                nodes.push_back(node);
            }
            if (node.size > max) {
                max = node.size;
            }
        }
    }
}

void read_net(ifstream& netfile) {
    string l;
    Node& cell = nodes.back();
    const int last_index = stoi(cell.name.substr(1));
    // cout << last_index << endl;
    int index = 0;
    while (getline(netfile, l)) {
        if (l.find("NetDegree") != string::npos) {
            Net net;
            istringstream iss(l);
            string temp;
            iss >> temp >> temp >> net.num_cell >> net.net_id;
            // cout << "Num cell: " << net.num_cell << "  " << "Cell Id: " << net.net_id <<endl;
            set<string> cellname;
            int num = 0;
            for (int i = 0; i < net.num_cell; i++) {
                getline(netfile, l);
                string name;
                istringstream iss(l);
                iss >> std::ws;
                iss >> name;
                if (l.find("p") != 0) {
                    if (stoi(name.substr(1)) <= last_index) {
                        if (cellname.find(name) == cellname.end()) {
                            Node& c = nodes[stoi(name.substr(1))];
                            net.cells.push_back(&c);
                            cellname.insert(name);
                            num += 1;
                        }
                    }
                }
                
            }
            
            if (num > 1) {
                for (auto cell: net.cells) {
                    cell->netlist.push_back(index);
                }
                net.num_cell = num;
                nets.push_back(net);
                index += 1;
            }
            cellname.clear();
        }
    }
}

void partition(int& cumulativeSizeSet_a, vector<Node> n, vector<Node*>& a, vector<Node*>& b, int& total_size) {
    int targetSizeSet1 = 0.4 * total_size;
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    mt19937 g(seed);
    shuffle(n.begin(), n.end(), g);
    for (Node& node : n) {
        Node& cell = nodes[stoi(node.name.substr(1))];
        if (cumulativeSizeSet_a + node.size <= targetSizeSet1) {
            cell.set = 1;
            a.push_back(&cell);
            cumulativeSizeSet_a += node.size;
            for (const int idx: cell.netlist) {
                Net& net = nets[idx];
                net.num_set1 += 1;
            }
        } 
        else {
            cell.set = 2;
            b.push_back(&cell);
            for (const int idx: cell.netlist) {
                Net& net = nets[idx];
                net.num_set2 += 1;
            }
        }
    }
}

void initial_gain(int& max_pin) {
    for (Node& node: nodes) {
        node.gain = 0;
        int num_pin = 0;
        for (const int& idx: node.netlist) {
            int Fn = 0;
            int Tn = 0;
            Net& net = nets[idx];
            if (node.set == 1) {
                Fn = net.num_set1;
                Tn = net.num_set2;
            }
            else {
                Fn = net.num_set2;
                Tn = net.num_set1;
            }

            if (Fn == 1) {
                node.gain += 1;
            }
            if (Tn == 0) {
                node.gain -= 1;
            }
            num_pin += 1;
        }
        node.num_pin = num_pin;
        if (num_pin > max_pin) {
            max_pin = num_pin;
        }
    }
}

void init_bucketlist(int& max_pin) {
    for (auto node: nodes) {
        if (node.locked) {
            cout << "BUG" << endl;
        }
        node.cell_to_bucketnode->prev = nullptr;
        node.cell_to_bucketnode->next = nullptr;
    }
    for (int i = 0; i <= 2*max_pin; i++) {
        bucketListSetA[i].init_head();
        bucketListSetB[i].init_head();
    }
    for (int i = 0; i <= 2*max_pin; i++) {
        if (!bucketListSetA[i].getHead()) {
            cout << "A fail" << endl;
        }
        if (!bucketListSetB[i].getHead()) {
            cout << "B fail" << endl;
        }
    } 
}

void build_bucketlist(int& max_pin) {
    for (int i = 0; i <= 2*max_pin; i++) {
        bucketListSetA.push_back(DoublyLinkedList());
        bucketListSetB.push_back(DoublyLinkedList());
        bucketListSetA[i].initial();
        bucketListSetB[i].initial();
    }

    for (Node& node: nodes) {
        if (node.set == 1) {
            bucketListSetA[node.gain + max_pin].push_back(&node);
        }
        else {
            bucketListSetB[node.gain + max_pin].push_back(&node);
        }
    }
}

void find_maxgain(int& set1_maxgain, int& set2_maxgain, int& max_pin) {
    
    set1_maxgain = -max_pin;
    set2_maxgain = -max_pin;

    for (int i = 2 * max_pin ; i >= 0; i--) {
        Node* head = bucketListSetA[i].getHead();
        if (head->next) {
            set1_maxgain = max(set1_maxgain, i - max_pin);
            break;
        }
    }

    for (int i = 2 * max_pin ; i >= 0; i--) {
        Node* head = bucketListSetB[i].getHead();
        if (head->next) {
            set2_maxgain = max(set2_maxgain, i - max_pin);
            break;
        }
    }
}

void findbase(float& lower, float& upper, int& size_a, int& set1_maxgain, int& set2_maxgain, int& max_pin) {
    if (set1_maxgain >= set2_maxgain) {
        bucketListSetA[set1_maxgain + max_pin].select_basecell_fromA(lower, upper, size_a, set1_maxgain, set2_maxgain, max_pin);
        maxptr = bucketListSetA[set1_maxgain + max_pin].getMax();
        if (maxptr == nullptr) {
            bucketListSetB[set2_maxgain + max_pin].select_basecell_fromB(lower, upper, size_a, set1_maxgain, set2_maxgain, max_pin);
            maxptr = bucketListSetB[set2_maxgain + max_pin].getMax();
            if (maxptr == nullptr) {
                cout << "ALL NULL" << endl;
                int index_a = 0;
                int index_b = 0;
                for (int i = set1_maxgain + max_pin - 1; i>=0; i--) {
                    bucketListSetA[i].select_basecell_fromA(lower, upper, size_a, set1_maxgain, set2_maxgain, max_pin);
                    if (bucketListSetA[i].getMax() != nullptr) {
                        index_a = i;
                        break;
                    }
                }
                for (int i = set2_maxgain + max_pin - 1; i>=0; i--) {
                    bucketListSetB[i].select_basecell_fromB(lower, upper, size_a, set1_maxgain, set2_maxgain, max_pin);
                    if (bucketListSetB[i].getMax() != nullptr) {
                        index_b = i;
                        break;
                    }
                }

                if (bucketListSetA[index_a].getMax() && bucketListSetB[index_b].getMax()) {
                    if (index_a > index_b) {
                        maxptr = bucketListSetA[index_a].getMax();
                    }
                    else {
                        maxptr = bucketListSetB[index_b].getMax();
                    }
                }
                else if (bucketListSetA[index_a].getMax() && !bucketListSetB[index_b].getMax()) {
                    maxptr = bucketListSetA[index_a].getMax();
                }
                else if (!bucketListSetA[index_a].getMax() && bucketListSetB[index_b].getMax()) {
                    maxptr = bucketListSetB[index_b].getMax();
                }
                else {
                    maxptr = nullptr;
                    cout << "Still NULL" << endl;
                }
            }
        }
    }
    else {
        bucketListSetB[set2_maxgain + max_pin].select_basecell_fromB(lower, upper, size_a, set1_maxgain, set2_maxgain, max_pin);
        maxptr = bucketListSetB[set2_maxgain + max_pin].getMax();
        if (maxptr == nullptr) {
            bucketListSetA[set1_maxgain + max_pin].select_basecell_fromA(lower, upper, size_a, set1_maxgain, set2_maxgain, max_pin);
            maxptr = bucketListSetA[set1_maxgain + max_pin].getMax();
            if (maxptr == nullptr) {
                cout << "ALL NULL" << endl;
                int index_a = 0;
                int index_b = 0;
                for (int i = set1_maxgain + max_pin - 1; i>=0; i--) {
                    bucketListSetA[i].select_basecell_fromA(lower, upper, size_a, set1_maxgain, set2_maxgain, max_pin);
                    if (bucketListSetA[i].getMax() != nullptr) {
                        index_a = i;
                        break;
                    }
                }
                for (int i = set2_maxgain + max_pin - 1; i>=0; i--) {
                    bucketListSetB[i].select_basecell_fromB(lower, upper, size_a, set1_maxgain, set2_maxgain, max_pin);
                    if (bucketListSetB[i].getMax() != nullptr) {
                        index_b = i;
                        break;
                    }
                }

                if (bucketListSetA[index_a].getMax() && bucketListSetB[index_b].getMax()) {
                    if (index_a > index_b) {
                        maxptr = bucketListSetA[index_a].getMax();
                    }
                    else {
                        maxptr = bucketListSetB[index_b].getMax();
                    }
                }
                else if (bucketListSetA[index_a].getMax() && !bucketListSetB[index_b].getMax()) {
                    maxptr = bucketListSetA[index_a].getMax();
                }
                else if (!bucketListSetA[index_a].getMax() && bucketListSetB[index_b].getMax()) {
                    maxptr = bucketListSetB[index_b].getMax();
                }
                else {
                    maxptr = nullptr;
                    cout << "Still NULL" << endl;
                }
            }
        }
        // cout << "Name: " << maxptr->bucketnode_to_cell->name << ", Gain: " << maxptr->bucketnode_to_cell->gain << endl;
    }
}

void update_gain(unordered_map<Node*, int>& pointerMap) {
    for (const int idx: maxptr->bucketnode_to_cell->netlist) {
        Net& net = nets[idx];
        int num_set1 = net.num_set1;
        int num_set2 = net.num_set2;
        int Tn, Fn;
        if (maxptr->bucketnode_to_cell->set == 1) {
            Tn = num_set2;
            Fn = num_set1 - 1;
        }
        if (maxptr->bucketnode_to_cell->set == 2) {
            Tn = num_set1;
            Fn = num_set2 - 1;
        }
        for (Node* cell: net.cells) {
            if (!cell->locked) {
                int offset = 0;
                if (cell->name != maxptr->bucketnode_to_cell->name) {
                    if (Tn == 0) {
                        offset += 1;
                    }

                    if (Tn == 1) {
                        if (cell->set != maxptr->bucketnode_to_cell->set) {
                            offset -= 1;
                        }
                    }

                    if (Fn == 0) {
                        offset -= 1;
                    }

                    if (Fn == 1) {
                        if (cell->set == maxptr->bucketnode_to_cell->set) {
                            offset += 1;
                        }
                    }

                    if (pointerMap.find(cell) != pointerMap.end()) {
                        pointerMap[cell] += offset;
                    }
                    else {
                        pointerMap[cell] = offset;
                    }
                }                
            }    
        }
        if (maxptr->bucketnode_to_cell->set == 1) {
            net.num_set1 -= 1;
            net.num_set2 += 1;
        }
        else {
            net.num_set1 += 1;
            net.num_set2 -= 1;
        }
    } 
}

    
void update_bucketlist(unordered_map<Node*, int> pointerMap, int& max_pin) {
    if (maxptr->bucketnode_to_cell->set == 1) {
        bucketListSetA[maxptr->bucketnode_to_cell->gain + max_pin].delete_cell(maxptr);
    }
    else {
        bucketListSetB[maxptr->bucketnode_to_cell->gain + max_pin].delete_cell(maxptr);
    }
    for (auto be_updated: pointerMap) {
        if (!be_updated.first->cell_to_bucketnode->prev) {
            cout << be_updated.first->locked << endl;
            cout<< "Crash" << endl;
        }
        if (be_updated.first->gain + max_pin > 18 || be_updated.first->gain + max_pin < 0) {
            cout << be_updated.first->gain + max_pin << endl;
            cout << "Out of bound" << endl;
        }
        if (!be_updated.first->cell_to_bucketnode) {
            cout << "NULL" << endl;
        }
        if (be_updated.second != 0) {
            if (be_updated.first->set == 1) {
                bucketListSetA[be_updated.first->gain + max_pin].delete_cell(be_updated.first->cell_to_bucketnode);
                bucketListSetA[be_updated.first->gain + be_updated.second + max_pin].insert(be_updated.first->cell_to_bucketnode);
            }
            else {
                bucketListSetB[be_updated.first->gain + max_pin].delete_cell(be_updated.first->cell_to_bucketnode);
                bucketListSetB[be_updated.first->gain + be_updated.second + max_pin].insert(be_updated.first->cell_to_bucketnode);
            }
            be_updated.first->gain += be_updated.second;
        }
    }
}

void final (int& size_a, int& szie_b) {
    if (maxptr->bucketnode_to_cell->set == 1) {
        size_a = size_a - maxptr->bucketnode_to_cell->size;
        szie_b = szie_b + maxptr->bucketnode_to_cell->size;    
    }
    else {
        size_a = size_a + maxptr->bucketnode_to_cell->size;
        szie_b = szie_b - maxptr->bucketnode_to_cell->size;
    }
    maxptr->bucketnode_to_cell->locked = true;
}

void cal_cutsize(int& cutsize) {
    for (const Net& net: nets) {
        if (net.num_set1!= 0 && net.num_set2!=0) {
            cutsize += 1;
        }
    }
}

void record(int& Gk, int& max_Gk, vector<Node*>& history) {
    Gk += maxptr->bucketnode_to_cell->gain;
    if (Gk > max_Gk) {
        max_Gk = Gk;
    }
    history.push_back(maxptr->bucketnode_to_cell);
}

void reconstruct_bucketlist(int& maxpin) {
    for (auto node: nodes) {
        if (node.set == 1) {
            bucketListSetA[node.gain + maxpin].insert(node.cell_to_bucketnode);
        }
        else {
            bucketListSetB[node.gain + maxpin].insert(node.cell_to_bucketnode);
        }
    }
}

void start(vector<Node*>& a, vector<Node*>& b, vector<Node*>& history, int total_size, int max_size, int& size_a, int& size_b, int& max_pin, int& init_cutsize ,int& max_Gk, int& Gk, int& index) {
    unordered_map<Node*, int> pointerMap;
    int set1_maxgain, set2_maxgain;
    float lower = 0.0;
    float upper = 0.0;
    lower = 0.4 * total_size - max_size;
    upper = 0.4 * total_size + max_size;
    
    if (index == 0) {
        build_bucketlist(max_pin);
    }

    else {
        init_bucketlist(max_pin);
        cout << "Initialize Successfully!" << endl;
        reconstruct_bucketlist(max_pin);
        cout << "Reconstruct Successfully!" << endl;
    }
    
    while(1) {
        find_maxgain(set1_maxgain, set2_maxgain, max_pin);
        findbase(lower, upper, size_a, set1_maxgain, set2_maxgain, max_pin);
        
        if (!maxptr) {
            int after_cutszie=0;
            cout << "Wow!" << endl;
            cout << "Initial Cut_size: " << init_cutsize << endl;
            cal_cutsize(after_cutszie);
            cout << "After Cut_size: " << after_cutszie << endl;
            cout << "MAX_Gk: " << max_Gk << endl;
            cout << "Gk: " << Gk << endl;
            break;
        }
        record(Gk, max_Gk, history);
        update_gain(pointerMap);
        update_bucketlist(pointerMap, max_pin);
        final(size_a, size_b);
        pointerMap.clear();

        if (Gk <= 0) {
            break;
        }
    }
}

int main(int argc, char** argv){
    ifstream nodefile(argv[1]);
    ifstream netfile(argv[2]);

    if (!nodefile.is_open()) {
        cerr << "Error: Could not open the .nodes file." << endl;
        return 1;
    }
    if (!netfile.is_open()) {
        cerr << "Error: Could not open the .nets file." << endl;
        return 1;
    }
    
    /* read nodes */
    cout << "Reading nodes ..." << endl;
    int max_size = 0;
    int total_size = 0;
    read_node(nodefile, total_size, max_size);
    nodefile.close();
    // for (auto node:nodes) {
    //     cout << node.name << " " << node.size <<endl;
    // }

    /* read nets */
    cout << "Reading nets ..." << endl;
    read_net(netfile);
    netfile.close();
    

    clock_t  t = clock();
    /* partition */
    cout << "Partition ..." << endl;
    vector<Node*> Set1;
    vector<Node*> Set2;
    vector<Node*> history;
    int init_cutsize = 0;
    int cumulativeSizeSet_a = 0;
    int cumulativeSizeSet_b = 0;
    partition(cumulativeSizeSet_a, nodes, Set1, Set2, total_size);
    cal_cutsize(init_cutsize);
    cumulativeSizeSet_b = total_size - cumulativeSizeSet_a;
    for (const Net& net: nets) {
        net_NumSet n;
        n.name = net.net_id;
        n.num_set1 = net.num_set1;
        n.num_set2 = net.num_set2;
        net_numset.push_back(n);
    }


    /* initial gain */
    cout << "Initial gain ..." << endl;
    int max_pin = 0;
    initial_gain(max_pin);

    
    /* Start FM*/
    cout << "Fucking ..." << endl;
    // start(Set1, Set2, history, total_size, max_size, cumulativeSizeSet_a, cumulativeSizeSet_b, max_pin, init_cutsize, max_Gk, Gk, i);
    int max_Gk = 0;
    int Gk = 0;
    for (int k = 0; k < stoi(argv[4]); k++) {
        max_Gk = 0;
        Gk = 0;

        if (k > 0) {
            for (auto node: history) {
                node->locked = false;
                if (node->set == 1) {
                    node->set = 2;
                }
                else {
                    node->set = 1;
                }
            }
            for (auto net: nets) {
                net.num_set1 = 0;
                net.num_set2 = 0;
                for (auto cell: net.cells) {
                    if (cell->set == 1) {
                        net.num_set1 += 1;
                    }
                    else {
                        net.num_set2 += 1;
                    }
                }
            }
            init_cutsize = 0;
            cal_cutsize(init_cutsize);
            net_numset.clear();
            for (const Net& net: nets) {
                net_NumSet n;
                n.name = net.net_id;
                n.num_set1 = net.num_set1;
                n.num_set2 = net.num_set2;
                net_numset.push_back(n);
                for (auto cell: net.cells) {
                    cell->gain = 0;
                }
            }
            cumulativeSizeSet_a = 0;
            for (auto node: nodes) {
                if (node.set == 1) {
                    cumulativeSizeSet_a += node.size;
                }
            }
            initial_gain(max_pin); 
            history.clear();       
        }
        start(Set1, Set2, history, total_size, max_size, cumulativeSizeSet_a, cumulativeSizeSet_b, max_pin, init_cutsize, max_Gk, Gk, k);
    }
    t = clock() - t;
    


    /* Output */
    cout << "Output file ..." << endl;
    int gk = 0;
    for (Node* node: history) {
        gk += node->gain;
        
        if (gk <= max_Gk) {
            for (auto idx: node->netlist) {
                net_NumSet& net = net_numset[idx];
                if (node->set == 1) {
                    net.num_set1 -= 1;
                    net.num_set2 += 1;
                }
                else {
                    net.num_set1 += 1;
                    net.num_set2 -= 1;
                }
            }

            if (node->set == 1) {
                node->set = 2;
            }
            else {
                node->set = 1;
            }
        }
        if (gk == max_Gk) {
            cout << "Ya" << endl;
            break;
        }
    }

    int G1_size = 0;
    int G2_size = 0;
    for (auto node: nodes) {
        if (node.set == 1) {
            G1_size += node.size;
        }
        else {
            G2_size += node.size;
        }
    }
    

    ofstream output;
    output.open(argv[3]);
    output << "Cut_size: " << init_cutsize - max_Gk << endl;
    output << endl;
    output << "G1_size: " << G1_size << endl;
    output << endl;
    output << "G2_size: " << G2_size << endl;
    output << endl;
    output << "Time: " << t/CLOCKS_PER_SEC << " s" << endl;
    output << endl;

    output << "G1:" << endl;
    output << endl;
    vector<Node> G1;
    vector<Node> G2;
    for (auto node: nodes) {
        if (node.set == 1) {
            G1.push_back(node);
        }
        else {
            G2.push_back(node);
        }
    }
    for (int i = 0; i < G1.size(); i++) {
        if (i == G1.size() - 1) {
            output << G1[i].name << ";";
        }
        else {
            output << G1[i].name << " ";
        }
    }
    output << endl;
    output << "G2:" << endl;
    output << endl;
    for (int i = 0; i < G2.size(); i++) {
        if (i == G2.size() - 1) {
            output << G2[i].name << ";";
        }
        else {
            output << G2[i].name << " ";
        }
    }
    output << endl;
    output << endl;
    output << "Cut_set:" << endl;
    output << endl;
    for (int i = 0; i < net_numset.size(); i++) {
        if (net_numset[i].num_set1!=0 && net_numset[i].num_set2!=0) {
            if (i == net_numset.size() - 1) {
                output << net_numset[i].name << ";";
            }
            else {
                output << net_numset[i].name << " ";
            }
        }
    }
    output.close();
    
    cout << "Finish" << endl;
}