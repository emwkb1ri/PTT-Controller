#pragma once

namespace UsbKeyboard {
void begin();
bool connected();
void pressAltT();
void releaseAll();
}

namespace BluetoothKeyboard {
bool begin(bool clearBonds);
bool connected();
void pressAltT();
void releaseAll();
}