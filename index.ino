/**
 * globals
 */
bool DEBUG = true;
enum state {
  NOT_IN_USE,
  ANALYSING,
  IN_USE_NUMBER1,
  IN_USE_NUMBER2,
  IN_USE_CLEANING,
  TRIGGERED_SPRAY_SHOT,
  MENU_ACTIVE
};
int current_state = state::NOT_IN_USE;
int RED_PIN = 11;
int GREEN_PIN = 10;
int BLUE_PIN = 9;

/**
 * functions
 */
void switchState(int new_state) {
  current_state = new_state;
  /** switch led color */
  switch(current_state) {
    case state::NOT_IN_USE: { set_rgb_led_color(255, 0, 0); break; }
    case state::ANALYSING: { set_rgb_led_color(0, 255, 0); break; }
    case state::IN_USE_NUMBER1: { set_rgb_led_color(0, 0, 255); break; }
    case state::IN_USE_NUMBER2: { set_rgb_led_color(255, 255, 0); break; }
    case state::IN_USE_CLEANING: { set_rgb_led_color(80, 0, 80); break; }
    case state::TRIGGERED_SPRAY_SHOT: { set_rgb_led_color(0, 255, 255); break; }
    case state::MENU_ACTIVE: { set_rgb_led_color(0, 20, 20); break; }
  }
  delay(1000);
}

/**
 * RGB LED
 */
void set_rgb_led_color(int red, int green, int blue)
{
  analogWrite(RED_PIN, red);
  analogWrite(GREEN_PIN, green);
  analogWrite(BLUE_PIN, blue);
}

/**
 * setup
 */
void setup()
{
  Serial.begin(9600);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
}

/**
 * loop
 */
void loop()
{
  switch(current_state) {
    /**
     * Not in use
     * @state-change to analysing
     */
    case state::NOT_IN_USE: 
    {
      // functions
      switchState(state::ANALYSING);
      break;
    }

    /**
     * Analysing
     * @state-change to number 1
     * @state-change to number 2
     * @state-change to cleaning
     */
    case state::ANALYSING:
    {
      // functions
      switchState(state::NOT_IN_USE);
      break;
    }

    /**
     * Number 1
     * @state-change to spray
     */
    case state::IN_USE_NUMBER1:
    {
      // functions
      break;
    }

    /**
     * Number 2
     * @state-change to spray
     */
    case state::IN_USE_NUMBER2:
    {
      // functions
      break;
    }

    /**
     * Cleaning
     * @state-change to not analysing
     */
    case state::IN_USE_CLEANING:
    {
      // functions
      break;
    }

    /**
     * Spray shot
     * @state-change to not in use
     */
    case state::TRIGGERED_SPRAY_SHOT:
    {
      // functions
      break;
    }

    /**
     * Menu active
     * @state-change to analysing
     * @state-change to not in use
     */
    case state::MENU_ACTIVE:
    {
      // functions
      break;
    }
  }
}