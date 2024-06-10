#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""
// Telegram BOT Token (Get from Botfather)
#define BOT_TOKEN ""

const int useramount = 5;
const int adminamount = 2;
int currentIndexuser = 0;
int currentIndexadmin = 1;
String todos[10]={"a","b"};
String admins[adminamount] = {""};
String users[useramount] = {};
int auth; // 0 = unauthorized, 1 = user, 2 = admin

const unsigned long BOT_MTBS = 1000; // mean time between scan messages

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime; // last time messages' scan has been done
bool Start = false;

void checkUser(String userid, String username) {
  auth = 0;
  for (int i = 0; i < sizeof(admins) / sizeof(admins[0]); i++) {
    if (userid == admins[i]) {
      auth = 2;
      break;
    }
  }
  if (!auth) {
    for (int i = 0; i < sizeof(users) / sizeof(users[0]); i++) {
      if (userid == users[i]) {
        auth = 1;
        break;
      }
    }
  }
  if (!auth) {
    bot.sendMessage(userid, "You are not verified yet, but don't worry, admins will have a look at your request");
    bot.sendMessage(String(admins[0]), String(username) + " requested verification. To accept, type /useraccept " + String(userid));
  }
}

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;
    Serial.println(text);
    checkUser(chat_id, from_name);
    if (auth) {
      if (from_name == "") from_name = "Guest";
      Serial.println(from_name);

      if (text == "/send_test_action") {
        bot.sendChatAction(chat_id, "typing");
        delay(4000);
        bot.sendMessage(chat_id, "Did you see the action message?");
      }

      if (text == "/start") {
        String welcome = "Welcome to Universal Arduino Telegram Bot library, " + from_name + ".\n";
        welcome += "This is Chat Action Bot example.\n\n";
        welcome += "/send_test_action : to send test chat action message\n";
        bot.sendMessage(chat_id, welcome);
      }

      if (text.indexOf("/adduser") >= 0) {
        if (auth == 2) {
        text.remove(0, 9);
        if (currentIndexuser < useramount) {
          users[currentIndexuser] = text;
          currentIndexuser++;
          Serial.println("added " + String(text));
          bot.sendMessage(chat_id, "user was added");
          bot.sendMessage(text, "Welcome! An admin just accepted you for using this bot.");
        }
       }
      }
      if(text.indexOf("/addadmin") >= 0){
        text.remove(0, 9);
        if (auth == 2){
        if (currentIndexadmin < adminamount) {
          admins[currentIndexadmin] = text;
          currentIndexadmin++;
          Serial.println("added admin " + String(text));
          bot.sendMessage(chat_id, "admin was added");
        } else {
          bot.sendMessage(chat_id, "Admin array is full");
        }
       }
      }

      if (text == "/shutdown") {
        if (auth==2){
          Serial.println("hello");
          String savedusers = "users: ";
          String savedadmins = "admins: ";
          String savedtodos = "todos: ";
          String message = "";
           for (int i = 0; i < currentIndexuser; i++) {
            savedusers += users[i];
           if (i != sizeof(users) / sizeof(users[0]) - 1) {
            savedusers += ", ";
        }
    }
          for (int i = 0; i < currentIndexadmin; i++) {
        savedadmins += admins[i];
        if (i != sizeof(admins) / sizeof(admins[0]) - 1) {
            savedadmins += ", ";
        }
    }
           for (int i = 0; i < sizeof(todos) / sizeof(todos[0]); i++) {
        savedtodos += todos[i];
        if (i != sizeof(todos) / sizeof(todos[0]) - 1) {
            savedtodos += ", ";
        }
    }
          message = savedusers + savedadmins + savedtodos;
          bot.sendMessage(chat_id, message);
        }
        else{
          bot.sendMessage(chat_id, "You are not authorized to do that");
        }
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  // attempt to connect to Wifi network:
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (millis() - bot_lasttime > BOT_MTBS) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      Serial.println("new message");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }
}
