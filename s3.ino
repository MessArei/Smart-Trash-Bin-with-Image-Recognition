#include <ESP32Servo.h>

// --- 硬體腳位定義 (Datapath Configuration) ---
const int PIN_SERVO_A = 13; // 塑膠 (Recycle)
const int PIN_SERVO_B = 14; // 一般 (Trash)
const int PIN_RX = 16;      // UART RX
const int PIN_TX = 17;      // UART TX

// --- 全域物件 ---
Servo servoTrash;   // 對應 B
Servo servoRecycle; // 對應 A
HardwareSerial ReceiverSerial(2); 

// --- 狀態變數 (State Registers - 對應 GRAFCET A3) ---
// 1 = Active, 0 = Inactive
int X30 = 1; // 初始狀態: Wait for Serial
int X31 = 0; // Parse Command
int X32 = 0; // Path A: Open Recycle
int X33 = 0; // Path A: Hold Time
int X34 = 0; // Path A: Close/Rest
int X35 = 0; // Path B: Open Trash
int X36 = 0; // Path B: Hold Time
int X37 = 0; // Path B: Close/Rest
int X38 = 0; // Loop End

// --- 資料暫存器 (Data Registers) ---
String receivedData = "";
const int TURN_TIME = 500;
const int HOLD_TIME = 5000;
const int REST_TIME = 3000;

void setup() {
  // 1. 初始化通訊
  Serial.begin(115200);
  ReceiverSerial.begin(115200, SERIAL_8N1, PIN_RX, PIN_TX);
  
  // 2. 初始化伺服馬達 (Hardware Initialization)
  servoTrash.setPeriodHertz(50);
  servoRecycle.setPeriodHertz(50);
  servoRecycle.attach(PIN_SERVO_A);
  servoTrash.attach(PIN_SERVO_B);

  // 3. 歸位 (Reset Datapath)
  servoTrash.write(90);
  servoRecycle.write(90);

  Serial.println("System Ready: High-Level Synthesis Controller Started.");
}

void loop() {
  // 核心迴圈：不斷執行控制器邏輯
  grafcet3(); 
}

// ---------------------------------------------------------
// 控制器合成 (Controller Synthesis) [參考講義 source: 213]
// 負責：狀態轉移 (State Transition)
// ---------------------------------------------------------
void grafcet3() {
  
  // [Transition 1]: X30 -> X31 (當收到訊號時)
  if (X30 == 1 && ReceiverSerial.available()) {
    X30 = 0;
    X31 = 1;
  }
  
  // [Transition 2]: X31 -> X32 (判斷是 A 塑膠)
  // 注意：這裡假設 action3() 已經在 X31 狀態下完成了讀取資料
  else if (X31 == 1 && receivedData == "A") {
    X31 = 0;
    X32 = 1;
  }
  
  // [Transition 3]: X31 -> X35 (判斷是 B 一般垃圾)
  else if (X31 == 1 && receivedData == "B") {
    X31 = 0;
    X35 = 1;
  }
  
  // [Transition 4 - Path A Sequence]: X32 -> X33 -> X34 -> X38
  // 這裡使用循序邏輯，假設 Action 是阻塞式的 (Blocking)
  else if (X32 == 1) { X32 = 0; X33 = 1; }
  else if (X33 == 1) { X33 = 0; X34 = 1; }
  else if (X34 == 1) { X34 = 0; X38 = 1; }

  // [Transition 5 - Path B Sequence]: X35 -> X36 -> X37 -> X38
  else if (X35 == 1) { X35 = 0; X36 = 1; }
  else if (X36 == 1) { X36 = 0; X37 = 1; }
  else if (X37 == 1) { X37 = 0; X38 = 1; }

  // [Transition 6]: X38 -> X30 (回到原點)
  else if (X38 == 1) {
    X38 = 0;
    X30 = 1; // Return to initial state
    // 清除資料以便下次判斷
    receivedData = ""; 
  }

  // 執行當前狀態對應的動作
  action3();
}

// ---------------------------------------------------------
// 動作合成 (Datapath Synthesis) [參考講義 source: 226]
// 負責：實際硬體操作 (Servo, UART, Delay)
// ---------------------------------------------------------
void action3() {
  
  // [State 31 Action]: 解析指令
  if (X31 == 1) {
    receivedData = ReceiverSerial.readStringUntil('\n');
    receivedData.trim(); // 去除空白
    Serial.print("Parsed Command: ");
    Serial.println(receivedData);
  }

  // --- Path A (塑膠) 動作區 ---
  
  // [State 32]: 開啟回收桶
  if (X32 == 1) {
    Serial.println("[A32] Open Recycle (180)");
    servoRecycle.write(180);
    delay(TURN_TIME); // 等待轉動到位
  }

  // [State 33]: 保持開啟
  if (X33 == 1) {
    Serial.println("[A33] Hold Recycle");
    delay(HOLD_TIME);
  }

  // [State 34]: 關閉並休息
  if (X34 == 1) {
    Serial.println("[A34] Close Recycle (90)");
    servoRecycle.write(90);
    delay(REST_TIME);
  }

  // --- Path B (一般垃圾) 動作區 ---

  // [State 35]: 開啟垃圾桶
  if (X35 == 1) {
    Serial.println("[A35] Open Trash (0)");
    servoTrash.write(0);
    delay(TURN_TIME);
  }

  // [State 36]: 保持開啟
  if (X36 == 1) {
    Serial.println("[A36] Hold Trash");
    delay(HOLD_TIME);
  }

  // [State 37]: 關閉並休息
  if (X37 == 1) {
    Serial.println("[A37] Close Trash (90)");
    servoTrash.write(90);
    delay(REST_TIME);
  }

  // [State 38]: 完成訊號
  if (X38 == 1) {
    Serial.println("--- Cycle Complete, Waiting for next signal ---");
  }
}
