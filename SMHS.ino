 /*
 * Semester Project
 */

#include<ESP8266WiFi.h>
#include<PubSubClient.h>

#include<SPI.h>
#include<Wire.h>
#include<Keypad.h>
#include<Password.h>
#include<Keypad_I2C.h>
#include<Adafruit_ST7735.h>
#include<Adafruit_GFX.h>

//Edit
boolean sendData =false;

//TFT-Display:
#define TFT_CS 15 //D8
#define TFT_DC 2  //A0
#define TFT_SCLK 14
#define TFT_MOSI 13
#define TFT_RST 17//GND

//I2C adress:
#define I2CADDR 0x38

Password password = Password( "12345" );

const byte ROWS = 4;
const byte COLS = 4;

//Keymap:
char hexaKeys[ROWS][COLS] = {
{'1', '2', '3', 'A'},
{'4', '5', '6', 'B'},
{'7', '8', '9', 'C'},
{'*', '0', '#', 'D'} 
};

byte rowPins[ROWS] = {0,1,2,3};
byte colPins[COLS] = {4,5,6,7};

//Led light & Sensor:
int LedPin = 16;     //D0           
int PIRpin = 0;      //D3

//Network details:(Editable per Network request)
const char* ssid = "KLEPTO";
const char* passWord = "255abcd1";
const char* mqtt_server = "www.stellarbd.com";

//Setting the client:
WiFiClient espClient;

char* topic = "SensorStat";
const char* Alert = "System has been compromised!";
void callback(char* topic, byte*payload, unsigned int length);


PubSubClient client(mqtt_server, 1883, callback, espClient);
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
Keypad_I2C customKeypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS, I2CADDR);

void setup() {
  //Led light & Sensor:
  Serial.begin(9600);
  delay(100);
  Serial.println("System Initialized");
  Serial.println("Waiting for the system to Connect to WiFi.");
  delay(100);
  setup_wifi();
  delay(100);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  delay(100);
  pinMode(LedPin,   OUTPUT);
  pinMode(PIRpin, INPUT);

  delay(50);
  TFT_screenSetup();
  tft.setCursor(7, 30);                   //(x,y)
  tft.println("Smart Home Security");
  tft.println(' ');
  tft.setCursor(7, 40);
  tft.println("Enter Password:");
  
  tft.println("");
  
  customKeypad.begin( );
  customKeypad.addEventListener(keypadEvent);
  tft.setCursor(7,50);
}

void loop() {
  //WiFi:
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  //Keypad:
  customKeypad.getKey();
  //edit
  //write the code for matching password here
  if(checkPassword()){
    sendData = !sendData;
    }

  if(sendData==true){
   //write the code for collecting and sending data 
    
    }
  
}

//Checks the state of sensor and send log in Server
void Sensor_state()
{
  digitalRead(PIRpin);
  Serial.println(PIRpin);
  if (PIRpin == 0)
  {
    Serial.println("System has been compromised!");
    client.publish("SensorStat", Alert);
    LEDblink();
  } else {
    digitalWrite(LedPin, LOW);
  }
}

void LEDblink()
{
  digitalWrite(LedPin, HIGH);
  delay(400);
  digitalWrite(LedPin, LOW);
  delay(400);
}

void callback(char* topic, byte* payload, unsigned int length)
{
  //convert topic to string to make it easier to work with
  String topicStr = topic;
}

//WiFi setup function:
void setup_wifi()
{
  delay(10);
  //Starting by Connecting to WiFi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, passWord);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("");
}

//Wifi reconnecting function:
void reconnect()
{
  // Loop until system reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect:
    //If Connection is established
    if (client.connect("ESP8266Client"))
    {
      Serial.println("Connection Established!!");
      // Once connected, publish an announcement...
      client.publish("System_Status", "Connection Established!!");
      // ... and resubscribe
      //client.subscribe("LEDstatus");
    }
    //If Fails:
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void TFT_screenSetup()
{
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST7735_BLACK);
  tft.setTextColor(ST7735_RED);
  tft.setTextSize(1);
}



boolean checkPassword(){
  if(password.evaluate()){
    tft.setCursor(7,60);
    tft.print("Success");
    //Security System:
    return true;
    
  }else{
    tft.setCursor(7,70);
    tft.print("Wrong entered!!");
    //add code here to run if password was wrong.
    return false;
  }
}

void keypadEvent(KeypadEvent eKey) {
  
  switch( customKeypad.getState()){
    case PRESSED:
  tft.print(eKey);
    
    switch (eKey) {
      case '*': checkPassword();  break;
      case '#': password.reset(); break;
      default:  password.append(eKey);
    }
  }
}
