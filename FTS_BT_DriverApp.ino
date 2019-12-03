#include <Arduino.h>
#include <HardwareSerial.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
//-------------------------------
BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t txValue = 0;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

#define LED1 33  //  WRITH
#define LED2 25  //  BLUE
//-------------------------------
unsigned long currentMillisLED1;
unsigned long LED1Millis;  // when button was released
unsigned long LED1OnDelay; // wait to turn on LED
bool ledState1 = false;
bool onLED1 = false;

unsigned long currentMillisLED2;
unsigned long LED2Millis;  // when button was released
unsigned long LED2OnDelay; // wait to turn on LED
bool ledState2 = false;
bool onLED2 = false;

unsigned long currentMillisLoop;
unsigned long LoopMillis;    // when button was released
unsigned long LoopOnDelay;   // wait to turn on LED
unsigned long LoopOffDelay;  // wait to turn on LED
unsigned long ledTurnedOnAt; // when led was turned on
bool ledReady = false;
bool onLoop = false;
bool statL = false;
bool ledstate = false;
//-----
char buf[512], indexbuf;
//char txString[20];
int A = 0;
int pCharacter;
String str , RxBlutooth = "";
char sbuf[512];
bool sendBT_flag = false , isNullRxBT = false;
//********************** LED Blink delay millis function ***********************
void LED1_blink()
{
  currentMillisLED1 = millis();
  if (onLED1 == true)
  {
    LED1Millis = currentMillisLED2;
    ledState1 = true;
    onLED1 = false;
  }
  if (ledState1 == true)
  {
    digitalWrite(LED1, HIGH);
    if ((unsigned long)(currentMillisLED1 - LED1Millis) >= LED1OnDelay)
    {
      digitalWrite(LED1, LOW);
      ledState1 = false;
    }
  }
}
void LED2_blink()
{
  currentMillisLED2 = millis();
  if (onLED2 == true)
  {
    LED2Millis = currentMillisLED2;
    ledState2 = true;
    onLED2 = false;
  }
  if (ledState2 == true)
  {
    digitalWrite(LED2, HIGH);
    if ((unsigned long)(currentMillisLED2 - LED2Millis) >= LED2OnDelay)
    {
      digitalWrite(LED2, LOW);
      ledState2 = false;
    }
  }
}

void LED2_Blinlkloop()
{
  currentMillisLoop = millis();
  if (onLoop == true)
  {
    if (statL == false)
    {
      LoopMillis = currentMillisLoop;
      statL = true;
      ledReady = true;
    }
  }
  else
  {
    ledReady = false;
    statL = false;
  }

  if (ledReady == true)
  {
    if ((unsigned long)(currentMillisLoop - LoopMillis) >= LoopOnDelay)
    {
      digitalWrite(LED2, HIGH);
      //Serial.println("*** LED2 ON *****");
      ledstate = true;
      ledTurnedOnAt = currentMillisLoop;
      ledReady = false;
    }
  }
  if (ledstate == true)
  {
    if ((unsigned long)(currentMillisLoop - ledTurnedOnAt) >= LoopOffDelay)
    {
      digitalWrite(LED2, LOW);
      //Serial.println("*** LED2 oFF *****");
      ledstate = false;
      statL = false;
    }
  }
}

void LED1_onBlinlk(unsigned long timeBlink)
{
  onLED1 = true;
  LED1OnDelay = timeBlink;
}

void LED2_onBlinlk(unsigned long timeBlink)
{
  onLED2 = true;
  LED2OnDelay = timeBlink;
}

void LED2_on_Blinlkloop(unsigned long timeOn, unsigned long timeOff)
{
  onLoop = true;
  LoopOnDelay =  timeOff;
  LoopOffDelay = timeOn;
}

void LED2_off_Blinlkloop()
{
  onLoop = false;
  digitalWrite(LED2, HIGH);
}

//*********************************************************************************

void clearTXdata_Serial(int Len)
{
  for (int i = 0; i <= Len; i++)
  {
    buf[i] = NULL;
  }
  indexbuf = 0;
}

// void clearTXdata_BT(int Len)
// {
//   for (int i = 0; i <= Len; i++)
//   {
//     RxBt[i] = NULL;
//   }
  
// }


//----------------------------------------------------
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
      char RxBt[512];
      if (rxValue.length() > 0) {
        isNullRxBT = true;
         for (int i = 0; i < rxValue.length(); i++){
           RxBt[i] = rxValue[i];
         }
     //    Serial2.println(" Serial BT Send. . ");
      }
      else isNullRxBT = false;
      RxBlutooth = RxBt;

       for (int i = 0; i <= rxValue.length(); i++)
  {
    RxBt[i] = NULL;
  }
    }
};

//---------------------------------------------------
void setup()
{
  // xTaskCreate(&TaskSerial, "TaskSerial",  4096, NULL, 4, NULL);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  Serial.begin(115200);
  //  Serial.setTimeout(30);
  Serial2.begin(9600);
  Serial2.setTimeout(45);
  Serial.setTimeout(45);
 // Serial2.println("BT - SMART Waitting to Pair . . . !");
  Serial.println("BT - SMART Waitting to Pair . . . !");
  digitalWrite(LED2, HIGH);
  
/// Create the BLE Device
//std::string ble_name = "บลูทูธ";
//char buffer_test[] = {0xE0,0xB8,0xA5,
//0xE0,0xB8,0xAD,
//0xE0,0xB8,0x87,
//0xE0,0xB8,0x94,
//0xE0,0xB8,0xB9,
//0xE0,0xB8,0x99,
//0xE0,0xB8,0xB0,
//0x0};      // null terminated string;

//strcpy(buffer_test, ble_name.c_str());  // copy from str to byte[]
//  for (int i = 0; i < sizeof(buffer_test)-1; i++)
//  {
//    Serial.print("0x");
//    Serial.print(buffer_test[i],HEX);
//    Serial.print(",");
//  }

// std::string ttt(buffer_test);
//   BLEDevice::init(ttt);
  BLEDevice::init("BT-529996");   // Bluetooth Name

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks()); 

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(
                    CHARACTERISTIC_UUID_TX,
                    BLECharacteristic::PROPERTY_NOTIFY
                  );
                      
  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
                       CHARACTERISTIC_UUID_RX,
                      BLECharacteristic::PROPERTY_WRITE
                    );

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println(" Waiting a client connection to notify . . . ");
}
//----------------------------------------------
void loop()
{
  LED1_blink();
  LED2_blink();
  LED2_Blinlkloop();
  
    if (deviceConnected) {
        pTxCharacteristic->setValue("Device Connected !!");
         LED1_onBlinlk(200);
         LED1_onBlinlk(200);
         Serial.println("  Device Connected ! !  ");
       
      if(sendBT_flag==true){
       pTxCharacteristic->setValue((unsigned char*)buf, indexbuf);
       pTxCharacteristic->notify();
      }
        // pTxCharacteristic->setValue(&txValue, 1);
        // pTxCharacteristic->notify();
        // txValue++;
    delay(10); // bluetooth stack will go into congestion, if too many packets are sent
  }

    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }

    if (Serial2.available() > 0)  
  {
    LED2_off_Blinlkloop();
    LED1_onBlinlk(100);
    indexbuf = Serial2.readBytes(buf, 512);
    for (int i = 0; i <= indexbuf; i++)
    {
   // Serial2.print(buf[i]);
      Serial.print(buf[i]);
    }
    sendBT_flag = true;
  //  Serial2.println( );
     Serial.println();
  }
  else sendBT_flag = false;

  if(isNullRxBT == true){
    LED2_on_Blinlkloop(50, 1000);
    String strrec = String("Recieve RxBlutooth : " + RxBlutooth);
  //  Serial2.println(strrec);
    Serial.println(strrec);
    isNullRxBT = false;
  }
}
//-------------------------------------
