/*
    * Monocular Depth Estimation via Live USB Webcam (Pinhole Camera Model)
    * Z = (fx * W_real) / w_pixel
*/

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <string>

int main() {
	std::string calibPath = "yaml/kalibrasi.yaml";
	std::string outputPath = "../assets/result/4-bev/depth_pinhole.jpg";
	std::string yamlPath = "yaml/depth.yaml";

	// 1. Baca parameter kalibrasi kamera dari file YAML
	cv::FileStorage fs(calibPath, cv::FileStorage::READ);
	if (!fs.isOpened()) {
		std::cerr << "Gagal membuka file kalibrasi: " << calibPath << std::endl;
		return -1;
	}

	double fx = fs["focal_length_x"];
	double fy = fs["focal_length_y"];
	double cx = fs["principal_point_x"];
	double cy = fs["principal_point_y"];
	int boardWidth = fs["board_width"];
	int boardHeight = fs["board_height"];
	float squareSize = fs["square_size"];
	fs.release();

	// Ukuran fisik asli objek referensi (lebar total chessboard dalam mm)
	float realWidthMM = (boardWidth - 1) * squareSize;

	// 2. Buka Live USB Webcam
	int cameraID = 0;
	cv::VideoCapture cap(cameraID);
	if (!cap.isOpened()) {
		std::cerr << "Gagal membuka USB Webcam pada index " << cameraID << std::endl;
		return -1;
	}

	cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
	cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

	cv::Mat frame, gray;
	cv::Size boardSize(boardWidth, boardHeight);

	std::cout << "=== LIVE MONOCULAR DEPTH ESTIMATION (WEBCAM) ===" << std::endl;
	std::cout << "Focal Length (fx, fy): (" << fx << ", " << fy << ") px" << std::endl;
	std::cout << "Principal Point (cx, cy): (" << cx << ", " << cy << ") px" << std::endl;
	std::cout << "Lebar Objek Referensi (W_real): " << realWidthMM << " mm" << std::endl;
	std::cout << "- Arahkan chessboard / objek referensi di depan webcam." << std::endl;
	std::cout << "- Tekan [S] untuk menyimpan foto snapshot & data depth ke YAML." << std::endl;
	std::cout << "- Tekan [ESC] atau [Q] untuk keluar." << std::endl;

	while (true) {
		cap >> frame;
		if (frame.empty()) {
			std::cerr << "Frame kosong dari webcam." << std::endl;
			break;
		}

		cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

		// 3. Deteksi Objek Referensi pada Frame Live (Chessboard)
		std::vector<cv::Point2f> corners;
		int flags = cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE | cv::CALIB_CB_FAST_CHECK;
		bool found = cv::findChessboardCorners(gray, boardSize, corners, flags);

		cv::Mat display = frame.clone();

		if (found) {
			cv::TermCriteria criteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.001);
			cv::cornerSubPix(gray, corners, cv::Size(11, 11), cv::Size(-1, -1), criteria);

			// Mengukur lebar objek pada piksel citra (w_pixel)
			cv::Point2f topLeft = corners[0];
			cv::Point2f topRight = corners[boardWidth - 1];
			float pixelWidth = cv::norm(topRight - topLeft);

			// 4. Hitung Jarak/Kedalaman Z (Pinhole Camera Model): Z = (fx * W_real) / w_pixel
			double depthZ = (fx * realWidthMM) / pixelWidth;

			// Pusat objek dalam piksel
			cv::Rect bbox = cv::boundingRect(corners);
			cv::Point2f centerPixel(bbox.x + bbox.width / 2.0f, bbox.y + bbox.height / 2.0f);

			// Hitung koordinat 3D (X, Y, Z) dalam mm
			double posX = ((centerPixel.x - cx) * depthZ) / fx;
			double posY = ((centerPixel.y - cy) * depthZ) / fy;

			// 5. Visualisasi Bounding Box, Pusat Objek, dan HUD Text
			cv::drawChessboardCorners(display, boardSize, corners, found);
			cv::rectangle(display, bbox, cv::Scalar(0, 255, 0), 2);
			cv::circle(display, centerPixel, 5, cv::Scalar(0, 0, 255), -1);

			char depthStr[64], posStr[64];
			snprintf(depthStr, sizeof(depthStr), "Depth Z: %.1f mm (%.2f m)", depthZ, depthZ / 1000.0);
			snprintf(posStr, sizeof(posStr), "3D Pos: X=%.1f, Y=%.1f, Z=%.1f mm", posX, posY, depthZ);

			cv::putText(display, depthStr, cv::Point(bbox.x, std::max(bbox.y - 25, 25)),
				cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);
			cv::putText(display, posStr, cv::Point(bbox.x, std::max(bbox.y - 8, 40)),
				cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 0), 1);
		} else {
			cv::putText(display, "Arahkan Chessboard ke Kamera...", cv::Point(20, 30),
				cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 255), 2);
		}

		cv::putText(display, "[S]: Simpan Log Depth | [ESC/Q]: Keluar", cv::Point(20, display.rows - 15),
			cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);

		cv::imshow("Live Monocular Depth - USB Webcam", display);

		char key = (char)cv::waitKey(30);
		if (key == 27 || key == 'q' || key == 'Q') { // ESC or Q
			break;
		}
		else if ((key == 's' || key == 'S') && found) {
			// Simpan foto snapshot hasil
			cv::imwrite(outputPath, display);

			// Simpan data depth ke YAML
			cv::FileStorage fsOut(yamlPath, cv::FileStorage::WRITE);
			if (fsOut.isOpened()) {
				cv::Point2f topLeft = corners[0];
				cv::Point2f topRight = corners[boardWidth - 1];
				float pixelWidth = cv::norm(topRight - topLeft);
				double depthZ = (fx * realWidthMM) / pixelWidth;
				cv::Rect bbox = cv::boundingRect(corners);
				cv::Point2f centerPixel(bbox.x + bbox.width / 2.0f, bbox.y + bbox.height / 2.0f);
				double posX = ((centerPixel.x - cx) * depthZ) / fx;
				double posY = ((centerPixel.y - cy) * depthZ) / fy;

				fsOut << "reference_width_real_mm" << realWidthMM;
				fsOut << "reference_width_pixel" << pixelWidth;
				fsOut << "focal_length_x" << fx;
				fsOut << "depth_z_mm" << depthZ;
				fsOut << "depth_z_meter" << (depthZ / 1000.0);
				fsOut << "position_3d_x_mm" << posX;
				fsOut << "position_3d_y_mm" << posY;
				fsOut << "position_3d_z_mm" << depthZ;
				fsOut.release();
			}

			std::cout << "--> Snapshot & Data Depth tersimpan di: " << outputPath << " dan " << yamlPath << std::endl;
		}
	}

	cap.release();
	cv::destroyAllWindows();
	return 0;
}
