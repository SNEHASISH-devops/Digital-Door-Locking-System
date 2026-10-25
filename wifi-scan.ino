
#include <Wire.h>
#include <Keypad.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ========== OLED DISPLAY ==========
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(
  SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1
);

// ========== KEYPAD CONFIGURATION ==========
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// Keypad pins connected to ESP32
byte rowPins[ROWS] = {13, 14, 16, 17};
byte colPins[COLS] = {18, 19, 23, 25};

Keypad keypad = Keypad(
  makeKeymap(keys),
  rowPins,
  colPins,
  ROWS,
  COLS
);

// ========== OUTPUT PINS ==========
const int RED_LED = 27;
const int GREEN_LED = 33;
const int BUZZER = 26;

// ========== PASSWORD SETTINGS ==========
const String correctPassword = "1234";
String enteredPassword = "";

bool doorOpen = false;
bool alarmActive = false;

// ========== OLED MESSAGE FUNCTION ==========
void showMessage(String line1,
                 String line2,
                 String line3 = "") {

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 5);
  display.println(line1);

  display.setCursor(0, 25);
  display.println(line2);

  display.setCursor(0, 45);
  display.println(line3);

  display.display();
}

// ========== PASSWORD ENTRY SCREEN ==========
void showPasswordScreen() {

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 3);

  if (alarmActive) {
    display.println("DOOR IS NOT OPEN");
  } else {
    display.println("CYBERSHIELD SYSTEM");
  }

  display.setCursor(0, 20);
  display.println("ENTER 4-DIGIT PIN");

  display.setCursor(0, 37);
  display.print("PIN: ");

  for (int i = 0; i < enteredPassword.length(); i++) {
    display.print("*");
  }

  display.setCursor(0, 53);

  if (alarmActive) {
    display.println("ALARM ACTIVE");
  } else {
    display.println("WAITING FOR INPUT");
  }

  display.display();
}

// ========== CORRECT PASSWORD ==========
void correctAccess() {

  doorOpen = true;
  alarmActive = false;

  // Green LED ON
  digitalWrite(GREEN_LED, HIGH);

  // Red LED OFF
  digitalWrite(RED_LED, LOW);

  // Stop buzzer
  noTone(BUZZER);
  digitalWrite(BUZZER, LOW);

  // Show result on OLED
  showMessage(
    "ACCESS GRANTED",
    "PASSWORD CORRECT",
    "DOOR OPEN"
  );

  Serial.println("ACCESS GRANTED");
  Serial.println("DOOR OPEN");
}

// ========== INCORRECT PASSWORD ==========
void wrongAccess() {

  alarmActive = true;

  // Red LED ON
  digitalWrite(RED_LED, HIGH);

  // Green LED OFF
  digitalWrite(GREEN_LED, LOW);

  // Start continuous buzzer
  tone(BUZZER, 2000);

  // Display error
  showMessage(
    "ACCESS DENIED",
    "WRONG PASSWORD",
    "DOOR IS NOT OPEN"
  );

  Serial.println("ACCESS DENIED");
  Serial.println("DOOR IS NOT OPEN");

  // Clear input for another attempt
  enteredPassword = "";

  delay(1200);

  // Keep alarm active while allowing another attempt
  showPasswordScreen();
}

// ========== PASSWORD CHECK ==========
void checkPassword() {

  if (enteredPassword == correctPassword) {

    correctAccess();

  } else {

    wrongAccess();

  }

  // Clear entered password after checking
  enteredPassword = "";
}

// ========== SETUP ==========
void setup() {

  Serial.begin(115200);

  // Configure LEDs and buzzer
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(BUZZER, LOW);

  // Initialize OLED
  if (!display.begin(
        SSD1306_SWITCHCAPVCC, 0x3C)) {

    Serial.println("OLED initialization failed!");

    while (true) {
      delay(100);
    }
  }

  // Startup message
  showMessage(
    "CYBERSHIELD",
    "DOOR SECURITY",
    "SYSTEM STARTING..."
  );

  delay(2000);

  showPasswordScreen();

  Serial.println("System Ready");
  Serial.println("Enter a 4-digit password");
}

// ========== MAIN LOOP ==========
void loop() {

  // Stop accepting input after successful access
  if (doorOpen) {
    return;
  }

  // Read keypad
  char key = keypad.getKey();

  if (key == NO_KEY) {
    return;
  }

  // Accept digits from 0 to 9
  if (key >= '0' && key <= '9') {

    // Maximum password length = 4 digits
    if (enteredPassword.length() < 4) {

      enteredPassword += key;

      // Update OLED immediately
      showPasswordScreen();

      // Automatically check after 4 digits
      if (enteredPassword.length() == 4) {

        checkPassword();

      }
    }
  }

  // Clear current entry using *
  else if (key == '*') {

    enteredPassword = "";

    showPasswordScreen();
  }
}
