#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <random>
#include <map>
#include <fstream>
#include <chrono>
#include <unordered_map>
#include <unordered_set>
using namespace std;

struct KTP {
    string NIK;
    string Nama;
    string TanggalLahir;
};

const vector<string> maleFirstNames = {
    "Ahmad", "Budi", "Cahyo", "Dedi", "Eko", "Fajar", "Gunawan", "Hadi", "Irfan", "Joko",
    "Kurniawan", "Lukman", "Mulyono", "Nugroho", "Oki", "Purnomo", "Rudi", "Surya", "Teguh", "Wahyu"
};

const vector<string> femaleFirstNames = {
    "Ani", "Bunga", "Citra", "Dewi", "Eka", "Fitri", "Gita", "Hani", "Intan", "Juli",
    "Kartika", "Lina", "Maya", "Nur", "Oki", "Putri", "Rini", "Sari", "Tuti", "Wulan"
};

const vector<string> lastNames = {
    "Santoso", "Wijaya", "Prabowo", "Hidayat", "Siregar", "Tanuwijaya", "Kusuma", "Setiawan",
    "Pratama", "Halim", "Susanto", "Wibowo", "Siregar", "Gunawan", "Saputra", "Irawan"
};

string generateBirthDate(mt19937& gen) {
    time_t now = time(nullptr);
    tm currentTime = *localtime(&now);

    uniform_int_distribution<> ageDist(17, 70);
    uniform_int_distribution<> dayDist(1, 28);
    uniform_int_distribution<> monthDist(1, 12);

    int age = ageDist(gen);
    int day = dayDist(gen);
    int month = monthDist(gen);
    int year = currentTime.tm_year + 1900 - age;

    ostringstream oss;
    oss << setw(2) << setfill('0') << day << "-"
        << setw(2) << setfill('0') << month << "-"
        << year;

    return oss.str();
}

string generateNIKFromDate(const string& birthDate, mt19937& gen, int provinceCode = 32, int regencyCode = 73) {
    int day = stoi(birthDate.substr(0, 2));
    int month = stoi(birthDate.substr(3, 2));
    int year = stoi(birthDate.substr(6, 4)) % 100;

    uniform_int_distribution<> randNum(1, 9999);
    int randomNum = randNum(gen);

    ostringstream oss;
    oss << setw(2) << setfill('0') << provinceCode
        << setw(2) << setfill('0') << regencyCode
        << setw(2) << setfill('0') << day
        << setw(2) << setfill('0') << month
        << setw(2) << setfill('0') << year
        << setw(4) << setfill('0') << randomNum;

    return oss.str();
}

string generateName(mt19937& gen) {
    uniform_int_distribution<> genderDist(0, 1);
    bool isMale = genderDist(gen);

    const vector<string>& firstNames = isMale ? maleFirstNames : femaleFirstNames;
    uniform_int_distribution<size_t> firstNameDist(0, firstNames.size() - 1);
    uniform_int_distribution<size_t> lastNameDist(0, lastNames.size() - 1);

    string firstName = firstNames[firstNameDist(gen)];
    string lastName = lastNames[lastNameDist(gen)];

    return firstName + " " + lastName;
}

vector<KTP> generateKTPData(int amount, int provinceCode, int regencyCode, mt19937& gen) {
    if (amount > 1000) {
        amount = 1000;
        cout << "Warning: Maximum 1000 data per call. Generating 1000 data." << endl;
    }

    vector<KTP> ktpList;
    unordered_set<string> usedNIK; 

    while (ktpList.size() < amount) {
        KTP data;
        data.TanggalLahir = generateBirthDate(gen);
        data.NIK = generateNIKFromDate(data.TanggalLahir, gen, provinceCode, regencyCode);

        if (usedNIK.count(data.NIK) > 0) {
            continue;
        }

        data.Nama = generateName(gen);
        ktpList.push_back(data);
        usedNIK.insert(data.NIK);
    }

    return ktpList;
}

struct BPTreeNode {
    bool isLeaf;
    vector<string> keys;
    vector<BPTreeNode*> children;
    vector<KTP> data;

    BPTreeNode(bool leaf = true) : isLeaf(leaf) {}
};

class BPTree {
private:
    BPTreeNode* root;
    int t; // Minimum degree

    void splitChild(BPTreeNode* parent, int index, BPTreeNode* child) {
        BPTreeNode* newChild = new BPTreeNode(child->isLeaf);
        newChild->keys.resize(t - 1);
        newChild->data.resize(t - 1);

        for (int i = 0; i < t - 1; i++) {
            newChild->keys[i] = child->keys[i + t];
            newChild->data[i] = child->data[i + t];
        }

        if (!child->isLeaf) {
            newChild->children.resize(t);
            for (int i = 0; i < t; i++) {
                newChild->children[i] = child->children[i + t];
            }
        }

        parent->children.insert(parent->children.begin() + index + 1, newChild);
        parent->keys.insert(parent->keys.begin() + index, child->keys[t - 1]);
        parent->data.insert(parent->data.begin() + index, child->data[t - 1]);

        child->keys.resize(t - 1);
        child->data.resize(t - 1);
    }
    void insertNonFull(BPTreeNode* node, const KTP& ktp) {
        int i = node->keys.size() - 1;

        if (node->isLeaf) {
            while (i >= 0 && ktp.NIK < node->keys[i]) {
                i--;
            }
            node->keys.insert(node->keys.begin() + i + 1, ktp.NIK);
            node->data.insert(node->data.begin() + i + 1, ktp);
        } else {
            while (i >= 0 && ktp.NIK < node->keys[i]) {
                i--;
            }
            i++;
            if (node->children[i]->keys.size() == 2 * t - 1) {
                splitChild(node, i, node->children[i]);
                if (ktp.NIK > node->keys[i]) {
                    i++;
                }
            }
            insertNonFull(node->children[i], ktp);
        }
    }
public:
    BPTree(int degree) : t(degree) {
        root = new BPTreeNode();
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
    void traverse(BPTreeNode* node) {
        if (node == nullptr) return;

        for (size_t i = 0; i < node->keys.size(); i++) {
            if (!node->isLeaf) {
                traverse(node->children[i]);
            }
            cout << "NIK: " << node->data[i].NIK << ", Nama: " << node->data[i].Nama
                 << ", Tanggal Lahir: " << node->data[i].TanggalLahir << endl;
        }
        if (!node->isLeaf) {
            traverse(node->children[node->keys.size()]);
        }
    }
    void display() {
        traverse(root);
    }
};


int main() {
    random_device rd;
    mt19937 gen(rd());

    vector<KTP> data = generateKTPData(1000, 32, 73, gen);

    string filename = "KTPData.csv";
    ofstream file(filename);
    file << "NIK,Nama,Tanggal Lahir\n";
    for (const auto& d : data)
        file << d.NIK << "," << d.Nama << "," << d.TanggalLahir << "\n";
    file.close();

    cout << "Sukses menyimpan 1000 data KTP ke file " << filename << endl;

    BPTree tree(3); 
    for (const auto& d : data) {
        tree.insert(d);
    }
    cout << "Data dalam B+ Tree:\n";
    tree.display();
    cout << "Selesai!" << endl;



    return 0;
}
