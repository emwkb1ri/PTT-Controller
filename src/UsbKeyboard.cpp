#include <Arduino.h>
#include <USB.h>
#include <USBHIDKeyboard.h>
#include <tusb.h>

#include "Config.h"
#include "KeyboardTransport.h"

namespace {
USBHIDKeyboard keyboard;
}

void UsbKeyboard::begin() {
    USB.productName(Config::deviceName);
    keyboard.begin();
    USB.begin();
}

bool UsbKeyboard::connected() {
    return tud_mounted() && !tud_suspended();
}

void UsbKeyboard::pressAltT() {
    KeyReport report = {};
    report.modifiers = 0x04;
    report.keys[0] = 0x17;
    keyboard.sendReport(&report);
}

void UsbKeyboard::releaseAll() {
    keyboard.releaseAll();
}