#include <Arduino.h>

#include "Config.h"
#include "DebouncedButton.h"
#include "KeyboardTransport.h"

namespace {
DebouncedButton button(Config::debounceMs);
bool bluetoothMode = false;
bool transportStarted = false;
bool wasConnected = false;
bool keyDown = false;
uint32_t keyPressedAt = 0;

bool connected() {
    return transportStarted && (bluetoothMode
        ? BluetoothKeyboard::connected() : UsbKeyboard::connected());
}

void releaseAll() {
    if (bluetoothMode) {
        BluetoothKeyboard::releaseAll();
    } else {
        UsbKeyboard::releaseAll();
    }
    keyDown = false;
}

void sendAltT() {
    if (bluetoothMode) {
        BluetoothKeyboard::pressAltT();
    } else {
        UsbKeyboard::pressAltT();
    }
    keyDown = true;
    keyPressedAt = millis();
}
}

void setup() {
    pinMode(Config::buttonPin, INPUT_PULLUP);
    pinMode(Config::modePin, INPUT_PULLUP);
    pinMode(Config::pairingResetPin, INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    const uint32_t startedAt = millis();
    bool resetHeld = true;
    while (static_cast<uint32_t>(millis() - startedAt) < Config::debounceMs) {
        resetHeld = resetHeld && digitalRead(Config::pairingResetPin) == LOW;
        yield();
    }

    bluetoothMode = digitalRead(Config::modePin) == HIGH;
    if (bluetoothMode) {
        transportStarted = BluetoothKeyboard::begin(resetHeld);
    } else {
        UsbKeyboard::begin();
        transportStarted = true;
    }

    button.begin(digitalRead(Config::buttonPin) == LOW, millis());
}

void loop() {
    const uint32_t now = millis();
    const bool isConnected = connected();

    if (isConnected != wasConnected) {
        releaseAll();
        wasConnected = isConnected;
    }

    if (keyDown && static_cast<uint32_t>(now - keyPressedAt) >= Config::keyHoldMs) {
        releaseAll();
    }

    if (button.update(digitalRead(Config::buttonPin) == LOW, now) && isConnected) {
        if (keyDown) {
            releaseAll();
        }
        sendAltT();
    }

    if (!transportStarted) {
        digitalWrite(LED_BUILTIN, (now / 100) % 2 == 0 ? LOW : HIGH);
    } else if (isConnected) {
        digitalWrite(LED_BUILTIN, LOW);
    } else {
        digitalWrite(LED_BUILTIN, (now / 500) % 2 == 0 ? LOW : HIGH);
    }

    yield();
}