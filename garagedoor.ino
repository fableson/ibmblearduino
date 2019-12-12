/*
 *  GarageDoor 
 *  BLE Peripheral to interact with Garage Door remote control
 *  Access via Bluetooth Low Energy
 *  For IBM Developerworks
 *  Frank Ableson
 *  fableson@navitend.com
 *  December 2019
 *  
 *  
 *  Sonme links of interest
 *  https://www.arduino.cc/en/Reference/ArduinoBLE
 *  https://www.bluetooth.com/specifications/gatt/services/
 *  https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.s132.api.v2.0.0%2Fgroup___b_l_e___a_p_p_e_a_r_a_n_c_e_s.html
 *  https://www.mouser.com/datasheet/2/408/TLP222A_datasheet_en_20190617-1134823.pdf
 *  
 *  
 */
#include <ArduinoBLE.h>

// create a service to expose our service to BLE Central Devices
BLEService garageDoorService("FA01");

BLEUnsignedCharCharacteristic buttonCharacteristic("2102", BLERead | BLEWrite | BLENotify);

BLEUnsignedCharCharacteristic batteryLevelChar("2101", BLERead | BLENotify);

BLEUnsignedCharCharacteristic scaledValueChar("2103", BLERead | BLENotify);
BLEDescriptor mydescriptor1("2103","ABC1");
BLEDescriptor mydescriptor2("2103","ABC2");
BLEDescriptor mydescriptor3("2103","ABC3");


byte buttonValue = 0x00;
int commandIterations = 0;

#define  BUTTONSWITCHPIN 10

void setup() {
  Serial.begin(9600);
  while (!Serial);

  pinMode(BUTTONSWITCHPIN,OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  if (!BLE.begin()) 
  {
    Serial.println("starting BLE failed!");
    while (1);
  }

  String address = BLE.address();
  Serial.println("Our address is [" + address + "]");

  BLE.setDeviceName("IBM Garage Opener");      // this sets Characteristic 0x2a00 of Service 0x1800
                                               // Service 0x1800 is the Generic Access Profile
                                               // Characteristic 0x2a00 is the Device Name
                                               // Characteristic 0x2a01 is the "Appearance"
  BLE.setAppearance(384);                      // BLE_APPEARANCE_GENERIC_REMOTE_CONTROL
                                               
  BLE.setLocalName("BLE Garage Opener");       // this sets the local name for the advertising data
  
  // tell the world about us
  BLE.setAdvertisedService(garageDoorService);
  garageDoorService.addCharacteristic(batteryLevelChar);
  garageDoorService.addCharacteristic(buttonCharacteristic);
  scaledValueChar.addDescriptor(mydescriptor1);
  scaledValueChar.addDescriptor(mydescriptor2);
  scaledValueChar.addDescriptor(mydescriptor3);
  garageDoorService.addCharacteristic(scaledValueChar);
  BLE.addService(garageDoorService);
  
  buttonCharacteristic.writeValue(buttonValue);      // start with a zero

  // advertise to the world so we can be found
  BLE.advertise();

  Serial.println("Bluetooth device active, waiting for connections...");

  // register new connection handler
  BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  
  // registeer disconnect handler
  BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);
  
  // handle Characteristic Written Handler
  buttonCharacteristic.setEventHandler(BLEWritten,switchCharacteristicWritten);
}

void switchCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic) {
  // central wrote new value to characteristic, see if care, etc.
  Serial.print("Characteristic event, written: ");
  Serial.println(characteristic.uuid());
  // see if this is the Button Characteristic
  if (characteristic.uuid() =="2102")
  {
    characteristic.readValue(buttonValue);
    Serial.print("new value ");
    Serial.println(buttonValue);
    if (buttonValue == 0x55)
    {
      digitalWrite(BUTTONSWITCHPIN,HIGH);
      delay(500);
      commandIterations++;
    }
    digitalWrite(BUTTONSWITCHPIN,LOW);
  }
}

void blePeripheralConnectHandler(BLEDevice central) {
  // central connected event handler
  Serial.print("Connected event, central: ");
  Serial.println(central.address());
  digitalWrite(LED_BUILTIN, HIGH);    // indicate that we have a connection
  digitalWrite(BUTTONSWITCHPIN,LOW);          //  make sure our button is NOT pressed
}

void blePeripheralDisconnectHandler(BLEDevice central) {
  // central disconnected event handler
  Serial.print("Disconnected event, central: ");
  Serial.println(central.address());
  digitalWrite(LED_BUILTIN, LOW);     // indicate that we no longer have a connection
  digitalWrite(BUTTONSWITCHPIN,LOW);          //  make sure our button is NOT pressed
}

void loop()
{
  BLEDevice central = BLE.central();
  
  if (central) 
  {
    while (central.connected()) {
    
          int battery = analogRead(A0);
          int batteryLevel = map(battery, 0, 1023, 0, 100);
          //Serial.print("Battery Level % is now: ");
          //Serial.println(batteryLevel);
          //batteryLevelChar.writeValue(batteryLevel);
          //scaledValueChar.writeValue(something * 5);
          delay(200);
    }
  }
}
