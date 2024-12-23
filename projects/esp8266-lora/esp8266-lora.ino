#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>

#define ss 15
#define rst 16
#define dio0 2
int counter = 0;

void setup()
{
  Serial.begin(115200);

  while (!Serial);
  Serial.println("LoRa Sender");
  LoRa.setPins(ss, rst, dio0);
  if (!LoRa.begin(915E6))
  {
    Serial.println("Starting LoRa failed!");
    delay(100);
    while (1)
      ;
  }
}

void loop()
{
  Serial.print("Sending packet: ");
  Serial.println(counter);

  // send packet
  LoRa.beginPacket();
  String msg = "{\"id\":\"deviceid\",\"model\":\"ESP8266\",\"c\":" + String(counter) + "}";
  // Send json string
  LoRa.print(msg);
  LoRa.endPacket();

  Serial.println(msg);

  counter++;

  delay(10000);
}