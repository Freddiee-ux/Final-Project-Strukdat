<h1> Laporan Final Project Struktur Data </h1>

<h2> Generate Data</h2>

```c
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

string generateNIKFromDate(const string& birthDate, int count, int provinceCode = 32, int regencyCode = 73) {
    int day = stoi(birthDate.substr(0, 2));
    int month = stoi(birthDate.substr(3, 2));
    int year = stoi(birthDate.substr(6, 4)) % 100;

    ostringstream oss;
    oss << setw(2) << setfill('0') << provinceCode
        << setw(2) << setfill('0') << regencyCode
        << setw(2) << setfill('0') << day
        << setw(2) << setfill('0') << month
        << setw(2) << setfill('0') << year
        << setw(4) << setfill('0') << count;

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
    int i = 0;
    while (ktpList.size() < amount) {
        KTP data;
        data.TanggalLahir = generateBirthDate(gen);
        data.NIK = generateNIKFromDate(data.TanggalLahir, i++, provinceCode, regencyCode);

        if (usedNIK.count(data.NIK) > 0) {
            continue;
        }

        data.Nama = generateName(gen);
        ktpList.push_back(data);
        usedNIK.insert(data.NIK);
    }

    return ktpList;
}
```
    
<h2> B+tree</h2>

```c
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

    // buat distribusi untuk umur, hari, dan bulan 
    // digunakan untuk merandomkan tanggal lahir
    uniform_int_distribution<> ageDist(17, 70);
    uniform_int_distribution<> dayDist(1, 28);
    uniform_int_distribution<> monthDist(1, 12);

    // generate umur, hari, bulan menggunakan distribusi
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

string generateNIKFromDate(const string& birthDate, int count, int provinceCode = 32, int regencyCode = 73) {
    int day = stoi(birthDate.substr(0, 2));
    int month = stoi(birthDate.substr(3, 2));
    int year = stoi(birthDate.substr(6, 4)) % 100;

    ostringstream oss;
    oss << setw(2) << setfill('0') << provinceCode
        << setw(2) << setfill('0') << regencyCode
        << setw(2) << setfill('0') << day
        << setw(2) << setfill('0') << month
        << setw(2) << setfill('0') << year
        << setw(4) << setfill('0') << count;

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
    int i = 0;
    while (ktpList.size() < amount) {
        KTP data;
        data.TanggalLahir = generateBirthDate(gen);
        data.NIK = generateNIKFromDate(data.TanggalLahir, i++, provinceCode, regencyCode);

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


BPTreeNode* root = nullptr; // Root of the B+ tree
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

KTP* search(const string& nik) {
    BPTreeNode* current = root;

    while (current != nullptr) {
        int i = 0;
        while (i < current->keys.size() && nik > current->keys[i]) i++;

        if (i < current->keys.size() && nik == current->keys[i])
            return &current->data[i];

        if (current->isLeaf) return nullptr;
        current = current->children[i];
    }

    return nullptr;
}

void searchRangeInBPTree(BPTreeNode* node, const string& startNIK, const string& endNIK, vector<KTP>& result) {
    if (!node) return;

    size_t i = 0;

    // Untuk leaf node: scan data di range
    if (node->isLeaf) {
        while (i < node->keys.size()) {
            if (node->keys[i] >= startNIK && node->keys[i] <= endNIK) {
                result.push_back(node->data[i]);
            }
            ++i;
        }
    } else {
        // Untuk internal node, lanjutkan ke child yang sesuai
        while (i < node->keys.size() && startNIK > node->keys[i]) {
            ++i;
        }
        for (size_t j = i; j < node->children.size(); ++j) {
            searchRangeInBPTree(node->children[j], startNIK, endNIK, result);
        }
    }
}




int main() {
    random_device rd;
    mt19937 gen(rd());

    int jumlahData;
    cout << "Masukkan jumlah data KTP yang ingin dibuat (maks 1000): ";
    cin >> jumlahData;
    vector<KTP> data = generateKTPData(jumlahData, 32, 73, gen);

    string filename = "KTPData.csv";
    ofstream file(filename);
    file << "NIK,Nama,Tanggal Lahir\n";
    for (const auto& d : data)
        file << d.NIK << "," << d.Nama << "," << d.TanggalLahir << "\n";
    file.close();

    cout << "Sukses menyimpan " <<  jumlahData << "data KTP ke file " << filename << endl;
    createBPTree(3); 
    for (const auto& d : data) {
        insert(d);
    }
    string searchNIK;
    cout << "Masukkan NIK yang ingin dicari: ";
    cin >> searchNIK;
    KTP* result = search(searchNIK);
    if (result) {
        cout << "Data ditemukan: " << result->NIK << ", " << result->Nama << ", " << result->TanggalLahir << endl;
    } else {
        cout << "Data tidak ditemukan." << endl;
    }
    string startNIK, endNIK;
    cout << "Masukkan rentang NIK (start end): ";
    cin >> startNIK >> endNIK;
    vector<KTP> rangeResults;
    searchRangeInBPTree(root, startNIK, endNIK, rangeResults);
    if (!rangeResults.empty()) {
        cout << "Data dalam rentang NIK dari " << startNIK << " hingga" << endNIK << ":\n";
        for (const auto& d : rangeResults) {
            cout << d.NIK << ", " << d.Nama << ", " << d.TanggalLahir << endl;
        }
    } else {
        cout << "Tidak ada data dalam rentang NIK tersebut." << endl;
    }
    return 0;
    }
```

<h2> Hash Map</h2>

```c
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

string generateNIKFromDate(const string& birthDate, int count, int provinceCode = 32, int regencyCode = 73) {
    int day = stoi(birthDate.substr(0, 2));
    int month = stoi(birthDate.substr(3, 2));
    int year = stoi(birthDate.substr(6, 4)) % 100;

    ostringstream oss;
    oss << setw(2) << setfill('0') << provinceCode
        << setw(2) << setfill('0') << regencyCode
        << setw(2) << setfill('0') << day
        << setw(2) << setfill('0') << month
        << setw(2) << setfill('0') << year
        << setw(4) << setfill('0') << count;

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
    int i = 0;
    while (ktpList.size() < amount) {
        KTP data;
        data.TanggalLahir = generateBirthDate(gen);
        data.NIK = generateNIKFromDate(data.TanggalLahir, i++, provinceCode, regencyCode);

        if (usedNIK.count(data.NIK) > 0) {
            continue;
        }

        data.Nama = generateName(gen);
        ktpList.push_back(data);
        usedNIK.insert(data.NIK);
    }

    return ktpList;
}

unordered_map<string, KTP> buildKTPHashMap(const vector<KTP>& dataKTP) {
    unordered_map<string, KTP> hashMap;
    for (const auto& ktp : dataKTP) {
        hashMap[ktp.NIK] = ktp;
    }
    return hashMap;
}

void searchNIK(const unordered_map<string, KTP>& hashMap, const string& nik) {
    auto start = chrono::high_resolution_clock::now();
    auto it = hashMap.find(nik);
    auto end = chrono::high_resolution_clock::now();

    if (it != hashMap.end()) {
        cout << "NIK ditemukan: " << it->second.NIK << ", Nama: " << it->second.Nama
             << ", Tanggal Lahir: " << it->second.TanggalLahir << endl;
    } else {
        cout << "NIK tidak ditemukan." << endl;
    }

    auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
    cout << "Waktu pencarian: " << duration.count() << " mikrodetik" << endl;
}

int main() {
    random_device rd;
    mt19937 gen(rd());

    int jumlahData;
    int kodeProvinsi = 32;
    int kodeKabupaten = 73;
    cout << "Masukkan jumlah data KTP yang ingin dibuat (maks 1000): ";
    cin >> jumlahData;
    vector<KTP> dataKTP = generateKTPData(jumlahData, kodeProvinsi, kodeKabupaten, gen);
    string filename = "KTPData.csv";
    ofstream file(filename);
    file << "NIK,Nama,Tanggal Lahir\n";
    for (const auto& d : dataKTP)
        file << d.NIK << "," << d.Nama << "," << d.TanggalLahir << "\n";
    file.close();

    unordered_map<string, KTP> hashMap = buildKTPHashMap(dataKTP);

    cout << "Sukses menyimpan " <<  jumlahData << "data KTP ke file " << filename << endl;
    string nikDicari;
    cout << "Masukkan NIK yang ingin dicari: ";
    cin >> nikDicari;
    cout << "Mencari NIK: " << nikDicari << endl;
    searchNIK(hashMap, nikDicari);

    return 0;
}
```
<h1> Laporan Final Project Struktur Data </h1>

<h2> Generate Data </h2>

Program ini menghasilkan data KTP palsu dengan atribut:
- `NIK` (Nomor Induk Kependudukan)
- `Nama`
- `Tanggal Lahir`

Fungsi utama yang digunakan:
- `generateBirthDate()` – menghasilkan tanggal lahir acak.
- `generateNIKFromDate()` – membuat NIK berdasarkan tanggal lahir dan kode wilayah.
- `generateName()` – memilih nama depan dan belakang secara acak berdasarkan gender.
- `generateKTPData()` – menghasilkan sejumlah data dan memastikan NIK unik.

---

<h2> B+ Tree </h2>

B+ Tree digunakan untuk menyimpan dan mencari data berdasarkan NIK. Keunggulan struktur ini adalah pencarian **dengan NIK tunggal maupun range NIK**.

Fungsi penting:
- `createBPTree(degree)` – inisialisasi tree dengan derajat tertentu.
- `insert(data)` – menyisipkan data baru ke tree.
- `search(nik)` – mencari data berdasarkan NIK.
- `searchRangeInBPTree(root, startNIK, endNIK, result)` – mencari data dalam rentang NIK.

B+ Tree cocok digunakan jika:
- Data perlu tersortir otomatis.
- Dibutuhkan pencarian dalam range NIK.

---

<h2> Hash Map </h2>

Hash Map (`unordered_map`) digunakan untuk menyimpan data KTP dengan key berupa NIK. Hash Map unggul dalam pencarian data tunggal karena waktu aksesnya rata-rata `O(1)`.

Fungsi penting:
- `buildKTPHashMap(data)` – membangun hash map dari vector data.
- `searchNIK(hashMap, nik)` – mencari data berdasarkan NIK dan mencatat waktu pencarian.

Hash Map cocok digunakan jika:
- Pencarian hanya dilakukan untuk **1 NIK**.
- Kecepatan akses menjadi prioritas utama.

---

<h2>Perbedaan: B+ Tree vs Hash Map</h2>

| Aspek                         | B+ Tree                                                                 | Hash Map                                                               |
|------------------------------|-------------------------------------------------------------------------|------------------------------------------------------------------------|
| **Struktur**                 | Tree terurut dengan data disimpan di leaf node                         | Tabel hash yang menggunakan fungsi hash untuk menyimpan data          |
| **Kunci Penyimpanan**        | Disimpan secara urut (sorting NIK)                                      | Disimpan acak berdasarkan hasil hash                                  |
| **Waktu Akses (Average)**    | O(log n) – pencarian berjenjang dari root ke leaf                      | O(1) – akses langsung via key hash                                    |
| **Waktu Akses (Worst-case)** | Bisa memburuk jika tree tidak seimbang                                 | Bisa memburuk menjadi O(n) saat banyak collision                      |
| **Pencarian Range**          | Mendukung pencarian rentang NIK (range query)                          | Tidak mendukung secara langsung                                       |
| **Traversal**                | Mudah: semua leaf terhubung via pointer → efisien untuk iterasi        | Sulit, harus iterasi seluruh bucket                                   |
| **Kelebihan**                | Terstruktur, mendukung query kompleks, cocok untuk database             | Simpel, sangat cepat untuk pencarian satu data                        |
| **Kekurangan**               | Lebih kompleks untuk diimplementasikan                                 | Tidak cocok untuk data terurut atau range query                       |
| **Kebutuhan Memori**         | Lebih hemat untuk penyimpanan terstruktur                              | Dapat boros memori jika ukuran hash table tidak optimal               |
| **Stabil di Data Besar**     | Ya – struktur tree menjaga performa stabil                             | Perlu tuning kapasitas atau rehashing saat data sangat besar          |

---

<h2>Contoh Penggunaan Nyata</h2>

| Skema Penggunaan                      | B+ Tree                                        | Hash Map                                 |
|--------------------------------------|------------------------------------------------|------------------------------------------|
| Cari satu NIK                        | Cukup cepat                                    | Sangat cepat                             |
| Cari NIK 3273010101900001            | O(log n)                                       | O(1)                                     |
| Cari rentang NIK                     | Mudah dan efisien (`searchRangeInBPTree`)      | Tidak bisa langsung                      |
| Ingin data selalu terurut            | Ya, tinggal traversal leaf nodes               | Tidak mendukung urutan                   |
| Ingin akses cepat dengan memori kecil| Tidak seefisien Hash Map                       | Ya, untuk akses langsung                 |
| Digunakan dalam database             | Sangat umum (MySQL, PostgreSQL, dll.)          | Jarang, kecuali untuk caching            |

---

<h2>Kesimpulan Teknis</h2>

- Jika **fokus utamanya adalah pencarian 1 data berdasarkan NIK**, dan data < 1000:
  > Gunakan **Hash Map**

- Jika **perlu pencarian rentang NIK** (misal NIK dari 3273010101900001 sampai 3273010101999999), atau ingin data **tersortir**:
  > Gunakan **B+ Tree**

- Jika ingin **sistem hybrid yang fleksibel**:
  > Gunakan keduanya secara bersamaan. Hash Map untuk pencarian cepat tunggal, B+ Tree untuk query kompleks dan range.

---

<h2>🧪 Studi Kasus Singkat</h2>

| Jumlah Data | Cari 1 NIK (Hash Map) | Cari 1 NIK (B+ Tree) | Cari Range NIK (B+ Tree) | Kesimpulan Utama                       |
|-------------|------------------------|------------------------|---------------------------|----------------------------------------|
| 10          | ✅ Sangat cepat         | ❌ Lambat karena overhead | ✅ Cepat tapi overkill     | Gunakan Hash Map                       |
| 100         | ✅ Cepat               | ✅ Cepat                | ✅ B+ Tree unggul           | Pilih sesuai kebutuhan                 |
| 1000        | ✅ Stabil              | ✅ Stabil               | ✅ Ideal untuk range        | Gunakan keduanya jika memungkinkan     |

---

