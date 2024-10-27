const int analogPin = A0; // Analog pin A0 of the ESP8266
const float multiplicationFactor = 1.33; // R1 (33k) + R2 (100k) / R2 (100k)
const float espVoltage = 3.3; // Reference voltage of the ESP8266
#define BATTV_MAX 4.1 // Maximum battery voltage
#define BATTV_MIN 3.2 // Minimum battery voltage
const float correctionFactor = 1.03; // Corrects reading based on the multimeter

void setup() {
  Serial.begin(115200);
}

void loop() {
  // Read the analog value (0 to 1023)
  int analogValue = analogRead(analogPin);
  
  // Convert the reading to a voltage (0 to 1V due to the internal divider)
  float inputReading = (analogValue / 1023.0) * espVoltage * correctionFactor;
  
  // Calculate the original input voltage using the inverse formula of the voltage divider
  float finalReading = inputReading * multiplicationFactor;

  // Battery voltage
  Serial.print("Measured input voltage: ");
  Serial.print(finalReading);
  Serial.println(" V");

  // Battery percentage
  float percentage = calculateBatteryPercentage(finalReading);
  Serial.print("Battery Percentage: ");
  Serial.print(percentage);
  Serial.println("%");
  
  delay(1000); // Waits 1 second before the next reading
}

float calculateBatteryPercentage(float current_voltage) {
    if (current_voltage >= BATTV_MAX) {
        return 100.0;
    } else if (current_voltage <= BATTV_MIN) {
        return 0.0;
    } else {
        return ((current_voltage - BATTV_MIN) / (BATTV_MAX - BATTV_MIN)) * 100.0;
    }
}