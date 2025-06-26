#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <chrono>
#include <random>
using namespace std;

struct KTP {
    int ID;
    string Nama;
    string TanggalLahir;
};

vector<KTP> readKTPData(const string& filename, int limit) {
    vector<KTP> data;
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Gagal membuka file " << filename << endl;
        return data;
    }
    string line;
    getline(file, line); // Skip header
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

unordered_map<int, KTP> buildHashMap(const vector<KTP>& data) {
    unordered_map<int, KTP> map;
    for (const auto& ktp : data) {
        map[ktp.ID] = ktp;
    }
    return map;
}

long long searchHashMap(const unordered_map<int, KTP>& map, int id) {
    auto start = chrono::high_resolution_clock::now();
    auto it = map.find(id);
    auto end = chrono::high_resolution_clock::now();
    return chrono::duration_cast<chrono::nanoseconds>(end - start).count();
}

long long searchRangeHashMap(const unordered_map<int, KTP>& map, int startID, int endID, vector<KTP>& result) {
    auto start = chrono::high_resolution_clock::now();
    for (int id = startID; id <= endID; ++id) {
        auto it = map.find(id);
        if (it != map.end()) {
            result.push_back(it->second);
        }
    }
    auto end = chrono::high_resolution_clock::now();
    return chrono::duration_cast<chrono::nanoseconds>(end - start).count();
}

int main() {
    string filename = "KTPData.csv";
    int jumlahData = 100000;
    vector<KTP> data = readKTPData(filename, jumlahData);
    cout << "Sukses membaca " << data.size() << " data dari " << filename << endl;

    // 1. Penyisipan ke hashmap
    auto startInsert = chrono::high_resolution_clock::now();
    unordered_map<int, KTP> map = buildHashMap(data);
    auto endInsert = chrono::high_resolution_clock::now();
    long long waktuInsert = chrono::duration_cast<chrono::nanoseconds>(endInsert - startInsert).count();
    cout << "\n1. Waktu penyisipan (Hash Map): " << waktuInsert << " nanodetik\n";

    // 2. Pencarian ID tunggal (acak 100x)
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(0, data.size() - 1);
    long long totalTime = 0;
    for (int i = 0; i < 100; ++i) {
        int randomID = data[dist(gen)].ID;
        totalTime += searchHashMap(map, randomID);
    }
    cout << "2. Rata-rata waktu pencarian 100 ID acak: " << (totalTime / 100.0) << " nanodetik\n";

    // 3. Pencarian rentang ID berdasarkan input
    int startID, endID;
    cout << "3. Masukkan rentang ID yang ingin dicari:\n   Start ID: ";
    cin >> startID;
    cout << "   End ID: ";
    cin >> endID;

    vector<KTP> hasilRange;
    long long rangeTime = searchRangeHashMap(map, startID, endID, hasilRange);
    cout << "   Waktu pencarian rentang ID [" << startID << " - " << endID << "]: " << rangeTime << " nanodetik\n";
    cout << "   Jumlah data ditemukan: " << hasilRange.size() << "\n";
    cout << "   Rata-rata waktu per item ditemukan: "
         << (hasilRange.empty() ? 0 : rangeTime / hasilRange.size()) << " nanodetik\n";

    return 0;
}
