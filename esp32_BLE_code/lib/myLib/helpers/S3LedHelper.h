#ifndef S3_LED_HELPER_H
#define S3_LED_HELPER_H

#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

#define LED_DATA_PIN  48  // breakout board esp32-s3-devkitc
#define LED_PWR_EN    45  // breakout board esp32-s3-devkitc

/** the sessings of the led perist, so use once to reduce extreme
 *  brightness of the neopixel led. 
 */
class S3LedHelper {
private:
    Adafruit_NeoPixel pixels;

public:
    // Constructor sets up comms to LED
    S3LedHelper() : pixels(1, LED_DATA_PIN, NEO_GRB + NEO_KHZ800) {
        doLedSetup();
    }

    void doLedSetup() {
        Serial.println("Setting up LED...");
        pinMode(LED_PWR_EN, OUTPUT);
        digitalWrite(LED_PWR_EN, HIGH);
        delay(10); // Give the power a moment to stabilize
        pixels.begin();
    }

    /**
     * Modified setColor to include brightness
     * @param r Red (0-255)
     * @param g Green (0-255)
     * @param b Blue (0-255)
     * @param bright Brightness (0-255)
     */
    void setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t bright) {
        pixels.setBrightness(bright); // Update global brightness scale
        pixels.setPixelColor(0, pixels.Color(r, g, b));
        pixels.show();
    }

    void off() {
        pixels.setPixelColor(0, pixels.Color(0, 0, 0));
        pixels.show();
    }
};

#endif