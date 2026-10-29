
#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
#include <Keypad.h>

// ============================================
// DO AN CUOI KY - HE THONG NHUNG
// NHOM 3 - KHOA CUA THONG MINH RFID
// LE HOAI BAO - K23DTCN554
// MAI HONG HUAN - K23DTVT097
// BUOC 9 - TICH HOP KEYPAD PIN 4X4
// ============================================

// 1. KHAI BAO CHAN
#define PIN_SS       5
#define PIN_RST      27
#define PIN_SERVO    13
#define PIN_LED_G    25
#define PIN_LED_R    26
#define PIN_BUZZER   14
#define PIN_SDA      21
#define PIN_SCL      22

// 2. THONG SO HE THONG
const unsigned long T_MO_KHOA = 5000;
const unsigned long T_TU_CHOI = 2000;
const unsigned long T_KHOA_TAM = 30000;
const byte SO_LAN_SAI_TOI_DA = 3;

const char PIN_DUNG[] = "1234";

// 3. THIET BI
MFRC522 rfid(PIN_SS, PIN_RST);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo servoKhoa;

// 4. KEYPAD THEO SO DO GIANG VIEN
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {32, 33, 12, 15};
byte colPins[COLS] = {4, 16, 17, 2};

Keypad keypad = Keypad(
  makeKeymap(keys),
  rowPins,
  colPins,
  ROWS,
  COLS
);

// 5. DANH SACH THE HOP LE
struct TheHopLe {
  byte uid[4];
  const char* ten;
};

TheHopLe danhSach[] = {
  {{0x01, 0x02, 0x03, 0x04}, "LE HOAI BAO"},
  {{0x11, 0x22, 0x33, 0x44}, "MAI HONG HUAN"},
  {{0x55, 0x66, 0x77, 0x88}, "GIANG VIEN"}
};

const byte SO_THE =
  sizeof(danhSach) / sizeof(danhSach[0]);

// 6. MAY TRANG THAI
enum TrangThai {
  CHO_QUET,
  MO_KHOA,
  TU_CHOI,
  KHOA_TAM
};

TrangThai trangThai = CHO_QUET;

// 7. BIEN QUAN LY
unsigned long mocTG = 0;
unsigned long mocCoi = 0;
unsigned long mocNhipCoi = 0;

byte soLanSai = 0;
unsigned int soLuotVao = 0;

bool coiDangBat = false;
byte soTiengConLai = 0;
int tanSoCoi = 2000;
unsigned int doDaiCoi = 150;

int giayTruoc = -1;

char pinNhap[5] = "";
byte doDaiPIN = 0;

// ============================================
// 8. NHAT KY SERIAL
// ============================================

void ghiLog(const char* muc, String noiDung) {
  Serial.print("[");
  Serial.print(muc);
  Serial.print("] [");
  Serial.print(millis());
  Serial.print(" ms] ");
  Serial.println(noiDung);
}

// ============================================
// 9. CHUYEN UID THANH CHUOI
// ============================================

String chuoiUID(byte* uid, byte doDai) {
  String s = "";

  for (byte i = 0; i < doDai; i++) {
    if (uid[i] < 0x10) s += "0";
    s += String(uid[i], HEX);

    if (i < doDai - 1) s += ":";
  }

  s.toUpperCase();
  return s;
}

// ============================================
// 10. TIM THE HOP LE
// ============================================

int timThe(byte* uid, byte doDai) {
  if (doDai != 4) return -1;

  for (byte i = 0; i < SO_THE; i++) {
    bool trung = true;

    for (byte j = 0; j < 4; j++) {
      if (uid[j] != danhSach[i].uid[j]) {
        trung = false;
        break;
      }
    }

    if (trung) return i;
  }

  return -1;
}

// ============================================
// 11. COI KHONG DUNG DELAY
// ============================================

void batDauCoi(byte soTieng, int tanSo,
               unsigned int thoiGian) {
  noTone(PIN_BUZZER);

  soTiengConLai = soTieng;
  tanSoCoi = tanSo;
  doDaiCoi = thoiGian;

  coiDangBat = false;
  mocNhipCoi = millis();

  if (soTiengConLai > 0) {
    tone(PIN_BUZZER, tanSoCoi);
    coiDangBat = true;
    mocCoi = millis();
    soTiengConLai--;
  }
}

void capNhatCoi() {
  unsigned long bayGio = millis();

  if (coiDangBat) {
    if (bayGio - mocCoi >= doDaiCoi) {
      noTone(PIN_BUZZER);
      coiDangBat = false;
      mocNhipCoi = bayGio;
    }
  }
  else if (soTiengConLai > 0) {
    if (bayGio - mocNhipCoi >= 90) {
      tone(PIN_BUZZER, tanSoCoi);
      coiDangBat = true;
      mocCoi = bayGio;
      soTiengConLai--;
    }
  }
}

// ============================================
// 12. XOA DU LIEU PIN
// ============================================

void xoaPIN() {
  memset(pinNhap, 0, sizeof(pinNhap));
  doDaiPIN = 0;
}

// ============================================
// 13. HIEN THI PIN DANG NHAP
// ============================================

void hienThiPIN() {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("NHAP MA PIN:");

  lcd.setCursor(0, 1);
  lcd.print("PIN: ");

  for (byte i = 0; i < doDaiPIN; i++) {
    lcd.print("*");
  }
}

// ============================================
// 14. MAN HINH CHO
// ============================================

void manHinhCho() {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("QUET THE DE MO");

  lcd.setCursor(0, 1);
  lcd.print("Luot vao: ");
  lcd.print(soLuotVao);
}

// ============================================
// 15. VE TRANG THAI CHO
// ============================================

void veChoQuet() {
  trangThai = CHO_QUET;

  servoKhoa.write(0);

  digitalWrite(PIN_LED_G, LOW);
  digitalWrite(PIN_LED_R, LOW);

  xoaPIN();
  manHinhCho();
}

// ============================================
// 16. MO KHOA DUNG CHUNG RFID VA PIN
// ============================================

void moKhoa(String phuongThuc,
           String chuThe,
           String uid = "") {
  soLanSai = 0;
  xoaPIN();

  ghiLog(
    "INFO",
    "DAT LAI BO DEM SAI = 0"
  );

  trangThai = MO_KHOA;
  soLuotVao++;

  servoKhoa.write(90);

  digitalWrite(PIN_LED_G, HIGH);
  digitalWrite(PIN_LED_R, LOW);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MO KHOA");

  lcd.setCursor(0, 1);
  lcd.print(chuThe.substring(0, 16));

  batDauCoi(1, 2000, 150);
  mocTG = millis();

  String log = "PHUONG THUC=" + phuongThuc;

  if (uid.length() > 0) {
    log += " | UID=" + uid;
  }

  log += " | KET QUA=HOP LE";
  log += " | CHU THE=" + chuThe;
  log += " | LUOT VAO=";
  log += String(soLuotVao);

  ghiLog("INFO", log);
}

// ============================================
// 17. TU CHOI DUNG CHUNG RFID VA PIN
// ============================================

void tuChoi(String phuongThuc, String uid = "") {
  soLanSai++;
  xoaPIN();

  servoKhoa.write(0);

  digitalWrite(PIN_LED_G, LOW);
  digitalWrite(PIN_LED_R, HIGH);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TU CHOI ");
  lcd.print(soLanSai);
  lcd.print("/3");

  lcd.setCursor(0, 1);

  if (phuongThuc == "PIN") {
    lcd.print("PIN KHONG DUNG");
  } else {
    lcd.print(uid.substring(0, 16));
  }

  batDauCoi(3, 700, 120);

  String log = "PHUONG THUC=" + phuongThuc;

  if (uid.length() > 0) {
    log += " | UID=" + uid;
  }

  log += " | KET QUA=TU CHOI";
  log += " | SO LAN SAI=";
  log += String(soLanSai);

  ghiLog("WARN", log);

  if (soLanSai >= SO_LAN_SAI_TOI_DA) {
    trangThai = KHOA_TAM;
    mocTG = millis();
    giayTruoc = -1;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("KHOA TAM THOI");

    ghiLog(
      "ALERT",
      "KHOA TAM 30 GIAY - SAI 3 LAN"
    );
  }
  else {
    trangThai = TU_CHOI;
    mocTG = millis();
  }
}

// ============================================
// 18. XU LY BAN PHIM
// ============================================

void xuLyPhim(char phim) {
  if (!phim) return;

  // Khi khoa tam, khong chap nhan PIN
  if (trangThai == KHOA_TAM) {
    ghiLog(
      "ALERT",
      "PHUONG THUC=PIN | KET QUA=BI CHAN"
    );
    return;
  }

  // Khong nhan PIN khi dang mo/tu choi
  if (trangThai != CHO_QUET) {
    return;
  }

  // Phim * xoa ma dang nhap
  if (phim == '*') {
    xoaPIN();
    manHinhCho();

    ghiLog(
      "INFO",
      "KEYPAD - DA XOA PIN DANG NHAP"
    );
    return;
  }

  // Phim # xac nhan
  if (phim == '#') {
    if (doDaiPIN != 4) {
      ghiLog(
        "WARN",
        "PIN CHUA DU 4 CHU SO"
      );

      hienThiPIN();
      return;
    }

    if (strcmp(pinNhap, PIN_DUNG) == 0) {
      moKhoa("PIN", "NGUOI DUNG PIN");
    }
    else {
      tuChoi("PIN");
    }

    return;
  }

  // Chi nhan cac phim so 0-9
  if (phim >= '0' && phim <= '9') {
    if (doDaiPIN < 4) {
      pinNhap[doDaiPIN] = phim;
      doDaiPIN++;
      pinNhap[doDaiPIN] = '\0';

      hienThiPIN();
    }
  }
}

// ============================================
// 19. DEM NGUOC KHOA TAM
// ============================================

void demNguoc(unsigned long bayGio) {
  unsigned long daQua = bayGio - mocTG;
  unsigned long conLai = T_KHOA_TAM - daQua;

  int giayConLai = (conLai + 999) / 1000;

  if (giayConLai != giayTruoc) {
    giayTruoc = giayConLai;

    lcd.setCursor(0, 1);
    lcd.print("Thu lai sau ");

    if (giayConLai < 10) lcd.print(" ");

    lcd.print(giayConLai);
    lcd.print("s ");
  }

  digitalWrite(
    PIN_LED_R,
    ((daQua / 500) % 2 == 0) ? HIGH : LOW
  );
}

// ============================================
// 20. KHOI TAO
// ============================================

void setup() {
  Serial.begin(115200);

  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  digitalWrite(PIN_LED_G, LOW);
  digitalWrite(PIN_LED_R, LOW);

  SPI.begin();
  rfid.PCD_Init();

  Wire.begin(PIN_SDA, PIN_SCL);
  lcd.init();
  lcd.backlight();

  servoKhoa.setPeriodHertz(50);
  servoKhoa.attach(PIN_SERVO, 500, 2400);
  servoKhoa.write(0);

  keypad.setDebounceTime(30);

  xoaPIN();
  manHinhCho();

  Serial.println();
  Serial.println("==============================");
  Serial.println("NHOM 3 - KHOA CUA RFID + PIN");
  Serial.println("ESP32 - MFRC522 - KEYPAD 4X4");
  Serial.println("==============================");

  ghiLog("INFO", "HE THONG SAN SANG");
  ghiLog("INFO", "RFID VA KEYPAD DA KHOI TAO");
  ghiLog("INFO", "SO THE HOP LE = " + String(SO_THE));
}

// ============================================
// 21. VONG LAP CHINH
// ============================================

void loop() {
  unsigned long bayGio = millis();

  capNhatCoi();

  // Doc phim trong moi trang thai
  char phim = keypad.getKey();

  // ----------------------------------------
  // MO KHOA
  // ----------------------------------------
  if (trangThai == MO_KHOA) {
    if (bayGio - mocTG >= T_MO_KHOA) {
      servoKhoa.write(0);

      ghiLog(
        "INFO",
        "CUA DA TU DONG DONG SAU 5 GIAY"
      );

      veChoQuet();
    }
    return;
  }

  // ----------------------------------------
  // TU CHOI
  // ----------------------------------------
  if (trangThai == TU_CHOI) {
    if (bayGio - mocTG >= T_TU_CHOI) {
      veChoQuet();
    }
    return;
  }

  // ----------------------------------------
  // KHOA TAM
  // ----------------------------------------
  if (trangThai == KHOA_TAM) {
    if (bayGio - mocTG >= T_KHOA_TAM) {
      soLanSai = 0;

      ghiLog(
        "INFO",
        "HET KHOA TAM - HE THONG PHUC HOI"
      );

      ghiLog(
        "INFO",
        "DAT LAI BO DEM SAI = 0"
      );

      veChoQuet();
      return;
    }

    demNguoc(bayGio);

    // Chan Keypad
    if (phim) {
      xuLyPhim(phim);
    }

    // Chan RFID
    if (rfid.PICC_IsNewCardPresent() &&
        rfid.PICC_ReadCardSerial()) {

      String uid = chuoiUID(
        rfid.uid.uidByte,
        rfid.uid.size
      );

      ghiLog(
        "ALERT",
        "PHUONG THUC=RFID | UID=" + uid +
        " | KET QUA=BI CHAN"
      );

      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
    }

    return;
  }

  // ----------------------------------------
  // CHO QUET - XU LY KEYPAD
  // ----------------------------------------
  if (phim) {
    xuLyPhim(phim);

    if (trangThai != CHO_QUET) {
      return;
    }
  }

  // ----------------------------------------
  // CHO QUET - XU LY RFID
  // ----------------------------------------
  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }

  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }

  String uid = chuoiUID(
    rfid.uid.uidByte,
    rfid.uid.size
  );

  int viTri = timThe(
    rfid.uid.uidByte,
    rfid.uid.size
  );

  if (viTri >= 0) {
    moKhoa(
      "RFID",
      danhSach[viTri].ten,
      uid
    );
  }
  else {
    tuChoi("RFID", uid);
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

// ============================================
// KET THUC CHUONG TRINH - BUOC 9
// ============================================

