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
  SPRAYING,
  MENU_ACTIVE
};

int previous_state = state::NOT_IN_USE;
int current_state = state::NOT_IN_USE;
int RED_PIN = 11;
int GREEN_PIN = 10;
int BLUE_PIN = 9;
int DOOR_PIN = 8;
int BUTTON_SPRAY = 4;
int BUTTON_ANALYSE = 3;
int BUTTON_RESET = 2;

int PREVIOUS_ACTION = 0;
int CURRENT_ACTION = millis();

/**
 * Sensors  
 * TODO: move into own files (.hpp and .cpp)
 */
class Sensors {
  public:
  /**
   * Hardware sensors
   */
  static bool is_door_closed() {
    return !(bool)digitalRead(DOOR_PIN);
  }
  static bool is_door_changed() {
    return true;
  }
  static bool is_light_on() {
    return true;
  }
  static bool is_person_on_toilet() {
    return true;
  }
  static bool is_button_pressed(int button) {
    return !(bool)digitalRead(button);
  }

  /**
   * Timing sensors
   */
  static bool is_time_passed(int interval_time) {
    CURRENT_ACTION = millis();
    if (CURRENT_ACTION - PREVIOUS_ACTION >= interval_time) {
      return true;
    }
    return false;
  }
  static void reset_time_passed() {
    PREVIOUS_ACTION = CURRENT_ACTION;
  }
};

/**
 * functions
 */
void switch_status(int new_state) {
  previous_state = current_state;
  current_state = new_state;
  /** switch led color */
  switch(current_state) {
    case state::NOT_IN_USE: { set_rgb_led_color(255, 0, 0); Serial.println("status switched to: NOT_IN_USE."); break; }
    case state::ANALYSING: { set_rgb_led_color(0, 255, 0); Serial.println("status switched to: ANALYSING."); break; }
    case state::IN_USE_NUMBER1: { set_rgb_led_color(0, 0, 255); Serial.println("status switched to: IN_USE_NUMBER1."); break; }
    case state::IN_USE_NUMBER2: { set_rgb_led_color(255, 255, 0); Serial.println("status switched to: IN_USE_NUMBER2."); break; }
    case state::IN_USE_CLEANING: { set_rgb_led_color(80, 0, 80); Serial.println("status switched to: IN_USE_CLEANING."); break; }
    case state::SPRAYING: { set_rgb_led_color(0, 255, 255); Serial.println("status switched to: SPRAYING."); break; }
    case state::MENU_ACTIVE: { set_rgb_led_color(0, 20, 20); Serial.println("status switched to: MENU_ACTIVE."); break; }
  }
  Sensors::reset_time_passed();
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
  pinMode(DOOR_PIN, INPUT_PULLUP);
  pinMode(BUTTON_SPRAY, INPUT_PULLUP);
  pinMode(BUTTON_ANALYSE, INPUT_PULLUP);
  pinMode(BUTTON_RESET, INPUT_PULLUP);
}

/**
 * loop
 */
void loop()
{
  /**
   * Buttons
   */
  if (Sensors::is_button_pressed(BUTTON_SPRAY)) { switch_status(state::SPRAYING); }
  if (Sensors::is_button_pressed(BUTTON_ANALYSE)) { switch_status(state::ANALYSING); }
  if (Sensors::is_button_pressed(BUTTON_RESET)) { switch_status(state::NOT_IN_USE); }

  /**
   * States
   */
  switch(current_state) {
    /**
     * Not in use
     * @state-change to analysing
     */
    case state::NOT_IN_USE: 
    {
      // functions
      if ( Sensors::is_time_passed(1000) ) { switch_status(state::ANALYSING); }
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
      // if ( Sensors::is_time_passed(1000) ) { switch_status(state::NOT_IN_USE); }

      /**
       * Number one requires:
       * - door to be shut
       * - light to be on
       * - is person on toilet
       */
      if (
        Sensors::is_door_closed() &&
        Sensors::is_light_on() &&
        Sensors::is_person_on_toilet()
      ) {
        switch_status(state::IN_USE_NUMBER1);
      }

      break;
    }

    /**
     * Number 1
     * @state-change to spray
     */
    case state::IN_USE_NUMBER1:
    {
      if (
        Sensors::is_time_passed(5000) &&
        Sensors::is_door_changed()
      ) {
        switch_status(state::SPRAYING);
      }
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
    case state::SPRAYING:
    {
      bool sprayed = false;
      bool led_changed = false;

      /**
       *  First spray
       *  @when - button press
       *  @when - previous state is number 1
       *  @when - previous state is number 2
       */
      if ( Sensors::is_time_passed(2500)  && !sprayed) { 
        if (!led_changed) { set_rgb_led_color(255, 0, 0); }
        led_changed = true;
        sprayed = true;
      }
      if ( Sensors::is_time_passed(2600)  && sprayed) { 
        if (led_changed) { set_rgb_led_color(0, 255, 255); }
        led_changed = false;
        if (previous_state != state::IN_USE_NUMBER2) {
          Sensors::reset_time_passed();
          switch_status(state::ANALYSING);
          break;
        }
      }

      /**
       *  Second spray
       *  @when - previous state is number 2
       */

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