/*
 * LeddyNode.cpp
 * Homie Node for an aquarium light with three colors + off
 *
 * Version: 1.0
 * Author: Lübbe Onken (http://github.com/luebbe)
 */

#include "LeddyNode.hpp"

LeddyNode::LeddyNode(const char *name, const int relayPin, const int ledPin)
    : HomieNode(name, "LeddyNode", "actor")
{
  _relayPin = relayPin;
  _ledPin = ledPin;

  advertise("state")
      .setDatatype("enum")
      .setFormat("off,sunny,sunnyblue,blue,step")
      .settable();
}

bool LeddyNode::handleInput(const HomieRange &range, const String &property, const String &value)
{
  if (property.equals("state"))
  {
    if (value == "off")
    {
      setState(STATE::OFF);
      return true;
    }
    else if (value == "sunny")
    {
      setState(STATE::SUNNY);
      return true;
    }
    else if (value == "sunnyblue")
    {
      setState(STATE::SUNNYBLUE);
      return true;
    }
    else if (value == "blue")
    {
      setState(STATE::BLUE);
      return true;
    }
    else if (value == "step")
    {
      step();
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }
}

void LeddyNode::step()
{
  switch (_currentState)
  {
  case STATE::OFF:
    setState(STATE::SUNNY);
    break;
  case STATE::SUNNY:
    setState(STATE::SUNNYBLUE);
    break;
  case STATE::SUNNYBLUE:
    setState(STATE::BLUE);
    break;
  default:
    setState(STATE::OFF);
    break;
  }
}

void LeddyNode::onReadyToOperate()
{
  setRelay(false);
  sendState();
};

void LeddyNode::printCaption()
{
  Homie.getLogger() << cCaption << endl;
}

String LeddyNode::getStateName(STATE state)
{
  switch (state)
  {
  case STATE::SUNNY:
    return "sunny";
  case STATE::SUNNYBLUE:
    return "sunnyblue";
  case STATE::BLUE:
    return "blue";
  case STATE::OFF:
    return "off";
  default:
    return "invalid";
  }
}

void LeddyNode::sendState()
{
  printCaption();
  String state = getStateName(_currentState);
  Homie.getLogger() << cIndent << " State " << state << endl;
  if (Homie.isConnected())
  {
    setProperty("state").send(state);
  }
}

void LeddyNode::setLed(bool on)
{
#ifdef DEBUG
  Homie.getLogger() << "LED: " << _ledPin << " " << on << endl;
#endif

  if (_ledPin > DEFAULTPIN)
  {
    digitalWrite(_ledPin, on ? LOW : HIGH); // LOW = LED on
  }
}

bool LeddyNode::getRelay()
{
  if (_relayPin > DEFAULTPIN)
  {
    return digitalRead(_relayPin) == HIGH;
  }
  else
  {
    return false;
  }
}

void LeddyNode::setRelay(bool on)
{
#ifdef DEBUG
  Homie.getLogger() << "Relay: " << _relayPin << " " << on << endl;
#endif

  if (_relayPin > DEFAULTPIN)
  {
    digitalWrite(_relayPin, on ? HIGH : LOW);
  }
  // Set Led according to relay
  setLed(on);
}

void LeddyNode::setState(STATE state)
{
  if (_currentState != state)
  {
    if (_currentState == STATE::OFF)
    {
      // If currently off, we have an odd number of ticks
      _ticks = (state - _currentState) * 2 - 1;
    }
    else
    {
      // Toggle relay off and on to get from one of the other three states to the next
      _ticks = ((state - _currentState + 3) % 3) * 2;
    }

#ifdef DEBUG
    Homie.getLogger() << "State: " << getStateName(_currentState) << " -> " << getStateName(state) << " Ticks: " << _ticks << endl;
#endif

    _currentState = state;

    if (state == STATE::OFF)
    {
      // turn relay off immediately
      setRelay(false);
      sendState();
    }
    else
    {
      _ticker.attach_ms(200, std::bind(&LeddyNode::tick, this));
    }
  }
}

void LeddyNode::tick(void)
{
#ifdef DEBUG
  Homie.getLogger() << "Tick: " << _ticks << endl;
#endif

  if (_ticks > 0)
  {
    toggleRelay();
    _ticks--;
  }
  else
  {
    _ticker.detach();
    sendState();
  }
}

void LeddyNode::toggleRelay()
{
  setRelay(!getRelay());
}

void LeddyNode::setup()
{
  printCaption();

  Homie.getLogger() << cIndent << "Relay Pin: " << _relayPin << endl
                    << cIndent << "Led Pin  : " << _ledPin << endl;

  if (_ledPin > DEFAULTPIN)
  {
    pinMode(_ledPin, OUTPUT);
  }

  if (_relayPin > DEFAULTPIN)
  {
    pinMode(_relayPin, OUTPUT);
  }
}

void LeddyNode::loop()
{
}
