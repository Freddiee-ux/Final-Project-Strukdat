#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <random>
#include <fstream>
#include <chrono>
#include <unordered_set>
#include <algorithm>
using namespace std;

struct KTP {
    int ID;
    string Nama;
    string TanggalLahir;
};

// Baca file CSV
vector<KTP> readKTPDataFromCSV(const string& filename) {
    vector<KTP> result;
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Gagal membuka file " << filename << endl;
        return result;
    }

    string line;
    getline(file, line); // skip header

    while (getline(file, line)) {
        stringstream ss(line);
        string idStr, nama, tanggal;
        if (getline(ss, idStr, ',') && getline(ss, nama, ',') && getline(ss, tanggal)) {
            try {
                int id = stoi(idStr);
                result.push_back({id, nama, tanggal});
            } catch (...) {
                cerr << "Format ID tidak valid: " << idStr << endl;
            }
        }
    }

    file.close();
    return result;
}

// B+ Tree Node
struct BPTreeNode {
    bool isLeaf;
    vector<int> keys;
    vector<BPTreeNode*> children;
    vector<KTP> data;
    BPTreeNode(bool leaf = true) : isLeaf(leaf) {}
};

BPTreeNode* root = nullptr;
int t;

void splitChild(BPTreeNode* parent, int index, BPTreeNode* child) {
    BPTreeNode* newChild = new BPTreeNode(child->isLeaf);
    newChild->keys.assign(child->keys.begin() + t, child->keys.end());
    newChild->data.assign(child->data.begin() + t, child->data.end());

    if (!child->isLeaf) {
        newChild->children.assign(child->children.begin() + t, child->children.end());
    }

    parent->children.insert(parent->children.begin() + index + 1, newChild);
    parent->keys.insert(parent->keys.begin() + index, child->keys[t - 1]);
    parent->data.insert(parent->data.begin() + index, child->data[t - 1]);

    child->keys.resize(t - 1);
    child->data.resize(t - 1);
    if (!child->isLeaf) child->children.resize(t);
}

void insertNonFull(BPTreeNode* node, const KTP& ktp) {
    int i = node->keys.size() - 1;

    if (node->isLeaf) {
        while (i >= 0 && ktp.ID < node->keys[i]) i--;
        node->keys.insert(node->keys.begin() + i + 1, ktp.ID);
        node->data.insert(node->data.begin() + i + 1, ktp);
    } else {
        while (i >= 0 && ktp.ID < node->keys[i]) i--;
        i++;
        if (node->children[i]->keys.size() == 2 * t - 1) {
            splitChild(node, i, node->children[i]);
            if (ktp.ID > node->keys[i]) i++;
        }
        insertNonFull(node->children[i], ktp);
    }
}

void createBPTree(int degree) {
    t = degree;
    root = new BPTreeNode(true);
}

void insert(const KTP& ktp) {
    if (root->keys.size() == 2 * t - 1) {
        BPTreeNode* newRoot = new BPTreeNode(false);
        newRoot->children.push_back(root);
        splitChild(newRoot, 0, root);
        root = newRoot;
    }
    insertNonFull(root, ktp);
}

long long searchBPTree(int id, KTP*& found) {
    auto start = chrono::high_resolution_clock::now();

    BPTreeNode* current = root;
    while (current != nullptr) {
        int i = 0;
        while (i < current->keys.size() && id > current->keys[i]) i++;
        if (i < current->keys.size() && id == current->keys[i]) {
            found = &current->data[i];
            auto end = chrono::high_resolution_clock::now();
            return chrono::duration_cast<chrono::nanoseconds>(end - start).count();
        }
        if (current->isLeaf) break;
        current = current->children[i];
    }

    found = nullptr;
    auto end = chrono::high_resolution_clock::now();
    return chrono::duration_cast<chrono::nanoseconds>(end - start).count();
}

long long searchRangeBPTree(BPTreeNode* node, int startID, int endID, vector<KTP>& result) {
    auto start = chrono::high_resolution_clock::now();
    if (!node) return 0;

    if (node->isLeaf) {
        for (size_t i = 0; i < node->keys.size(); ++i) {
            if (node->keys[i] >= startID && node->keys[i] <= endID) {
                result.push_back(node->data[i]);
            }
        }
    } else {
        for (size_t i = 0; i <= node->keys.size(); ++i) {
            searchRangeBPTree(node->children[i], startID, endID, result);
        }
    }
    auto end = chrono::high_resolution_clock::now();
    return chrono::duration_cast<chrono::nanoseconds>(end - start).count();
}

int main() {
    vector<KTP> data = readKTPDataFromCSV("KTPData.csv");
    cout << "Sukses membaca " << data.size() << " data KTP dari file KTPData.csv\n";

    createBPTree(3);
    for (const auto& d : data) insert(d);

    // 100 pencarian acak
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(0, data.size() - 1);
    long long totalTime = 0;
    for (int i = 0; i < 100; ++i) {
        int randomIndex = dist(gen);
        int randomID = data[randomIndex].ID;
        KTP* found = nullptr;
        totalTime += searchBPTree(randomID, found);
    }
    cout << "\nRata-rata waktu pencarian 100 ID acak (B+ Tree): " << (totalTime / 100.0) << " nanodetik\n";

    // Pencarian rentang ID
    int startID = 250, endID = 300;
    vector<KTP> hasilRange;
    long long rangeTime = searchRangeBPTree(root, startID, endID, hasilRange);
    cout << "\nPencarian rentang ID [" << startID << " - " << endID << "]\n";
    cout << "Jumlah data ditemukan: " << hasilRange.size() << endl;
    cout << "Waktu pencarian rentang (B+ Tree): " << rangeTime << " nanodetik\n";
    cout << "Rata-rata waktu per item ditemukan: "
         << (hasilRange.empty() ? 0 : rangeTime / hasilRange.size()) << " nanodetik\n";

    return 0;
}
