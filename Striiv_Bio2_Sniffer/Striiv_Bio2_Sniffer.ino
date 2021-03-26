/**
 * Project: BLE Sniffer
 * Description:  Discover Services and characteristics and read values from them.
 * Author: Luciano Aguerre
 * Contact: luchoaguerre@gmail.com
 * Plattform: ESP32-WROOM-32 DEV Board
 */


#include "BLEDevice.h"
//#include "BLEScan.h"

// *
// * Services
// * ---------------------------------------------------------------------
static BLEUUID GenAccservice("00001800-0000-1000-8000-00805f9b34fb");
static BLEUUID GenAttservice("00001801-0000-1000-8000-00805f9b34fb");

static BLEUUID MULTIPLE_CHARACTERISTIC("0000fff0-0000-1000-8000-00805f9b34fb");
static BLEUUID SINGLE_CHARACTERISTIC("278B67FE-266B-406C-BD40-25379402B58D");    // Advertized service
static BLEUUID CLIENT_CHARACTERISTIC_CONFIG("00002902-0000-1000-8000-00805f9b34fb");

// *
// * Caracteristicas
// * ---------------------------------------------------------------------

static BLEUUID TX0("0000fff1-0000-1000-8000-00805f9b34fb");
static BLEUUID TX1("0000fff2-0000-1000-8000-00805f9b34fb");
static BLEUUID TX2("0000fff3-0000-1000-8000-00805f9b34fb");
static BLEUUID TX3("0000fff4-0000-1000-8000-00805f9b34fb");
static BLEUUID RX0("0000fff5-0000-1000-8000-00805f9b34fb");
static BLEUUID RX1("0000fff6-0000-1000-8000-00805f9b34fb");
static BLEUUID RX2("0000fff7-0000-1000-8000-00805f9b34fb");
static BLEUUID RX3("0000fff8-0000-1000-8000-00805f9b34fb");
static BLEUUID DB0("0000fff9-0000-1000-8000-00805f9b34fb");
static BLEUUID RXS("30E29A0A-2C25-4331-89C1-C17E26B1D22B");        // Write and write without response
static BLEUUID TXS("E57BFE23-2E2C-4332-B1A1-846E15E9FB75");        // Notify
static BLEUUID DBS("F37DDCAE-B6E7-4CFB-B9C4-D083D8118000");        
static BLEUUID DEVICE_ID("12B54A02-52F5-4A61-8DA5-7E722D428359");  // Read 


static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;


static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLERemoteService* pRemoteService;
static BLEAdvertisedDevice* myDevice;
static BLEClient*  pClient;

String trama [20];
String tramaAnt[20];

static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) 
{
    std::string value = (char*)pData;
    if (length > 0) 
    {
      for (int i = 0; i < length; i++) 
      {
        Serial.print(String(value[i],HEX));
        trama[i] = value[i];
        Serial.print(" ");              
      }
    }
    if (trama != tramaAnt)
    {
      
    }
    Serial.println();
    
}

class MyClientCallback : public BLEClientCallbacks 
{
    void onConnect(BLEClient* pclient) 
    {
    }
    void onDisconnect(BLEClient* pclient) 
    {
        connected = false;
        Serial.println("Connection lost");
    }
};

//------------------------------------------------------------------------------------------
bool connectToServer() 
{
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());

       
    pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");
    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");

    // Obtain a reference to the service we are after in the remote BLE server.
    readGeneralInfo();

    readServiceUUID(SINGLE_CHARACTERISTIC);  
    delay(100);
    readCharacteristicUUID(TXS);
    pRemoteCharacteristic->registerForNotify(notifyCallback);
    delay(100);
    const uint8_t notificationOn[] = {0x1, 0x0};
    pRemoteCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
            
    connected = true;
}
/**
 * Esta es la callback que scanea los dispositivos
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks 
{
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) 
  {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(SINGLE_CHARACTERISTIC)) {

      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    } // Found our server
  } // onResult
}; // MyAdvertisedDeviceCallbacks



//--------------------------------------------------------------
void readGeneralInfo (void)
{
    static BLEUUID GENDATA00("00002a00-0000-1000-8000-00805f9b34fb"); 
    static BLEUUID GENDATA01("00002a01-0000-1000-8000-00805f9b34fb"); 
    static BLEUUID GENDATA04("00002a04-0000-1000-8000-00805f9b34fb"); 
    static BLEUUID GENDATA05("00002a05-0000-1000-8000-00805f9b34fb"); 
    
    readServiceUUID(GenAccservice);  
    Serial.print("Device name: "); 
    readCharacteristicUUID(GENDATA00);
    delay(100);
    Serial.print("Appearance: "); 
    readCharacteristicUUID(GENDATA01);
    delay(100);
    Serial.print("Perpheral preferred connection: "); 
    readCharacteristicUUID(GENDATA04);
    delay(100);
    Serial.println();
    
    readServiceUUID(SINGLE_CHARACTERISTIC);  
    Serial.print("Device ID: "); 
    delay(100);
    readCharacteristicUUID(DEVICE_ID);
  
}



//--------------------------------------------------------------
void readCharacteristicValue (void)
{
    // Read the value of the characteristic.
    if(pRemoteCharacteristic->canRead()) 
    {
      std::string value = pRemoteCharacteristic->readValue();
      int len = value.length();
      Serial.print("Lenght: ");
      Serial.print(len);
      Serial.print("   ");
      Serial.print("Value: ");
      if (value.length() > 0) 
      {
          for (int i = 0; i < value.length(); i++) 
          {
              Serial.print(String(value[i]));
          }
      }
      Serial.print("  HEX: ");
      if (value.length() > 0) 
      {
          for (int i = 0; i < value.length(); i++) 
          {
              Serial.print(String(value[i],HEX));
          }
      }
      Serial.println();
    }
    else
    {
        Serial.println("No se puede leer la caracteritica");  
    }
}
//----------------------------------------------------------
void readCharacteristicUUID (BLEUUID readChar)
{
    
    pRemoteCharacteristic = pRemoteService->getCharacteristic(readChar);
    if (pRemoteCharacteristic == nullptr) 
    {
        Serial.println("Failed: ");
    }
    else
    {
        readCharacteristicValue();
    }
}
//----------------------------------------------------------
void readServiceUUID (BLEUUID readService)
{
    pRemoteService = pClient->getService(readService);
    
    if (pRemoteService == nullptr) 
    {
        Serial.print("Failed to find our service UUID: ");
    }
}
//--------------------------------------------------------------------------------------------------
void setup() 
{
    Serial.begin(115200);
    Serial.println("Starting Arduino BLE Client application...");
    BLEDevice::init("");

    // Retrieve a Scanner and set the callback we want to use to be informed when we
    // have detected a new device.  Specify that we want active scanning and start the
    // scan to run for 5 seconds.
    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);
    pBLEScan->start(5, false);
}  // End of setup.


//-------------------------------------------------------------------------------------------------
void loop() {
    int contador = 0;
    
    // If the flag "doConnect" is true then we have scanned for and found the desired
    // BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
    // connected we set the connected flag to be true.
  
    if (doConnect == true) 
    {
        if (connectToServer()) 
        {
            Serial.println("We are now connected to the BLE Server.");
        } 
        else 
        {
            Serial.println("We have failed to connect to the server; there is nothin more we will do.");
        }
        doConnect = false;
    }
    delay(1000*5); // Delay a second between loops.
    if (connected)
    {
        pRemoteService = pClient->getService(SINGLE_CHARACTERISTIC);
        pRemoteCharacteristic = pRemoteService->getCharacteristic(RXS);
        if (pRemoteCharacteristic->canWrite())
        {
            pRemoteCharacteristic->writeValue(0x04,true);
            contador++;
        }
     }
} // End of loop



/*** end of file ***/
