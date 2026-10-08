#include <WiFi.h>
#include <Wire.h>
#include <time.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>


// =====================================================
//                 WIFI SETTINGS
// =====================================================

const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

const long GMT_OFFSET_SEC = 6 * 3600;
const int DAYLIGHT_OFFSET_SEC = 0;


// =====================================================
//                    PIN SETTINGS
// =====================================================

#define OLED_SDA    8
#define OLED_SCL    9

#define TOUCH_PIN   10
#define BUZZER_PIN  11


// =====================================================
//                    OLED SETTINGS
// =====================================================

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

Adafruit_SH1106G display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);


// =====================================================
//                    TIMING
// =====================================================

const unsigned long LONG_PRESS = 1200;
const unsigned long DOUBLE_TAP = 450;

// 2 minutes no touch = Sleep
const unsigned long SLEEP_TIMEOUT = 2UL * 60UL * 1000UL;

// Message display time = 4 seconds
const unsigned long MESSAGE_TIME = 4000;


// =====================================================
//                    VARIABLES
// =====================================================

unsigned long lastActivityTime = 0;

unsigned long touchStartTime = 0;
unsigned long lastTapTime = 0;

int tapCount = 0;

bool touchActive = false;
bool isSleeping = false;


// =====================================================
//                    MODES
// =====================================================

enum BotMode {
  EMOTION_MODE,
  WATCH_MODE
};

BotMode currentMode = EMOTION_MODE;


// =====================================================
//                    EMOTIONS
// =====================================================

enum Emotion {

  HAPPY,
  EXCITED,
  LOVE,
  SAD,
  ANGRY,
  SLEEPY,
  SURPRISED,
  CONFUSED,
  COOL,
  SHY,
  PLAYFUL,
  BORED,
  WORRIED,
  PROUD,
  CALM,
  NEUTRAL
};

Emotion currentEmotion = HAPPY;

const int EMOTION_COUNT = 16;


// =====================================================
//                  EMOTION NAMES
// =====================================================

const char* emotionNames[EMOTION_COUNT] = {

  "HAPPY",
  "EXCITED",
  "LOVE",
  "SAD",
  "ANGRY",
  "SLEEPY",
  "SURPRISED",
  "CONFUSED",
  "COOL",
  "SHY",
  "PLAYFUL",
  "BORED",
  "WORRIED",
  "PROUD",
  "CALM",
  "NEUTRAL"
};


// =====================================================
//                  EMOTION MESSAGES
// =====================================================

const char* messages[EMOTION_COUNT][3] = {

  // HAPPY
  {
    "Ami onek khushi!",
    "Tomake dekhe valo lage!",
    "Ajke amar mood great!"
  },

  // EXCITED
  {
    "Wow! Ami excited!",
    "Ki hote jacche?",
    "Ami ready!"
  },

  // LOVE
  {
    "Tomake onek valo lage!",
    "You are special!",
    "Amar heart happy!"
  },

  // SAD
  {
    "Ajke amar mon kharap...",
    "Amake ektu time dao...",
    "I need a little care."
  },

  // ANGRY
  {
    "Ami ektu angry!",
    "Amake disturb koro na!",
    "Grrrr...!"
  },

  // SLEEPY
  {
    "Amar ghum pacche...",
    "Ektu ghumai?",
    "Good night..."
  },

  // SURPRISED
  {
    "Oh! Eta ki holo?",
    "Wow! Seriously?",
    "Ami surprised!"
  },

  // CONFUSED
  {
    "Hmm... bujhte parchi na!",
    "Eta abar ki?",
    "Ami confused!"
  },

  // COOL
  {
    "Take it easy!",
    "Ami ekdom cool!",
    "Cool mood activated!"
  },

  // SHY
  {
    "Ektu lojja lagche...",
    "Amake eto dekho na!",
    "Hehe..."
  },

  // PLAYFUL
  {
    "Cholo game kheli!",
    "Catch me if you can!",
    "Hehehe...!"
  },

  // BORED
  {
    "Ami bored hoye gechi...",
    "Kichu interesting koro!",
    "Abar kichu kori?"
  },

  // WORRIED
  {
    "Ami ektu worried...",
    "Sob thik ache to?",
    "Something feels strange..."
  },

  // PROUD
  {
    "Yes! Ami perechi!",
    "I am proud!",
    "Dekhle? Ami smart!"
  },

  // CALM
  {
    "Everything is okay.",
    "Relax and breathe.",
    "Ami calm achi."
  },

  // NEUTRAL
  {
    "Hello there!",
    "Ami ekhane achi.",
    "Ki korcho?"
  }
};


// =====================================================
//                     SETUP
// =====================================================

void setup() {

  Serial.begin(115200);

  delay(500);

  Serial.println();
  Serial.println("==========================");
  Serial.println("      PICO BOT START");
  Serial.println("==========================");


  // Pins
  pinMode(TOUCH_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);


  // OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  Wire.setClock(100000);

  if (!display.begin(0x3C, true)) {

    Serial.println("SH1106 OLED NOT FOUND!");

    while (true) {
      delay(100);
    }
  }

  Serial.println("SH1106 OLED OK");


  randomSeed(analogRead(0));


  // Boot animation
  bootAnimation();


  // WiFi
  connectWiFi();


  // Starting emotion
  currentEmotion = HAPPY;

  showEmotionFace(currentEmotion);


  // Start inactivity timer
  lastActivityTime = millis();

}


// =====================================================
//                      LOOP
// =====================================================

void loop() {


  // ===================================================
  //              AUTOMATIC SLEEP CHECK
  // ===================================================

  if (!isSleeping &&
      currentMode == EMOTION_MODE &&
      millis() - lastActivityTime >= SLEEP_TIMEOUT) {

    goToSleep();

  }


  // ===================================================
  //                    TOUCH
  // ===================================================

  bool touched = digitalRead(TOUCH_PIN);


  // Touch started
  if (touched && !touchActive) {

    touchActive = true;

    touchStartTime = millis();

    // Any touch counts as activity
    lastActivityTime = millis();

  }


  // Touch released
  if (!touched && touchActive) {

    touchActive = false;

    unsigned long pressDuration =
      millis() - touchStartTime;


    // -----------------------------------------------
    // If bot is sleeping
    // -----------------------------------------------

    if (isSleeping) {

      wakeUp();

      return;
    }


    // -----------------------------------------------
    // Long press
    // -----------------------------------------------

    if (pressDuration >= LONG_PRESS) {

      tapCount = 0;

      changeMode();

      return;
    }


    // -----------------------------------------------
    // Short press
    // -----------------------------------------------

    tapCount++;

    lastTapTime = millis();

  }


  // ===================================================
  //                 DOUBLE / SINGLE TAP
  // ===================================================

  if (tapCount > 0 &&
      millis() - lastTapTime > DOUBLE_TAP) {


    if (tapCount >= 2) {

      // Double touch
      tapCount = 0;

      lastActivityTime = millis();

      randomEmotion();

    }

    else {

      // Single touch
      tapCount = 0;

      lastActivityTime = millis();

      nextEmotion();

    }
  }


  // ===================================================
  //                 EMOTION MODE
  // ===================================================

  if (currentMode == EMOTION_MODE &&
      !isSleeping) {

    autoBlink();

  }


  // ===================================================
  //                   WATCH MODE
  // ===================================================

  if (currentMode == WATCH_MODE &&
      !isSleeping) {

    showWatch();

    delay(200);

  }

}


// =====================================================
//                   BOOT ANIMATION
// =====================================================

void bootAnimation() {

  display.clearDisplay();

  display.setTextColor(SH110X_WHITE);

  display.setTextSize(2);

  display.setCursor(28, 15);

  display.println("PICO");

  display.setCursor(30, 40);

  display.println("BOT");

  display.display();

  tone(BUZZER_PIN, 900, 100);

  delay(500);


  display.clearDisplay();

  display.setTextSize(1);

  display.setCursor(35, 28);

  display.println("Starting...");

  display.display();

  delay(700);

}


// =====================================================
//                    WIFI CONNECT
// =====================================================

void connectWiFi() {

  display.clearDisplay();

  display.setTextSize(1);

  display.setCursor(25, 20);

  display.println("Connecting WiFi");

  display.setCursor(42, 38);

  display.println("Please wait...");

  display.display();


  WiFi.mode(WIFI_STA);

  WiFi.begin(
    WIFI_SSID,
    WIFI_PASSWORD
  );


  Serial.print("Connecting WiFi");


  unsigned long startTime = millis();


  while (WiFi.status() != WL_CONNECTED &&
         millis() - startTime < 15000) {

    delay(500);

    Serial.print(".");

  }


  Serial.println();


  if (WiFi.status() == WL_CONNECTED) {

    Serial.println("WiFi Connected!");

    Serial.print("IP Address: ");

    Serial.println(WiFi.localIP());


    display.clearDisplay();

    display.setTextSize(1);

    display.setCursor(32, 18);

    display.println("WiFi Connected");

    display.setCursor(18, 38);

    display.println(WiFi.localIP());

    display.display();


    tone(BUZZER_PIN, 1200, 100);

    delay(100);


    tone(BUZZER_PIN, 1600, 100);

    delay(800);


    // Bangladesh time
    configTime(
      GMT_OFFSET_SEC,
      DAYLIGHT_OFFSET_SEC,
      "pool.ntp.org",
      "time.nist.gov",
      "time.google.com"
    );


    Serial.println("NTP Time Sync Started");

  }

  else {

    Serial.println("WiFi Connection Failed");


    display.clearDisplay();

    display.setTextSize(1);

    display.setCursor(28, 20);

    display.println("WiFi Failed");

    display.setCursor(18, 38);

    display.println("Emotion Mode OK");

    display.display();

    delay(1000);

  }

}


// =====================================================
//                  CHANGE MODE
// =====================================================

void changeMode() {

  lastActivityTime = millis();

  tapCount = 0;


  if (currentMode == EMOTION_MODE) {

    currentMode = WATCH_MODE;


    tone(BUZZER_PIN, 1200, 100);

    delay(120);

    tone(BUZZER_PIN, 1600, 120);


    showWatchIntro();

  }

  else {

    currentMode = EMOTION_MODE;


    tone(BUZZER_PIN, 1600, 100);

    delay(120);

    tone(BUZZER_PIN, 1200, 120);


    showEmotionIntro();

    showEmotionFace(currentEmotion);

  }

}


// =====================================================
//                  NEXT EMOTION
// =====================================================

void nextEmotion() {

  currentEmotion =
    (Emotion)((int)currentEmotion + 1);


  if ((int)currentEmotion >= EMOTION_COUNT) {

    currentEmotion = HAPPY;

  }


  emotionReaction(currentEmotion);

}


// =====================================================
//                 RANDOM EMOTION
// =====================================================

void randomEmotion() {

  int newEmotion =
    random(0, EMOTION_COUNT);


  currentEmotion =
    (Emotion)newEmotion;


  emotionReaction(currentEmotion);

}


// =====================================================
//                EMOTION REACTION
// =====================================================

void emotionReaction(Emotion emotion) {


  // Quick surprised reaction
  drawSurprised();


  tone(
    BUZZER_PIN,
    random(1000, 1700),
    80
  );


  delay(180);


  // Show selected emotion
  showEmotionFace(emotion);


  emotionSound(emotion);


  delay(250);


  // Show message
  showEmotionMessage(emotion);


  // Message stays 4 seconds
  delay(MESSAGE_TIME);


  // Return to emotion face
  showEmotionFace(emotion);

}


// =====================================================
//                 EMOTION MESSAGE
// =====================================================

void showEmotionMessage(Emotion emotion) {

  display.clearDisplay();


  display.setTextColor(SH110X_WHITE);


  // Emotion name
  display.setTextSize(1);

  display.setCursor(2, 2);

  display.print(emotionNames[(int)emotion]);


  // Random message
  int messageIndex =
    random(0, 3);


  const char* msg =
    messages[(int)emotion][messageIndex];


  display.setTextSize(1);


  // Simple word wrapping
  String text = String(msg);

  int y = 25;

  String line = "";

  for (int i = 0; i < text.length(); i++) {

    char c = text[i];

    if (c == ' ' || i == text.length() - 1) {

      if (i == text.length() - 1) {
        line += c;
      }


      if (line.length() > 20) {

        display.setCursor(2, y);

        display.println(line.substring(0, 20));

        y += 12;

        display.setCursor(2, y);

        display.println(line.substring(20));

        y += 12;

      }

      else {

        display.setCursor(2, y);

        display.println(line);

        y += 12;

      }


      line = "";

    }

    else {

      line += c;

    }

  }


  display.display();

}


// =====================================================
//                  EMOTION SOUND
// =====================================================

void emotionSound(Emotion emotion) {

  switch (emotion) {


    case HAPPY:

      tone(BUZZER_PIN, 1400, 80);

      delay(90);

      tone(BUZZER_PIN, 1800, 100);

      break;


    case EXCITED:

      tone(BUZZER_PIN, 1500, 70);

      delay(80);

      tone(BUZZER_PIN, 1900, 70);

      delay(80);

      tone(BUZZER_PIN, 2200, 100);

      break;


    case LOVE:

      tone(BUZZER_PIN, 1200, 100);

      delay(100);

      tone(BUZZER_PIN, 1500, 150);

      break;


    case SAD:

      tone(BUZZER_PIN, 700, 180);

      delay(100);

      tone(BUZZER_PIN, 500, 250);

      break;


    case ANGRY:

      tone(BUZZER_PIN, 300, 120);

      delay(70);

      tone(BUZZER_PIN, 220, 150);

      break;


    case SLEEPY:

      tone(BUZZER_PIN, 600, 150);

      delay(120);

      tone(BUZZER_PIN, 450, 180);

      break;


    case SURPRISED:

      tone(BUZZER_PIN, 2000, 100);

      break;


    case CONFUSED:

      tone(BUZZER_PIN, 900, 100);

      delay(100);

      tone(BUZZER_PIN, 1200, 100);

      break;


    case COOL:

      tone(BUZZER_PIN, 1000, 80);

      break;


    case SHY:

      tone(BUZZER_PIN, 800, 80);

      break;


    case PLAYFUL:

      tone(BUZZER_PIN, 1400, 70);

      delay(80);

      tone(BUZZER_PIN, 1000, 70);

      break;


    case BORED:

      tone(BUZZER_PIN, 500, 250);

      break;


    case WORRIED:

      tone(BUZZER_PIN, 900, 100);

      delay(100);

      tone(BUZZER_PIN, 700, 120);

      break;


    case PROUD:

      tone(BUZZER_PIN, 1200, 80);

      delay(80);

      tone(BUZZER_PIN, 1700, 120);

      break;


    case CALM:

      tone(BUZZER_PIN, 900, 180);

      break;


    case NEUTRAL:

      tone(BUZZER_PIN, 1100, 80);

      break;

  }

}


// =====================================================
//                  SHOW EMOTION FACE
// =====================================================

void showEmotionFace(Emotion emotion) {

  switch (emotion) {

    case HAPPY:
      drawHappy();
      break;

    case EXCITED:
      drawExcited();
      break;

    case LOVE:
      drawLove();
      break;

    case SAD:
      drawSad();
      break;

    case ANGRY:
      drawAngry();
      break;

    case SLEEPY:
      drawSleepy();
      break;

    case SURPRISED:
      drawSurprised();
      break;

    case CONFUSED:
      drawConfused();
      break;

    case COOL:
      drawCool();
      break;

    case SHY:
      drawShy();
      break;

    case PLAYFUL:
      drawPlayful();
      break;

    case BORED:
      drawBored();
      break;

    case WORRIED:
      drawWorried();
      break;

    case PROUD:
      drawProud();
      break;

    case CALM:
      drawCalm();
      break;

    case NEUTRAL:
      drawNeutral();
      break;

  }

}


// =====================================================
//                    EYE HELPER
// =====================================================

void eye(
  int x,
  int y,
  int w,
  int h
) {

  display.fillRoundRect(
    x,
    y,
    w,
    h,
    5,
    SH110X_WHITE
  );

}


// =====================================================
//                  PUPIL HELPER
// =====================================================

void pupil(
  int x,
  int y,
  int r
) {

  display.fillCircle(
    x,
    y,
    r,
    SH110X_BLACK
  );

}


// =====================================================
//                 HEART HELPER
// =====================================================

void heart(
  int x,
  int y
) {

  display.fillCircle(
    x - 5,
    y,
    5,
    SH110X_WHITE
  );

  display.fillCircle(
    x + 5,
    y,
    5,
    SH110X_WHITE
  );

  display.fillTriangle(
    x - 10,
    y + 2,
    x + 10,
    y + 2,
    x,
    y + 14,
    SH110X_WHITE
  );

}


// =====================================================
//                  SMILE HELPER
// =====================================================

void smile() {

  display.drawArc(
    64,
    38,
    15,
    10,
    20,
    160,
    SH110X_WHITE
  );

}


// =====================================================
//                    HAPPY
// =====================================================

void drawHappy() {

  display.clearDisplay();

  eye(18, 20, 35, 25);
  eye(75, 20, 35, 25);

  pupil(35, 32, 6);
  pupil(92, 32, 6);

  display.drawLine(48, 48, 80, 48, SH110X_WHITE);

  display.drawLine(52, 50, 76, 50, SH110X_WHITE);

  display.display();

}


// =====================================================
//                   EXCITED
// =====================================================

void drawExcited() {

  display.clearDisplay();

  eye(15, 17, 40, 30);
  eye(73, 17, 40, 30);

  pupil(35, 32, 8);
  pupil(92, 32, 8);

  display.fillCircle(64, 52, 5, SH110X_WHITE);

  display.display();

}


// =====================================================
//                     LOVE
// =====================================================

void drawLove() {

  display.clearDisplay();

  heart(35, 30);
  heart(92, 30);

  display.drawLine(
    52,
    50,
    76,
    50,
    SH110X_WHITE
  );

  display.display();

}


// =====================================================
//                      SAD
// =====================================================

void drawSad() {

  display.clearDisplay();

  eye(18, 22, 35, 22);
  eye(75, 22, 35, 22);

  pupil(35, 31, 5);
  pupil(92, 31, 5);

  display.drawLine(
    53,
    49,
    64,
    44,
    SH110X_WHITE
  );

  display.drawLine(
    64,
    44,
    75,
    49,
    SH110X_WHITE
  );

  display.display();

}


// =====================================================
//                     ANGRY
// =====================================================

void drawAngry() {

  display.clearDisplay();

  display.drawLine(
    15,
    20,
    50,
    28,
    SH110X_WHITE
  );

  display.drawLine(
    78,
    28,
    113,
    20,
    SH110X_WHITE
  );

  eye(20, 28, 28, 18);
  eye(80, 28, 28, 18);

  pupil(34, 37, 4);
  pupil(94, 37, 4);

  display.drawLine(
    52,
    51,
    76,
    51,
    SH110X_WHITE
  );

  display.display();

}


// =====================================================
//                    SLEEPY
// =====================================================

void drawSleepy() {

  display.clearDisplay();

  display.drawLine(
    18,
    32,
    50,
    32,
    SH110X_WHITE
  );

  display.drawLine(
    78,
    32,
    110,
    32,
    SH110X_WHITE
  );

  display.setTextSize(1);

  display.setCursor(55, 10);

  display.print("Z");

  display.setCursor(67, 3);

  display.print("z");

  display.display();

}


// =====================================================
//                  SURPRISED
// =====================================================

void drawSurprised() {

  display.clearDisplay();

  display.drawCircle(
    35,
    32,
    15,
    SH110X_WHITE
  );

  display.drawCircle(
    92,
    32,
    15,
    SH110X_WHITE
  );

  display.fillCircle(
    35,
    32,
    5,
    SH110X_WHITE
  );

  display.fillCircle(
    92,
    32,
    5,
    SH110X_WHITE
  );

  display.fillCircle(
    64,
    52,
    5,
    SH110X_WHITE
  );

  display.display();

}


// =====================================================
//                   CONFUSED
// =====================================================

void drawConfused() {

  display.clearDisplay();

  eye(18, 21, 35, 23);
  eye(75, 21, 35, 23);

  pupil(40, 32, 5);
  pupil(87, 32, 5);

  display.drawLine(
    53,
    49,
    70,
    46,
    SH110X_WHITE
  );

  display.display();

}


// =====================================================
//                      COOL
// =====================================================

void drawCool() {

  display.clearDisplay();

  display.fillRoundRect(
    14,
    22,
    45,
    20,
    4,
    SH110X_WHITE
  );

  display.fillRoundRect(
    69,
    22,
    45,
    20,
    4,
    SH110X_WHITE
  );

  display.drawLine(
    59,
    31,
    69,
    31,
    SH110X_WHITE
  );

  display.drawLine(
    48,
    52,
    80,
    52,
    SH110X_WHITE
  );

  display.display();

}


// =====================================================
//                      SHY
// =====================================================

void drawShy() {

  display.clearDisplay();

  eye(20, 25, 32, 20);
  eye(76, 25, 32, 20);

  pupil(40, 35, 4);
  pupil(88, 35, 4);

  display.fillCircle(
    50,
    48,
    5,
    SH110X_WHITE
  );

  display.fillCircle(
    78,
    48,
    5,
    SH110X_WHITE
  );

  display.display();

}


// =====================================================
//                    PLAYFUL
// =====================================================

void drawPlayful() {

  display.clearDisplay();

  eye(18, 20, 35, 25);
  eye(75, 20, 35, 25);

  pupil(40, 32, 6);
  pupil(87, 32, 6);

  display.drawLine(
    55,
    49,
    65,
    53,
    SH110X_WHITE
  );

  display.drawLine(
    65,
    53,
    75,
    49,
    SH110X_WHITE
  );

  display.display();

}


// =====================================================
//                      BORED
// =====================================================

void drawBored() {

  display.clearDisplay();

  display.drawLine(
    18,
    30,
    50,
    30,
    SH110X_WHITE
  );

  display.drawLine(
    78,
    30,
    110,
    30,
    SH110X_WHITE
  );

  display.drawLine(
    53,
    50,
    75,
    50,
    SH110X_WHITE
  );

  display.display();

}


// =====================================================
//                     WORRIED
// =====================================================

void drawWorried() {

  display.clearDisplay();

  display.drawLine(
    18,
    22,
    50,
    18,
    SH110X_WHITE
  );

  display.drawLine(
    78,
    18,
    110,
    22,
    SH110X_WHITE
  );

  eye(20, 25, 30, 20);
  eye(78, 25, 30, 20);

  pupil(35, 35, 4);
  pupil(93, 35, 4);

  display.drawCircle(
    64,
    51,
    4,
    SH110X_WHITE
  );

  display.display();

}


// =====================================================
//                      PROUD
// =====================================================

void drawProud() {

  display.clearDisplay();

  eye(18, 20, 35, 25);
  eye(75, 20, 35, 25);

  pupil(38, 32, 6);
  pupil(89, 32, 6);

  display.drawLine(
    52,
    47,
    64,
    52,
    SH110X_WHITE
  );

  display.drawLine(
    64,
    52,
    76,
    47,
    SH110X_WHITE
  );

  display.display();

}


// =====================================================
//                       CALM
// =====================================================

void drawCalm() {

  display.clearDisplay();

  display.drawLine(
    20,
    32,
    50,
    32,
    SH110X_WHITE
  );

  display.drawLine(
    78,
    32,
    108,
    32,
    SH110X_WHITE
  );

  display.drawLine(
    53,
    49,
    75,
    49,
    SH110X_WHITE
  );

  display.display();

}


// =====================================================
//                     NEUTRAL
// =====================================================

void drawNeutral() {

  display.clearDisplay();

  eye(18, 21, 35, 23);
  eye(75, 21, 35, 23);

  pupil(35, 32, 5);
  pupil(92, 32, 5);

  display.drawLine(
    54,
    49,
    74,
    49,
    SH110X_WHITE
  );

  display.display();

}


// =====================================================
//                  AUTO BLINK
// =====================================================

void autoBlink() {

  static unsigned long nextBlink =
    0;


  if (millis() > nextBlink) {

    // Close eyes
    display.clearDisplay();

    display.drawLine(
      18,
      32,
      50,
      32,
      SH110X_WHITE
    );

    display.drawLine(
      78,
      32,
      110,
      32,
      SH110X_WHITE
    );

    display.display();


    delay(120);


    // Normal face
    showEmotionFace(currentEmotion);


    // Next blink
    nextBlink =
      millis() + random(2500, 5500);

  }

}


// =====================================================
//                   SLEEP MODE
// =====================================================

void goToSleep() {

  isSleeping = true;


  Serial.println("BOT GOING TO SLEEP...");


  // Sleep sound
  tone(
    BUZZER_PIN,
    700,
    120
  );

  delay(150);

  tone(
    BUZZER_PIN,
    500,
    180
  );

  delay(250);


  // Sleep animation
  for (int i = 0; i < 3; i++) {

    display.clearDisplay();


    // Closed eyes
    display.drawLine(
      25,
      30,
      48,
      30,
      SH110X_WHITE
    );

    display.drawLine(
      80,
      30,
      103,
      30,
      SH110X_WHITE
    );


    display.display();

    delay(200);


    display.clearDisplay();

    display.setTextSize(1);

    display.setCursor(
      48,
      27
    );

    display.print("Z");

    display.setCursor(
      63,
      18
    );

    display.print("z");

    display.display();

    delay(200);

  }


  // Final sleeping face
  display.clearDisplay();


  display.drawLine(
    25,
    32,
    48,
    32,
    SH110X_WHITE
  );

  display.drawLine(
    80,
    32,
    103,
    32,
    SH110X_WHITE
  );


  display.setTextSize(1);

  display.setCursor(
    51,
    48
  );

  display.print("Zzz...");


  display.display();

}


// =====================================================
//                    WAKE UP
// =====================================================

void wakeUp() {

  Serial.println("BOT WAKING UP...");


  isSleeping = false;


  // Reset activity timer
  lastActivityTime = millis();


  // Wake sound
  tone(
    BUZZER_PIN,
    1000,
    80
  );

  delay(100);

  tone(
    BUZZER_PIN,
    1500,
    100
  );


  // Wake animation
  display.clearDisplay();

  display.setTextSize(2);

  display.setCursor(
    34,
    20
  );

  display.print("WAKE!");

  display.display();


  delay(700);


  // Return to previous emotion
  showEmotionFace(currentEmotion);

}


// =====================================================
//                  WATCH INTRO
// =====================================================

void showWatchIntro() {

  display.clearDisplay();

  display.setTextSize(2);

  display.setCursor(
    25,
    18
  );

  display.println("WATCH");

  display.setTextSize(1);

  display.setCursor(
    30,
    43
  );

  display.println("Mode ON");

  display.display();

  delay(1000);

}


// =====================================================
//                 EMOTION INTRO
// =====================================================

void showEmotionIntro() {

  display.clearDisplay();

  display.setTextSize(2);

  display.setCursor(
    20,
    18
  );

  display.println("EMOTION");

  display.setTextSize(1);

  display.setCursor(
    38,
    43
  );

  display.println("Mode ON");

  display.display();

  delay(1000);

}


// =====================================================
//                    WATCH MODE
// =====================================================

void showWatch() {

  struct tm timeinfo;


  if (!getLocalTime(&timeinfo, 100)) {

    display.clearDisplay();

    display.setTextSize(1);

    display.setCursor(
      30,
      25
    );

    display.println("Time unavailable");

    display.display();

    return;

  }


  int hour12 = timeinfo.tm_hour % 12;

  if (hour12 == 0) {
    hour12 = 12;
  }


  const char* ampm =
    timeinfo.tm_hour >= 12
      ? "PM"
      : "AM";


  // Blinking colon
  bool blinkColon =
    (millis() / 500) % 2;


  char timeString[12];


  if (blinkColon) {

    sprintf(
      timeString,
      "%02d:%02d",
      hour12,
      timeinfo.tm_min
    );

  }

  else {

    sprintf(
      timeString,
      "%02d %02d",
      hour12,
      timeinfo.tm_min
    );

  }


  display.clearDisplay();


  // Time
  display.setTextSize(2);

  display.setCursor(
    18,
    8
  );

  display.print(timeString);


  // AM / PM
  display.setTextSize(1);

  display.setCursor(
    98,
    14
  );

  display.print(ampm);


  // Date
  char dateString[20];


  sprintf(
    dateString,
    "%02d/%02d/%04d",
    timeinfo.tm_mday,
    timeinfo.tm_mon + 1,
    timeinfo.tm_year + 1900
  );


  display.setCursor(
    35,
    40
  );

  display.print(dateString);


  // Small WATCH label
  display.setCursor(
    50,
    55
  );

  display.print("WATCH");


  display.display();

}