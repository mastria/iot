<p align="center">
    <img src="water-tank.gif" width="150" alt="Home Assistant card">
</p>

# [ATMEGA328] LoRa Water tank level meter

This project measures the water tank level using the **JSN-SR04T** ultrasonic sensor, with data processing handled by an **ATMEGA328** and transmission via LoRa using the **SX1278** module.

The system is powered by a **18650 battery with solar charging**, and the battery voltage and charge percentage are also monitored. Ensuring battery operation was crucial, as the location is approximately **120 meters from the receiver** and has no power supply. In my tests, the system ran for **15+ days on battery** power alone (without recharging).

On the receiving end, a **LoRa32** receives the transmitted data and forwards it to an MQTT server, enabling integration with monitoring or automation systems.
<br><br>

## List of Components

Here is the list of components used in this project, including links for purchasing:

- [JSN-SR04T Ultrasonic Sensor](https://pt.aliexpress.com/item/1005006320348756.html)
- [ATMEGA328 3.3V 8MHz Microcontroller](https://pt.aliexpress.com/item/1005005096327373.html)
- [XL63000 Voltage Regulator](https://pt.aliexpress.com/item/1005006240120884.html)
- [SX1278 LoRa Module](https://pt.aliexpress.com/item/1005005096327373.html)
- [18650 Battery Holder Case](https://pt.aliexpress.com/item/1005006283741317.html)
- [18650 Lithium Battery](https://pt.aliexpress.com/item/1005006227891015.html)
- [TP4056 Battery Charger Module](https://pt.aliexpress.com/item/1005006585278260.html)
- [3W, 5V Solar Panel](https://pt.aliexpress.com/item/1005006259573646.html)
- [33kΩ and 100kΩ Resistors (Pack)](https://pt.aliexpress.com/item/1005002344985336.html)
- [Jumper Wires (Pack)](https://pt.aliexpress.com/item/1005005945668553.html)
- [LILYGO® Ttgo LoRa32 (915MHz)](https://pt.aliexpress.com/item/32872078587.html)

*This list includes all the components and modules required to build the project. Be sure to double-check the specifications of each item before purchasing.*
<br><br>

## Transmitter Wiring Diagram

<img src="transmitter.png" width="100%">

<br>

## Pin Connections

#### SX1278 LoRa Module to ATMEGA328 Connections

| **SX1278 Pin** | **ATMEGA328 Pin** | **Description**     |
| -------------------- | ----------------------- | ------------------------- |
| VCC                  | 3.3V                    | Power Supply              |
| GND                  | GND                     | Ground                    |
| SCK                  | Pin 13 (SCK)            | Serial Clock (SPI)        |
| MISO                 | Pin 12 (MISO)           | Master In Slave Out (SPI) |
| MOSI                 | Pin 11 (MOSI)           | Master Out Slave In (SPI) |
| NSS                  | Pin 10 (CS)             | Chip Select (SPI)         |
| DIO0                 | Pin 2 (INT0)            | Interrupt Pin (optional)  |

#### JSN-SR04T Ultrasonic Sensor to ATMEGA328 Connections

| **JSN-SR04T Pin** | **ATMEGA328 Pin** | **Description**        |
| ----------------------- | ----------------------- | ---------------------------- |
| VCC                     | 3.3V                    | Power Supply                 |
| GND                     | GND                     | Ground                       |
| TRIG                    | Pin 4 (Digital I/O)     | Trigger Pin                  |
| ECHO                    | Pin 5 (Digital I/O)     | Echo Pin (Input from sensor) |

#### Additional Connections

The remaining connections for the battery, solar panel, and other components are straightforward and follow the schematic shown above. Please refer to the schematic diagram for detailed wiring.
<br><br>

## ATMEGA328 Code

Below is the code used on the ATMEGA328 for this project. It handles the water level measurement using the JSN-SR04T sensor and sends the data via LoRa using the SX1278 module.

The provided code is a baseline and should be adjusted according to your project's specifics and requirements. Key areas to review and customize include:

- **Timing Parameters**: The deep sleep interval is approximately **1 minute and 30 seconds**, optimized for energy efficiency. Adjust this duration in the loop if your project needs more frequent or less frequent measurements.
- **Voltage and Calibration Factors**: Battery voltage range and correction factors are tailored for the 3.3V ATMEGA328 used in this project. Ensure these values align with your specific microcontroller and power setup.
- **Sensor and Module Pins**: Pins defined for the JSN-SR04T and SX1278 modules may differ depending on your circuit design. Adjust the pin configurations accordingly.

#### Libs

- [LoRa.h](https://github.com/sandeepmistry/arduino-LoRa)
- [LowPower.h](https://github.com/LowPowerLab/LowPower)

#### Code

```cpp
#include <LoRa.h>
#include "LowPower.h"

// SR04
#define triggerPin 4
#define echoPin 5
#define measureInterval 30  // MAX 35 ms for up to 6.0m, MIN 5 ms for up to 0.8m
#define measureCount 5      // Number of measurements to take
float distance;
float simpleReading();
float calculateDistance();
float batteryReading();

// LoRa
#define ss 10   // NSS connected to D10
#define rst 9   // RST connected to D9
#define dio0 2  // DIO0 connected to D2

// Battery
#define BATTERY_MAX 4.075  // Maximum battery voltage
#define BATTERY_MIN 3.2    // Minimum battery voltage
const int batteryPin = 24; // A1
const float multiplicationFactor = 1.33;  // R1 (33k) + R2 (100k) / R2 (100k)
const float arduinoVoltage = 3.3;         // Reference voltage of the ATMEGA328
const float correctionFactor = 0.9255;   // Corrects reading based on the multimeter

void setup() {
  // SR04
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // LoRa
  LoRa.setPins(ss, rst, dio0);
  if (!LoRa.begin(915E6)) { // 915MHz
    delay(100);
    while (true)
      ;
  }
}

void loop() {
  // Deep Sleep
  for (int i = 0; i < 10; i++) {  // 10 x 8s = 80s (~1 minute and 20 seconds)
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  }
  delay(100);

  // SR04
  distance = calculateDistance();

  delay(100);

  // Battery
  int batteryValue = batteryReading(); // Valid battery reading
  float inputReading = (batteryValue / 1023.0) * arduinoVoltage * correctionFactor;
  float batteryVoltage = inputReading * multiplicationFactor;
  float batteryPercentage = calculateBatteryPercentage(batteryVoltage);

  delay(100);

  // LoRa
  LoRa.beginPacket();
  String msg = "{\"id\":\"watertank\",\"v\":" + String(batteryVoltage) + ",\"p\":" + String(batteryPercentage) + ",\"d\":" + String(distance) + "}";
  LoRa.print(msg);
  LoRa.endPacket();
}

float batteryReading() {
  float batterySum = 0;
  float batteryResult = 0;
  for (int index = 0; index < measureCount; index++) {
    delay(5);
    batterySum += analogRead(batteryPin);
  }
  batteryResult = (float)batterySum / measureCount;
  return batteryResult;
}

float calculateDistance() {
  float distanceSum = 0;
  float distanceResult = 0;
  for (int index = 0; index < measureCount; index++) {
    delay(measureInterval);
    distanceSum += simpleReading();
  }
  distanceResult = (float)distanceSum / measureCount;
  return distanceResult;
}

float simpleReading() {
  long duration = 0;
  float distanceResult = 0;
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distanceResult = duration * 0.03532;
  return distanceResult;
}

float calculateBatteryPercentage(float currentVoltage) {
  if (currentVoltage >= BATTERY_MAX) {
    return 100.0;
  } else if (currentVoltage <= BATTERY_MIN) {
    return 0.0;
  } else {
    return ((currentVoltage - BATTERY_MIN) / (BATTERY_MAX - BATTERY_MIN)) * 100.0;
  }
}
```

<br>

## LoRa32 Receiver - TTGO LoRa32 V2.1

<p align="center">
    <img src="lora32.jpg" width="30%" alt="LoRa32">
</p>

The **TTGO LoRa32 V2.1** receiver is used with the **OpenMQTTGateway** library as a LoRa gateway. This setup allows the receiver to receive LoRa messages and forward them via MQTT.

For configuration, simply follow the [OpenMQTTGateway documentation](https://docs.openmqttgateway.com/upload/web-install.html) to set up the receiver and connect it to your MQTT platform.

This solution enables seamless communication between LoRa devices using the 915 MHz frequency.
<br><br>

## Plus: Home Assistant Integration

To integrate the sensor with Home Assistant, you can use the following YAML configuration. This code will allow Home Assistant to read the sensor data and display it on your dashboard.

#### Sensor
```yaml
mqtt:
  sensor:
    # WATER TANK
    - name: "Water Tank - Distance"
      state_topic: "lora/ESP32_LORA/LORAtoMQTT/watertank"
      device_class: "distance"
      unit_of_measurement: "cm"
      value_template: "{{ value_json.d }}"
      unique_id: "watertank_distance"
    - name: "Water Tank - Voltage"
      state_topic: "lora/ESP32_LORA/LORAtoMQTT/watertank"
      device_class: "voltage"
      unit_of_measurement: "V"
      value_template: "{{ value_json.v }}"
      unique_id: "watertank_voltage"
    - name: "Water Tank - Percentage"
      state_topic: "lora/ESP32_LORA/LORAtoMQTT/watertank"
      device_class: "battery"
      unit_of_measurement: "%"
      value_template: "{{ value_json.p }}"
      unique_id: "watertank_percentage"
```

#### Sensor template
```yaml
sensor:
 - platform: template
    sensors:
      water_tank_percentage:
        unique_id: water_tank_percentage
        friendly_name: "Water Tank"
        unit_of_measurement: "%"
        icon_template: "mdi:water-percent"
        value_template: >-
          {% set full_height = 21 %}
          {% set empty_height = 120 %}
          {% set measured_height = states('sensor.water_tank_distance') | float(empty_height) %}
          {% if measured_height < full_height %}
            100
          {% elif measured_height > empty_height %}
            0
          {% else %}
            {{ ((empty_height - measured_height) / (empty_height - full_height) * 100) | round(1) }}
          {% endif %}
```
<br>

## References

- [Powering ESP32/ESP8266 with Solar Panels and Battery Level Monitoring](https://randomnerdtutorials.com/power-esp32-esp8266-solar-panels-battery-level-monitoring/)
- [Distance Measurement Project by abtom87](https://github.com/abtom87/Distance-measurement-)
- [Deep Sleep Example for ATMEGA328P](https://github.com/RalphBacon/Arduino-Deep-Sleep/blob/master/Sleep_ATMEGA328P.ino)
- [LoRa Communication with Arduino P2P](https://www.makerhero.com/blog/comunicacao-lora-com-arduino-p2p/)
- [OTAA-Based LoRaWAN Node with Arduino SX1276](https://how2electronics.com/otaa-based-lorawan-node-with-arduino-lora-sx1276/)
- [Mailbox Guard Project](https://github.com/PricelessToolkit/MailBoxGuard)
- [Onedrive Link (Cleber Augusto Borges)](https://onedrive.live.com/?authkey=%21AIzzyuwUAotKD8A&id=CD05BF6E6E1D1C15%214646&cid=CD05BF6E6E1D1C15)
- [LoRa with ESP8266 Blog Post](https://fazerlab.wordpress.com/2020/09/08/lora-com-esp8266/)
- [OpenMQTTGateway Example: Lora Temperature](https://github.com/1technophile/OpenMQTTGateway/blob/development/examples/LoraTemperature/src/main.cpp)
- [Arduino Pro Mini Wiki](https://land-boards.com/blwiki/index.php?title=Arduino_Pro_Mini)
- [Thingiverse: LoRa Antenna Project](https://www.thingiverse.com/thing:6768553)

<br>

## Acknowledgments

Thank you for checking out this project! I hope it helps you with your own IoT applications.

A special thank you to the authors and contributors of the resources in the references section.

Contributions, suggestions, and improvements are always welcome. Feel free to fork the repository and open a pull request with your changes.
