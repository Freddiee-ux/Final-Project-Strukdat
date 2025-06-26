#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <random>
using namespace std;

struct KTP {
    int ID;
    string Nama;
    string TanggalLahir;
};

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
        for (size_t i = 0; i < node->keys.size(); ++i) {
            // Traverse left child
            searchRangeBPTree(node->children[i], startID, endID, result);

            // Check current key
            if (node->keys[i] >= startID && node->keys[i] <= endID) {
                result.push_back(node->data[i]);
            }
        }
        // Traverse last child
        searchRangeBPTree(node->children.back(), startID, endID, result);
    }

    auto end = chrono::high_resolution_clock::now();
    return chrono::duration_cast<chrono::nanoseconds>(end - start).count();
}


vector<KTP> readKTPData(const string& filename, int limit) {
    vector<KTP> data;
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Gagal membuka file " << filename << endl;
        return data;
    }
    string line;
    getline(file, line); // skip header
    while (getline(file, line) && data.size() < limit) {
        stringstream ss(line);
        string idStr, nama, tanggal;
        if (getline(ss, idStr, ',') && getline(ss, nama, ',') && getline(ss, tanggal)) {
            try {
                data.push_back({stoi(idStr), nama, tanggal});
            } catch (...) {
                cerr << "Format ID tidak valid: " << idStr << endl;
            }
        }
    }
    return data;
}

int main() {
    string filename = "KTPData.csv";
    int jumlahData = 100000;
    vector<KTP> data = readKTPData(filename, jumlahData);
    cout << "Sukses membaca " << data.size() << " data dari " << filename << endl;

    createBPTree(20);

    // 1. Penyisipan
    auto startInsert = chrono::high_resolution_clock::now();
    for (const auto& d : data) insert(d);
    auto endInsert = chrono::high_resolution_clock::now();
    long long waktuInsert = chrono::duration_cast<chrono::nanoseconds>(endInsert - startInsert).count();
    cout << "\n1. Waktu penyisipan (B+ Tree): " << waktuInsert << " nanodetik\n";

    // 2. Pencarian ID tunggal
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(0, data.size() - 1);
    long long totalTime = 0;
    for (int i = 0; i < 100; ++i) {
        int randomID = data[dist(gen)].ID;
        KTP* found = nullptr;
        totalTime += searchBPTree(randomID, found);
    }
    cout << "2. Rata-rata waktu pencarian 100 ID acak: " << (totalTime / 100.0) << " nanodetik\n";

    // 3. Pencarian rentang ID dari cin
    int startID, endID;
    cout << "3. Masukkan rentang ID yang ingin dicari:\n   Start ID: ";
    cin >> startID;
    cout << "   End ID: ";
    cin >> endID;

    vector<KTP> hasilRange;
    long long rangeTime = searchRangeBPTree(root, startID, endID, hasilRange);
    cout << "   Waktu pencarian rentang ID [" << startID << " - " << endID << "]: " << rangeTime << " nanodetik\n";
    cout << "   Jumlah data ditemukan: " << hasilRange.size() << "\n";
    cout << "   Rata-rata waktu per item ditemukan: "
         << (hasilRange.empty() ? 0 : rangeTime / hasilRange.size()) << " nanodetik\n";

    return 0;
}
