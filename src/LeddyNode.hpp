
/*
 * LeddyNode.cpp
 * Homie Node for an aquarium light with three colors + off
 *
 * Author: Lübbe Onken (http://github.com/luebbe)
 */

#pragma once

#include <Homie.hpp>

// #define DEBUG

#define DEFAULTPIN -1

class LeddyNode : public HomieNode
{
public:
  enum STATE
  {
    OFF,
    SUNNY,
    SUNNYBLUE,
    BLUE
  };

private:
  const char *cCaption = "• Leddy:";
  const char *cIndent = "  ◦ ";
  const char *cOnTopic = "on";
  const char *cStateTopic = "state";

  Ticker _ticker;

  int _relayPin;
  int _ledPin;

  bool _stateChangeBlocked = false;
  STATE _currentState = STATE::OFF;
  STATE _nextState = STATE::OFF;
  int _ticks;

  String getStateName(STATE state);

  void printCaption();
  void sendState();

  void setupRelay();
  bool getRelay();
  void setRelay(bool on);
  void toggleRelay();

  void setLed(bool on);

  void tick(void);
  void unblockStateChange();

protected:
  virtual bool handleInput(const HomieRange &range, const String &property, const String &value) override;
  virtual void onReadyToOperate() override;
  virtual void setup() override;
  virtual void loop() override;

public:
  explicit LeddyNode(const char *name, const int relayPin = DEFAULTPIN, const int ledPin = DEFAULTPIN);
  void setState(STATE state);
  void step();
};
