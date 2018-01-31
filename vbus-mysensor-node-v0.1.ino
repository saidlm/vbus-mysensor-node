// Enable auto node ID
#define MY_NODE_ID 11

// Enable RS485 transport layer
#define MY_RS485

// Define this to enables DE-pin management on defined pin
// 7 - DE
// 8 - RX
// 9 - TX
#define MY_RS485_DE_PIN 7

// Set RS485 baud rate to use
#define MY_RS485_BAUD_RATE 19200

// Interval between reporting to MySensors network
#define REPORTINTERVAL (5 * 60 * 1000UL)

// MySensors prefixes for child IDs
#define MS_SYSTEM 0
#define MS_TEMP 10
#define MS_PUMP 60
#define MS_RELAY 70

#include <MySensors.h>
#include <SoftwareSerial.h>
#include <vbusdecoder.h>

// VBUS hardware and object initialization
SoftwareSerial vbusSerial(6,5);  // RX, TX
VBUSDecoder vbus(&vbusSerial);

// Initialize temperature message
MyMessage msgTemp(MS_TEMP,V_TEMP);
MyMessage msgStat(MS_SYSTEM,V_STATUS);
MyMessage msgPump(MS_PUMP, V_POWER_FACTOR);
MyMessage msgRelay(MS_RELAY, V_STATUS);

uint32_t lastMillis = millis();
uint8_t tempNum;
uint8_t pumpNum;
uint8_t relayNum;
uint8_t connectAttempt;
bool comErr = false;

float tempReadConvert(float tempC) {
  if (getControllerConfig().isMetric)
    return tempC;
  else
    return (tempC * 1.8 + 32);
}

void presentation() {
  uint8_t i;

  sendSketchInfo("Resol VBUS convertor", "1.0");
  present(0, S_BINARY);  
  
  while (!vbus.isReady()) {
    vbus.loop();
    if ((millis() - lastMillis) > REPORTINTERVAL)
      send(msgTemp.setSensor(0).setType(V_STATUS).set(1));
  }
  
  tempNum = vbus.getTempNum();
  relayNum = vbus.getRelayNum();
  pumpNum = vbus.getPumpNum();

  for (i = MS_TEMP; i < MS_TEMP + tempNum; i++)
    present(i, S_TEMP);
 
  for (i = MS_PUMP; i < MS_PUMP + pumpNum; i++)
    present(i, S_POWER);
    
  for (i = MS_RELAY; i < MS_RELAY + relayNum; i++)
    present(i, S_BINARY);
}

void before() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  
  vbusSerial.begin(9600);
  vbus.begin();

  Serial.begin(115200);
  Serial.println(F("Starting test of VBUS code.")); 
}

void setup() {
}

void loop() {
  vbus.loop();

  if ((millis() - lastMillis) > REPORTINTERVAL) {
    digitalWrite(LED_BUILTIN, HIGH);
    if (vbus.getVbusStat()) {
      if (vbus.isReady()) {
        for (uint8_t i = 0; i < tempNum; i++) {
          send(msgTemp.setSensor(i + MS_TEMP).setType(V_TEMP).set(tempReadConvert(vbus.getTemp(i)),1));
        }
  
        for (uint8_t i = 0; i < pumpNum; i++) {
          send(msgTemp.setSensor(i + MS_PUMP).setType(V_POWER_FACTOR).set(vbus.getPump(i)));
        }
  
        for (uint8_t i = 0; i < relayNum; i++) {
          send(msgTemp.setSensor(i + MS_RELAY).setType(V_STATUS).set(vbus.getRelay(i)));
        }
        comErr = false;
        connectAttempt = 0;
        digitalWrite(LED_BUILTIN, LOW);
      }
      else { 
        connectAttempt++;
      }
    }
    else 
      comErr = true;

    if (connectAttempt > 3)
      comErr = true;
      
    send(msgTemp.setSensor(0).setType(V_STATUS).set(comErr));
   
    lastMillis = millis();
  }
}
