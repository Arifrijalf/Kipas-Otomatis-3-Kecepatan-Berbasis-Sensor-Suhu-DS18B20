# Kipas Otomatis 3 Kecepatan - DS18B20 + ESP32

Proyek ini bikin kipas DC 12V bisa ngatur kecepatannya sendiri berdasarkan suhu ruangan. Sensor yang dipakai DS18B20, otaknya ESP32. Jadi kalau ruangan panas, kipas muter makin kencang, kalau dingin ya pelan atau mati sekalian.

Kecepatannya ada 3 tingkat: pelan, sedang, kencang. Pindah antar tingkatnya otomatis tergantung suhu yang kebaca sensor.

## Daftar isi

- [Cara kerja](#cara-kerja)
- [Alat dan bahan](#alat-dan-bahan)
- [Wiring](#wiring)
- [Software yang dipakai](#software-yang-dipakai)
- [Cara upload program](#cara-upload-program)
- [Pengaturan suhu dan kecepatan](#pengaturan-suhu-dan-kecepatan)
- [Kalau ada masalah](#kalau-ada-masalah)
- [Struktur folder](#struktur-folder)
- [Catatan](#catatan)
- [Progress 1 minggu](#progress-1-minggu)
- [Dokumentasi](#dokumentasi)

## Cara kerja

Gampangnya begini:

- Suhu dibaca tiap 1 detik dari DS18B20 (pin GPIO 4).
- ESP32 ngeluarin sinyal PWM 25 kHz dari GPIO 15 ke MOSFET, MOSFET ini yang ngatur kipas.
- Kipas nyala kalau suhu sudah 25,5 derajat ke atas, dan mati total kalau suhu turun di bawah 24,5 derajat. Selisih ini sengaja biar kipas nggak cetak-cetek hidup mati terus di batas suhu.
- Kalau sensornya lepas atau rusak, kipas otomatis dimatikan biar aman.

Tabelnya:

| Suhu | PWM | Kipas |
| ---- | --- | ----- |
| Di bawah 24,5 | 0 | Mati |
| 25,5 - 29,9 | 85 (sekitar 33%) | Speed 1, pelan |
| 30 - 34,9 | 170 (sekitar 66%) | Speed 2, sedang |
| 35 ke atas | 255 (100%) | Speed 3, kencang |

## Alat dan bahan

Daftar lengkap sama gambar rangkaiannya ada di [WIRING.md](WIRING.md). Intinya butuh:

ESP32 DevKit V1, sensor DS18B20, resistor 4,7k (wajib buat pull-up sensor), MOSFET IRLZ44N, resistor 220 ohm, dioda 1N4007, kipas DC 12V, sama adaptor 12V.

## Wiring

Kabel intinya cuma ini:

- DS18B20: VDD ke 3V3, GND ke GND, DATA ke GPIO 4 (jangan lupa resistor 4,7k antara DATA dan 3V3).
- GPIO 15 ke resistor 220 ohm terus ke Gate MOSFET. Source MOSFET ke GND, Drain ke kabel minus kipas.
- Kabel plus kipas ke 12V. Dioda dipasang antara Drain dan 12V ( garisnya ke arah 12V).
- GND adaptor 12V digabung ke GND ESP32. Ini penting, sering kelupaan.

Selengkapnya lihat [WIRING.md](WIRING.md).

## Software yang dipakai

- PlatformIO, bisa lewat VS Code atau CLI.
- Board `esp32doit-devkit-v1`, framework Arduino. Settingnya sudah ada di `platformio.ini`.
- Library `OneWire` sama `DallasTemperature`, otomatis keinstal waktu build.
- Serial monitor 115200 baud.

## Cara upload program

Lewat VS Code tinggal Build terus Upload, lalu buka Serial Monitor (115200).

Lewat terminal, dari folder proyek:

```bash
pio run
pio run -t upload
pio device monitor -b 115200
```

Saran saya: pertama upload tanpa kipas dipasang dulu. Cek di serial monitor suhunya masuk akal atau tidak. Kalau sudah bener baru pasang kipas 12V-nya.

Output di serial monitor kira-kira seperti ini:

```text
Smart Fan initialized
Temperature: 27.0 C | PWM: 85 | Speed: 33% | Fan: SPEED 1
Temperature: 32.5 C | PWM: 170 | Speed: 66% | Fan: SPEED 2
Temperature: 36.0 C | PWM: 255 | Speed: 100% | Fan: SPEED 3
Temperature: SENSOR ERROR | PWM: 0 | Speed: 0% | Fan: OFF
```

## Pengaturan suhu dan kecepatan

Angka-angkanya ada di bagian atas `src/main.cpp`:

```cpp
constexpr uint8_t ONE_WIRE_PIN = 4;           // pin data DS18B20
constexpr uint8_t FAN_PWM_PIN = 15;           // pin PWM ke MOSFET
constexpr uint32_t PWM_FREQUENCY = 25000;     // 25 kHz biar tidak bunyi
constexpr float FAN_ON_TEMPERATURE = 25.5f;   // mulai nyala
constexpr float FAN_OFF_TEMPERATURE = 24.5f;  // mati total
constexpr float SPEED2_TEMPERATURE = 30.0f;   // pindah ke speed 2
constexpr float SPEED3_TEMPERATURE = 35.0f;   // pindah ke speed 3
constexpr uint8_t SPEED1_PWM = 85;            // pelan
constexpr uint8_t SPEED2_PWM = 170;           // sedang
constexpr uint8_t SPEED3_PWM = 255;           // kencang
constexpr unsigned long SENSOR_INTERVAL_MS = 1000;
```

Kalau mau ganti batas suhunya tinggal ubah angka di atas. Hysteresis on/off-nya (25,5 dan 24,5) sebaiknya tetap biar kipas tidak kedip-kedip di sekitar batas.

## Kalau ada masalah

| Gejala | Kemungkinan penyebabnya |
| ------ | ----------------------- |
| Muncul `SENSOR ERROR` terus | Resistor 4,7k belum dipasang, atau kabel DATA/VDD/GND ketukar, atau sensornya rusak. |
| Suhu -127 atau tidak berubah | Sama kayak di atas. Cek juga pin-nya bener GPIO 4 atau tidak. |
| Di serial sudah SPEED 1/2/3 tapi kipas diam | Adaptor 12V belum dicolok, GND adaptor tidak gabung ke GND ESP32, kaki Drain-Source ketukar, atau dioda kebalik. |
| MOSFET panas banget | MOSFET-nya bukan tipe logic-level, atau kipasnya kegedean. Pastikan pakai IRLZ44N, tambah heatsink kalau perlu. |
| Kipas bunyi decit | Frekuensi PWM keganti. Balikin ke 25000. |
| Upload gagal | Di board clone kadang harus tahan tombol BOOT pas upload. Cek juga kabel USB-nya, harus kabel data bukan kabel cas doang. |

## Struktur folder

```text
.
├── platformio.ini
├── src/
│   └── main.cpp
├── include/
├── lib/
├── test/
├── docs/        # foto dan log progres harian
├── WIRING.md
└── README.md
```

## Catatan

- Jangan sambungin 12V ke ESP32, kipas 12V jalurnya sendiri.
- Dioda jangan kebalik, garisnya ke arah 12V.
- Uji lewat serial dulu sebelum pasang kipas.

---

## Progress 1 minggu

Catatan harian saya selama ngerjain proyek ini. Tiap hari diisi tujuan, yang dikerjain, hasilnya, kendalanya, sama dokumentasinya. Ganti `[...]` dengan catatan masing-masing hari. Foto dan log disimpan di folder `docs/`.

<details>
<summary>Hari 1 [...] (Tanggal: YYYY-MM-DD)</summary>

Tujuan: [...]

Yang dikerjain:

- [ ] ...
- [ ] ...

Hasil: [...]

Kendala: [...]

Dokumentasi: `docs/hari-01/` (foto: ..., video: ..., log: ...)

Status: Belum mulai / Berjalan / Selesai

</details>

<details>
<summary>Hari 2 [...] (Tanggal: YYYY-MM-DD)</summary>

Tujuan: [...]

Yang dikerjain:

- [ ] ...
- [ ] ...

Hasil: [...]

Kendala: [...]

Dokumentasi: `docs/hari-02/`

Status: Belum mulai / Berjalan / Selesai

</details>

<details>
<summary>Hari 3 [...] (Tanggal: YYYY-MM-DD)</summary>

Tujuan: [...]

Yang dikerjain:

- [ ] ...
- [ ] ...

Hasil: [...]

Kendala: [...]

Dokumentasi: `docs/hari-03/`

Status: Belum mulai / Berjalan / Selesai

</details>

<details>
<summary>Hari 4 [...] (Tanggal: YYYY-MM-DD)</summary>

Tujuan: [...]

Yang dikerjain:

- [ ] ...
- [ ] ...

Hasil: [...]

Kendala: [...]

Dokumentasi: `docs/hari-04/`

Status: Belum mulai / Berjalan / Selesai

</details>

<details>
<summary>Hari 5 [...] (Tanggal: YYYY-MM-DD)</summary>

Tujuan: [...]

Yang dikerjain:

- [ ] ...
- [ ] ...

Hasil: [...]

Kendala: [...]

Dokumentasi: `docs/hari-05/`

Status: Belum mulai / Berjalan / Selesai

</details>

<details>
<summary>Hari 6 [...] (Tanggal: YYYY-MM-DD)</summary>

Tujuan: [...]

Yang dikerjain:

- [ ] ...
- [ ] ...

Hasil: [...]

Kendala: [...]

Dokumentasi: `docs/hari-06/`

Status: Belum mulai / Berjalan / Selesai

</details>

<details>
<summary>Hari 7 [...] (Tanggal: YYYY-MM-DD)</summary>

Tujuan: [...]

Yang dikerjain:

- [ ] ...
- [ ] ...

Hasil: [...]

Kendala: [...]

Dokumentasi: `docs/hari-07/`

Status: Belum mulai / Berjalan / Selesai

</details>

### Rekap akhir minggu

Ringkasan: [...]

Yang berhasil: [...]

Yang belum / lanjutannya: [...]

## Dokumentasi

Foto, video, dan log serial disimpan di folder `docs/` biar README tidak penuh:

```text
docs/
├── hari-01/
├── hari-02/
├── hari-03/
├── hari-04/
├── hari-05/
├── hari-06/
├── hari-07/
└── final/
```

Penamaan filenya misal `docs/hari-01/foto-rangkaian.jpg`, `docs/hari-01/log-serial.txt`, `docs/final/demo.mp4`. Kalau mau nampilin gambar di README tinggal pakai:

```markdown
![Foto rangkaian hari pertama](docs/hari-01/foto-rangkaian.jpg)
```
