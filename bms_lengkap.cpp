// ============================================================
// PENGUKUR KAPASITAS BATERAI Li-ion 6S4P  -  VERSI LENGKAP
// Praktikum Algoritma & Pemrograman Komputer (NTRO236017)
//
// HALAMAN 1 : Judul + START
// HALAMAN 2 : Input tegangan (manual / frame serial), bisa beberapa kali
// HALAMAN 3 : Langkah perhitungan, hasil, riwayat, simulasi balancing
//             lalu RESET kembali ke Halaman 1
//
// Peta modul:
//  Modul 3 : #define & const, tipe data (int/float/double/char/bool/string),
//            if-else bertingkat, switch-case, operator && || !, prototipe fungsi
//  Modul 4 : while, do-while, for, nested loop, akumulator,
//            break, continue, exit()
//  Modul 5 : array 1D & 2D, Max-Min Search, function overloading
//  Modul 6 : parsing string (find/rfind/substr/front/back/length),
//            dekomposisi fungsi (Input - Proses - Output)
//  Modul 8 : assert() unit test, exp() <cmath>, setw/setprecision,
//            refactoring (kode dipecah jadi fungsi kecil)
// ============================================================
#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>
#include <cassert>
#include <cstdlib>
using namespace std;

// ---------- MODUL 3: konstanta preprocessor (#define) ----------
#define JUDUL_APLIKASI "PENGUKUR KAPASITAS BATERAI 6S4P"

// ---------- MODUL 3: konstanta bertipe (const) ----------
const int    JML_SEL        = 6;       // 6S : 6 grup seri
const int    JML_PARALEL    = 4;       // 4P : 4 sel paralel per grup
const int    MAKS_RIWAYAT   = 5;       // maksimal pengukuran per sesi
const int    MAKS_INVALID   = 3;       // frame salah berturut-turut yang ditoleransi
const int    LEBAR          = 60;      // lebar garis tampilan
const float  V_NOMINAL      = 3.70f;   // tegangan nominal 1 sel
const float  KAP_SEL_AH     = 2.5f;    // 2.500 mAh per sel
const float  KAP_PACK_AH    = JML_PARALEL * KAP_SEL_AH;   // 4 x 2.5 = 10 Ah
const float  V_PENUH        = 4.20f;   // SOC 100%
const float  V_KOSONG       = 3.00f;   // SOC 0%
const float  V_KRITIS_ATAS  = 4.30f;   // di atas ini : bahaya (shutdown)
const float  V_KRITIS_BAWAH = 2.80f;   // di bawah ini: bahaya (shutdown)
const float  V_MAKS_INPUT   = 5.00f;   // batas input masuk akal
const float  SELISIH_MAKS   = 0.10f;   // batas ketidakseimbangan antar grup (V)

// Deklarasi (prototipe) fungsi - definisinya ada di bawah
bool ubahKeAngka(string teks, float& hasil);

// ================= MODUL PROSES =================

// Tegangan -> persen isi (SOC)
float hitungSOC(float v) {
    float soc = (v - V_KOSONG) / (V_PENUH - V_KOSONG) * 100.0f;
    if (soc < 0.0f)   soc = 0.0f;
    if (soc > 100.0f) soc = 100.0f;
    return soc;
}

float hitungKapasitasTersisa(float soc) {
    return soc / 100.0f * KAP_PACK_AH;
}

// Akumulator (Modul 4)
float hitungTotal(const float v[], int n) {
    float total = 0.0f;
    for (int i = 0; i < n; i++) total += v[i];
    return total;
}

// Max-Min Search (Modul 5)
int cariGrupTerlemah(const float v[], int n) {
    int idx = 0;
    for (int i = 1; i < n; i++)
        if (v[i] < v[idx]) idx = i;
    return idx;
}
int cariGrupTertinggi(const float v[], int n) {
    int idx = 0;
    for (int i = 1; i < n; i++)
        if (v[i] > v[idx]) idx = i;
    return idx;
}

// ---------- MODUL 5: Function Overloading ----------
// Versi 1: selisih dua nilai
float hitungSelisih(float a, float b) {
    return fabs(a - b);
}
// Versi 2: selisih tertinggi - terendah dari satu array
float hitungSelisih(const float v[], int n) {
    return v[cariGrupTertinggi(v, n)] - v[cariGrupTerlemah(v, n)];
}

// ---------- MODUL 3: if-else bertingkat -> kode, switch-case -> teks ----------
int tentukanZona(float v) {
    if (v > V_KRITIS_ATAS || v < V_KRITIS_BAWAH) return 4;   // kritis
    else if (v > V_PENUH)                        return 2;   // over-voltage
    else if (v < V_KOSONG)                       return 3;   // under-voltage
    else                                         return 1;   // normal
}

string namaZona(int zona) {
    switch (zona) {
        case 1:  return "NORMAL";
        case 2:  return "OVER";
        case 3:  return "UNDER";
        case 4:  return "KRITIS";
        default: return "?";
    }
}

int tentukanStatus(float soc) {
    if (soc >= 95.0f)      return 1;
    else if (soc >= 50.0f) return 2;
    else if (soc >= 20.0f) return 3;
    else if (soc > 0.0f)   return 4;
    else                   return 5;
}

string pesanStatus(int kode) {
    switch (kode) {
        case 1:  return "PENUH";
        case 2:  return "BAIK";
        case 3:  return "SEDANG - segera isi ulang";
        case 4:  return "RENDAH - isi ulang sekarang!";
        case 5:  return "HABIS - hentikan pemakaian!";
        default: return "TIDAK DIKENAL";
    }
}

// Apakah ada grup yang kritis? (operator logika)
bool adaGrupKritis(const float v[], int n) {
    bool kritis = false;
    for (int i = 0; i < n; i++)
        if (tentukanZona(v[i]) == 4) kritis = true;
    return kritis;
}

// Validasi: semua nilai masuk akal?
bool sensorValid(const float v[], int n) {
    for (int i = 0; i < n; i++)
        if (v[i] < 0.0f || v[i] > V_MAKS_INPUT) return false;
    return true;
}

string buatBar(float soc) {
    const int PANJANG = 20;
    int terisi = (int)(soc / 100.0f * PANJANG + 0.5f);
    string bar = "[";
    for (int i = 0; i < PANJANG; i++)
        bar += (i < terisi) ? '#' : '-';
    bar += "]";
    return bar;
}

// ---------- MODUL 6: Parsing frame serial ----------
// Format: $BMS,V,v1,v2,v3,v4,v5,v6#
bool parseFrame(const string& frame, float v[]) {
    // validasi header '$' dan footer '#'
    if (frame.length() < 3 || frame.front() != '$' || frame.back() != '#')
        return false;

    string isi = frame.substr(1, frame.length() - 2);      // buang header & footer

    size_t pos = isi.find(',');                            // field 1: "BMS"
    if (pos == string::npos || isi.substr(0, pos) != "BMS") return false;
    isi = isi.substr(pos + 1);

    pos = isi.find(',');                                   // field 2: "V"
    if (pos == string::npos || isi.substr(0, pos) != "V") return false;
    isi = isi.substr(pos + 1);

    for (int i = 0; i < JML_SEL - 1; i++) {                // 5 nilai pertama
        pos = isi.find(',');
        if (pos == string::npos) return false;             // field kurang
        if (!ubahKeAngka(isi.substr(0, pos), v[i])) return false;
        isi = isi.substr(pos + 1);
    }
    if (isi.rfind(',') != string::npos) return false;      // masih ada koma = field berlebih
    return ubahKeAngka(isi, v[JML_SEL - 1]);               // nilai terakhir
}

// Teks -> angka. Koma desimal ("3,85") dianggap titik. false bila bukan angka murni.
bool ubahKeAngka(string teks, float& hasil) {
    string bersih = "";
    for (size_t i = 0; i < teks.length(); i++) {
        if (teks[i] == ',')       bersih += '.';
        else if (teks[i] != ' ' && teks[i] != '\t' && teks[i] != '\r') bersih += teks[i];
    }
    if (bersih.empty()) return false;
    try {
        size_t posisi;
        hasil = stof(bersih, &posisi);
        if (posisi != bersih.length()) return false;
    } catch (...) {
        return false;
    }
    return true;
}

// ---------- MODUL 8: Unit test ----------
void jalankanUnitTest() {
    // SOC & kapasitas
    assert(fabs(hitungSOC(3.0f) - 0.0f)   < 0.01f);
    assert(fabs(hitungSOC(4.2f) - 100.0f) < 0.01f);
    assert(fabs(hitungSOC(3.6f) - 50.0f)  < 0.01f);
    assert(hitungSOC(2.5f) == 0.0f);
    assert(hitungSOC(4.5f) == 100.0f);
    assert(fabs(hitungKapasitasTersisa(50.0f)  - 5.0f)  < 0.01f);
    assert(fabs(hitungKapasitasTersisa(100.0f) - 10.0f) < 0.01f);
    // Max-Min & overloading
    float uji[4] = {3.8f, 3.9f, 3.7f, 3.85f};
    assert(cariGrupTerlemah(uji, 4)  == 2);
    assert(cariGrupTertinggi(uji, 4) == 1);
    assert(fabs(hitungTotal(uji, 4) - 15.25f) < 0.0001f);
    assert(fabs(hitungSelisih(4.0f, 3.9f) - 0.1f) < 0.0001f);
    assert(fabs(hitungSelisih(uji, 4)     - 0.2f) < 0.0001f);
    // zona & status
    assert(tentukanZona(3.70f) == 1);
    assert(tentukanZona(4.25f) == 2);
    assert(tentukanZona(2.90f) == 3);
    assert(tentukanZona(4.40f) == 4);
    assert(tentukanZona(2.50f) == 4);
    assert(tentukanZona(4.20f) == 1);                    // tepat di batas = normal
    assert(tentukanStatus(100.0f) == 1);
    assert(tentukanStatus(70.0f)  == 2);
    assert(tentukanStatus(30.0f)  == 3);
    assert(tentukanStatus(10.0f)  == 4);
    assert(tentukanStatus(0.0f)   == 5);
    float aman[3]   = {3.7f, 3.8f, 3.9f};
    float bahaya[3] = {3.7f, 4.4f, 3.9f};
    assert(!adaGrupKritis(aman, 3));
    assert(adaGrupKritis(bahaya, 3));
    // konversi teks & parser
    float h = 0.0f;
    assert(ubahKeAngka("3.85", h) && fabs(h - 3.85f) < 0.0001f);
    assert(ubahKeAngka("3,85", h) && fabs(h - 3.85f) < 0.0001f);
    assert(!ubahKeAngka("abc", h));
    assert(!ubahKeAngka("3.85V", h));
    assert(!ubahKeAngka("", h));
    float v[JML_SEL];
    assert(parseFrame("$BMS,V,3.85,3.87,3.86,3.84,3.85,3.88#", v) == true);
    assert(fabs(v[1] - 3.87f) < 0.0001f && fabs(v[5] - 3.88f) < 0.0001f);
    assert(parseFrame("INVALID_PACKET", v) == false);
    assert(parseFrame("$BMS,V,3.85,3.87#", v) == false);                         // kurang
    assert(parseFrame("$BMS,V,3.85,3.87,3.86,3.84,3.85,3.88,3.9#", v) == false); // lebih
    assert(parseFrame("$BMS,V,3.85,abc,3.86,3.84,3.85,3.88#", v) == false);
    assert(parseFrame("$XXX,V,3.85,3.87,3.86,3.84,3.85,3.88#", v) == false);
}

// ================= ALAT BANTU TAMPILAN =================
void bersihkanLayar() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void cetakGaris(char c) {
    cout << string(LEBAR, c) << endl;
}

void cetakHeader(int no, const string& judul) {
    cetakGaris('=');
    cout << "  " << JUDUL_APLIKASI << "   HALAMAN " << no << " / 3" << endl;
    cout << "  " << judul << endl;
    cetakGaris('=');
}

string bacaBaris() {                    // aman: tidak pernah membuat cin error
    string baris;
    if (!getline(cin, baris)) {
        cout << "\n[ERROR] Input terputus. Program dihentikan." << endl;
        exit(1);
    }
    return baris;
}

void tungguEnter(const string& pesan) {
    cout << pesan;
    bacaBaris();
}

char bacaPilihan() {                    // ambil karakter pertama, ' ' bila kosong
    string s = bacaBaris();
    return s.empty() ? ' ' : s[0];
}

// ================= HALAMAN 1 : JUDUL + START =================
bool halaman1() {
    char pilih;
    bool valid;
    do {
        bersihkanLayar();
        cetakGaris('=');
        cout << "\n";
        cout << "         SISTEM PENGUKUR KAPASITAS BATERAI" << endl;
        cout << "          Battery Management System (BMS)" << endl;
        cout << "            Baterai Li-ion 6S4P (24 sel)" << endl;
        cout << "\n";
        cetakGaris('=');
        cout << fixed << setprecision(1);
        cout << "  Susunan sel       : " << JML_SEL << " seri x " << JML_PARALEL << " paralel" << endl;
        cout << "  Tegangan nominal  : " << JML_SEL << " x " << V_NOMINAL << " V = "
             << JML_SEL * V_NOMINAL << " V" << endl;
        cout << "  Kapasitas pack    : " << JML_PARALEL << " x " << KAP_SEL_AH * 1000
             << " mAh = " << KAP_PACK_AH * 1000 << " mAh (" << KAP_PACK_AH << " Ah)" << endl;
        cetakGaris('-');
        cout << "   [1]  START   - mulai pengukuran" << endl;
        cout << "   [0]  KELUAR  - tutup program" << endl;
        cetakGaris('-');
        cout << "  Pilih menu : ";
        pilih = bacaPilihan();

        valid = (pilih == '1' || pilih == '0');
        if (!valid) {
            cout << "  [ERROR] Pilihan tidak valid! Ketik 1 atau 0." << endl;
            tungguEnter("  Tekan Enter untuk mencoba lagi...");
        }
    } while (!valid);
    return (pilih == '1');
}

// ================= HALAMAN 2 : INPUT =================

// Cara 1: ketik satu per satu (do-while untuk validasi)
bool inputManual(float v[]) {
    cout << "\n  Masukkan tegangan tiap grup (0 - " << V_MAKS_INPUT << " V):" << endl;
    for (int i = 0; i < JML_SEL; i++) {
        bool valid;
        do {
            cout << "  Grup S" << i + 1 << "P" << JML_PARALEL << " (Volt) : ";
            valid = ubahKeAngka(bacaBaris(), v[i]) && v[i] >= 0.0f && v[i] <= V_MAKS_INPUT;
            if (!valid)
                cout << "  [ERROR] Masukkan angka 0 - " << V_MAKS_INPUT
                     << " (contoh: 3.85)!" << endl;
        } while (!valid);
    }
    return true;
}

// Cara 2: tempel frame serial. Frame salah -> continue; 3x berturut-turut -> break
bool inputDariFrame(float v[]) {
    cout << "\n  Format : $BMS,V,v1,v2,v3,v4,v5,v6#" << endl;
    cout << "  Contoh : $BMS,V,3.85,3.87,3.86,3.84,3.85,3.88#" << endl;
    int percobaanInvalid = 0;
    while (true) {
        cout << "\n  Frame : ";
        string frame = bacaBaris();

        if (!parseFrame(frame, v)) {
            percobaanInvalid++;
            cout << "  [ERROR] Format frame tidak valid! (" << percobaanInvalid
                 << "/" << MAKS_INVALID << ")" << endl;
            if (percobaanInvalid >= MAKS_INVALID) break;     // berhenti
            continue;                                        // ulangi
        }
        if (!sensorValid(v, JML_SEL)) {
            percobaanInvalid++;
            cout << "  [WARNING] Ada nilai di luar rentang 0 - " << V_MAKS_INPUT
                 << " V, data diabaikan. (" << percobaanInvalid << "/" << MAKS_INVALID << ")" << endl;
            if (percobaanInvalid >= MAKS_INVALID) break;
            continue;
        }
        cout << "  [OK] Frame valid, 6 tegangan terbaca." << endl;
        return true;
    }
    cout << "\n  Sensor/komunikasi bermasalah, periksa kalibrasi!" << endl;
    return false;
}

// Halaman 2: boleh beberapa kali pengukuran, disimpan di array 2D riwayat
void halaman2(float riwayat[][JML_SEL], int& jml) {
    bool ulangi;
    do {
        bersihkanLayar();
        cetakHeader(2, "Memasukkan Tegangan Tiap Grup");
        cout << "  Pengukuran ke-" << jml + 1 << " (maksimal " << MAKS_RIWAYAT << ")" << endl;
        cetakGaris('-');
        cout << "   [1]  Ketik tegangan satu per satu" << endl;
        cout << "   [2]  Tempel frame serial (hasil parsing)" << endl;
        cout << "   [0]  Kembali ke Halaman 1" << endl;
        cetakGaris('-');
        cout << "  Pilih : ";

        float v[JML_SEL] = {0};
        bool berhasil = false;
        switch (bacaPilihan()) {
            case '1': berhasil = inputManual(v);    break;
            case '2': berhasil = inputDariFrame(v); break;
            case '0': return;
            default:  cout << "  [ERROR] Pilihan tidak valid!" << endl;
        }

        if (berhasil) {
            for (int s = 0; s < JML_SEL; s++) riwayat[jml][s] = v[s];
            jml++;
            cout << "\n  Pengukuran ke-" << jml << " tersimpan." << endl;
        } else {
            tungguEnter("\n  Tekan Enter untuk kembali ke menu input...");
        }

        // tentukan apakah lanjut menambah pengukuran
        if (jml >= MAKS_RIWAYAT) {
            cout << "  Riwayat penuh (" << MAKS_RIWAYAT << " pengukuran)." << endl;
            ulangi = false;
        } else if (jml == 0) {
            ulangi = true;                               // belum ada data
        } else {
            cout << "  Tambah pengukuran lagi? (y/n) : ";
            char lagi = bacaPilihan();
            ulangi = (lagi == 'y' || lagi == 'Y');
        }
    } while (ulangi);

    if (jml > 0) tungguEnter("\n  Tekan Enter untuk MENGHITUNG...");
}

// ================= HALAMAN 3 : HASIL =================

// MODUL 8: simulasi balancing pasif dengan exp()
void simulasiBalancing(double vAtas, double vTarget) {
    const double R   = 10.0;                 // ohm  (resistor balancing, model)
    const double C   = 0.5;                  // farad (model pendekatan)
    const double tau = R * C;                // konstanta waktu
    cout << "\nSIMULASI BALANCING PASIF (model RC, tau = R x C = " << fixed
         << setprecision(1) << tau << " s)" << endl;
    cout << "  Grup tertinggi diturunkan menuju tegangan grup terlemah." << endl;
    cout << "  V(t) = Vtarget + (V0 - Vtarget) x exp(-t / tau)" << endl;
    cetakGaris('-');
    cout << left << setw(14) << "  Waktu t (s)" << setw(18) << "V grup tertinggi"
         << "Selisih" << endl;
    cout << fixed << setprecision(3);
    for (int k = 0; k <= 3; k++) {
        double t  = k * tau;
        double vt = vTarget + (vAtas - vTarget) * exp(-t / tau);
        cout << left << "  " << setw(12) << setprecision(1) << t
             << setprecision(3) << setw(18) << vt << vt - vTarget << endl;
    }
    cout << "  (model pendekatan, bukan perilaku baterai sebenarnya)" << endl;
    cout << setprecision(2);                 // kembalikan ke 2 desimal
}

// Riwayat (array 2D) + ringkasan per grup (nested loop)
void tampilkanRiwayat(const float riwayat[][JML_SEL], int jml) {
    cout << "\nRIWAYAT PENGUKURAN (" << jml << " kali)" << endl;
    cetakGaris('-');
    cout << right << setw(5) << "No";
    for (int s = 0; s < JML_SEL; s++) cout << setw(6) << ("S" + to_string(s + 1));
    cout << setw(8) << "Total" << endl;
    cout << fixed << setprecision(2);
    for (int k = 0; k < jml; k++) {
        cout << setw(5) << k + 1;
        for (int s = 0; s < JML_SEL; s++) cout << setw(6) << riwayat[k][s];
        cout << setw(8) << hitungTotal(riwayat[k], JML_SEL) << endl;
    }
    cetakGaris('-');
    for (int s = 0; s < JML_SEL; s++) {                  // nested loop
        float vMax = riwayat[0][s], vMin = riwayat[0][s], total = 0.0f;
        for (int k = 0; k < jml; k++) {
            if (riwayat[k][s] > vMax) vMax = riwayat[k][s];
            if (riwayat[k][s] < vMin) vMin = riwayat[k][s];
            total += riwayat[k][s];
        }
        cout << "  S" << s + 1 << "  Vmax=" << vMax << "  Vmin=" << vMin
             << "  Vrata2=" << total / jml << endl;
    }
    // overloading versi 2 nilai: selisih total antar dua pengukuran terakhir
    float totalBaru = hitungTotal(riwayat[jml - 1], JML_SEL);
    float totalLama = hitungTotal(riwayat[jml - 2], JML_SEL);
    cout << "  Selisih total tegangan vs pengukuran sebelumnya : "
         << hitungSelisih(totalBaru, totalLama) << " V" << endl;
}

void halaman3(const float riwayat[][JML_SEL], int jml) {
    const float* v = riwayat[jml - 1];                   // pengukuran terbaru

    // --- Proses ---
    float total     = hitungTotal(v, JML_SEL);
    int   idx       = cariGrupTerlemah(v, JML_SEL);
    int   idxMax    = cariGrupTertinggi(v, JML_SEL);
    float vLemah    = v[idx];
    float socMentah = (vLemah - V_KOSONG) / (V_PENUH - V_KOSONG) * 100.0f;
    float soc       = hitungSOC(vLemah);
    float kapSisa   = hitungKapasitasTersisa(soc);
    float kapPakai  = KAP_PACK_AH - kapSisa;
    float selisih   = hitungSelisih(v, JML_SEL);
    int   kode      = tentukanStatus(soc);

    // --- Output ---
    bersihkanLayar();
    cetakHeader(3, "Langkah Perhitungan & Hasil");
    cout << fixed << setprecision(2);

    cout << "\nLANGKAH 1 : Jumlahkan tegangan semua grup (rangkaian seri)" << endl;
    cout << "   ";
    for (int i = 0; i < JML_SEL; i++) cout << v[i] << (i < JML_SEL - 1 ? " + " : "");
    cout << " = " << total << " V" << endl;

    cout << "\nLANGKAH 2 : Cari grup terlemah (tegangan paling rendah)" << endl;
    cout << "   Grup terlemah = S" << idx + 1 << "P" << JML_PARALEL
         << " dengan tegangan " << vLemah << " V" << endl;
    cout << "   (seperti rantai: kekuatannya ditentukan mata rantai terlemah)" << endl;

    cout << "\nLANGKAH 3 : Hitung SOC (persen isi) grup terlemah" << endl;
    cout << "   SOC = (V - Vkosong) / (Vpenuh - Vkosong) x 100%" << endl;
    cout << "       = (" << vLemah << " - " << V_KOSONG << ") / (" << V_PENUH << " - "
         << V_KOSONG << ") x 100%" << endl;
    cout << "       = " << socMentah << " %" << endl;
    if (socMentah != soc)
        cout << "   (dibatasi pada rentang 0 - 100 %, menjadi " << soc << " %)" << endl;

    cout << "\nLANGKAH 4 : Hitung kapasitas tersisa" << endl;
    cout << "   Kapasitas pack = " << JML_PARALEL << " x " << KAP_SEL_AH << " Ah = "
         << KAP_PACK_AH << " Ah" << endl;
    cout << "   Tersisa        = " << soc << " % x " << KAP_PACK_AH << " Ah = "
         << kapSisa << " Ah" << endl;

    cout << "\nLANGKAH 5 : Hitung kapasitas terpakai" << endl;
    cout << "   Terpakai = " << KAP_PACK_AH << " - " << kapSisa << " = "
         << kapPakai << " Ah" << endl;

    cout << "\nLANGKAH 6 : Tentukan status baterai" << endl;
    cout << "   SOC " << setprecision(0) << soc << setprecision(2) << " %  ->  "
         << pesanStatus(kode) << endl;

    // --- Hasil akhir ---
    cout << "\n";
    cetakGaris('=');
    cout << "  HASIL AKHIR" << endl;
    cetakGaris('=');
    cout << "  " << left << setw(8) << "Grup" << right << setw(8) << "Tegangan"
         << setw(8) << "SOC" << "   " << left << "Zona" << endl;
    for (int i = 0; i < JML_SEL; i++) {
        cout << "  " << left << setw(8) << ("S" + to_string(i + 1) + "P" + to_string(JML_PARALEL))
             << right << setw(6) << v[i] << " V"
             << setw(5) << setprecision(0) << hitungSOC(v[i]) << " %" << setprecision(2)
             << "   " << left << setw(8) << namaZona(tentukanZona(v[i]))
             << (i == idx ? "<-- terlemah" : "") << endl;
    }
    cetakGaris('-');
    cout << left << setw(26) << "  Tegangan Total Pack" << ": " << total    << " V"  << endl;
    cout << left << setw(26) << "  Kapasitas Pack"      << ": " << KAP_PACK_AH << " Ah" << endl;
    cout << left << setw(26) << "  Kapasitas Tersisa"   << ": " << kapSisa  << " Ah"  << endl;
    cout << left << setw(26) << "  Kapasitas Terpakai"  << ": " << kapPakai << " Ah"  << endl;
    cout << left << setw(26) << "  Selisih Antar Grup"  << ": " << selisih  << " V"   << endl;
    cout << left << setw(26) << "  Isi Baterai"         << ": " << buatBar(soc)
         << " " << setprecision(0) << soc << " %" << setprecision(2) << endl;
    cout << left << setw(26) << "  Status"              << ": " << pesanStatus(kode) << endl;
    cetakGaris('=');

    // --- Riwayat (bila lebih dari 1 pengukuran) ---
    if (jml > 1) tampilkanRiwayat(riwayat, jml);

    // --- Balancing (bila grup tidak seimbang) ---
    if (selisih > SELISIH_MAKS) {
        cout << "\n[WARNING] Grup tidak seimbang (selisih " << selisih << " V > "
             << SELISIH_MAKS << " V)." << endl;
        simulasiBalancing(v[idxMax], v[idx]);
    } else {
        cout << "\n[INFO] Grup seimbang, balancing tidak diperlukan." << endl;
    }

    // --- Emergency shutdown (Modul 4: exit) ---
    if (adaGrupKritis(v, JML_SEL)) {
        cout << "\n[CRITICAL] Tegangan grup di luar batas aman ("
             << V_KRITIS_BAWAH << " - " << V_KRITIS_ATAS << " V)!" << endl;
        cout << "EMERGENCY SHUTDOWN (exit)!" << endl;
        exit(1);
    }

    cout << endl;
    tungguEnter("  Tekan Enter untuk RESET dan kembali ke Halaman 1...");
}

// ================= PROGRAM UTAMA =================
void resetData(float riwayat[][JML_SEL], int& jml) {
    for (int k = 0; k < MAKS_RIWAYAT; k++)               // nested loop
        for (int s = 0; s < JML_SEL; s++)
            riwayat[k][s] = 0.0f;
    jml = 0;
}

int main() {
    jalankanUnitTest();                  // bila ada yang salah, program berhenti

    float riwayat[MAKS_RIWAYAT][JML_SEL] = {{0}};
    int  jml     = 0;                    // jumlah pengukuran tersimpan
    int  halaman = 1;                    // penanda halaman
    bool jalan   = true;

    while (jalan) {
        switch (halaman) {
            case 1:
                if (halaman1()) halaman = 2;
                else            jalan = false;
                break;
            case 2:
                halaman2(riwayat, jml);
                halaman = (jml > 0) ? 3 : 1;             // tanpa data -> kembali
                break;
            case 3:
                halaman3(riwayat, jml);
                resetData(riwayat, jml);                 // reset
                halaman = 1;
                break;
        }
    }

    bersihkanLayar();
    cout << "Terima kasih. Program selesai." << endl;
    return 0;
}
