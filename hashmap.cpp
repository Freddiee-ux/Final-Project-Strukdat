#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <random>
#include <fstream>
#include <chrono>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
using namespace std;

struct KTP {
    int ID;
    string Nama;
    string TanggalLahir;
};

vector<KTP> readKTPDataFromCSV(const string& filename) {
    vector<KTP> result;
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Gagal membuka file " << filename << endl;
        return result;
    }

    string line;
    getline(file, line); // Lewati header

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

unordered_map<int, KTP> buildKTPHashMap(const vector<KTP>& dataKTP) {
    unordered_map<int, KTP> hashMap;
    for (const auto& ktp : dataKTP) {
        hashMap[ktp.ID] = ktp;
    }
    return hashMap;
}

long long searchByID(const unordered_map<int, KTP>& hashMap, int id) {
    auto start = chrono::high_resolution_clock::now();
    auto it = hashMap.find(id);
    auto end = chrono::high_resolution_clock::now();

    if (it != hashMap.end()) {
        // Uncomment ini jika ingin tampilkan data
        // cout << "ID: " << it->second.ID << ", Nama: " << it->second.Nama << ", Tanggal Lahir: " << it->second.TanggalLahir << "\n";
    }

    return chrono::duration_cast<chrono::nanoseconds>(end - start).count();
}

long long searchByRange(const unordered_map<int, KTP>& hashMap, int minID, int maxID, vector<KTP>& found) {
    auto start = chrono::high_resolution_clock::now();
    for (int id = minID; id <= maxID; ++id) {
        auto it = hashMap.find(id);
        if (it != hashMap.end()) {
            found.push_back(it->second);
        }
    }
    auto end = chrono::high_resolution_clock::now();
    return chrono::duration_cast<chrono::nanoseconds>(end - start).count();
}

int main() {
    vector<KTP> data = readKTPDataFromCSV("KTPData.csv");
    cout << "Sukses membaca " << data.size() << " data KTP dari file KTPData.csv\n";

    unordered_map<int, KTP> hashMap = buildKTPHashMap(data);

    // Ambil 100 ID acak untuk pengujian waktu pencarian individual
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(0, data.size() - 1);

    long long totalWaktuIndividu = 0;
    cout << "\n🔍 Melakukan pencarian terhadap 100 ID secara acak...\n";

    for (int i = 0; i < 100; ++i) {
        int randomIndex = dist(gen);
        int randomID = data[randomIndex].ID;
        totalWaktuIndividu += searchByID(hashMap, randomID);
    }

    cout << "Rata-rata waktu pencarian individual (Hash Map): " << (totalWaktuIndividu / 100.0) << " nanodetik\n";

    // Pencarian rentang
    int minRange = 250;
    int maxRange = 300;
    vector<KTP> hasilRange;
    long long waktuRange = searchByRange(hashMap, minRange, maxRange, hasilRange);

    cout << "\n📦 Pencarian data dalam rentang ID [" << minRange << " - " << maxRange << "]\n";
    cout << "Jumlah ditemukan: " << hasilRange.size() << endl;
    cout << "Waktu pencarian range (Hash Map): " << waktuRange << " nanodetik\n";
    cout << "Rata-rata waktu per item ditemukan: " 
         << (hasilRange.empty() ? 0 : (waktuRange / hasilRange.size())) << " nanodetik\n";

    return 0;
}



