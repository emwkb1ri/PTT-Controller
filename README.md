# PTT-Controller

PlatformIO / Arduino firmware for the **Seeed Studio XIAO ESP32-S3**.
It sends one complete **Left Alt+T** shortcut on each debounced button
press and another on each debounced release. It does not hold Alt+T for
the duration of the physical button press.

Based on the functional outline supplied in `C:\GitHub\PTT-Controller.txt`.
This is a USB/BLE keyboard accessory, not a radio PTT output or audio device.

## Wiring

| Function | XIAO pad | ESP32-S3 GPIO | Connection |
| --- | --- | --- | --- |
| PTT pushbutton | D0 / A0 | 1 | Normally open switch to GND |
| Mode strap | D1 / A1 | 2 | GND = USB; open or 3.3 V = BLE |
| Optional bond-reset button | D3 / A3 | 4 | Normally open switch to GND |

All three inputs use internal pull-ups. Do not apply 5 V to any input.
Connect switches to the board's GND. Unused optional bond-reset input can
remain open. For a fixed USB-only installation, strap D1 to GND.

GPIO4 deliberately replaces the chat's suggested GPIO3 for the optional
bond-reset button: GPIO3 is a chip strapping pin. **D3 means GPIO4, not GPIO3.**
Do not use the onboard BOOT button as the PTT or bond-reset button.
For long switch leads, consider an external pull-up, filtering, and ESD
protection appropriate to the installation.

## Operation

- Mode is sampled once at startup. Reset or power-cycle after changing D1.
- Each physical input state must remain stable for 25 ms before an event.
- Each shortcut is a combined Alt+T HID report followed by an all-keys-up
  report after at least 10 ms. There is no blocking debounce or key-hold delay.
- The initial button state is adopted without sending a startup shortcut.
  If held at startup, its eventual debounced release sends a shortcut.
- Button edges while disconnected are discarded, never queued or replayed.
- Reconnection clears the local key report; it does not send an Alt+T shortcut.
- The yellow user LED is solid when connected, slow-blinking while waiting
  for a host, and fast-blinking if BLE initialization/bond clearing fails.
  Connection does not prove that the target application received a shortcut.

Pins and timing constants are in `include/Config.h`.

### USB

Strap D1 to GND before startup and connect a USB **data** cable to the XIAO.
The firmware uses native USB HID/TinyUSB, not USB serial/JTAG. No special
keyboard driver should be needed on the host. Normal firmware has USB CDC
disabled and does not provide a serial monitor port.

### Bluetooth LE

Leave D1 open before startup. Attach the XIAO's supplied antenna and provide
power, for example through USB. In this mode, USB provides power but is not
started as a keyboard or serial device.

Pair the host with **XIAO-AltT-Key** in its Bluetooth settings. Existing
bonds are retained across resets, and the keyboard advertises again after
disconnect so a previously paired host can reconnect. The host controls
automatic reconnection. New pairing is also possible during advertising;
the bond-reset button is not a pairing authorization gate.

To forget previous hosts, hold the external D3-to-GND button while resetting
or powering on in BLE mode. Keep it held through startup, then release it.
The firmware requires the input to stay low throughout its 25 ms startup
sampling window and requests removal of stored BLE bonds before advertising.
Forget/remove the old keyboard entry on the host as well, then pair again.
Pressing this button after startup has no effect; USB mode ignores it.

Pairing uses bonding with no display or passkey entry (Just Works). It does
not provide MITM-authenticated pairing. Pair in a trusted environment. The
reported battery level is fixed at 100%; no battery measurement is implemented.

## Build And Upload

Open this folder in VS Code with the recommended **PlatformIO IDE** extension.
The default environment is `xiao_esp32s3`. Dependencies are pinned to
Espressif32 6.10.0 (Arduino-ESP32 2.0.17) and ESP32 BLE Keyboard 0.3.2;
do not independently upgrade to Arduino-ESP32 3.x without checking BLE APIs.

Use the VS Code tasks **PTT: Build Firmware** and **PTT: Upload Firmware**.
The tasks use PlatformIO's standard Windows installation location. From a
PlatformIO terminal, the equivalent commands are:

```powershell
pio run -e xiao_esp32s3
pio run -e xiao_esp32s3 -t upload
```

The firmware image is `.pio/build/xiao_esp32s3/firmware.bin`.

For first upload or whenever the application exposes no serial port, enter
the ROM bootloader: hold BOOT, press and release RESET, then release BOOT.
Upload using the resulting COM port. If detection is ambiguous:

```powershell
pio run -e xiao_esp32s3 -t upload --upload-port COM7
```

Replace COM7 with the actual bootloader port. Press RESET after uploading
if the board remains in the bootloader. Firmware is not automatically flashed
by project creation.

### BLE Connection Diagnostics

For repeated Bluetooth disconnects, the optional `xiao_esp32s3_ble_debug`
environment enables USB CDC serial logging without changing the normal build.
Enter the ROM bootloader as above, then upload from a PlatformIO terminal:

```powershell
pio run -e xiao_esp32s3_ble_debug -t upload --upload-port COM20
```

Replace COM20 with the actual bootloader port. Press RESET and leave D1 open
for BLE mode. Find the new USB serial port in Device Manager, then open:

```powershell
pio device monitor --port COM21 --baud 115200
```

Replace COM21 with the running firmware's port. Capture authentication results
and disconnect reason codes while reproducing the issue. Status lines every
five seconds include uptime in milliseconds and the last ESP32 reset reason;
uptime restarting suggests board resets rather than only BLE link loss.
Close the monitor before uploading again. Upload the normal `xiao_esp32s3`
environment afterward to restore operation without USB serial.

## Verification

Run the VS Code task **PTT: Test Debounce**. It supplies the PATH needed by
PlatformIO's managed Windows MinGW compiler. From PowerShell instead:

```powershell
$env:Path = "$env:USERPROFILE\.platformio\packages\toolchain-gccmingw32\bin;$env:Path"
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" test -e native
```

PlatformIO installs the compiler package on the first test run. The five
host tests cover both edges, bounce on both edges, startup state, short
pulses, and the 32-bit millisecond timer wrapping around. Firmware compilation
checks both transports; host tests do not emulate USB, Bluetooth, or the board.

Hardware acceptance checks, still required:

1. In USB mode, verify one shortcut on press and one on release in a test
   application configured to count Alt+T. Holding the button must not repeat.
2. Verify startup, quick taps, contact bounce, and a button held at boot.
3. In BLE mode, pair as XIAO-AltT-Key and repeat the button checks. Verify
   reconnect after reset and confirm no shortcuts arrive merely on reconnect.
4. Hold D3 at boot to clear bonds, forget the host entry, and verify re-pairing.
5. Verify the mode strap takes effect only after reset, and USB mode never
   sends BLE keyboard reports.
6. Disconnect during a press/release cycle and verify the host application's
   state is checked and restored manually before use.

**Toggle limitation:** Alt+T does not communicate an absolute PTT state.
A lost shortcut, disconnect, wrong application focus, host sleep, or another
keyboard shortcut can leave the host's transmit/mute state out of sync with
the physical button. This design cannot guarantee release-to-stop-transmit
across connection loss. Validate the application's shortcut and fail-safe
behavior before using it with an actual transmitter.
