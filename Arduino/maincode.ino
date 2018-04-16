/*****************************************************************
   Merjenje temperature in slanosti s pomočjo
   temperaturne sonde SeaBird SBE-3,
   sonde za slanost Seabird SBE-4 ter miktonotrolerja
   Arduino UNO

   Peter Valenčič, 05.12.2017

   1.) Sprememba preračuna slanosti iz prevodnosti, 15.04.2018
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
int cnt_prevodnost = 0;

//frekvenca iz SBE3 in SBE4
int f_temp = 0;
int f_prevodnost = 0;

boolean bPrvaMeritev = false;

EthernetServer server = EthernetServer(80);


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

    //frekvenca iz SBE3 in SBE4
    f_temp = cnt_temp;
    f_prevodnost = cnt_prevodnost;

    //resetiramo števce
    cnt_temp = 0;
    cnt_prevodnost = 0;
  
}

//števec, ki se povečuje v prekinitvi za temperaturo
void beriTemperaturo()
{
  cnt_temp++;
}

//števec, ki se povečuje v prekinitvi za slanost
void beriSlanost()
{
  cnt_prevodnost++;
}

//metoda preračuna temperaturo iz frekvence
double calcTemp(double frekvenca)
{
  double logRes = log(f_sbe3 / frekvenca);
  return (1.0 / (g_sbe3 + (h_sbe3 * logRes) + (i_sbe3 * pow(logRes, 2.0)) + (j_sbe3 * pow(logRes, 3.0))) - 273.15);
}

//metoda preračuna prevodnost S/m iz (frekvence in temperature)
double calcPrevodnost(double f,double temperatura)
{
  double frek = f;
  double tmp = temperatura;
  
  if (frek <= 0) return 0;

  frek = frek / 1000; //frekvenco pretvorimo v kHz
  return (g_sbe4+h_sbe4*pow(frek,2)+i_sbe4*pow(frek,3)+j_sbe4*pow(frek,4))/10*(1+Tcor*tmp+Pcor*p) ;  //mS/cm = S/m * 10
}

//metoda za preračun slanosti PSU za podano temperaturo, prevodnost in tlak
double calcSlanost(double t, double c, double p)
{
  double a0 = 0.008;
  double a1 = -0.1692;
  double a2 = 25.3851;
  double a3 = 14.0941;
  double a4 = -7.0261;
  double a5 = 2.7081;
  double b0 = 0.0005;
  double b1 = -0.0056;
  double b2 = -0.0066;
  double b3 = -0.0375;
  double b4 = 0.0636;
  double b5 = -0.0144;
  double k = 0.0162;

  double Aa1 = 2.070e-5;
  double Aa2 = -6.370e-10;
  double Aa3 = 3.989e-15;

  double Bb1 = 0.03426;
  double Bb2 = 0.0004464;
  double Bb3 = 0.4215;
  double Bb4 = -0.003107;
    
  double c0 = 0.6766097;
  double c1 = 0.0200564;
  double c2 = 0.0001104259;
  double c3 = -0.00000069698;
  double c4 = 0.0000000010031;

  double temp = 1.00024*t;
  double R = c / 42.914;

  double rt = c0+c1*temp+c2*temp*temp+c3*temp*temp*temp+c4*temp*temp*temp*temp;
  double alpha = (Aa1*p+Aa2*p*p+Aa3*p*p*p)/(1+Bb1*temp+Bb2*temp*temp+Bb3*R+Bb4*temp*R);
  double Rt = R/(rt*(1+alpha));
  
  double S = a0+a1*pow(Rt,0.5)+a2*Rt+a3*pow(Rt,1.5)+a4*Rt*Rt+a5*pow(Rt,2.5)+ (temp-15)*(b0+b1*pow(Rt,0.5)+b2*Rt+b3*pow(Rt,1.5)+b4*Rt*Rt+b5*pow(Rt,2.5))/(1+k*(temp-15));
  
  return S;
}

//Glavna zanka programa
void loop() {

 
  //prvo meritev izpustimo
  if (bPrvaMeritev == false)
  {
    bPrvaMeritev = true;
    f_temp = 0;
    f_prevodnost = 0;
    return;
  }
  
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
          
          //dobimo temperaturo
          double local_temp = calcTemp(f_temp) ;
          
          //dobimo prevodnost S/m in preračunamo slanost 
          double local_sal  = calcPrevodnost(f_prevodnost,local_temp);

          //pri slanosti moramo enoto S/m množiti z 10 da dobimo mS/cm
          //predpostavimo da je tlak 0.2 dBar
          local_sal = calcSlanost(local_temp, (local_sal * 10), 0.2);
          
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/xml");
          client.println("Connection: close"); 
          client.println(); 
          client.println("<?xml version='1.0' encoding='UTF-8'?>");
          client.println("<root>");
          client.println("<temperature>");
          client.println("<value>");
          client.print(local_temp, 3);
          client.println("</value>");
          client.println("<freq>");
          client.print(f_temp);
          client.println("</freq>");
          client.println("</temperature>");
          client.println("<salinity>");
          client.println("<value>");
          client.print(local_sal,4);
          client.println("</value>");
          client.println("<freq>");
          client.print(f_prevodnost);
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
