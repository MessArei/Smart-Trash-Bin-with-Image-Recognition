#include <ESP32Servo.h>

Servo servoTrash;   // 對應訊號 A (一般垃圾)
Servo servoRecycle; // 對應訊號 B (一般垃圾)

// --- 腳位設定 (保留原本的) ---
const int PIN_SERVO_A = 13; 
const int PIN_SERVO_B = 14; 

// --- 通訊腳位 (新增的) ---
const int PIN_RX = 16; 
const int PIN_TX = 17;

HardwareSerial ReceiverSerial(2); // 定義接收端 Serial

// --- 參數設定 (保留原本的) ---
const int TURN_TIME = 500; 

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 啟動 (保留動作邏輯 + 辨識功能)...");

  // 初始化通訊
  ReceiverSerial.begin(115200, SERIAL_8N1, PIN_RX, PIN_TX);

  // 基本設定
  servoTrash.setPeriodHertz(50);
  servoRecycle.setPeriodHertz(50);
  
  servoRecycle.attach(PIN_SERVO_A);
  servoTrash.attach(PIN_SERVO_B);

  // 初始化：確保兩顆馬達一開始是停下的
  servoTrash.write(90);
  servoRecycle.write(90);
}

// --- 這是把你原本 loop 裡的動作邏輯搬過來 ---
// 參數: 要動哪顆馬達(servo), 轉動方向(speed: 0 或 180)
void triggerMotorAction(Servo &myServo, int speed) {
  
  // 1. 轉動 (複製你原本的邏輯)
  // 你原本是 write 之後接兩個 delay (TURN_TIME + 5000)
  myServo.write(speed);   
  delay(TURN_TIME);       
  delay(5000); // 這是你強調要保留的 "狂轉/維持" 時間

  // 2. 定格/停止
  myServo.write(90);      
  delay(3000); // 這是你原本最後的休息時間
}

void loop() {
  // 監聽是否有訊號進來
  if (ReceiverSerial.available()) {
    String data = ReceiverSerial.readStringUntil('\n');
    data.trim(); // 去除空白

    if (data.length() == 0) return;

    Serial.print("收到訊號: ");
    Serial.println(data);

    // --- 辨識邏輯 ---
    if (data == "B") {
      Serial.println(">>> 執行 B 動作 垃圾(有機物)");
      // 呼叫函式：使用 servoTrash，速度 180 (你原本 Code 裡的設定)
      triggerMotorAction(servoTrash, 0);
    } 
    else if (data == "A") {
      Serial.println(">>> 執行 A 動作 塑膠");
      // 呼叫函式：使用 servoRecycle，速度 0 (你原本 Code 裡的設定)
      triggerMotorAction(servoRecycle, 180);
    }
  }
}
