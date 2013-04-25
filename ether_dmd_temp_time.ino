#include <SPI.h>        //SPI.h must be included as DMD is written by SPI (the IDE complains otherwise)
#include <DMD.h>        //
#include <TimerOne.h>   //
#include "SystemFont5x7.h"
#include "Arial_black_16.h"
#include <Wire.h>
#include "RTClib.h"

#define DISPLAYS_ACROSS 1
#define DISPLAYS_DOWN 1
DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);

#include "DHT.h"
#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE);
RTC_DS1307 RTC;

#include <SPI.h>
#include <Ethernet.h>

// Enter a MAC address, IP address and Portnumber for your Server below.
// The IP address will be dependent on your local network:
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress serverIP(192,168,111,42);
int serverPort=8888;

// Initialize the Ethernet server library
// with the IP address and port you want to use
EthernetServer server(serverPort);

char dmdMessage[41] = "Gold Coast TechSpace";

void ScanDMD()
{ 
  dmd.scanDisplayBySPI();
}

void setup () {

  //initialize TimerOne's interrupt/CPU usage used to scan and refresh the display
  Timer1.initialize( 5000 );           //period in microseconds to call ScanDMD. Anything longer than 5000 (5ms) and you can see flicker.
  Timer1.attachInterrupt( ScanDMD );   //attach the T

  Wire.begin();
  RTC.begin();

  if (! RTC.isrunning()) {
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }

  dht.begin();  

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, serverIP);
  server.begin();
  dmd.clearScreen( true ); 
}

void loop () {
  serviceNetwork();
  time();
  delay( 1000 );
  stats();
  delay( 1000 );
  message();
  delay( 1000 );
}

void message(void) {
  dmd.clearScreen( true );
  dmd.selectFont(Arial_Black_16);

  dmd.drawMarquee(dmdMessage, strlen(dmdMessage),(32*DISPLAYS_ACROSS)-1,0);
  long start=millis();
  long timer=start;
  boolean ret=false;
  while(!ret){
    if ((timer+30) < millis()) {
      ret=dmd.stepMarquee(-1,0);
      timer=millis();
    }
  }
}

void time(void) {
  DateTime now = RTC.now();
  char buf[9];

  dmd.clearScreen( true );
  dmd.selectFont(Arial_Black_16);

  snprintf(buf, 6, "%d:%02d", now.hour(), now.minute(), now.second());

  dmd.drawString(0, 0, buf, strlen(buf), GRAPHICS_NORMAL );
}

void stats(void) {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // check if returns are valid, if they are NaN (not a number) then something went wrong!
  if (!isnan(t) || !isnan(h)) {
    // convert float values to text
    char temp[5];
    char humid[5];

    dtostrf(h,4,1,humid);
    dtostrf(t,4,1,temp);

    // display some text
    dmd.clearScreen( true );
    dmd.selectFont(System5x7);
    dmd.drawString(0,0, temp, 4, GRAPHICS_NORMAL );
    dmd.drawString(0,9, humid, 4, GRAPHICS_NORMAL );
    dmd.drawString(27,0, "C", 1, GRAPHICS_NORMAL );
    dmd.drawString(27,9, "%", 1, GRAPHICS_NORMAL );
    dmd.drawBox(23, 0, 25, 2, GRAPHICS_NORMAL );
  }
}

void serviceNetwork(void) {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    String clientMsg ="";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        //Serial.print(c);
        clientMsg+=c;//store the recieved chracters in a string
        if (c == '\n') {
          clientMsg.toCharArray(dmdMessage, 41);
          client.println("OK");
          break;
        }
      }
    }
    // give the Client time to receive the data
    delay(1);
    // close the connection:
    client.stop();
  }
}






