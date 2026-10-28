#include <Arduino.h>
#include <BLEDevice.h>
#include <BleKeyboard.h>
#include <esp_gap_ble_api.h>
#include <esp_system.h>

#include <vector>

#include "Config.h"
#include "KeyboardTransport.h"

namespace {
BleKeyboard keyboard(Config::deviceName, "PTT-Controller", 100);

#if defined(PTT_BLE_DEBUG)
void logGapEvent(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t* parameters) {
    if (event == ESP_GAP_BLE_AUTH_CMPL_EVT) {
        Serial.printf("[%lu] BLE authentication: success=%u reason=0x%02X\n",
                      millis(), parameters->ble_security.auth_cmpl.success,
                      parameters->ble_security.auth_cmpl.fail_reason);
    }
}

void logGattsEvent(esp_gatts_cb_event_t event, esp_gatt_if_t,
                   esp_ble_gatts_cb_param_t* parameters) {
    if (event == ESP_GATTS_CONNECT_EVT) {
        Serial.printf("[%lu] BLE connected\n", millis());
    } else if (event == ESP_GATTS_DISCONNECT_EVT) {
        Serial.printf("[%lu] BLE disconnected: reason=0x%02X\n",
                      millis(), parameters->disconnect.reason);
    }
}
#endif

bool removeBonds() {
    int count = esp_ble_get_bond_device_num();
    if (count < 0) {
        return false;
    }
    if (count == 0) {
        return true;
    }

    std::vector<esp_ble_bond_dev_t> bonds(count);
    if (esp_ble_get_bond_device_list(&count, bonds.data()) != ESP_OK) {
        return false;
    }

    for (int index = 0; index < count; ++index) {
        if (esp_ble_remove_bond_device(bonds[index].bd_addr) != ESP_OK) {
            return false;
        }
    }
    return true;
}
}

bool BluetoothKeyboard::begin(bool clearBonds) {
#if defined(PTT_BLE_DEBUG)
    Serial.begin(115200);
    BLEDevice::setCustomGapHandler(logGapEvent);
    BLEDevice::setCustomGattsHandler(logGattsEvent);
#endif
    BLEDevice::init(Config::deviceName);
    if (clearBonds && !removeBonds()) {
        return false;
    }

    keyboard.begin();
    uint8_t authentication = ESP_LE_AUTH_REQ_SC_BOND;
    uint8_t capability = ESP_IO_CAP_NONE;
    uint8_t keys = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    return esp_ble_gap_set_security_param(
               ESP_BLE_SM_AUTHEN_REQ_MODE, &authentication,
               sizeof(authentication)) == ESP_OK &&
           esp_ble_gap_set_security_param(
               ESP_BLE_SM_IOCAP_MODE, &capability,
               sizeof(capability)) == ESP_OK &&
           esp_ble_gap_set_security_param(
               ESP_BLE_SM_SET_INIT_KEY, &keys, sizeof(keys)) == ESP_OK &&
           esp_ble_gap_set_security_param(
               ESP_BLE_SM_SET_RSP_KEY, &keys, sizeof(keys)) == ESP_OK;
}

bool BluetoothKeyboard::connected() {
#if defined(PTT_BLE_DEBUG)
    static uint32_t lastStatusAt = 0;
    const uint32_t now = millis();
    if (static_cast<uint32_t>(now - lastStatusAt) >= 5000) {
        Serial.printf("[%lu] BLE status: connected=%u reset_reason=%d\n",
                      now, keyboard.isConnected(), static_cast<int>(esp_reset_reason()));
        lastStatusAt = now;
    }
#endif
    return keyboard.isConnected();
}

void BluetoothKeyboard::pressAltT() {
    KeyReport report = {};
    report.modifiers = 0x04;
    report.keys[0] = 0x17;
    keyboard.sendReport(&report);
}

void BluetoothKeyboard::releaseAll() {
    keyboard.releaseAll();
}