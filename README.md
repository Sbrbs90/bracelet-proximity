# Bracelet proximity

Two wearable bracelets that sense each other over Bluetooth. As the wearers get closer, a single LED runs along the strip and speeds up. A button click connects the bracelets and keeps all LEDs on. A double click disconnects them.

Part of my interaction design portfolio: 

## Hardware

- 2x Arduino Nano 33 Sense (a Nano 33 BLE also works)
- 2x Seeed Grove Shield for Arduino Nano (switch set to 3.3V)
- 2x flexible NeoPixel LED strip, 7 LEDs, on port A2
- 1x Grove button on port D2 (bracelet A only)
- USB power bank or battery for each board

## Libraries

Install from Arduino IDE (Tools > Manage Libraries):

- ArduinoBLE
- Adafruit NeoPixel

## Files

- `bracelet_A/bracelet_A.ino`: has the button. Finds bracelet B, connects, sends the connected/disconnected state.
- `bracelet_B/bracelet_B.ino`: no button. Waits for bracelet A and follows its state.

## How to use

1. Upload `bracelet_A` to the board with the button and `bracelet_B` to the other one.
2. Power B first, then A. They link on their own after a few seconds.
3. Start more than a metre apart: LEDs are off.
4. Get closer: one LED runs along the strip, faster as you approach.
5. Click the button once: both bracelets flash and all LEDs stay on.
6. Double click: the LEDs turn off one by one on both bracelets.

If the link drops, both bracelets turn off.

## How it works

Bluetooth Low Energy signal strength (RSSI) is used as a rough measure of distance. Bracelet B advertises and waits, bracelet A finds it and connects. Once linked, both boards read the signal every 100 ms, smooth it, and map it to the speed of the running LED.

The connected state is a single value on a shared BLE characteristic. A writes it when the button is clicked, B reads it and reacts.

## Tuning

Distances depend on two values at the top of each sketch, which must be the same on both boards:

- `farRssi` (-80): the effect starts here. A number closer to zero means you must be closer.
- `nearRssi` (-50): maximum speed.

On my table, the signal was roughly -37 to -46 at contact, -50 to -54 at 30 cm and -70 to -79 at 1 metre. Values change when the bracelets are worn, because the body absorbs part of the signal, so retune on the wrist.

Set `debug = true` to print the signal strength to the Serial Monitor.

## Known limits

- Bluetooth signal strength is not precise, so distances are approximate.
- Only bracelet A has a button.
- LEDs react with a small delay.

## Next steps

- Make the bracelets wearable (band, slim battery, fabric to diffuse the light)
- Button on both bracelets
- Test with real pairs of people
