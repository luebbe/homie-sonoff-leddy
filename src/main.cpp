/*
 * Firmware for a Sonoff S20 to control an aquarium light
 * with three colors + off
 *
 * Author: Lübbe Onken (http://github.com/luebbe)
 */

#define FW_NAME "sonoff-s20-leddy"
#define FW_VERSION "1.0.1"

#include <Homie.hpp>
#include "push_button.h"
#include "LeddyNode.hpp"

// Sonoff S20 PINs
const int PIN_BUTTON = 0;     // D3 on D1 Mini
const int PIN_RELAY = 12;     // D6 on D1 Mini
const int PIN_GREEN_LED = 13; // D7 on D1 Mini

using pb::PushButton;

PushButton button(PIN_BUTTON);

LeddyNode leddyNode("leddy", PIN_RELAY, PIN_GREEN_LED);

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

  switch (button.getEvent())
  {
  case PushButton::Event::SHORT_PRESS:
    leddyNode.step();
    break;
  // use all long press actions to reset
  case PushButton::Event::LONG_PRESS:
  case PushButton::Event::LONG_HOLD:
  case PushButton::Event::INVALID:
    leddyNode.setState(LeddyNode::STATE::OFF);
    break;
  default:
    break;
  }
}
