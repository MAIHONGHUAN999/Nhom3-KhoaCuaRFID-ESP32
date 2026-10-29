# NHÓM 3 – KHÓA CỬA THÔNG MINH RFID + PIN

## 1. Giới thiệu đề tài

Đồ án xây dựng hệ thống khóa cửa thông minh sử dụng vi điều khiển ESP32, cho phép xác thực người dùng bằng thẻ RFID hoặc mã PIN thông qua bàn phím 4×4.

Hệ thống được thiết kế và mô phỏng trên nền tảng Wokwi.

## 2. Thành phần phần cứng

- ESP32 DevKit V4
- Module RFID MFRC522
- LCD1602 giao tiếp I2C
- Bàn phím ma trận 4×4
- Động cơ Servo
- LED xanh và LED đỏ
- Buzzer
- Điện trở và dây kết nối

## 3. Chức năng chính

- Mở khóa bằng thẻ RFID hợp lệ.
- Mở khóa bằng mã PIN hợp lệ.
- Hiển thị thông báo trên LCD1602.
- Điều khiển Servo đóng/mở cửa.
- Tự động đóng cửa sau 5 giây.
- Cảnh báo khi xác thực không hợp lệ.
- Khóa tạm thời 30 giây khi nhập sai 3 lần.
- Chặn cả RFID và PIN trong thời gian khóa.
- Tự động phục hồi sau khi hết thời gian khóa.
- Đếm số lượt vào hợp lệ.
- Ghi nhật ký hoạt động trên Serial Monitor.

## 4. Công nghệ và thư viện

- Arduino Framework
- Ngôn ngữ C/C++
- MFRC522
- LiquidCrystal_I2C
- ESP32Servo
- Keypad
- Wokwi Simulator

## 5. Cấu trúc mã nguồn

- `sketch.ino`: Chương trình điều khiển ESP32.
- `diagram.json`: Sơ đồ kết nối phần cứng trên Wokwi.
- `libraries.txt`: Danh sách thư viện.
- `README.md`: Tài liệu giới thiệu dự án.

## 6. Mô phỏng trực tuyến

Truy cập dự án Wokwi:

https://wokwi.com/projects/479217550259595265

## 7. Kết quả kiểm thử

Hệ thống đã được kiểm thử các tình huống:

- Khởi động hệ thống.
- Xác thực RFID hợp lệ và không hợp lệ.
- Tự động đóng cửa sau 5 giây.
- Khóa tạm sau 3 lần xác thực sai.
- Chặn RFID và PIN trong thời gian khóa.
- Phục hồi hoạt động sau 30 giây khóa tạm.

Các tình huống trên đã đạt theo kết quả mô phỏng và nhật ký Serial Monitor.

## 8. Nhóm thực hiện

**Nhóm 3 – Đồ án Khóa cửa thông minh RFID + PIN**

Thông tin thành viên và giảng viên hướng dẫn sẽ được bổ sung theo báo cáo chính thức.
