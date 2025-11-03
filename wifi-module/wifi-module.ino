#include <SoftwareSerial.h>

// --- Конфигурация пинов ---
const int espRxPin = 2; // D2 Arduino -> TX ESP (SoftwareSerial RX)
const int espTxPin = 3; // D3 Arduino -> RX ESP (SoftwareSerial TX, через 1kΩ резистор)
const int ledPin = 9;   // D9 Arduino -> LED с резистором

// *** ОБЯЗАТЕЛЬНО ЗАМЕНИТЕ ЭТИ ЗНАЧЕНИЯ ***
const char* ssid = "YOUR_SSID";     // Имя вашей Wi-Fi сети
const char* password = "YOUR_PASSWORD"; // Пароль от вашей Wi-Fi сети
// ****************************************

// Скорость обмена данными с ESP8266
#define ESP_BAUD_RATE 115200

SoftwareSerial espSerial(espRxPin, espTxPin); // (RX, TX)

// --- Хелпер функции ---

// Управляет LED на D9
void checkLedCommand(String command) {
  if (command.startsWith("L1")) {
    digitalWrite(ledPin, HIGH);
    Serial.println("-> LED D9 ON");
  } else if (command.startsWith("L0")) {
    digitalWrite(ledPin, LOW);
    Serial.println("-> LED D9 OFF");
  }
}

// Отправляет AT команду, ждет ответа и выводит его в Serial Monitor
void sendATCommand(String command, const int timeout) {
  Serial.print("\n--- Sending: ");
  Serial.print(command);
  Serial.println(" ---");

  // 1. Отправляем команду
  espSerial.print(command);
  espSerial.print("\r\n"); // Обязательный CR/LF для AT-команд

  // 2. Ждем ответа
  long time = millis();
  while ((time + timeout) > millis()) {
    while (espSerial.available()) {
      Serial.write(espSerial.read()); // Печатаем ответ ESP
      delay(1); 
    }
  }
  Serial.println("--------------------------------");
}


// --- Основные функции Arduino ---

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); // LED по умолчанию выключен

  Serial.println("--- System Ready ---");
  Serial.println("Commands:");
  Serial.println("  AT      - Test connection");
  Serial.println("  L1/L0   - Control LED");
  Serial.println("  TEST_WIFI - Run automated Wi-Fi connection sequence");

  espSerial.begin(ESP_BAUD_RATE);
  delay(1000); // Даем ESP время на загрузку
  
  // Начальная проверка
  sendATCommand("AT", 1000); 
}


void loop() {
  // 1. Чтение и обработка команд из Serial Monitor (PC)
  if (Serial.available()) {
    delay(10);
    
    // Считываем всю команду в строку
    String cmd = "";
    while (Serial.available()) {
      cmd += (char)Serial.read();
    }
    cmd.trim(); 

    // Проверяем, является ли это командой управления LED
    checkLedCommand(cmd);

    // Запускаем автоматическую последовательность тестирования Wi-Fi
    if (cmd.equals("TEST_WIFI")) {
      Serial.println("\n--- Starting Automated Wi-Fi Test Sequence ---");
      
      // 1. AT: Проверка связи (уже сделано в setup, но для надежности можно повторить)
      sendATCommand("AT", 1000);
      
      // 2. AT+CWMODE=1: Устанавливаем режим клиента (Station Mode)
      sendATCommand("AT+CWMODE=1", 2000); 
      
      // 3. AT+CWJAP="SSID","PASSWORD": Подключаемся к точке доступа
      // Используем строковую переменную для подстановки
      String joinCommand = "AT+CWJAP=\"" + String(ssid) + "\",\"" + String(password) + "\"";
      sendATCommand(joinCommand, 15000); // Ждем до 15 секунд для подключения
      
      // 4. AT+CIFSR: Запрашиваем IP адрес
      sendATCommand("AT+CIFSR", 3000); 
      
      Serial.println("--- Wi-Fi Test Complete (Check for OK and IP address) ---");
    }

    // Если введена любая другая AT-команда, пропускаем ее через sendATCommand
    // Это позволяет вам вводить другие команды вручную (например, "AT+RST")
    if (cmd.startsWith("AT")) {
       sendATCommand(cmd, 3000); 
    }
  }

  // 2. Отображаем любые внезапные ответы ESP (например, уведомления о дисконнекте)
  if (espSerial.available()) {
    while (espSerial.available()) {
      Serial.write(espSerial.read());
    }
  }
}