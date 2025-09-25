// Пины
const int sensorPower = 7;   // Пин, который подаёт питание на датчик
const int sensorPin   = A0;  // Аналоговый пин для считывания влажности

// Режим работы
bool continuousMode = false;  

// Калибровка
const int dryValue = 1023;   // значение в сухой земле
const int wetValue = 300;   // значение в воде

void setup() {
  Serial.begin(9600);
  pinMode(sensorPower, OUTPUT);
  digitalWrite(sensorPower, LOW); // по умолчанию питание выключено

  Serial.println("Soil moisture sensor ready.");
  Serial.println("Commands:");
  Serial.println("  R - read once");
  Serial.println("  C - continuous mode");
  Serial.println("  S - stop continuous mode");
}

int readSoilMoistureRaw() {
  digitalWrite(sensorPower, HIGH);   // включаем питание
  delay(300);                        // ждём стабилизацию
  int value = analogRead(sensorPin); // считываем показание
  digitalWrite(sensorPower, LOW);    // отключаем питание
  return value;
}


int convertToPercent(int rawValue) {
  // Чем суше почва, тем выше rawValue
  int percent = map(rawValue, dryValue, wetValue, 0, 100);
  // Ограничим диапазон
  if (percent < 0) percent = 0;
  if (percent > 100) percent = 100;
  return percent;
}

void loop() {
  // Проверяем входящие команды
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == 'R') {
      int raw = readSoilMoistureRaw();
      int moisturePercent = convertToPercent(raw);
      Serial.print("Soil moisture: ");
      Serial.print(moisturePercent);
      Serial.println("%");
    } else if (cmd == 'C') {
      continuousMode = true;
      Serial.println("Continuous mode enabled.");
    } else if (cmd == 'S') {
      continuousMode = false;
      Serial.println("Continuous mode stopped.");
    }
  }

  // Если включен циклический режим
  if (continuousMode) {
    int raw = readSoilMoistureRaw();
    int moisturePercent = convertToPercent(raw);
    Serial.print("Soil moisture: ");
    Serial.print(moisturePercent);
    Serial.println("%");
    delay(2000); // замер каждые 2 секунды
  }
}
