# WIRING — Kipas Otomatis 3 Kecepatan Berbasis Sensor Suhu DS18B20 + ESP32

Panduan wiring lengkap untuk proyek ini. Untuk panduan software, build, dan pengoperasian, lihat [README.md](README.md).

## 📋 Daftar Komponen

| **No** | **Komponen**                | **Spesifikasi**                        | **Jumlah** | **Keterangan**                 |
| ------ | --------------------------- | -------------------------------------- | ---------- | ------------------------------ |
| 1      | **ESP32 Development Board** | DOIT ESP32 DEVKIT V1 / Generic         | 1          | Mikrokontroler utama           |
| 2      | **Sensor Suhu DS18B20**     | Digital, 1-Wire, TO-92 atau Waterproof | 1          | Mengukur suhu ruangan          |
| 3      | **MOSFET IRLZ44N**          | N-Channel, Logic-Level, TO-220         | 1          | Driver untuk Fan (PWM)         |
| 4      | **Dioda 1N4007**            | 1A, 1000V, DO-41                       | 1          | Flyback protection             |
| 5      | **Resistor 4.7kΩ**          | 1/4 Watt                               | 1          | Pull-up untuk DS18B20 (WAJIB!) |
| 6      | **Resistor 220Ω**           | 1/4 Watt                               | 1          | Resistor Gate untuk MOSFET     |
| 7      | **Fan DC 12V**              | 40mm - 80mm                            | 1          | Output utama                   |
| 8      | **Power Supply 12V**        | Adaptor 12V, minimal 1A                | 1          | Daya untuk Fan                 |
| 9      | **Kabel USB**               | Kabel data/power                       | 1          | Daya & programming ESP32       |
| 10     | **Breadboard / PCB**        | -                                      | 1          | Tempat merakit komponen        |
| 11     | **Kabel Jumper**            | Male-to-Female / Male-to-Male          | 1 set      | Koneksi antar komponen         |

> Jangan mengganti IRLZ44N dengan MOSFET non-logic-level (mis. IRF540N tanpa driver tambahan):
> Vgs 3,3 V dari ESP32 tidak cukup untuk membuka IRF540N sepenuhnya sehingga MOSFET panas.

---

## 🔌 Pin Mapping ESP32

| **Komponen**       | **Pin/Kaki**        | **→** | **ESP32**   | **Keterangan**              |
| ------------------ | ------------------- | ----- | ----------- | --------------------------- |
| **DS18B20**        | VDD (Merah)         | →     | **3.3V**    | Daya sensor                 |
| **DS18B20**        | GND (Hitam)         | →     | **GND**     | Ground sensor               |
| **DS18B20**        | DATA (Kuning/Putih) | →     | **GPIO 4**  | Jalur data 1-Wire           |
| **Resistor 4.7kΩ** | Kaki 1              | →     | **3.3V**    | Pull-up source              |
| **Resistor 4.7kΩ** | Kaki 2              | →     | **DATA sensor** | Pull-up resistor (WAJIB!) |
| **MOSFET IRLZ44N** | Gate (G)            | →     | **GPIO 15** | PWM output (via R 220Ω)     |
| **MOSFET IRLZ44N** | Source (S)          | →     | **GND**     | Ground bersama              |
| **MOSFET IRLZ44N** | Drain (D)           | →     | **Fan (−)** | Kontrol sisi negatif fan    |

Sesuai `src/main.cpp`:

```cpp
constexpr uint8_t ONE_WIRE_PIN = 4;   // DS18B20 DATA
constexpr uint8_t FAN_PWM_PIN = 15;   // MOSFET Gate (via 220Ω)
```

---

## ⚡ Skema Rangkaian

### 1. Sensor suhu DS18B20

```text
ESP32 3.3V ──┬── DS18B20 VDD (Merah)
             └── Resistor 4.7kΩ kaki 1

Resistor 4.7kΩ kaki 2 ──┬── DS18B20 DATA (Kuning/Putih)
                        └── ESP32 GPIO 4

ESP32 GND ─── DS18B20 GND (Hitam)
```

Resistor 4.7kΩ dipasang antara 3.3V dan jalur DATA. Tanpa resistor ini sensor tidak terbaca.

### 2. Driver kipas (MOSFET + dioda flyback + fan 12V)

```text
Adaptor 12V (+) ──┬── Fan (+) Merah
                  └── Dioda 1N4007 KATODA (ujung bergaris)

Fan (-) Hitam ────┬── MOSFET Drain (D, kaki tengah pada IRLZ44N TO-220)
                  └── Dioda 1N4007 ANODA

ESP32 GPIO 15 ─── Resistor 220Ω ─── MOSFET Gate (G)

MOSFET Source (S) ─── GND bersama
Adaptor 12V (-) ─── GND bersama ─── ESP32 GND
```

Prinsip kerja: ESP32 mengeluarkan PWM 25 kHz dari GPIO 15 → Gate MOSFET → MOSFET menghubungkan/memutus jalur negatif fan ke GND. Dioda flyback meredam lonjakan tegangan dari motor saat MOSFET mati.

### 3. Ground bersama (paling sering dilupakan!)

```text
ESP32 GND ─── DS18B20 GND ─── MOSFET Source ─── Adaptor 12V (-)
```

Semua GND harus tersambung jadi satu titik referensi. Tanpa ini PWM tidak stabil dan pembacaan sensor bisa ngaco.

> ⚠️ Jangan pernah menyambungkan 12V ke pin mana pun di ESP32. ESP32 hanya disuplai dari USB 5V / pin VIN 5V, sedangkan jalur 12V hanya untuk fan dan dioda.

---

## 📋 Tabel Wiring Lengkap (Pin-to-Pin)

| **No** | **Dari**       | **Kaki**        | **→** | **Ke**         | **Kaki**        | **Keterangan**   |
| ------ | -------------- | --------------- | ----- | -------------- | --------------- | ---------------- |
| 1      | **ESP32**      | 3.3V            | →     | **DS18B20**    | VDD (Merah)     | Daya sensor      |
| 2      | **ESP32**      | GND             | →     | **DS18B20**    | GND (Hitam)     | Ground sensor    |
| 3      | **ESP32**      | GPIO 4          | →     | **DS18B20**    | DATA (Kuning)   | Data 1-Wire      |
| 4      | **ESP32**      | 3.3V            | →     | **R1 (4.7kΩ)** | Kaki 1          | Pull-up source   |
| 5      | **R1 (4.7kΩ)** | Kaki 2          | →     | **DS18B20**    | DATA (Kuning)   | Pull-up (WAJIB!) |
| 6      | **ESP32**      | GPIO 15         | →     | **R2 (220Ω)**  | Kaki 1          | PWM output       |
| 7      | **R2 (220Ω)**  | Kaki 2          | →     | **MOSFET**     | Gate (G)        | Resistor Gate    |
| 8      | **MOSFET**     | Source (S)      | →     | **GND**        | GND             | Ground bersama   |
| 9      | **MOSFET**     | Drain (D)       | →     | **Fan**        | Kabel (−) Hitam | Kontrol negatif  |
| 10     | **Fan**        | Kabel (+) Merah | →     | **12V Power**  | (+)             | Daya fan         |
| 11     | **12V Power**  | (+)             | →     | **Dioda**      | Katoda (garis)  | Flyback source   |
| 12     | **Dioda**      | Anoda           | →     | **MOSFET**     | Drain (D)       | Flyback target   |
| 13     | **12V Power**  | GND (−)         | →     | **GND**        | GND             | Ground bersama   |

---

## ⚠️ Catatan Penting

| **No** | **Catatan**                                                                                          |
| ------ | ---------------------------------------------------------------------------------------------------- |
| 1      | **Resistor 4.7kΩ WAJIB** dipasang antara 3.3V dan DATA DS18B20. Tanpa ini, sensor tidak terbaca!     |
| 2      | **Resistor 220Ω WAJIB** dipasang antara GPIO 15 dan Gate MOSFET. Melindungi pin GPIO ESP32.          |
| 3      | **Dioda 1N4007 WAJIB** dipasang dengan ARAH BENAR: Katoda (garis) ke 12V (+), Anoda ke Drain MOSFET. |
| 4      | **Ground semua komponen** harus disatukan (ESP32, DS18B20, MOSFET, Power Supply 12V).                |
| 5      | **Power Supply 12V** harus terpisah dari ESP32. Jangan sambungkan 12V ke ESP32!                      |
| 6      | Letakkan **DS18B20 di luar project box** agar membaca suhu ruangan yang sebenarnya.                  |
| 7      | Saat pengujian pertama, **jangan sambungkan Fan** dulu. Uji di Serial Monitor terlebih dahulu.       |

---

## 🧪 Langkah Pengujian

| **No** | **Langkah**                                         | **Yang Diharapkan**                                              |
| ------ | --------------------------------------------------- | ---------------------------------------------------------------- |
| 1      | Upload program ke ESP32                             | Tidak ada error                                                  |
| 2      | Buka Serial Monitor (115200 baud)                   | Menampilkan `Smart Fan initialized`                              |
| 3      | Pegang sensor DS18B20 dengan jari                   | Suhu naik melewati 30 / 35 °C, status berubah SPEED 1 → 2 → 3    |
| 4      | Dekatkan es batu ke sensor                          | Suhu turun, status turun ke SPEED 1, kipas mati di bawah 24,5 °C |
| 5      | Setelah semua data serial benar, sambungkan Fan 12V | Fan berputar: pelan / sedang / kencang sesuai suhu               |

Contoh output serial yang benar (format dari `src/main.cpp`):

```text
Smart Fan initialized
Temperature: 27.0 C | PWM: 85 | Speed: 33% | Fan: SPEED 1
Temperature: 32.5 C | PWM: 170 | Speed: 66% | Fan: SPEED 2
Temperature: 36.0 C | PWM: 255 | Speed: 100% | Fan: SPEED 3
Temperature: SENSOR ERROR | PWM: 0 | Speed: 0% | Fan: OFF
```

---

## 🔗 Referensi Pin ESP32 yang Dipakai

Proyek ini hanya memakai 4 pin ESP32:

| **Pin**   | **Fungsi**              | **Catatan**                                    |
| --------- | ----------------------- | ---------------------------------------------- |
| GPIO 4    | DS18B20 DATA (1-Wire)   | Pin input digital biasa, aman dipakai          |
| GPIO 15   | PWM kipas (25 kHz)      | Pin output PWM, lewat resistor 220Ω ke Gate    |
| 3.3V      | Daya DS18B20 + pull-up  | Jangan dibebani perangkat lain yang besar      |
| GND       | Ground bersama          | Wajib tersambung ke GND adaptor 12V            |

Posisi fisik GPIO 4 dan GPIO 15 berbeda antar varian board, jadi selalu cek tulisan sablon (silkscreen) pada board Anda sendiri. Jangan memakai pin flash (GPIO 6–11) untuk sensor/aktuator.

---

## 📁 Struktur File Proyek

```text
.
├── platformio.ini
├── src/
│   └── main.cpp
├── include/
├── lib/
├── test/
├── WIRING.md
└── README.md
```

---

**Dokumen ini berisi semua informasi wiring dan komponen untuk proyek Smart Fan. Simpan file ini di root folder proyek Anda.**
