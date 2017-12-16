/*****************************************************************
   Merjenje temperature in slanosti s pomočjo
   temperaturne sonde SeaBird SBE-3,
   sonde za slanost Seabird SBE-4 ter miktonotrolerja
   Arduino UNO

   Peter Valenčič, 05.12.2017
 ****************************************************************/

#include <SPI.h>
#include <SD.h>
#include <Ethernet.h>

//mrežne nastavitve
byte myMac[6];
byte myIP[4];
byte myNM[4];
byte myGW[4];
byte myDNS[4];

//priključne sponke za SBE3 in SBE4
const byte interruptPinTemp = 2;
const byte interruptPinSlanost = 3;


//konstante za SBE3
//Kalibrirano Sea-Bird GmbH, SBE3 T121219Mar16
const double g_sbe3 = 4.85442486e-003;
const double h_sbe3 = 6.77400494e-004;
const double i_sbe3 = 2.65502731e-005;
const double j_sbe3 = 2.06782794e-006;
const double f_sbe3 = 1000;

//konstante za SBE4
//Kalibrirano Sea-Bird GmbH, SBE4 C141601Mar16
const double g_sbe4 = -4.10731453e+000;
const double h_sbe4 =  5.02267656e-001;
const double i_sbe4 = -1.63378659e-004;
const double j_sbe4 =  3.70269818e-005;
const double f_sbe4 = 1000;
const double Pcor   = -9.5700e-008;
const double Tcor   = 3.2500e-006;
const double p      = 0;

//pomožne spremenljivke
int timer1_counter;
int cnt_temp = 0;
int cnt_slanost = 0;

int l_temp;
int l_slanost;

boolean bPrvaMeritev = false;

EthernetServer server = EthernetServer(80);

/**
   setup(), metoda se izvede ob priključitvi napetosti na mikroknotroler.
   V metodi se nastavijo vsi potrebni parametri
*/
void setup() {
  Serial.begin(9600);
  noInterrupts();     //prekinemo izvajanje interruptov
  delay(500);
  Serial.println("+----------------------+");

  //določimo vhode za branje frekvence
  pinMode(interruptPinTemp, INPUT_PULLUP);
  pinMode(interruptPinSlanost, INPUT_PULLUP);

  //določimo metodo, ki se bo ob prekinitvi izvedla
  attachInterrupt(digitalPinToInterrupt(interruptPinTemp), beriTemperaturo, CHANGE);
  attachInterrupt(digitalPinToInterrupt(interruptPinSlanost), beriSlanost, CHANGE);

  //nastavimo časovno prekinitev na 1 sekundo pri taktu 16 MHz
  TCCR1A = 0;
  TCCR1B = 0;
  timer1_counter = 34286;   // preload timer 65536-16MHz/256/2Hz

  TCNT1 = timer1_counter;   // preload timer
  TCCR1B |= (1 << CS12);    // 256 prescaler
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt

  // preveri če obstaja SD kartica
  if (!SD.begin(4))
  {
    Serial.println("SD card not present...");
    return;
  }

  //prebere datoteko networ.txt, v njej so shranjene mrežne nastavitve
  File fh = SD.open("network.txt", FILE_READ);
  char netBuffer[32];

  int chPos = 0;
  int lineNo = 0;

  //sprehodimo se skozi vse zapise v datoteki in ustrezno zapišemo vrednosti
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

  //startamo server socket
  server.begin();

  //omogočimo prekinitve
  interrupts();

}


/**
   Prekinitvena servisna rutina. Izvaja se 1x na sekundo
*/
ISR(TIMER1_OVF_vect)
{

  if (bPrvaMeritev == false) return;

  TCNT1 += timer1_counter; //preload časovnika

  
    l_temp = cnt_temp;
    l_slanost = cnt_slanost;

  
    //resetiramo števce
    cnt_temp = 0;
    cnt_slanost = 0;
  
}

//števec, ki se povečuje v prekinitvi za temperaturo
void beriTemperaturo()
{
  cnt_temp++;
}

//števec, ki se povečuje v prekinitvi za slanost
void beriSlanost()
{
  cnt_slanost++;
}

//metoda preračuna temperaturo iz frekvence
double calcTemp(double frekvenca)
{
  
  double logRes = log(f_sbe3 / frekvenca);
  return (1.0 / (g_sbe3 + (h_sbe3 * logRes) + (i_sbe3 * pow(logRes, 2.0)) + (j_sbe3 * pow(logRes, 3.0))) - 273.15);
}

//metoda preračuna slanost iz frekvence
double calcSlanost(double f,double temperatura)
{
  return (g_sbe4+h_sbe4*pow(f,2)+i_sbe4*pow(f,3)+j_sbe4*pow(f,4))/10*(1+Tcor*temperatura+Pcor*p); //   1.0 / (g_sbe3 + (h_sbe3 * logRes) + (i_sbe3 * pow(logRes, 2.0)) + (j_sbe3 * pow(logRes, 3.0))) - 273.15;
}

/**
   Glavna zanka programa
*/
void loop() {

  //prvo meritev izpustimo
  if (bPrvaMeritev == false)
  {
    bPrvaMeritev = true;
    l_temp = 0;
    l_slanost = 0;
    return;
  }
  
     Serial.println("xxx");
     Serial.println(l_temp);
     Serial.println(l_slanost);
     
    // Serial.println(calcTemp(l_temp),3);
    
    

  EthernetClient client = server.available();

  if (client) {
    Serial.println("nova zahteva");
    boolean bTekocaVrstica = true;
    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        Serial.write(c);
        if (c == '\n' && bTekocaVrstica)
        {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/xml");
          client.println("Connection: close"); 
          client.println(); 
          client.println("<?xml version='1.0' encoding='UTF-8'?>");
          client.println("<root>");
          client.println("<temperature>");
          client.println("<value>");
          client.print(calcTemp(l_temp), 3);
          client.println("</value>");
          client.println("<freq>");
          client.println(l_temp);
          client.println("</freq>");
          client.println("</temperature>");
          client.println("<salinity>");
          client.println("<value>");
          client.print(calcSlanost(l_slanost,0), 3);
          client.println("</value>");
          client.println("<freq>");
          client.println(l_slanost);
          client.println("</freq>");
          client.println("</salinity>");
          client.println("</root>");
          client.println();
          break;
        }

        if (c == '\n')
        {
          bTekocaVrstica = true;
        } else if (c != '\r') {
          bTekocaVrstica = false;
        }

      }
    }
    delay(1);
    // zapremo povezavo
    client.stop();
  }


 
}

//metoda prebere MAC vrednost in jo zapiše v array
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

//metoda prebere IP naslov in ga zapiše v array
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
