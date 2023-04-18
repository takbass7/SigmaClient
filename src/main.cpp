#include <Arduino.h>

#include "Sigma.h"
#include "BLEDevice.h"

#define bleServerName "MKII"

/* UUID's of the service, characteristic that we want to read*/
// BLE Service
static BLEUUID bmeServiceUUID("9ab83c41-05ac-4f2a-bede-3dee193f24b0");

// BLE Characteristics
//#ifdef temperatureCelsius
  //Temperature Celsius Characteristic
//  static BLEUUID temperatureCharacteristicUUID("cba1d466-344c-4be3-ab3f-189f80dd7518");
//#else
  //Temperature Fahrenheit Characteristic
//  static BLEUUID temperatureCharacteristicUUID("f78ebbff-c8b7-4107-93de-889a6a06d408");
//#endif

// Humidity Characteristic
static BLEUUID humidityCharacteristicUUID("af0357e2-be4a-4c3e-bba6-070c7aa15a45");

//Flags stating if should begin connecting and if the connection is up
static boolean doConnect = false;
static boolean connected = false;

//Address of the peripheral device. Address will be found during scanning...
static BLEAddress *pServerAddress;

//Characteristicd that we want to read
//static BLERemoteCharacteristic* temperatureCharacteristic;
static BLERemoteCharacteristic* humidityCharacteristic;

//Activate notify
const uint8_t notificationOn[] = {0x1, 0x0};
const uint8_t notificationOff[] = {0x0, 0x0};

//Variables to store temperature and humidity
//char* temperatureChar;
char* humidityChar;

//Flags to check whether new temperature and humidity readings are available
//boolean newTemperature = false;
boolean newHumidity = false;


//--- For Input String ---
bool isScan = true;
bool onScan = false;

BLEClient* pClient = NULL;
BLEScan* pBLEScan = NULL;
Sigma* sg = NULL;

//When the BLE Server sends a new humidity reading with the notify property
static void humidityNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic,
                                    uint8_t* pData, size_t length, bool isNotify) {
  //store humidity value
  humidityChar = (char*)pData;
  newHumidity = true;
  Serial.print(newHumidity);
}

//Callback function that gets called, when another device's advertisement has been received
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {

    String devName =  advertisedDevice.getName().c_str();

    if (devName.length() > 0) {
      Serial.println(devName);
    } else {
      Serial.println("*");
    }

    if (advertisedDevice.getName() == bleServerName) { //Check if the name of the advertiser matches
      isScan = false;
      onScan = false;
      advertisedDevice.getScan()->stop(); //Scan can be stopped, we found what we are looking for
      advertisedDevice.getScan()->clearResults(); // Clear scan Result to free buffer

      pServerAddress = new BLEAddress(advertisedDevice.getAddress()); //Address of advertiser is the one we need
      doConnect = true; //Set indicator, stating that we are ready to connect
      Serial.println("Connecting -> " + devName);
    }
  }
};

class MyClientCallbacks: public BLEClientCallbacks {
    void onConnect(BLEClient *pClient) {
        Serial.println("Client Connected");
    }

    void onDisconnect(BLEClient *pClient) {
        Serial.println("Client Disconnected");
        //--- Start Scan ---
        isScan = true;
        onScan = false;
    }
};

//Connect to the BLE Server that has the name, Service, and Characteristics
bool connectToServer(BLEAddress pAddress) {
    pClient = BLEDevice::createClient();

    pClient->setClientCallbacks(new MyClientCallbacks());
    // Connect to the remove BLE Server.
    pClient->connect(pAddress);
    Serial.println(" - Connected to server");

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(bmeServiceUUID);

    if (pRemoteService == nullptr) {
      Serial.print("Failed service UUID: ");
      Serial.println(bmeServiceUUID.toString().c_str());
      return (false);
    } else {
      Serial.print("Nice Service UUID:");
      Serial.println(bmeServiceUUID.toString().c_str());
    }

  // Obtain a reference to the characteristics in the service of the remote BLE server.
  //temperatureCharacteristic = pRemoteService->getCharacteristic(temperatureCharacteristicUUID);

  humidityCharacteristic = pRemoteService->getCharacteristic(humidityCharacteristicUUID);

  if (humidityCharacteristic == nullptr) {
      Serial.print("Failed Characteristic UUID: ");
      Serial.println(humidityCharacteristicUUID.toString().c_str());
  } else {
      Serial.print("Nice Characteristic UUID: ");
      Serial.println(humidityCharacteristicUUID.toString().c_str());
  }

  Serial.println ( humidityCharacteristic->readValue().c_str() );

  //Assign callback functions for the Characteristics
  //temperatureCharacteristic->registerForNotify(temperatureNotifyCallback);
  //humidityCharacteristic->registerForNotify(humidityNotifyCallback);
  return true;
}
//When the BLE Server sends a new temperature reading with the notify property
static void temperatureNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic,
                                        uint8_t* pData, size_t length, bool isNotify) {
  //store temperature value
  //temperatureChar = (char*)pData;
  //newTemperature = true;
}



void setup() {
  sg = new Sigma();
  sg->println("BLE Client V 2.0");

  //Init BLE device
  BLEDevice::init("");
}

void loop() {
  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 30 seconds.
  if (isScan && !onScan) {
      pBLEScan = BLEDevice::getScan();
      pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
      pBLEScan->setActiveScan(true);
      pBLEScan->start(30);
      onScan = true;
  }

// If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    if (connectToServer(*pServerAddress)) {
      sg->println("connected to  BLE Server.");

      //Activate the Notify property of each Characteristic
      //temperatureCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
      //humidityCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);

      connected = true;
    } else {
      Serial.println("failed to connect to the server");
    }
    doConnect = false;
  }

  //--- if Input Ready ---
  if (sg->readComplete) {
      sg->inputString.trim();
      sg->println(sg->inputString);

      const char* str = sg->inputString.c_str();

      if (strcmp(str,"X") == 0) {
          sg->println("End Session");
          pClient->disconnect();
      }

      humidityCharacteristic->writeValue(str,false );
      sg->clearread();
  }

  //if new temperature readings are available, print in the OLED
/*   if (newTemperature && newHumidity){
    newTemperature = false;
    newHumidity = false;
    printReadings();
  } */
  //delay(1000); // Delay a second between loops.
}


void serialEvent() {
  //--- If Connected then Ready For Input
  if (connected) { sg->readln(); }
}