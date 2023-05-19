/*
//This program was made to receive the raw signals of an IR remote
//The pedestal fan that I use did not follow any common IR protocols (like NEC), so emitting the hexadecimal codes from the IR emitter would not work and hence
//the timestamps where the IR diode would be on/off in microseconds was used to replicate the remote

//Below are the commands that are used by my household fan

//POWER: 0x40D
uint16_t rawData[23] = {1380,370, 1380,370, 480,1220, 1380,370, 1330,420, 480,1220, 480,1220, 430,1270, 430,1220, 480,1220, 480,1220, 1380};

//SPEED: 0x20D
uint16_t rawData[23] = {1330,420, 1330,420, 430,1270, 1280,470, 1280,420, 430,1270, 430,1270, 430,1270, 430,1270, 380,1320, 1280,470, 430};

//TRIANGLE: 0x10D
uint16_t rawData[23] = {1330,420, 1330,420, 430,1270, 1330,420, 1280,470, 430,1220, 430,1270, 430,1320, 380,1270, 1280,470, 430,1270, 430};

//TIME: 0x8D
uint16_t rawData[23] = {1380,420, 1330,420, 480,1220, 1330,420, 1330,470, 430,1220, 480,1220, 480,1220, 1380,420, 430,1270, 430,1270, 480};  

//SWIVEL: 0x4D
uint16_t rawData[23] = {1380,370, 1380,370, 480,1220, 1330,370, 1380,270, 630,1170, 530,1170, 1380,370, 480,1220, 480,1220, 480,1220, 480};
*/

const uint16_t rawData[23] = {1380,370, 1380,370, 480,1220, 1330,370, 1380,270, 630,1170, 530,1170, 1380,370, 480,1220, 480,1220, 480,1220, 480};

#include <IRremote.hpp>

#define PIN_LED 2
#define PIN_RECEIVER 15
#define PIN_SENDER 4

int receive_or_send = 1; //0 for receiving raw IR signals, 1 for sending raw IR signals

void receive_setup() {
  Serial.println("Receiving...");
  IrReceiver.begin(PIN_RECEIVER, ENABLE_LED_FEEDBACK);
}

void send_setup() {
  Serial.println("Sending Setup");
  IrSender.begin(PIN_SENDER);
}

void setup() {
  // put your setup code here, to run once:
  pinMode(PIN_LED, OUTPUT);
  Serial.begin(115200);
  if (receive_or_send == 0)
    receive_setup();
  else
    send_setup();
}

void loop() {
  // put your main code here, to run repeatedly:
  while (receive_or_send == 0) {
    if (IrReceiver.decode()) {
      IrReceiver.printIRResultShort(&Serial);
      Serial.println("Raw Data: ");
      IrReceiver.compensateAndPrintIRResultAsCArray(&Serial, true); // Output the results as uint16_t source code array of micros
    }
    IrReceiver.resume();
  }
  while (receive_or_send == 1) {
    Serial.println("SENDING SIGNAL");
    IrSender.sendRaw_P(rawData, sizeof(rawData) / sizeof(rawData[0]), NEC_KHZ);    
    delay(3000);
  }
}