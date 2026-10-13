# Modul 4 - Bird's Eye View & Kalibrasi Kamera dengan OpenCV

---

Modul ini menjelaskan teknik transformasi perspektif Bird's Eye View (BEV),
kalibrasi single camera (pinhole model), dan monocular depth estimation menggunakan library OpenCV di C++.
Terdapat tiga program yang mencakup transformasi BEV dari gambar jalan,
kalibrasi single camera dengan chessboard pattern, dan monocular depth estimation berbasis Pinhole Camera Model.

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

```
4-bev/
├── 1_bev.cpp       # Program transformasi Bird's Eye View
├── 2_calib.cpp     # Program kalibrasi single camera (pinhole model)
├── 3_depth.cpp     # Program monocular depth estimation (pinhole model)
├── yaml/
│   ├── kalibrasi.yaml   # Output hasil kalibrasi intrinsik kamera
│   └── depth.yaml       # Output data depth 3D terhitung
└── README.md       # Dokumentasi modul
```

Asset yang digunakan oleh ketiga program terdapat di folder `assets/`:

```
assets/
├── photos/
│   └── road3.jpeg
├── calib/
│   └── left01.jpg - left14.jpg    # Gambar kalibrasi single camera
└── result/
    └── 4-bev/
        ├── BEV_view.jpg
        ├── Segmented_Image.jpg
        └── depth_pinhole.jpg
```

---

## 1. 1_bev.cpp - Transformasi Bird's Eye View

Program ini membaca gambar jalan, mendeteksi garis kuning dan putih menggunakan
color thresholding (HLS), kemudian melakukan transformasi perspektif untuk
menghasilkan tampilan Bird's Eye View.

### Cara Compile

```bash
g++ 1_bev.cpp -o 1_bev $(pkg-config --cflags --libs opencv4)
```

### Cara Jalankan

```bash
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

## 2. 2_calib.cpp - Live USB Webcam Calibration (Pinhole Camera Model)

Program ini melakukan kalibrasi kamera tunggal (single camera / monocular) secara **live interaktif** menggunakan **USB Webcam**.
Kalibrasi menghasilkan matriks intrinsik kamera ($K$), focal length ($f_x, f_y$), titik utama ($c_x, c_y$), dan koefisien distorsi yang disimpan ke file `yaml/kalibrasi.yaml`.

### Kontrol Tombol Interaktif

- **`[SPASI]`**: Mengambil foto snapshot saat chessboard terdeteksi.
- **`[C]`**: Menjalankan perhitungan kalibrasi `cv::calibrateCamera()` (setelah mengambil minimal 5–10 foto).
- **`[ESC]`**: Batal / keluar dari program.

### Cara Compile & Jalankan

```bash
g++ 2_calib.cpp -o 2_calib $(pkg-config --cflags --libs opencv4)
./2_calib
```

### Penjelasan Langkah Kode

1. **Inisialisasi USB Webcam**: Membuka kamera `cv::VideoCapture(0)` dan mengunci resolusi ke $640 \times 480$.
2. **Interactive Frame Capture**: Membaca frame live, mendeteksi sudut chessboard (`cv::findChessboardCorners`), dan menggambar overlay pola jika terdeteksi.
3. **Penyimpanan Snapshot**: Saat tombol `SPASI` ditekan, koordinat 2D sudut disempurnakan (`cv::cornerSubPix`) dan disimpan ke memori.
4. **Kalibrasi & Simpan YAML**: Menghitung `cv::calibrateCamera` dan menyimpan matriks $K$ serta nilai $f_x, f_y, c_x, c_y$ ke `yaml/kalibrasi.yaml`.

---

## 3. 3_depth.cpp - Live Monocular Depth Estimation (Pinhole Camera Model)

Program ini menghitung estimasi jarak/kedalaman ($Z$) dan posisi 3D ($X, Y, Z$) secara **real-time** dari aliran video **USB Webcam** menggunakan **Pinhole Camera Model**.

### Kontrol Tombol Interaktif

- **`[S]`**: Menyimpan foto snapshot visualisasi saat ini ke `assets/result/4-bev/depth_pinhole.jpg` dan data log ke `yaml/depth.yaml`.
- **`[ESC]` / `[Q]`**: Keluar dari video stream.

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
   - Jarak fokus lensa kamera dalam piksel, diperoleh dari hasil kalibrasi webcam (`2_calib.cpp`) pada matriks intrinsik $K$:

$$
K = \begin{bmatrix} f_x & 0 & c_x \\\\ 0 & f_y & c_y \\\\ 0 & 0 & 1 \end{bmatrix}
$$

2. **$c_x$ & $c_y$ (*Principal Point / Titik Pusat Optik*)**:
   - Titik pusat optik lensa pada sumbu horizontal ($c_x$) dan vertikal ($c_y$). Digunakan untuk menghitung pergeseran koordinat 3D ($X, Y$):

$$X = \frac{(x_{\text{pixel}} - c_x) \cdot Z}{f_x}, \quad Y = \frac{(y_{\text{pixel}} - c_y) \cdot Z}{f_y}$$

3. **$W_{\text{real}}$ (*Real World Width*)**:
   - Ukuran lebar fisik objek referensi dalam mm (pada contoh ini $W_{\text{real}} = 200\text{ mm}$).

4. **$w_{\text{pixel}}$ (*Image Pixel Width*)**:
   - Ukuran lebar proyeksi objek yang terukur pada frame live USB webcam dalam piksel.

5. **$Z$ (*Depth / Distance*)**:
   - Jarak kedalaman dari kamera ke permukaan objek dalam mm atau meter.

### Cara Compile & Jalankan

```bash
g++ 3_depth.cpp -o 3_depth $(pkg-config --cflags --libs opencv4)
./3_depth
```

### Penjelasan Langkah Kode

1. **Load Parameter Kalibrasi**: Membaca $f_x, f_y, c_x, c_y$ dari `yaml/kalibrasi.yaml`.
2. **Live Capture**: Membuka stream USB webcam (`cv::VideoCapture cap(0)`).
3. **Deteksi Objek & Pixel Width**: Mendeteksi objek referensi pada frame live dan mengukur $w_{\text{pixel}}$.
4. **Kalkulasi Real-Time**: Menghitung $Z = \frac{f_x \cdot W_{\text{real}}}{w_{\text{pixel}}}$ dan posisi 3D ($X, Y, Z$) di setiap frame.
5. **HUD Overlay Video Stream**: Menampilkan bounding box, titik pusat, dan label kedalaman ($Z$) secara real-time pada jendela `cv::imshow`.

---

## Compile Semua Program Sekaligus

```bash
g++ 1_bev.cpp -o 1_bev $(pkg-config --cflags --libs opencv4) && \
g++ 2_calib.cpp -o 2_calib $(pkg-config --cflags --libs opencv4) && \
g++ 3_depth.cpp -o 3_depth $(pkg-config --cflags --libs opencv4)
```

## Membersihkan File Binary

```bash
rm -f 1_bev 2_calib 3_depth
```

## Catatan

- Hubungkan USB Webcam ke port USB komputer sebelum menjalankan `2_calib` atau `3_depth`.
- Jalankan `2_calib` terlebih dahulu untuk menghasilkan `yaml/kalibrasi.yaml` khas USB webcam tersebut.
- Pengujian `3_depth` akan menampilkan nilai $Z$ yang berubah secara dinamis saat objek didekatkan/dijauhkan dari webcam.
