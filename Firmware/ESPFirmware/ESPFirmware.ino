//Par Gunderson
//10/04/2026
//V-ARC ESP32-S3 Firmware
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <BleKeyboard.h>

//Pin Defs
#define PIN_SW_PAGE D1
#define OLED_SDA    D4
#define OLED_SCL    D5
#define UART_TX     D6
#define UART_RX     D7

//OLED Setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//BLE Setup
BleKeyboard bleKeyboard("Violin Pedalboard", "Custom", 100);

//Pedal time vars
const unsigned long DEBOUNCE_DELAY = 50;
const unsigned long HOLD_THRESHOLD = 500;

//Button State vars
unsigned long press_start_time = 0;
bool is_pressed = false;
bool hold_action_triggered = false;

void setup() {
    //Init Switch
    pinMode(PIN_SW_PAGE, INPUT_PULLUP);

    //Init UART
    Serial1.begin(115200, SERIAL_8N1, UART_RX, UART_TX);

    //Init I2C and OLED
    Wire.begin(OLED_SDA, OLED_SCL);
    if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        display.println("V-ARC");
        display.println("BLE: Pairing...");
        display.display();
    }

    //Start BLE
    bleKeyboard.begin();
}

void loop() {
    bool current_state = digitalRead(PIN_SW_PAGE);

    if (bleKeyboard.isConnected()) {
        //Check Pressed
        if (current_state == LOW && !is_pressed) {
            is_pressed = true;
            press_start_time = millis();
            hold_action_triggered = false;
        }

        if (is_pressed && current_state == LOW) {
            //Check held
            if (!hold_action_triggered && (millis() - press_start_time >= HOLD_THRESHOLD)) {
                bleKeyboard.write(KEY_LEFT_ARROW);
                hold_action_triggered = true;
                
                //Tell daisy
                Serial1.write(0xA1);
            }
        }
        
        if (current_state == HIGH && is_pressed) {
            unsigned long press_duration = millis() - press_start_time;
            is_pressed = false;

            //When Tapped
            if (!hold_action_triggered && (press_duration >= DEBOUNCE_DELAY)) {
                bleKeyboard.write(KEY_RIGHT_ARROW);
                
                //Tell daisy
                Serial1.write(0xA2);
            }
        }
    }

    //Update Display
    static unsigned long last_ui_update = 0;
    if (millis() - last_ui_update > 250) {
        last_ui_update = millis();
        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("BLE: ");
        display.println(bleKeyboard.isConnected() ? "CONNECTED" : "WAITING...");
        
        display.setCursor(0, 16);
        display.println("DSP STATUS: ACTIVE");
        display.display();
    }
}