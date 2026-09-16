/*
    * Single Camera Calibration via Live USB Webcam (Pinhole Camera Model)
*/

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <string>

int main() {
	cv::Size boardSize(8, 6); // 8x6 inner corners (dari 9x7 kotak)
	float squareSize = 23.0f; // 23 mm
	std::string outputPath = "yaml/kalibrasi.yaml";

	// Pakai 0,1 atau 2 tergantung webcam yang terpasang di PC/Laptop
	int cameraID = 0;
	cv::VideoCapture cap(cameraID);
	if (!cap.isOpened()) {
		std::cerr << "Gagal membuka USB Webcam pada index " << cameraID << std::endl;
		return -1;
	}

	cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
	cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

	std::vector<cv::Point3f> objp;
	for (int i = 0; i < boardSize.height; i++)
		for (int j = 0; j < boardSize.width; j++)
			objp.push_back(cv::Point3f(j * squareSize, i * squareSize, 0.0f));

	std::vector<std::vector<cv::Point3f>> objectPoints;
	std::vector<std::vector<cv::Point2f>> imagePoints;
	cv::Size imageSize;

	cv::Mat frame, gray;
	int capturedCount = 0;
	int targetSnapshots = 25;

	std::cout << " KALIBRASI LIVE USB WEBCAM " << std::endl;
	std::cout << "- Arahkan chessboard (8x6 inner corners) ke webcam." << std::endl;
	std::cout << "- Tekan [SPASI] untuk mengambil snapshot foto." << std::endl;
	std::cout << "- Tekan [C] untuk memproses kalibrasi (min " << targetSnapshots << " foto)." << std::endl;
	std::cout << "- Tekan [ESC] untuk keluar/batal." << std::endl;

	while (true) {
		cap >> frame;
		if (frame.empty()) {
			std::cerr << "Frame kosong dari webcam." << std::endl;
			break;
		}

		if (imageSize.empty()) imageSize = frame.size();

		cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

		std::vector<cv::Point2f> corners;
		int flags = cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE | cv::CALIB_CB_FAST_CHECK;
		bool found = cv::findChessboardCorners(gray, boardSize, corners, flags);

		cv::Mat display = frame.clone();
		if (found) {
			cv::drawChessboardCorners(display, boardSize, corners, found);
		}

		char countStr[128];
		snprintf(countStr, sizeof(countStr), "Foto Terambil: %d / %d", capturedCount, targetSnapshots);
		cv::putText(display, countStr, cv::Point(20, 35), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
		cv::putText(display, "[SPASI]: Ambil Foto | [C]: Hitung Kalibrasi | [ESC]: Batal", cv::Point(20, display.rows - 20),
			cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);

		cv::imshow("Live USB Webcam Calibration", display);

		char key = (char)cv::waitKey(30);
		if (key == 27) { // ESC
			std::cout << "Kalibrasi dibatalkan oleh pengguna." << std::endl;
			return 0;
		}
		else if (key == ' ' && found) {
			cv::TermCriteria criteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.001);
			cv::cornerSubPix(gray, corners, cv::Size(11, 11), cv::Size(-1, -1), criteria);

			objectPoints.push_back(objp);
			imagePoints.push_back(corners);
			capturedCount++;

			std::cout << "--> Snapshot ke-" << capturedCount << " berhasil terambil!" << std::endl;
		}
		else if ((key == 'c' || key == 'C') || capturedCount >= targetSnapshots) {
			if (capturedCount < 5) {
				std::cout << "Foto belum cukup! Ambil minimal " << targetSnapshots << " foto dulu." << std::endl;
				continue;
			}
			break;
		}
	}

	cap.release();
	cv::destroyAllWindows();

	std::cout << "\nMemproses Kalibrasi Kamera dari " << capturedCount << " snapshot..." << std::endl;

	cv::Mat cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
	cv::Mat distCoeffs = cv::Mat::zeros(8, 1, CV_64F);
	std::vector<cv::Mat> rvecs, tvecs;

	double rms = cv::calibrateCamera(objectPoints, imagePoints, imageSize,
		cameraMatrix, distCoeffs, rvecs, tvecs);

	cv::FileStorage fs(outputPath, cv::FileStorage::WRITE);
	if (!fs.isOpened()) {
		std::cerr << "Gagal membuka file output: " << outputPath << std::endl;
		return -1;
	}

	fs << "image_width" << imageSize.width;
	fs << "image_height" << imageSize.height;
	fs << "board_width" << boardSize.width;
	fs << "board_height" << boardSize.height;
	fs << "square_size" << squareSize;
	fs << "images_used" << (int)objectPoints.size();
	fs << "camera_matrix" << cameraMatrix;
	fs << "distortion_coefficients" << distCoeffs;
	fs << "focal_length_x" << cameraMatrix.at<double>(0, 0);
	fs << "focal_length_y" << cameraMatrix.at<double>(1, 1);
	fs << "principal_point_x" << cameraMatrix.at<double>(0, 2);
	fs << "principal_point_y" << cameraMatrix.at<double>(1, 2);
	fs << "reprojection_error" << rms;
	fs.release();

	std::cout << "=== HASIL KALIBRASI WEBCAM ===" << std::endl;
	std::cout << "RMS Reprojection Error: " << rms << " px" << std::endl;
	std::cout << "Focal Length (fx, fy): (" << cameraMatrix.at<double>(0, 0) << ", " << cameraMatrix.at<double>(1, 1) << ") px" << std::endl;
	std::cout << "Principal Point (cx, cy): (" << cameraMatrix.at<double>(0, 2) << ", " << cameraMatrix.at<double>(1, 2) << ") px" << std::endl;
	std::cout << "Hasil kalibrasi tersimpan di: " << outputPath << std::endl;

	return 0;
}
