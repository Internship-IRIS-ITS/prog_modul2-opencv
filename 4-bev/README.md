# Modul 4 - Bird's Eye View & Kalibrasi Kamera dengan OpenCV

---

Modul ini menjelaskan teknik transformasi perspektif Bird's Eye View (BEV),
kalibrasi single camera (pinhole model), dan monocular depth estimation menggunakan library OpenCV di C++.

Dalam modul ini:
1. **`1_bev.cpp`**: Program contoh implementasi transformasi Bird's Eye View dari gambar jalan (disediakan sebagai bahan materi).
2. **`2_calib.cpp`** *(Tugas Mandiri 1)*: Program kalibrasi live webcam berbasis chessboard pattern yang **dibuat secara mandiri oleh peserta**.
3. **`3_depth.cpp`** *(Tugas Mandiri 2)*: Program live monocular depth estimation berbasis Pinhole Camera Model yang **dibuat secara mandiri oleh peserta**.

## Prasyarat

- Compiler C++ (g++)
- Library OpenCV 4
- pkg-config (untuk mempermudah proses compile)

### Instalasi OpenCV (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install libopencv-dev
```

## Struktur Folder

```text
4-bev/
├── 1_bev.cpp       # Program transformasi Bird's Eye View (Disediakan)
├── 2_calib.cpp     # [Tugas Mandiri] Program kalibrasi kamera (dibuat oleh peserta)
├── 3_depth.cpp     # [Tugas Mandiri] Program monocular depth estimation (dibuat oleh peserta)
├── yaml/
│   ├── kalibrasi.yaml   # Output hasil kalibrasi intrinsik kamera (dari 2_calib)
│   └── depth.yaml       # Output data depth 3D terhitung (dari 3_depth)
└── README.md       # Dokumentasi modul & panduan tugas mandiri
```

Asset yang digunakan oleh program terdapat di folder `assets/`:

```text
assets/
├── photos/
│   └── road3.jpeg                 # Gambar jalan untuk pengujian 1_bev.cpp
└── result/
    └── 4-bev/
        ├── BEV_view.jpg           # Output hasil transformasi BEV
        ├── Segmented_Image.jpg    # Output segmentasi jalan
        └── depth_pinhole.jpg      # Output snapshot pengujian depth
```

---

## 1. 1_bev.cpp - Transformasi Bird's Eye View

Program ini membaca gambar jalan, mendeteksi garis kuning dan putih menggunakan
color thresholding (HLS), kemudian melakukan transformasi perspektif untuk
menghasilkan tampilan Bird's Eye View.

### Cara Compile & Jalankan

```bash
g++ 1_bev.cpp -o 1_bev $(pkg-config --cflags --libs opencv4)
./1_bev
```

### Cara Jalankan (tanpa pkg-config)

```bash
g++ 1_bev.cpp -o 1_bev -I/usr/include/opencv4 -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lopencv_highgui -lopencv_calib3d
./1_bev
```

### Penjelasan Kode

1. `cv::imread("../assets/photos/road3.jpeg")` - Membaca gambar jalan dari path
   yang ditentukan dan menyimpannya ke dalam objek `cv::Mat`.
2. `cv::resize(image, image, cv::Size(...))` - Mengubah ukuran gambar menjadi
   setengah dari ukuran asli.
3. `cv::Rect ROI_area(...)` - Mendefinisikan Region of Interest untuk area jalan
   bagian bawah gambar.
4. `cv::cvtColor(image(ROI_area), yellow_thresholded, cv::COLOR_BGR2HLS)` -
   Mengkonversi ROI ke color space HLS untuk deteksi garis kuning.
5. `cv::inRange(...)` - Melakukan thresholding untuk memisahkan garis kuning
   (`Scalar(15, 30, 80)` - `Scalar(35, 200, 255)`) dan garis putih
   (`Scalar(0, 180, 0)` - `Scalar(255, 255, 60)`).
6. `cv::bitwise_or(yellow_thresholded, white_thresholded, combined)` - Menggabungkan
   hasil deteksi garis kuning dan putih.
7. `cv::erode()` dan `cv::dilate()` - Operasi morfologi untuk membersihkan noise.
8. `cv::findContours(...)` - Mendeteksi kontur dari hasil thresholding.
9. `cv::approxPolyDP(...)` - Mengambil titik sudut dari kontur terbesar.
10. `cv::getPerspectiveTransform(src, dst)` - Menghitung matriks transformasi
    perspektif dari 4 titik sumber ke 4 titik tujuan.
11. `cv::warpPerspective(...)` - Menerapkan transformasi perspektif untuk
    menghasilkan Bird's Eye View.
12. `cv::imwrite(...)` - Menyimpan hasil BEV dan segmentasi ke folder result.

---

## 2. [Tugas Mandiri 1] 2_calib.cpp - Live USB Webcam Calibration (Pinhole Camera Model)

> [!IMPORTANT]
> File `2_calib.cpp` **tidak disediakan** di repositori ini. Peserta diharapkan membuat dan mengimplementasikan program kalibrasi ini secara mandiri sesuai dengan panduan spesifikasi di bawah.

Program ini bertujuan untuk melakukan kalibrasi kamera tunggal (single camera / monocular) secara **live interaktif** menggunakan **USB Webcam / Kamera Laptop**.
Kalibrasi ini penting untuk menghitung matriks intrinsik kamera ($K$), focal length ($f_x, f_y$), titik utama ($c_x, c_y$), dan koefisien distorsi, kemudian menyimpannya ke file `yaml/kalibrasi.yaml`.

### Spesifikasi Fungsional

1. **Konfigurasi Chessboard**:
   - Definisikan jumlah sudut dalam pola papan catur (*inner corners*). Standar acuan: `cv::Size(8, 6)` (dari pola catur $9 \times 7$ kotak) dengan ukuran fisik kotak `squareSize = 23.0f` (mm) atau disesuaikan dengan pola cetak masing-masing.
2. **Inisialisasi USB Webcam**:
   - Buka video stream webcam (`cv::VideoCapture cap(0)`).
   - Set resolusi frame standar (misal: $640 \times 480$ piksel).
3. **Deteksi Sudut Chessboard Interaktif**:
   - Konversi frame ke grayscale (`cv::cvtColor`).
   - Deteksi koordinat 2D sudut pola papan catur dengan `cv::findChessboardCorners`.
   - Gambar visualisasi overlay sudut yang terdeteksi dengan `cv::drawChessboardCorners`.
4. **Penyimpanan Snapshot 2D–3D**:
   - Buat kontrol tombol (rekomendasi: **`[SPASI]`**) untuk mengambil snapshot saat pola terdeteksi.
   - Sempurnakan akurasi titik sudut hingga sub-piksel dengan `cv::cornerSubPix`.
   - Simpan titik 3D objek (`objectPoints`) dan titik 2D citra (`imagePoints`).
   - Kumpulkan minimal 15–25 snapshot dari berbagai variasi jarak dan orientasi sudut kemiringan.
5. **Kalkulasi Kalibrasi**:
   - Buat pemicu tombol (rekomendasi: **`[C]`**) untuk menjalankan fungsi `cv::calibrateCamera(...)`.
   - Hitung matriks intrinsik kamera ($K$), koefisien distorsi lensa, dan evaluasi nilai reprojection error (RMS).
6. **Ekspor Parameter ke YAML**:
   - Tulis seluruh hasil kalibrasi ke file `yaml/kalibrasi.yaml` menggunakan `cv::FileStorage` (mode `WRITE`).
   - Parameter yang wajib disimpan: `camera_matrix`, `distortion_coefficients`, `focal_length_x`, `focal_length_y`, `principal_point_x`, `principal_point_y`, `board_width`, `board_height`, `square_size`.

### Rekomendasi Kontrol Tombol Interaktif

- **`[SPASI]`**: Mengambil foto snapshot saat chessboard terdeteksi.
- **`[C]`**: Menjalankan perhitungan kalibrasi `cv::calibrateCamera()` (setelah mengambil minimal snapshot yang cukup).
- **`[ESC]`**: Batal / keluar dari program.

### Cara Compile & Jalankan (Setelah Kode Dibuat)

```bash
g++ 2_calib.cpp -o 2_calib $(pkg-config --cflags --libs opencv4)
./2_calib
```

---

## 3. [Tugas Mandiri 2] 3_depth.cpp - Live Monocular Depth Estimation (Pinhole Camera Model)

> [!IMPORTANT]
> File `3_depth.cpp` **tidak disediakan** di repositori ini. Peserta diharapkan membuat dan mengimplementasikan program estimasi kedalaman ini secara mandiri dengan menerapkan rumus matematika Pinhole Camera Model.

Program ini bertujuan untuk menghitung estimasi jarak/kedalaman ($Z$) dan posisi 3D ($X, Y, Z$) secara **real-time** dari aliran video **USB Webcam** menggunakan **Pinhole Camera Model** dengan memanfaatkan hasil kalibrasi kamera (`yaml/kalibrasi.yaml`).

### Alur Penurunan Rumus Pinhole Camera Model

Pinhole Camera Model memanfaatkan prinsip **kesamaan segitiga** (*triangle similarity*) antara objek fisik di ruang 3D dunia nyata dan bayangan/proyeksi objek pada bidang citra 2D (sensor kamera):

```text
       Objek Fisik (W_real)
      |-------------------|
       \                 /
        \               /
         \  Jarak (Z)  /
          \           /
           \         /
            \       /
             \  •  /  <--- Pusat Optik Kamera (Camera Center / Focal Point)
              /   \
             /     \
            /_______ \
       Bayangan Piksel (w_pixel)
       [Focal Length (fx)]
```

Dari hubungan kesamaan segitiga di atas, perbandingan antara ukuran fisik dan ukuran citra adalah konstan:

$$\frac{w_{\text{pixel}}}{f_x} = \frac{W_{\text{real}}}{Z}$$

Dengan mengisolasi $Z$, kita mendapatkan rumus kedalaman/jarak objek:

$$Z = \frac{f_x \cdot W_{\text{real}}}{w_{\text{pixel}}}$$

### Keterangan Parameter & Variabel

1. **$f_x$ & $f_y$ (*Focal Length in Pixels*)**:
   - Jarak fokus lensa kamera dalam piksel, diperoleh dari hasil kalibrasi webcam (`yaml/kalibrasi.yaml`) pada matriks intrinsik $K$:

$$
K = \begin{bmatrix} f_x & 0 & c_x \\\\ 0 & f_y & c_y \\\\ 0 & 0 & 1 \end{bmatrix}
$$

2. **$c_x$ & $c_y$ (*Principal Point / Titik Pusat Optik*)**:
   - Titik pusat optik lensa pada sumbu horizontal ($c_x$) dan vertikal ($c_y$). Digunakan untuk menghitung pergeseran koordinat 3D ($X, Y$):

$$X = \frac{(x_{\text{pixel}} - c_x) \cdot Z}{f_x}, \quad Y = \frac{(y_{\text{pixel}} - c_y) \cdot Z}{f_y}$$

3. **$W_{\text{real}}$ (*Real World Width*)**:
   - Ukuran lebar fisik objek referensi dalam mm (misalnya lebar total pola chessboard atau benda acuan lainnya).

4. **$w_{\text{pixel}}$ (*Image Pixel Width*)**:
   - Ukuran lebar proyeksi objek yang terukur pada frame live USB webcam dalam satuan piksel.

5. **$Z$ (*Depth / Distance*)**:
   - Jarak kedalaman dari kamera ke permukaan objek dalam mm atau meter.

### Spesifikasi Fungsional

1. **Load Parameter Kalibrasi**: Membaca $f_x, f_y, c_x, c_y$ dari `yaml/kalibrasi.yaml` menggunakan `cv::FileStorage` (tidak boleh di-*hardcode*).
2. **Live Capture**: Membuka stream USB webcam (`cv::VideoCapture cap(0)`).
3. **Deteksi Objek & Pixel Width**: Mendeteksi objek referensi pada frame live dan mengukur $w_{\text{pixel}}$ (misal jarak antartitik terluar).
4. **Kalkulasi Real-Time**: Menghitung jarak kedalaman $Z = \frac{f_x \cdot W_{\text{real}}}{w_{\text{pixel}}}$ dan koordinat lateral $(X, Y)$ di setiap frame.
5. **HUD Overlay Video Stream**: Menampilkan bounding box, titik pusat (*centroid*), dan label kedalaman ($Z$) serta posisi 3D ($X, Y, Z$) secara real-time pada jendela tampilan `cv::imshow`.
6. **Simpan Snapshot & Log**: Sediakan tombol interaktif (rekomendasi: **`[S]`**) untuk menyimpan snapshot visualisasi ke `assets/result/4-bev/depth_pinhole.jpg` dan mencatat hasil kalkulasi ke `yaml/depth.yaml`.

### Rekomendasi Kontrol Tombol Interaktif

- **`[S]`**: Menyimpan foto snapshot visualisasi saat ini ke `assets/result/4-bev/depth_pinhole.jpg` dan data log ke `yaml/depth.yaml`.
- **`[ESC]` / `[Q]`**: Keluar dari video stream.

### Cara Compile & Jalankan (Setelah Kode Dibuat)

```bash
g++ 3_depth.cpp -o 3_depth $(pkg-config --cflags --libs opencv4)
./3_depth
```

---

## Compile Program

Untuk mengompilasi program `1_bev.cpp` yang telah disediakan:

```bash
g++ 1_bev.cpp -o 1_bev $(pkg-config --cflags --libs opencv4)
```

Setelah Anda selesai membuat file tugas mandiri `2_calib.cpp` dan `3_depth.cpp`, seluruh program dapat dikompilasi secara bersamaan:

```bash
g++ 1_bev.cpp -o 1_bev $(pkg-config --cflags --libs opencv4) && \
g++ 2_calib.cpp -o 2_calib $(pkg-config --cflags --libs opencv4) && \
g++ 3_depth.cpp -o 3_depth $(pkg-config --cflags --libs opencv4)
```

## Membersihkan File Binary

```bash
rm -f 1_bev 2_calib 3_depth
```

---

## Catatan

- File `2_calib.cpp` dan `3_depth.cpp` **sengaja tidak disertakan dalam repositori** karena merupakan tugas mandiri agar peserta dapat mempraktikkan proses kalibrasi kamera dan pemodelan pinhole secara langsung.
- Hubungkan USB Webcam ke port USB komputer sebelum menjalankan program `2_calib` atau `3_depth`.
- Jalankan `2_calib` terlebih dahulu untuk menghasilkan `yaml/kalibrasi.yaml` khusus untuk webcam yang digunakan.
- Pengujian `3_depth` akan menampilkan nilai $Z$ yang berubah secara dinamis saat objek didekatkan/dijauhkan dari webcam.
