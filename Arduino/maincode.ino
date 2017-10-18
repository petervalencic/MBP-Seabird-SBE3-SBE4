
#include <SPI.h>
#include <SD.h>
#include <Ethernet.h>

byte myMac[6];
byte myIP[4];
byte myNM[4];
byte myGW[4];
byte myDNS[4];

EthernetServer server = EthernetServer(80);


void setup() {
  Serial.begin(9600);
  delay(500);
  Serial.println("+----------------------+");
  Serial.println("|   MBP - SBE3 - SBE4  |");
  Serial.println("+----------------------+");

  // preberi datoteko iz SD kartice
  if (!SD.begin(4))
  {
    Serial.println("SD card not present...");
    return;
  }

  //read from file which contains configuration for network
  File fh = SD.open("network.txt", FILE_READ);
  char netBuffer[32];

  int chPos = 0;
  int lineNo = 0;

  while (fh.available())
  {
    char ch = fh.read();
    if (ch == '\n') {
      chPos = 0;
      switch (lineNo) {
        case 0:
          if (getMAC(netBuffer, myMac)) Serial.println(F("mac ok"));
          break;

        case 1:
          if (getIP(netBuffer, myIP)) Serial.println(F("ip ok"));
          break;

        case 2:
          if (getIP(netBuffer, myNM)) Serial.println(F("netmask ok"));
          break;

        case 3:
          if (getIP(netBuffer, myGW)) Serial.println(F("gateway ok"));
          break;

        case 4:
          if (getIP(netBuffer, myDNS)) Serial.println(F("dns ok"));
          break;
      }

      lineNo++;
    }
    else if (ch == '\r') {
      // do nothing
    }
    else if (chPos < 31) {
      netBuffer[chPos] = ch;
      chPos++;
      netBuffer[chPos] = 0;
    }
  }

  fh.close();

  int x;

  Serial.print("\r\nmac ");
  for (x = 0; x < 6; x++) {
    Serial.print(myMac[x], HEX);
    if (x < 5) Serial.print(":");
  }

  Serial.print("\r\nip ");
  for (x = 0; x < 4; x++) {
    Serial.print(myIP[x], DEC);
    if (x < 3) Serial.print(".");
  }

  Serial.print("\r\nnetmask ");
  for (x = 0; x < 4; x++) {
    Serial.print(myNM[x], DEC);
    if (x < 3) Serial.print(".");
  }

  Serial.print("\r\ngateway ");
  for (x = 0; x < 4; x++) {
    Serial.print(myGW[x], DEC);
    if (x < 3) Serial.print(".");
  }

  Serial.print("\r\ndns ");
  for (x = 0; x < 4; x++) {
    Serial.print(myDNS[x], DEC);
    if (x < 3) Serial.print(".");
  }

  Serial.println("\r\nStarting ethernet");
  Ethernet.begin(myMac, myIP, myDNS, myGW, myNM);
  Serial.println(Ethernet.localIP());
  Serial.println("\r\nStarting webserver");  
  server.begin();

  

}

void loop() {
  EthernetClient client = server.available();

  if (client) {
    Serial.println("new client");

    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }

}


byte getMAC(char* macBuf, byte* thisMAC) {
  byte thisLen = strlen(macBuf);
  byte thisOctet = 1;

  thisMAC[0] = strtol(&macBuf[0], NULL, 16);

  for (int x = 0; x < thisLen; x++) {
    if (macBuf[x] == ':') {
      thisMAC[thisOctet] = strtol(&macBuf[x + 1], NULL, 16);
      thisOctet++;
    }
  }

  if (thisOctet == 6) return (1);
  else return (0);

}

byte getIP(char* ipBuf, byte* thisIP) {
  byte thisLen = strlen(ipBuf);
  byte thisOctet = 1;

  thisIP[0] = atoi(&ipBuf[0]);

  for (int x = 0; x < thisLen; x++) {
    if (ipBuf[x] == '.') {
      thisIP[thisOctet] = atoi(&ipBuf[x + 1]);
      thisOctet++;
    }
  }

  if (thisOctet == 4) return (1);
  else return (0);
}
