// Пин, к которому подключен датчик
const int sensorPin = A2;

// Максимальное напряжение, которое подаётся на датчик
const float sensorMaxVoltage = 9.0; // Вольты
// Максимальное значение АЦП Arduino (10 бит)
const int adcMax = 1023;
// Напряжение питания Arduino
const float voltageRef = 4.95; // Вольт

const float factor = 0.2;

void setup() {
  Serial.begin(9600); // Запуск Serial Monitor
}

void loop() {
  int sensorValue = analogRead(sensorPin); // Считываем значение с датчика (0-1023)
  
  // Переводим значение АЦП в реальное напряжение
  float voltageOut = (sensorValue / adcMax) * voltageRef;

  float voltageIn = voltageOut / factor;
  
  // Примерная оценка процента заряда батареи (для Li-ion 3.0–4.2V на элемент)
  float percentage = batteryPercentage(voltageIn);
  
  Serial.print("Значение с датчика: ");
  Serial.print(sensorValue);
  Serial.print(" V\t");
  Serial.print("Напряжение батареи: ");
  Serial.print(voltageIn);
  Serial.print(" V\t");
  Serial.print("Процент заряда: ");
  Serial.print(percentage);
  Serial.println("%");
  
  delay(1000); // Обновляем данные каждые 1 сек
}

// Функция для перевода напряжения в процент заряда
float batteryPercentage(float voltage) {
  float minV = 6.0; // Минимальное напряжение батареи
  float maxV = 9.6; // Полностью заряженная батарея
  if(voltage <= minV) return 0.0;
  if(voltage >= maxV) return 100.0;
  // Линейная аппроксимация (можно заменить на более точную таблицу для Li-ion)
  return (voltage - minV) / (maxV - minV) * 100.0;
}
