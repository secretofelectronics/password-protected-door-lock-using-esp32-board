#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>
#include <Preferences.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Preferences preferences;

const int relayPin = 23;

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {18, 5, 17, 16};
byte colPins[COLS] = {26, 25, 33, 32};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

String password = "";
String enteredPassword = "";
bool isPasswordSet = false;

void setup() {
  Serial.begin(115200);
  preferences.begin("doorlock", false);

  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (1);
  }
  display.clearDisplay();

  if (preferences.isKey("password")) {
    password = preferences.getString("password");
    isPasswordSet = true;
    displayMessage("Enter Password", "");
  } else {
    displayMessage("Set New Password", "Enter on keypad");
  }
}

void loop() {
  char key = keypad.getKey();
  if (key) {
    if (!isPasswordSet) {
      setInitialPassword(key);
    } else if (key == '#') {
      changePasswordMode();
    } else {
      verifyPassword(key);
    }
  }
}

void displayMessage(String title, String message) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor((SCREEN_WIDTH - title.length() * 6) / 2, 10);
  display.println(title);
  display.setCursor((SCREEN_WIDTH - message.length() * 6) / 2, 30);
  display.println(message);
  display.display();
}

void displayPasswordProgress(String message) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor((SCREEN_WIDTH - message.length() * 12) / 2, 20);
  display.println(message);
  display.display();
}

void setInitialPassword(char key) {
  if (key == 'A') {
    password = enteredPassword;
    preferences.putString("password", password);
    isPasswordSet = true;
    enteredPassword = "";
    displayMessage("Password Saved", "Successfully");
    delay(1000);
    displayMessage("Enter Password", "");
  } else if (key == 'C') {
    if (!enteredPassword.isEmpty()) {
      enteredPassword.remove(enteredPassword.length() - 1);
      displayPasswordProgress(enteredPassword);
    }
  } else {
    enteredPassword += key;
    displayPasswordProgress(enteredPassword);
  }
}

void verifyPassword(char key) {
  if (key == 'A') {
    if (enteredPassword == password) {
      displayMessage("Access Granted", "Door Unlocked");
      unlockDoor();
    } else {
      displayMessage("Access Denied", "Wrong Password");
      delay(1000); // Show "Access Denied" for 1 second
      displayMessage("Enter Password", ""); // Prompt to re-enter password
    }
    enteredPassword = "";
  } else if (key == 'C') {
    if (!enteredPassword.isEmpty()) {
      enteredPassword.remove(enteredPassword.length() - 1);
      displayPasswordProgress(enteredPassword);
    }
  } else {
    enteredPassword += key;
    displayPasswordProgress(enteredPassword);
  }
}

void changePasswordMode() {
  displayMessage("Change Password", "Enter current pass:");
  while (true) {
    char key = keypad.getKey();
    if (key == 'A') {
      if (enteredPassword == password) {
        displayMessage("Current pass OK", "Enter new pass:");
        enteredPassword = "";
        while (true) {
          key = keypad.getKey();
          if (key == 'A') {
            password = enteredPassword;
            preferences.putString("password", password);
            displayMessage("Password Updated", "");
            delay(1000);
            displayMessage("Enter Password", "");
            enteredPassword = "";
            break;
          } else if (key == 'C') {
            if (!enteredPassword.isEmpty()) {
              enteredPassword.remove(enteredPassword.length() - 1);
              displayPasswordProgress(enteredPassword);
            }
          } else if (key) {
            enteredPassword += key;
            displayPasswordProgress(enteredPassword);
          }
        }
        break;
      } else {
        displayMessage("Wrong Password", "Try Again");
        delay(1000); // Show "Wrong Password" message for 1 second
        displayMessage("Enter Password", "");
        enteredPassword = "";
        break;
      }
    } else if (key == 'C') {
      if (!enteredPassword.isEmpty()) {
        enteredPassword.remove(enteredPassword.length() - 1);
        displayPasswordProgress(enteredPassword);
      }
    } else if (key) {
      enteredPassword += key;
      displayPasswordProgress(enteredPassword);
    }
  }
}

void unlockDoor() {
  digitalWrite(relayPin, HIGH);
  delay(5000);
  digitalWrite(relayPin, LOW);
  displayMessage("Enter Password", "");
}