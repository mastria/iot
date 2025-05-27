

#define ETH_PHY_MDC 23
#define ETH_PHY_MDIO 18
#define ETH_PHY_POWER 16

#include <EthernetESP32.h>

EMACDriver driver(ETH_PHY_LAN8720, ETH_PHY_MDC, ETH_PHY_MDIO, ETH_PHY_POWER, EMAC_CLK_IN_GPIO, EMAC_CLK_EXT_IN);

char server[] = "arduino.tips";

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 1, 5);

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

void setup()
{

    Serial.begin(115200);
    delay(1000);
    while (!Serial)
        ;

    Ethernet.init(driver);

    // start the Ethernet connection:
    Serial.println("Initialize Ethernet with DHCP:");
    if (Ethernet.begin() == 0)
    {
        Serial.println("Failed to configure Ethernet using DHCP");
        // Check for Ethernet hardware present
        if (Ethernet.hardwareStatus() == EthernetNoHardware)
        {
            Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
            while (true)
            {
                delay(1); // do nothing, no point running without Ethernet hardware
            }
        }
        if (Ethernet.linkStatus() == LinkOFF)
        {
            Serial.println("Ethernet cable is not connected.");
        }
        // try to configure using IP address instead of DHCP:
        Ethernet.begin(ip);
    }
    else
    {
        Serial.print("  DHCP assigned IP ");
        Serial.println(Ethernet.localIP());
    }

    Serial.print("connecting to ");
    Serial.print(server);
    Serial.println("...");

    // if you get a connection, report back via serial:
    if (client.connect(server, 80))
    {
        Serial.print("connected to ");
        Serial.println(client.remoteIP());
        // Make a HTTP request:
        client.println("GET /asciilogo.txt HTTP/1.1");
        client.print("Host: ");
        client.println(server);
        client.println("Connection: close");
        client.println();
    }
    else
    {
        // if you didn't get a connection to the server:
        Serial.println("connection failed");
    }
}

void loop()
{
    // if there are incoming bytes available
    // from the server, read them and print them:
    while (client.available())
    {
        char c = client.read();
        Serial.write(c);
    }

    // if the server's disconnected, stop the client:
    if (!client.connected())
    {
        Serial.println();
        Serial.println("disconnecting.");
        client.stop();

        // do nothing forevermore:
        while (true)
        {
            delay(1);
        }
    }
}