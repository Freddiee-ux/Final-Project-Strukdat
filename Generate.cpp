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
    int ID;  // Ganti dari string ke int
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

string generateName(mt19937& gen) {
    uniform_int_distribution<> genderDist(0, 1);
    bool isMale = genderDist(gen);
    const vector<string>& firstNames = isMale ? maleFirstNames : femaleFirstNames;
    uniform_int_distribution<size_t> firstNameDist(0, firstNames.size() - 1);
    uniform_int_distribution<size_t> lastNameDist(0, lastNames.size() - 1);

    return firstNames[firstNameDist(gen)] + " " + lastNames[lastNameDist(gen)];
}

vector<KTP> generateKTPData(int amount, mt19937& gen) {
    vector<KTP> ktpList;
    for (int i = 0; i < amount; ++i) {
        KTP data;
        data.ID = i;  // Langsung gunakan int sebagai ID
        data.Nama = generateName(gen);
        data.TanggalLahir = generateBirthDate(gen);
        ktpList.push_back(data);
    }
    shuffle(ktpList.begin(), ktpList.end(), gen); // Acak urutan
    return ktpList;
}

int main(){
    random_device rd;
    mt19937 gen(rd());

    int jumlahData;
    cout << "Masukkan jumlah data KTP yang ingin dibuat: ";
    cin >> jumlahData;

    // Generate dan simpan ke file
    vector<KTP> dataKTP = generateKTPData(jumlahData, gen);
    string filename = "KTPData.csv";
    ofstream file(filename);
    file << "ID,Nama,Tanggal Lahir\n";
    for (const auto& d : dataKTP)
        file << d.ID << "," << d.Nama << "," << d.TanggalLahir << "\n";
    file.close();

    cout << "Data berhasil disimpan ke " << filename << "!\n";
    return 0;
}
