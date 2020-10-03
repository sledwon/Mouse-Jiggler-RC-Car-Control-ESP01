/*
 * Pins on ESP-01
    VCC, Voltage (+3.3 V; can handle up to 3.6 V)
    GND, Ground (0 V)
    RX, (Also GPIO3) - Used as an input - Low puts it into debug mode - short cycle - OTA
    TX, (Also GPIO1)
    CH_PD, Chip power-down 
    RST, Reset - No Connection
    GPIO0, Grounded to program , Used as Output PWM - Input to the RX2 type chip
    GPIO2, Use as an Output
    
 */

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

const char* ssid = "anttrail";
const char* password = "pswd3667";

#define GPIO0 0
#define GPIO2 2
#define BLUELED 1
#define TX 1
#define GPIO1 1
#define RX 3
#define GPIO3 3

#define DELAY_LONG_MS 600000    // 10 minutes
//#define DELAY_LONG_MS  15000    // 15 seconds
#define DELAY_SHORT_MS 10000    // 10 seconds

#define w1freq 995
#define w2freq 500
#define w2micros 2000
#define w2num 4

#define w1duty 511
#define w2duty 767
#define w1micros 1000
#define NUMENDS 16
#define ENDTIMETENTHS 2

//W1
#define STOP           4
#define FORWARD        10
#define FORWARD_TURBO  16
#define TURBO          22
#define FORWARD_LEFT   28
#define FORWARD_RIGHT  34
#define BACKWARD       40
#define BACKWARD_RIGHT 46
#define BACKWARD_LEFT  52
#define LEFT           58
#define RIGHT          64

int cmd_time_tenths = 20;  //20 tenth seconds = 2 seconds
int delay_ms;


void setup() {

  pinMode(GPIO0,OUTPUT);
  pinMode(BLUELED, OUTPUT);
  pinMode(GPIO2, INPUT_PULLUP);
  pinMode(GPIO3, INPUT_PULLUP);
  analogWrite(GPIO0,0);
  delay(5000);
  if (digitalRead(GPIO3)==LOW){ // debug mode - short cycle
    flash_led (BLUELED, 250, 250, 4);
    delay_ms = DELAY_SHORT_MS;
  //Serial.begin(115200);
  //Serial.println("Booting");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    //Serial.println("Connection Failed! Rebooting...");
      delay(5000);
      ESP.restart();
    }

    // Port defaults to 8266
    // ArduinoOTA.setPort(8266);
  
    // Hostname defaults to esp8266-[ChipID]
    // ArduinoOTA.setHostname("myesp8266");
  
    // No authentication by default
    // ArduinoOTA.setPassword("admin");
  
    // Password can be set with it's md5 value as well
    // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
    // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");
  
    ArduinoOTA.onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else { // U_SPIFFS
        type = "filesystem";
      }
  
      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    //Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
    //Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    //Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
    //Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
      //Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
      //Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
      //Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
      //Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) {
      //Serial.println("End Failed");
      }
    });
    ArduinoOTA.begin();
  //Serial.println("Ready");
  //Serial.print("IP address: ");
  //Serial.println(WiFi.localIP());
    jiggle_left_right(3);
  }
  else{
    flash_led (BLUELED, 500, 500, 4);
    delay_ms = DELAY_LONG_MS;
    WiFi.mode( WIFI_OFF );
    WiFi.forceSleepBegin();
    jiggle_fwd_backward(3);
  }
  delay(2000);
  flash_led (BLUELED, 250, 250, 5);
}

void loop() {
  if (digitalRead(GPIO3)==LOW){ // debug mode - short cycle
    ArduinoOTA.handle();
    flash_led (BLUELED, 250, 250, 3);
    jiggle_left_right(3);
    delay(DELAY_SHORT_MS); 
  }
  else{
    flash_led (BLUELED, 100, 100, 3);  
    jiggle_fwd_backward(3);
    delay(DELAY_LONG_MS);
  }
}

void jiggle_fwd_backward(int numtimes){
  digitalWrite(BLUELED, LOW);
  analogWrite(GPIO0,0);
  for (int j=0;j<=numtimes-1;j++){
    send_cmds(FORWARD, 4);
    send_cmds(STOP, 4);
    send_cmds(BACKWARD, 2);
    send_cmds(STOP, 4);
  }
  send_cmds(STOP, 16);
  analogWrite(GPIO0,0);
  digitalWrite(BLUELED, HIGH);
}

void jiggle_left_right(int numtimes){
  digitalWrite(BLUELED , LOW);
  analogWrite(GPIO0,0);
  for (int j=0;j<=numtimes-1;j++){
    send_cmds(LEFT, 2);
    send_cmds(STOP, 4);
    send_cmds(RIGHT, 2);
    send_cmds(STOP, 4);
  }
  send_cmds(STOP, 16);
  analogWrite(GPIO0,0);
  digitalWrite(BLUELED, HIGH);
}
// This will send the command for the time period specified in integer tenths of seconds
// i.e. 20 tenths would be 2 seconds
void send_cmd_tenths(int rc_cmd, int cmd_time_tenths){
  int rc_cmd_time_ms = (w2micros*w2num + rc_cmd*w1micros)/1000;
  int num_cmds = (cmd_time_tenths * 100)/rc_cmd_time_ms;
  for (int k=0;k<=num_cmds-1;k++){
    analogWriteFreq(w2freq);
    analogWrite(GPIO0,w2duty);
    delayMicroseconds(w2micros*w2num-100);
  
    analogWriteFreq(w1freq);
    analogWrite(GPIO0,w1duty);
    delayMicroseconds(w1micros*rc_cmd);
  }
}
// just sends the number of commands specified 
void send_cmds(int rc_cmd, int num_cmds){
  
  for (int i=0;i<=num_cmds-1;i++){
    analogWriteFreq(w2freq);  
    analogWrite(GPIO0,w2duty);
    delayMicroseconds(w2micros*w2num-100);
  
    analogWriteFreq(w1freq);  
    analogWrite(GPIO0,w1duty);
    delayMicroseconds(w1micros*rc_cmd);
    
  }
}

void flash_led (int ledpin, int ontime, int offtime, int numtimes){
  pinMode(ledpin, OUTPUT);
  for (int i=0; i < numtimes;i++){
    digitalWrite(ledpin, LOW);
    delay (ontime);
    digitalWrite(ledpin, HIGH);
    delay (offtime);
  }
}
