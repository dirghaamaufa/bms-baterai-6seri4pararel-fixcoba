# Sistem Pemantauan Kapasitas Baterai Li-ion 6S4P (BMS)

Proyek mata kuliah **Algoritma dan Pemrograman Komputer** (NTRO236017), Teknik Elektro, Universitas Negeri Malang.

Program menghitung sisa kapasitas baterai Li-ion **6S4P** (6 grup seri, 4 sel paralel, 2.500 mAh per sel, total 10 Ah) dari tegangan tiap grup.

## Demo web

https://dirghaamaufa.github.io/bms-baterai-6s4p/

## Cara kerja singkat

1. Jumlahkan tegangan 6 grup (seri) untuk mendapat tegangan total.
2. Cari grup dengan tegangan terendah (grup terlemah).
3. Hitung SOC: `(V - 3,0) / (4,2 - 3,0) x 100%`.
4. Kapasitas tersisa = SOC x 10 Ah.
5. Tentukan status: Penuh, Baik, Sedang, Rendah, atau Habis.

## Isi repository

| File | Keterangan |
|---|---|
| `bms_lengkap.cpp` | **Versi utama.** C++ console 3 halaman (Start, Input, Hasil) dengan seluruh materi Modul 3, 4, 5, 6, dan 8 |
| `index.html` | Versi web (tampilan pelengkap, ditulis dengan JavaScript) |
| `bms_kapasitas_3halaman.cpp` | Versi C++ 3 halaman yang lebih sederhana |
| `bms_kapasitas.cpp` | Versi C++ sederhana, satu layar |
| `bms_monitor.cpp` | Versi C++ pemantauan tegangan dan arus (dashboard) |

## Menjalankan versi C++

```bash
g++ bms_lengkap.cpp -o bms_lengkap
./bms_lengkap          # Windows: bms_lengkap.exe
```

Jalankan di terminal (bukan panel Output) agar layar bisa dibersihkan antar halaman.

## Alur program (bms_lengkap.cpp)

- **Halaman 1:** judul dan tombol Start.
- **Halaman 2:** input tegangan 6 grup, diketik satu per satu atau lewat frame serial `$BMS,V,v1,...,v6#`. Bisa sampai 5 pengukuran.
- **Halaman 3:** langkah perhitungan, hasil, riwayat, simulasi balancing, lalu Reset ke Halaman 1.

## Materi modul yang dipakai

| Modul | Materi |
|---|---|
| 3 | `#define`, `const`, tipe data, if-else bertingkat, `switch-case`, operator logika, prototipe fungsi |
| 4 | `while`, `do-while`, `for`, nested loop, `break`, `continue`, `exit()` |
| 5 | Array 1D dan 2D, Max-Min Search, function overloading |
| 6 | Parsing string (`find`, `rfind`, `substr`, `front`, `back`), dekomposisi fungsi |
| 8 | `assert()` unit test, `exp()`, `setw`, `setprecision` |

## Catatan

- Kapasitas dihitung dengan hubungan tegangan-SOC linear (3,0 V = 0%, 4,2 V = 100%). Ini perkiraan sederhana, kurva baterai sebenarnya tidak linear.
- Simulasi balancing memakai model RC sederhana, bukan perilaku baterai sebenarnya.
- Di luar materi modul, program memakai `stof` dengan `try/catch` untuk konversi teks ke angka, dan `system()` untuk membersihkan layar.
