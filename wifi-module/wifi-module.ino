#include <SoftwareSerial.h>

// --- Конфигурация пинов ---
const int espRxPin = 2; // D2 Arduino -> TX ESP (SoftwareSerial RX)
const int espTxPin = 3; // D3 Arduino -> RX ESP (SoftwareSerial TX, через 1kΩ резистор)
const int ledPin = 9;   // D9 Arduino -> LED с резистором

// Скорость обмена данными с ESP8266
#define ESP_BAUD_RATE 115200

SoftwareSerial espSerial(espRxPin, espTxPin); // (RX, TX)

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); // LED по умолчанию выключен

  Serial.println("--- System Ready ---");
  Serial.println("Commands:");
  Serial.println("  AT - check ESP connection");
  Serial.println("  L1 - Turn LED ON (D9)");
  Serial.println("  L0 - Turn LED OFF (D9)");

  espSerial.begin(ESP_BAUD_RATE);
}

// Проверяет, пришла ли команда L0 или L1 и управляет LED
void checkLedCommand(String command) {
  if (command.startsWith("L1")) {
    digitalWrite(ledPin, HIGH);
    Serial.println("-> LED D9 ON");
  } else if (command.startsWith("L0")) {
    digitalWrite(ledPin, LOW);
    Serial.println("-> LED D9 OFF");
  }
}

void loop() {
  // 1. Чтение и обработка команд из Serial Monitor (PC)
  if (Serial.available()) {
    delay(10); // Ждем, пока вся строка придет
    
    // Считываем всю команду в строку
    String cmd = "";
    while (Serial.available()) {
      cmd += (char)Serial.read();
    }
    cmd.trim(); // Удаляем пробелы, включая CR/LF

    // Проверяем, является ли это командой управления LED
    checkLedCommand(cmd);

    // Пересылаем команду на ESP (если это не LED-команда)
    // Мы предполагаем, что AT-команды будут отправлены с "Both NL & CR"
    if (cmd.startsWith("AT")) {
       espSerial.print(cmd);
       // Добавляем требуемый CR/LF для AT-команд
       espSerial.print("\r\n"); 
    }
  }

  // 2. Отображаем все, что ответил ESP-01S
  if (espSerial.available()) {
    while (espSerial.available()) {
      Serial.write(espSerial.read());
    }
  }
}