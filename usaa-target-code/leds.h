// -*- mode: C++;  tab-width: 2; c-basic-offset: 2;  -*-

#ifndef USAA_LEDS_H
#define USAA_LEDS_H

enum LedState {
  OFF,
  REGULAR,
  HIT,
  DISCONNECTED
};

void writeLEDs(LedState state, bool goSlow = false);


#endif