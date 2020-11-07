#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include <Servo.h>

WiFiUDP Udp;
IPAddress broadcastIp(255, 255, 255, 255);
unsigned int BROADCAST_PORT = 54321;
unsigned int TCP_SOCKET_PORT = 12345;
WiFiServer server(TCP_SOCKET_PORT);

unsigned int NUM_SERVOS = 3;
unsigned int SERVO_SIGNAL_LENGTH = 4;
Servo turn;  // PIN 1
Servo tilt;  // PIN 0
Servo mouth; // PIN 2

char id[] = "OINKULA";
char network[] = "<WiFi SSID>";
char password[] = "<WiFi Password>";

void setup() {
  Serial.begin(9600);
  turn.attach(1);
  tilt.attach(0);
  mouth.attach(2);
  
  int status = WiFi.begin(network, password);
  if (status == WL_CONNECTED) {
    server.begin();
    Udp.begin(BROADCAST_PORT);
  }
}

void loop() {
  // Phone application discovers Arduino through UDP broadcast
  Udp.beginPacket(broadcastIp, BROADCAST_PORT);
  Udp.write(id, 10);
  Udp.endPacket();  // Servo movement controlled over TCP socket
  int servoNum, signalIdx = 0;
  WiFiClient client = server.available();
  
  while (client.connected()) {
    while (client.available()) {
      // parse string, format XXXXYYYYZZZZ
      while (servoNum < NUM_SERVOS) {
        String servoCommand;
        while (signalIdx < SERVO_SIGNAL_LENGTH) {
          char signalChar = client.read();
          servoCommand += String(signalChar);
          signalIdx += 1;
        }
        switch (servoNum) {
          case 0: { tilt.writeMicroseconds(servoCommand.toInt()); break; }
          case 1: { turn.writeMicroseconds(servoCommand.toInt()); break; }
          case 2: { mouth.writeMicroseconds(servoCommand.toInt()); break; }
        }
        signalIdx = 0;
        servoNum += 1;
      }
      servoNum = 0;
    }
  }  delay(1000);
}
