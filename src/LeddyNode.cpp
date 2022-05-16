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
      _nextState = STATE::SUNNY;
    }
    else if (value == "false")
    {
      _nextState = STATE::OFF;
    }
    return true;
  }
  else if (property.equals(cStateTopic))
  {
    if (value == "off")
    {
      _nextState = STATE::OFF;
    }
    else if (value == "sunny")
    {
      _nextState = STATE::SUNNY;
    }
    else if (value == "sunnyblue")
    {
      _nextState = STATE::SUNNYBLUE;
    }
    else if (value == "blue")
    {
      _nextState = STATE::BLUE;
    }
    else if (value == "step")
    {
      step();
    }
    return true;
  }
  return false;
}

void LeddyNode::step()
{
  switch (_currentState)
  {
  case STATE::OFF:
    _nextState = STATE::SUNNY;
    break;
  case STATE::SUNNY:
    _nextState = STATE::SUNNYBLUE;
    break;
  case STATE::SUNNYBLUE:
    _nextState = STATE::BLUE;
    break;
  default:
    _nextState = STATE::OFF;
    break;
  }
  Homie.getLogger() << "State: " << getStateName(_currentState) << "->" << getStateName(_nextState) << endl;
}

void LeddyNode::onReadyToOperate()
{
  setState(STATE::OFF);
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
}

void LeddyNode::setState(STATE state)
{
  if (_stateChangeBlocked)
  {
    Homie.getLogger() << F("State changes blocked") << endl;
    return;
  }

  if (state == STATE::OFF)
  {
    Homie.getLogger() << F("Turning OFF") << endl
                      << F("State changes blocked for 10 seconds") << endl;
    // Block state changes for 10 seconds, turn on LED while blocked
    setLed(true);
    setRelay(false);
    _currentState = STATE::OFF;
    _stateChangeBlocked = true;
    _ticker.once_ms(10000, std::bind(&LeddyNode::unblockStateChange, this));
    sendState();
  }
  else if (_currentState != state)
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
    _ticker.attach_ms(200, std::bind(&LeddyNode::tick, this));
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
    // Detach and send state when we're done stepping
    _ticker.detach();
    sendState();
  }
}

void LeddyNode::unblockStateChange()
{
  setLed(false);
  _stateChangeBlocked = false;
  Homie.getLogger() << F("State changes unblocked") << endl;
}

void LeddyNode::toggleRelay()
{
  setRelay(!getRelay());
}

void LeddyNode::loop()
{
  if (!_stateChangeBlocked && (_nextState != _currentState))
  {
    setState(_nextState);
  }
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
