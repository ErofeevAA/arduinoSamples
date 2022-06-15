#include <ezButton.h>
#include <Thread.h>
#include <ThreadController.h>

#define PRESS_TIME 500
#define COUNT_LEDS 4

static unsigned long pressedTime = 0;
static unsigned long releasedTime = 0;

static ezButton button(2);

static const int speed[3] = {1000, 500, 300};
static int speed_it = 0;

static int arrayPins[COUNT_LEDS] = {11, 10, 9, 6};

static ThreadController threadController = ThreadController();
static Thread ledsThreads[COUNT_LEDS];
static Thread secondModeThread;

void (*mode) ();
void (*modePress) ();

void blinkFirstLed() {
  static bool state = false;
  state = !state;
  digitalWrite(arrayPins[0], int (state) * HIGH);
}

void blinkSecondLed() {
  static bool state = false;
  state = !state;
  digitalWrite(arrayPins[1], int (state) * HIGH);
}

void blinkThirdLed() {
 static int state = 250;
 static int sign = -1;
 analogWrite(arrayPins[2], state);
 state += sign;
 if (state == 250 || state == 0) {
   sign = -sign;
 }
}

void blinkFourthLed() {
  static int state = 250;
  static int sign = -2;
  analogWrite(arrayPins[3], state);
  state += sign;
  if (state == 250 || state == 0) {
    sign = -sign;
  }
}

void secondThreadMode() {
  static int it = 0;
  digitalWrite(arrayPins[it], LOW);
  ++it;
  it = ((it == COUNT_LEDS) ? 0 : it);
  digitalWrite(arrayPins[it], HIGH);
}

void firstMode() {
  threadController.run();
}

void secondMode() {
  if (secondModeThread.shouldRun()) {
    secondModeThread.run();
  }
}

void firstPressMode() {
  int tmp = arrayPins[3];
  for(int i = COUNT_LEDS - 1; i >= 1; --i) {
    arrayPins[i] = arrayPins[i - 1];
  }
  arrayPins[0] = tmp;
}

void secondPressMode() {
  ++speed_it;
  speed_it = speed_it > 2 ? 0 : speed_it;
  secondModeThread.setInterval(speed[speed_it]);
}

void changeMode() {
  for(int i = 0; i < COUNT_LEDS; ++i) {
    digitalWrite(arrayPins[i], LOW);
  }
  if(mode == firstMode) {
    mode = secondMode;
    modePress = secondPressMode;
    return;
  }
  mode = firstMode;
  modePress = firstPressMode;
}

void setupFirstMode() {
  ledsThreads[0].onRun(blinkFirstLed);
  ledsThreads[0].setInterval(1000);
  ledsThreads[1].onRun(blinkSecondLed);
  ledsThreads[1].setInterval(500);
  ledsThreads[2].onRun(blinkThirdLed);
  ledsThreads[2].setInterval(4);
  ledsThreads[3].onRun(blinkFourthLed);
  ledsThreads[3].setInterval(4);
  for(int i = 0; i < COUNT_LEDS; ++i) {
    threadController.add(&(ledsThreads[i]));
  }
}

void setupLedPins() {
    for(int i = 0; i < COUNT_LEDS; ++i) {
      pinMode(arrayPins[i], OUTPUT);
    }
}

void setup() {
  button.setDebounceTime(50);
  setupLedPins();
  setupFirstMode();

  secondModeThread = Thread();
  secondModeThread.onRun(secondThreadMode);
  secondModeThread.setInterval(speed[0]);

  mode = firstMode;
  modePress = firstPressMode;
}

void loop() {
  button.loop();

  mode();

  if(button.isPressed())
    pressedTime = millis();

  if(button.isReleased()) {
    releasedTime = millis();

    long pressDuration = releasedTime - pressedTime;

    if( pressDuration < PRESS_TIME ) {
      modePress();
    }

    if( pressDuration >= PRESS_TIME ) {
      changeMode();
    }
  }
}
