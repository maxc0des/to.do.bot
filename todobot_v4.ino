#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#define buzzer 21
#define WIFI_SSID ""
#define WIFI_PASSWORD ""
// Telegram BOT Token (Get from Botfather)
#define BOT_TOKEN ""

const int maxuseramount = 5;
const int maxadminamount = 2;
const int maxtodoamount = 20;
int currentIndexuser = 0;
int currentIndexadmin = 1;
int currentIndextodo = 0;
String owner = "";//your telegram id
String todos[maxtodoamount]={};
String admins[maxadminamount] = {};
String users[maxuseramount] = {};
int auth; // 0 = unauthorized, 1 = user, 2 = admin

const unsigned long BOT_MTBS = 1000;

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime;
bool Start = false;

void removeString(String arr[], int& size, String strToRemove) {
  for (int i = 0; i < size; i++) {
    if (arr[i] == strToRemove) {
      for (int j = i; j < size - 1; j++) {
        arr[j] = arr[j + 1];
      }
      arr[size - 1] = ""; // Letztes Element leeren
      size--; // Größe des Arrays anpassen
      break; // Wenn du nur ein Vorkommen entfernen willst
    }
  }
}

void checkUser(String userid, String username) {
  auth = 0;
  if (userid == owner) {
    auth = 2;
  }
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

      if (text == "/start") {
        String message = "Welcome to the to do bot, " + from_name + ".\n";
        message += "This bot will help you not to forget your to do's.\nCheck /commands for a list of commands\n";
        message += "You can find more information and the full source code to this project on https://github.com/maxc0des/to.do.bot\n";
        bot.sendMessage(chat_id, message);
      }

      if (text == "/commands") {
        String message = "/todo.add add a to do to your list\n/todo.add show all todos on your list\n/todo.done mark a to do as done and remove it from your list\n";
        if (auth == 2) {
          message += "/adduser to authorize a user\n/addadmin to add an admin\n/shutdown to receive a shutdown code to restore the data after a restart (feture not available yet)\n/restore comming soon..";
        }
        bot.sendMessage(chat_id, message);
      }

      if (text.indexOf("/adduser") >= 0) {
        if (auth == 2) {
        text.remove(0, 9);
        if (currentIndexuser < maxuseramount) {
          users[currentIndexuser] = text;
          currentIndexuser++;
          Serial.println("added " + String(text));
          bot.sendMessage(chat_id, "user was added");
          bot.sendMessage(text, "Welcome! An admin just accepted you for using this bot.");
        }
       }
       else{
          bot.sendMessage(chat_id, "You are not authorized to do that");
        }
      }
      if(text.indexOf("/addadmin") >= 0){
        text.remove(0, 9);
        if (auth == 2){
        if (currentIndexadmin < maxadminamount) {
          admins[currentIndexadmin] = text;
          currentIndexadmin++;
          Serial.println("added admin " + String(text));
          bot.sendMessage(chat_id, "admin was added");
          removeString(users, currentIndexuser, text);
        } else {
          bot.sendMessage(chat_id, "Admin array is full");
        }
       }
       else{
          bot.sendMessage(chat_id, "You are not authorized to do that");
        }
      }
      if(text.indexOf("/todo.add") >= 0){
        text.remove(0, 9);
        todos[currentIndextodo] = text;
        currentIndextodo++;
        bot.sendMessage(chat_id, "to do was added");
        tone(buzzer, 404);
        delay(200);
        noTone(buzzer);
        delay(200);
        tone(buzzer, 404);
        delay(200);
        noTone(buzzer);
            }
      if(text.indexOf("/todo.show") >= 0){
        text.remove(0, 10);
        String message = "You need to.. ";
        for (int i = 0; i < currentIndextodo; i++) {
            message += todos[i];
            message += " ";
            }
            bot.sendMessage(chat_id, message);
          }
      if(text.indexOf("/todo.done") >= 0){
        text.remove(0, 10);
        removeString(todos, currentIndextodo, text);
        bot.sendMessage(chat_id, "well done! to do was removed from the list");
        tone(buzzer, 404);
        delay(200);
        noTone(buzzer);
        delay(200);
        tone(buzzer, 404);
        delay(200);
        noTone(buzzer);
            }

      if (text == "/shutdown") {
        if (auth==2){
          Serial.println("hello");
          String savedusers = "";
          String savedadmins = "";
          String savedtodos = "";
          String message = "";
           for (int i = 0; i < currentIndexuser; i++) {
            savedusers += users[i];
    }
          for (int i = 0; i < currentIndexadmin; i++) {
        savedadmins += admins[i];
    }
           for (int i = 0; i < currentIndextodo; i++) {
        savedtodos += todos[i];
        savedtodos += ",";
    }
          message ="Shut down Code: " + savedusers +"/"+ savedadmins+"/"+ savedtodos;
          bot.sendMessage(chat_id, message);
        }
        else{
          bot.sendMessage(chat_id, "You are not authorized to do that");
        }
      }
      if (text.indexOf("/restore")>=0) {
        if(auth == 2){
        String data = text.substring(8);
        int firstSlashIndex = data.indexOf('/');
        int secondSlashIndex = data.indexOf('/', firstSlashIndex + 1);
        String usersData = data.substring(0, firstSlashIndex);
        String adminsData = data.substring(firstSlashIndex + 1, secondSlashIndex);
        String todosData = data.substring(secondSlashIndex + 1);

        int userCount = usersData.length() / 10;
        for (int i = 0; i < userCount; i++) { 
          users[currentIndexuser++] = usersData.substring(i * 5, (i + 1) * 5);
     }
        int adminCount = adminsData.length() / 10;
        for (int i = 0; i < adminCount; i++) {
          admins[currentIndexadmin++] = adminsData.substring(i * 5, (i + 1) * 5);
        }
          int startIndex = 0;
          int commaIndex = todosData.indexOf(',');
          while (commaIndex != -1) {
            todos[currentIndextodo++] = todosData.substring(startIndex, commaIndex);
            startIndex = commaIndex + 1;
            commaIndex = todosData.indexOf(',', startIndex);
         }
        todos[currentIndextodo++] = todosData.substring(startIndex);
        bot.sendMessage(chat_id, "Data was succesfully restored");
      }
      else {
      bot.sendMessage(chat_id, "You are not authorized to do that");
      }
    }
    }
    }
    }

void setup() {
  Serial.begin(115200);
  Serial.println();
  pinMode(buzzer, OUTPUT);
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
  bot.sendMessage(owner, "Welcome! The bot is online now");
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
