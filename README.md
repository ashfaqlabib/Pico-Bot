# Pico Bot — ESP32-S3 Emotion Robot

A small interactive robot built with **ESP32-S3**, **SH1106 OLED**, **TTP223 touch sensor**, and a **buzzer**.

Pico Bot can express multiple emotions using animated robot-style eyes, respond to touch, play emotion-based sounds, display Banglish messages, show Bangladesh time in Watch Mode, and automatically go to sleep when left inactive.

---

## Features

* 16 different emotions
* Animated robot-style facial expressions
* Multiple Banglish messages for each emotion
* Random message selection
* Natural automatic blinking
* Emotion-specific buzzer sounds
* Single-touch emotion switching
* Double-touch random emotion
* Long-touch mode switching
* Wi-Fi connectivity
* NTP-based accurate Bangladesh time
* 12-hour Watch Mode with AM/PM
* Date display in Watch Mode
* Automatic sleeping after 2 minutes of inactivity
* Touch-based wake-up system
* SH1106 128×64 OLED display

---

## Emotions

Pico Bot currently supports:

1. Happy
2. Excited
3. Love
4. Sad
5. Angry
6. Sleepy
7. Surprised
8. Confused
9. Cool
10. Shy
11. Playful
12. Bored
13. Worried
14. Proud
15. Calm
16. Neutral

---

## Hardware

| Component           | Description           |
| ------------------- | --------------------- |
| ESP32-S3 Super Mini | Main microcontroller  |
| SH1106 OLED         | 128×64 I2C OLED       |
| TTP223              | Touch sensor          |
| Buzzer              | Sound feedback        |
| Jumper Wires        | Connections           |
| USB Cable           | Programming and power |

---

## Pin Configuration

### OLED

| OLED | ESP32-S3 |
| ---- | -------- |
| VCC  | 3.3V     |
| GND  | GND      |
| SDA  | GPIO 8   |
| SCL  | GPIO 9   |

OLED I2C Address:

```text
0x3C
```

### TTP223

| TTP223 | ESP32-S3 |
| ------ | -------- |
| VCC    | 3.3V     |
| GND    | GND      |
| SIG    | GPIO 4   |

### Buzzer

| Buzzer | ESP32-S3 |
| ------ | -------- |
| +      | GPIO 5   |
| -      | GND      |

---

## Touch Controls

| Touch Action        | Function                  |
| ------------------- | ------------------------- |
| Single Touch        | Next Emotion              |
| Double Touch        | Random Emotion            |
| Long Touch ~1.2 sec | Emotion Mode ↔ Watch Mode |
| Touch During Sleep  | Wake Up                   |

---

## Emotion Mode

In Emotion Mode, Pico Bot displays different facial expressions.

For example:

```text
HAPPY

  [ Happy Eyes ]

Ami onek khushi!
```

After displaying the message, the robot returns to its emotion face.

Each emotion contains three different messages, and one message is randomly selected whenever that emotion is triggered.

---

## Watch Mode

Long-touching the TTP223 switches Pico Bot into Watch Mode.

The OLED displays:

```text
      02:35 AM
      09/10/2026
         WATCH
```

The time is synchronized using Wi-Fi and NTP.

Timezone:

```text
UTC +6
```

The clock uses 12-hour format with AM/PM.

---

## Auto Sleep Mode

Pico Bot automatically enters Sleep Mode after:

```text
2 minutes
```

without any touch activity.

During Sleep Mode:

* Eyes close
* Sleeping animation appears
* `Zzz...` is displayed
* A sleep sound is played

Touching the TTP223 wakes the robot and returns it to the previous emotion.

---

## Required Arduino Libraries

Install the following libraries from Arduino IDE Library Manager:

### Adafruit GFX Library

Used for drawing graphics and text on the OLED.

### Adafruit SH110X

Used to control the SH1106 OLED display.

The project uses:

```cpp
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
```

The project also uses the ESP32 built-in libraries:

```cpp
WiFi.h
Wire.h
time.h
```

---

## Wi-Fi Configuration

Before uploading the code, update:

```cpp
const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
```

Example:

```cpp
const char* WIFI_SSID = "MyWiFi";
const char* WIFI_PASSWORD = "12345678";
```

**Do not upload your real Wi-Fi password to a public GitHub repository.**

For a public repository, keep the credentials as placeholders or use a separate local configuration file that is excluded from Git.

---

## How It Works

### Startup

When Pico Bot starts:

```text
Power ON
   ↓
OLED Initialization
   ↓
Boot Animation
   ↓
Wi-Fi Connection
   ↓
NTP Time Synchronization
   ↓
Happy Emotion
```

### Emotion Interaction

```text
Touch
  ↓
Detect Touch Type
  ↓
Single / Double / Long
  ↓
Select Action
  ↓
Emotion Reaction
  ↓
Buzzer Sound
  ↓
Random Message
  ↓
Message Display
  ↓
Return to Emotion Face
```

### Sleep System

```text
No Touch
   ↓
2 Minutes
   ↓
Sleep Mode
   ↓
Touch
   ↓
Wake Up
   ↓
Previous Emotion
```

---

## Project Structure

```text
Pico-Bot-ESP32-S3/
│
├── pico_bot.ino
├── README.md
├── images/
│   └── pico-bot.jpg
│
└── LICENSE
```

---

## Future Improvements

Possible future versions may include:

* More realistic eye animations
* More emotions
* More message variations
* Emotion history
* Mobile phone control
* Web-based control panel
* Voice interaction
* AI-powered emotion detection
* Bluetooth control
* Battery-powered wearable version
* Custom 3D printed robot body
* Music and sound effects
* Birthday mode
* Interactive games

---

## Project Goal

The goal of Pico Bot is to create a small, interactive and expressive robot that combines embedded systems, display animation, touch interaction, sound feedback, Wi-Fi connectivity and intelligent behavior in a compact device.

---

## Technologies

* ESP32-S3
* Arduino IDE
* C/C++
* I2C
* SH1106 OLED
* Wi-Fi
* NTP
* Touch sensing
* Embedded systems

---

## Author

**Ashfaq Labib**

Pico Bot — ESP32-S3 Interactive Emotion Robot

---

## License

This project is open-source and can be used for learning, experimentation and educational purposes.

If you use or modify this project, giving credit to the original project is appreciated.
