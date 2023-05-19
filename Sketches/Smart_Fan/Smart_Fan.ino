#include <WiFi.h>
#include <SinricPro.h>
#include <SinricProFan.h>
#include <IRremote.hpp>
#include <WebServer.h>

WebServer server(80);

//WiFi Network name and password
#define WIFI_SSID  "XXXXXX"
#define WIFI_PASS  "XXXXXX"

//Sinric Pro credentials
#define DEVICE_ID  "XXXXXX"
#define APP_KEY    "XXXXXX"
#define APP_SECRET "XXXXXX"

#define BAUD_RATE 115200

//IR emitter pin 
//#define PIN_RECEIVER 15
#define PIN_SENDER 4

//Indexes for each IR signal (no protocol) to refer to IRData array
#define POWER 0
#define SPEED 1
#define TIME 2
#define SWIVEL 3
#define CLIMATE 4
#define SIGNAL_SIZE 23

uint16_t IRData[5][SIGNAL_SIZE] = {{1380,370, 1380,370, 480,1220, 1380,370, 1330,420, 480,1220, 480,1220, 430,1270, 430,1220, 480,1220, 480,1220, 1380},
                           {1330,420, 1330,420, 430,1270, 1280,470, 1280,420, 430,1270, 430,1270, 430,1270, 430,1270, 380,1320, 1280,470, 430},
                           {1380,420, 1330,420, 480,1220, 1330,420, 1330,470, 430,1220, 480,1220, 480,1220, 1380,420, 430,1270, 430,1270, 480},
                           {1380,370, 1380,370, 480,1220, 1330,370, 1380,270, 630,1170, 530,1170, 1380,370, 480,1220, 480,1220, 480,1220, 480},
                           {1330,420, 1330,420, 430,1270, 1330,420, 1280,470, 430,1220, 430,1270, 430,1320, 380,1270, 1280,470, 430,1270, 430}};

struct {
  bool powerState = false;
  int speed = 0; //0,1,2
  int time = 0; //0 ... 15, 0 is unlimited time
  bool swivel = false;
  int climate = 0; //0,1,2
} device_state; 

//Common variables used in the handleRoot() function to see the mode which needs to be set, and the number of IR pulses to set to that mode
int mode_to_set_to = 0;
int num_ir_pulses = 0;

void setupWiFi() {
  Serial.println("Wifi Connecting");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf(".");
    delay(500);
  }
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Wi-Fi Set Up!");
}

void setupIRSend() {
  IrSender.begin(PIN_SENDER);
  Serial.println("IR Sending Set Up!");
}

void setupSinricPro() {
  SinricProFan &myFan = SinricPro[DEVICE_ID];
  myFan.onPowerState(onPowerState);
  SinricPro.begin(APP_KEY, APP_SECRET);  
  Serial.println("Sinric Pro Set Up!");
}

void setupServer() {
  server.on("/", handleRoot);
  server.begin();
  Serial.println("Web Server Set Up!");
}

void sendIRSignal(uint16_t *rawData, size_t size, int num_signals = 1) {
  //Size is always 23 for the fan, but I kept it as a parameter to use for other applications
  for (int i = 0; i < num_signals; i++) {
    Serial.println("SENDING SIGNAL");
    IrSender.sendRaw_P(rawData, size, NEC_KHZ);
    delay(200);
  } 
}

bool onPowerState(const String &deviceId, bool &state) {
  sendIRSignal(IRData[POWER], SIGNAL_SIZE);
  device_state.powerState = !device_state.powerState;

  device_state.speed = 0;
  device_state.time = 0;
  device_state.swivel = false;
  device_state.climate = 0;

  Serial.println("Power signal sent");
  return true;
}

void handleRoot() {
  String html = "<!DOCTYPE html>\
                  <html>\
                    <head>\
                      <meta charset='UTF-8'>\
                      <meta name='viewport' content='width=device-width, initial-scale=1.0'>\
                      <title>Smart Home Fan Automation</title>\
                      <style>\
                        .button {\
                          background-color: #007AFF;\
                          padding: 8px;\
                          color: white;\
                          border-radius: 15px;\
                          font-weight: bold;\
                        }\
                      </style>\
                    </head>\
                    <body>\
                      <h1>Smart Home Fan Automation</h1>\
                      <h2>Power</h2>\
                      <a href='/?power=on-off'><button class='button'>Power On/Off</button></a>\
                      <h2>Speed</h2>\
                      <a href='/?speed=slow'><button class='button'>Slow</button></a>\
                      <a href='/?speed=medium'><button class='button'>Medium</button></a>\
                      <a href='/?speed=fast'><button class='button'>Fast</button></a>\
                      <h2>Time</h2>\
                      <p>By default, the fan is on indefinitely. Multi-select the buttons below to set a fan timer:</p>\
                      <form action='/'>\
                      <input type='checkbox' id='half-hour' name='time' value='0.5h'>\
                      <label for='half-hour'>30 Minutes</label><br>\
                      <input type='checkbox' id='one-hour' name='time' value='1h'>\
                      <label for='one-hour'>1 Hour</label><br>\
                      <input type='checkbox' id='two-hours' name='time' value='2h'>\
                      <label for='two-hours'>2 Hours</label><br>\
                      <input type='checkbox' id='four-hours' name='time' value='4h'>\
                      <label for='four-hours'>4 Hours</label><br>\
                      <input type='submit' value='Set Time'>\
                      </form>\
                      <h2>Swivel</h2>\
                      <a href='/?swivel=on-off'><button class='button'>Swivel On/Off</button></a>\
                      <h2>Climate Control</h2>\
                      <p>By default, the fan has no climate control mode, click the buttons below to set a climate mode</p>\
                      <a href='/?climate=oasis'><button class='button'>Oasis Mode</button></a>\
                      <a href='/?climate=night'><button class='button'>Night Mode</button></a>\
                    </body>\
                  </html>";

  server.send(200, "text/html", html);

  if (server.arg("power") == "on-off") {
    onPowerState(DEVICE_ID, device_state.powerState);
  }
  else if (server.hasArg("speed")) {
    //SPEED SELECTED
    //int speed = 0; //0,1,2
    if (server.arg("speed") == "slow")
      mode_to_set_to = 0; 
    else if (server.arg("speed") == "medium")
      mode_to_set_to = 1; 
    else if (server.arg("speed") == "fast")
      mode_to_set_to = 2;
    
    //Calculating how many times the speed signal must be sent depending on current and future state
    if (device_state.speed > mode_to_set_to)
      num_ir_pulses = mode_to_set_to + 3 - device_state.speed; //3 as that is the number of speed options
    else
      num_ir_pulses = mode_to_set_to - device_state.speed;
    
    sendIRSignal(IRData[SPEED], SIGNAL_SIZE, num_ir_pulses);
    device_state.speed = mode_to_set_to;
    Serial.println("Speed signal sent");
  }  
  else if (server.hasArg("time")) {
    //TIME SELECTED
    
    //This number represents the number of time the signal needs to be sent to get to said time
    mode_to_set_to = 0;

    int paramsCount = server.args();

    //Going through the multi-selected options and summing up the number of times the time signal must be sent
    for (int i = 0; i < paramsCount; i++) {
      String option = server.arg(i);
    
      if (option == "0.5h")
        mode_to_set_to+=pow(2,0); //or 1
      else if (option == "1h")
        mode_to_set_to+=pow(2,1); //or 2
      else if (option == "2h")
        mode_to_set_to+=pow(2,2); //or 4
      else if (option == "4h")
        mode_to_set_to+=pow(2,3); //or 8
    }
    Serial.println(mode_to_set_to);

    if (device_state.time > mode_to_set_to)
      num_ir_pulses = mode_to_set_to + 16 - device_state.time; //16 as that is the number of time options
    else
      num_ir_pulses = mode_to_set_to - device_state.time;
    
    sendIRSignal(IRData[TIME], SIGNAL_SIZE, num_ir_pulses);
    device_state.time = mode_to_set_to;
    Serial.println("Time signal sent");
  }
  else if (server.arg("swivel") == "on-off") {
    //SWIVEL SELECTED

    sendIRSignal(IRData[SWIVEL], SIGNAL_SIZE);
    device_state.swivel = !device_state.swivel;
    Serial.println("Swivel signal sent");
  }
  else if (server.hasArg("climate")) {
    //CLIMATE SELECTED

    //int climate = 0; //0,1,2
    if (server.arg("climate") == "oasis")
      mode_to_set_to = 1; 
    else if (server.arg("climate") == "night")
      mode_to_set_to = 2; 

    //Calculating how many times the climate signal must be sent depending on current and future state
    if (device_state.climate > mode_to_set_to)
      num_ir_pulses = mode_to_set_to + 3 - device_state.climate; //3 as that is the number of climate options
    else
      num_ir_pulses = mode_to_set_to - device_state.climate;
    
    sendIRSignal(IRData[CLIMATE], SIGNAL_SIZE, num_ir_pulses);
    device_state.climate = mode_to_set_to;
    Serial.println("Climate signal sent");
  }  
}

void setup() {
  Serial.begin(BAUD_RATE);
  setupWiFi();
  setupSinricPro();
  setupIRSend();
  setupServer();
}

void loop() {
  // put your main code here, to run repeatedly:
  SinricPro.handle();
  server.handleClient();
}
