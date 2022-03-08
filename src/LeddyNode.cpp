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

  advertise(cOnTopic)
      .setDatatype("boolean")
      .settable();

  advertise(cStateTopic)
      .setDatatype("enum")
      .setFormat("off,sunny,sunnyblue,blue,step")
      .settable();
}

bool LeddyNode::handleInput(const HomieRange &range, const String &property, const String &value)
{
#ifdef DEBUG
  Homie.getLogger() << "Message: " << property << " " << value << endl;
#endif
  if (property.equals(cOnTopic))
  {
    if (value == "true")
    {
      return setState(STATE::SUNNY);
    }
    else if (value == "false")
    {
      return reset();
    }
  }
  else if (property.equals(cStateTopic))
  {
    if (value == "off")
    {
      return reset();
    }
    else if (value == "sunny")
    {
      return setState(STATE::SUNNY);
    }
    else if (value == "sunnyblue")
    {
      return setState(STATE::SUNNYBLUE);
    }
    else if (value == "blue")
    {
      return setState(STATE::BLUE);
    }
    else if (value == "step")
    {
      return step();
    }
  }
  return false;
}

bool LeddyNode::step()
{
  switch (_currentState)
  {
  case STATE::OFF:
    return setState(STATE::SUNNY);
  case STATE::SUNNY:
    return setState(STATE::SUNNYBLUE);
  case STATE::SUNNYBLUE:
    return setState(STATE::BLUE);
  default:
    return setState(STATE::OFF);
  }
}

void LeddyNode::unblockStateChange()
{
  _stateChangeBlocked = false;
  Homie.getLogger() << F("State changes unblocked") << endl;
}

bool LeddyNode::reset()
{
  Homie.getLogger() << F("Resetting") << endl
                    << F("State changes blocked for 10 seconds") << endl;
  _currentState = STATE::OFF;
  setRelay(false);
  sendState();

  // Block state changes for 10 seconds
  _stateChangeBlocked = true;
  _ticker.once_ms(10000, std::bind(&LeddyNode::unblockStateChange, this));

  return true;
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
  bool on = getRelay();
  String state = getStateName(_currentState);

  printCaption();
  Homie.getLogger() << cIndent << "is " << (on ? "on" : "off") << endl;
  Homie.getLogger() << cIndent << "state " << state << endl;
  if (Homie.isConnected())
  {
    setProperty(cOnTopic).send(on ? "true" : "false");
    setProperty(cStateTopic).send(state);
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

bool LeddyNode::setState(STATE state)
{
  if (_stateChangeBlocked)
  {
    Homie.getLogger() << F("State changes blocked") << endl;
    return false;
  }

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
  return true;
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
    // Detach and send state when we're done stepping
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
