#include <ESP8266WiFi.h>
#include <Servo.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// Initialize Wifi connection to the router
char ssid[] = "Mousetrap_Wifi";     // your network SSID (name)
char password[] = "88888888"; // your network key

const int SENSOR_PIN = D5; 
const int SERVO_PIN = D7;
Servo servo;
volatile bool sensorButtonPressedFlag = false;
volatile bool mouseCaughtFlag = false;
String chat_id  = "649330718";
String location = "Kitchen";
enum TrapModes {Caught, Empty};
enum TrapModes trapdoor = Caught;


// Initialize Telegram BOT
#define BOTtoken "674456410:AAFTXYH_seycpwGS-XQ6FACq84EP_Oq9fSA"  // your Bot Token (Get from Botfather)

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int Bot_mtbs = 1000; //mean time between scan messages
long Bot_lasttime;   //last time messages' scan has been done
bool Start = false;

String handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    chat_id = String(bot.messages[i].chat_id);
    Serial.println(chat_id);
    String text = bot.messages[i].text;

    String from_name = bot.messages[i].from_name;
    if (from_name == "") from_name = "Guest";

    if (text == "/kitchen") {
        bot.sendChatAction(chat_id, "typing");
        location = "Kitchen";
        delay(4000);
        bot.sendMessage(chat_id, "Your location identifier is set to kitchen.");
    }

    if (text == "/bedroom") {
        bot.sendChatAction(chat_id, "typing");
        location = "Bedroom";
        delay(4000);
        bot.sendMessage(chat_id, "Your location identifier is set to bedroom.");
    }

    if (text == "/garage") {
        bot.sendChatAction(chat_id, "typing");
        location = "Garage";
        delay(4000);
        bot.sendMessage(chat_id, "Your location identifier is set to garage.");
    }

    if (text == "/force") {
        bot.sendChatAction("649330718", "typing");
        delay(4000);
        bot.sendMessage("649330718", "Kitchen mousetrap has caught a mouse.");
    }
    
    if (text == "/open") {
        bot.sendChatAction(chat_id, "typing");
        delay(1000);
        servo.write(250);
        mouseCaughtFlag = false;
        String opengate = location + " mousetrap has opened the door\n";
        bot.sendMessage(chat_id, opengate, "");
        trapdoor = Empty;
    }

    if(text == "/status") {
      String response = "The mouestrap ";
      switch(trapdoor){
        case Caught:
          response += "has caught a mouse";
          break;
        case Empty:
          response += "is empty";
          break;
      }
      bot.sendMessage(chat_id, response, "");
    }

    /*if (text == "/status") {
        bot.sendChatAction(chat_id, "typing");
        delay(1000);
        if (mouseCaughtFlag == true){
        String trapstatus = location + " mousetrap has caught a mouse\n";
        bot.sendMessage(chat_id, trapstatus);
        }
        else if (mouseCaughtFlag == false){
        String trapstatus = location + " mousetrap is empty\n";
        bot.sendMessage(chat_id, trapstatus);
        }
    }*/

    if (text == "/start") {
      String welcome = "Welcome to Smart Mousetrap setup guide, " + from_name + ".\n";
      welcome += "This is a product of EK210 group project.\n\n";
      welcome += "Please turn on notification for Telegram app\n";
      welcome += "You will recieve a message from this bot when a mouse is caught.\n\n";
      welcome += "Use command below to setup your mousetrap:\n";
      welcome += "/kitchen : to set the mousetrap location identifier to kitchen\n";
      welcome += "/bedroom : to set the mousetrap location identifier to bedroom\n";
      welcome += "/garage : to set the mousetrap location identifier to garage\n";
      welcome += "/open : to open the mousetrap gate release mouse\n";
      welcome += "/status :  to check the capture state of the mousetrap\n";
      bot.sendMessage(chat_id, welcome);
    }
  }
  return location;
}


void setup() {
  Serial.begin(921600);
  //set up the sesnor input
  pinMode(SENSOR_PIN, INPUT);
  servo.attach (SERVO_PIN);
  attachInterrupt(SENSOR_PIN, sensorButtonPressed, RISING);

  // Set WiFi to station mode and disconnect from an AP if it was Previously
  // connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

//detect sensor pressed
void sensorButtonPressed() {
  Serial.println("sensorButtonPressed");
  int button = digitalRead(SENSOR_PIN);
  if((button == HIGH) && (mouseCaughtFlag == false))
  {
    Serial.println("sensorButtonHigh");
    mouseCaughtFlag == true;
    sensorButtonPressedFlag = true;
    trapdoor = Caught;
    Serial.print("Caught");
  }
  return;
}
void sendTelegramMessage() {
  Serial.println("Mouse caught");
  String caught = location + " mousetrap has caught a mouse\n";
  Serial.println(chat_id);
  bot.sendMessage(chat_id, caught);
  sensorButtonPressedFlag = false;
}

String check_chat_id(int numNewMessages) {
  for (int i=0; i<numNewMessages; i++) {
    chat_id = String(bot.messages[i].chat_id);
  }
  return chat_id;
}

void loop() {
  if (millis() > Bot_lasttime + Bot_mtbs)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      chat_id = check_chat_id(numNewMessages);
      Serial.println("ID confirmed");
      location = handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    Bot_lasttime = millis();
  }
  else if ( sensorButtonPressedFlag ) {
    servo.write(90);
    delay(1000);
    String caught = location + " mousetrap has caught a mouse\n";
    bot.sendMessage("649330718", caught, "");
    sensorButtonPressedFlag = false;  
    
    /*Serial.println(sensorButtonPressedFlag);
    Serial.println("Message Sent");
    Serial.println(location);
    sendTelegramMessage();*/
  }
}
