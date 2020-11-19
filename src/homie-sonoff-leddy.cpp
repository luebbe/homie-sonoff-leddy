/*
 * Firmware for a Sonoff S20 to control an aquarium light
 * with three colors + off
 *
 * Author: Lübbe Onken (http://github.com/luebbe)
 */

#define FW_NAME "sonoff-s20-leddy"
#define FW_VERSION "1.0.1"

#include <Homie.hpp>
#include "ButtonNode.hpp"
#include "LeddyNode.hpp"

// Sonoff S20 PINs
const int PIN_BUTTON = 0;
const int PIN_RELAY = 12;
const int PIN_GREEN_LED = 13;

LeddyNode leddyNode("leddy", PIN_RELAY);

// Initialize button node with callback to button press
ButtonNode buttonNode("button", PIN_BUTTON, []() {
  leddyNode.step();
});

//################################################

void setup()
{
  Homie_setFirmware(FW_NAME, FW_VERSION);

  Serial.begin(SERIAL_SPEED);
  Serial << endl
         << endl;

  pinMode(PIN_BUTTON, INPUT_PULLUP);

  Homie.disableResetTrigger();
  Homie.setLedPin(PIN_GREEN_LED, LOW);

  Homie.setup();
}

void loop()
{
  Homie.loop();
}
