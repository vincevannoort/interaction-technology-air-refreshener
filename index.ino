/**
 * globals
 */
bool DEBUG = true;
enum state {
  SETUP,
  NOT_IN_USE,
  ANALYSING,
  IN_USE_NUMBER1,
  IN_USE_NUMBER2,
  IN_USE_CLEANING,
  SPRAYING,
  MENU_ACTIVE
};

int previous_state = state::SETUP;
int current_state = state::NOT_IN_USE;
int RED_PIN = 11;
int GREEN_PIN = 10;
int BLUE_PIN = 9;
int DOOR_PIN = 8;
int LIGHT_PIN = 0;
int MOTION_PIN = 7;
int BUTTON_SPRAY = 4;
int BUTTON_ANALYSE = 3;
int BUTTON_MENU = 2;

int DOOR_STATUS = 0;
bool DOOR_STATUS_CHANGED = false;
int PREVIOUS_ACTION = 0;
int CURRENT_ACTION = millis();
int FADE_STEPS = 50;

/**
 * Sensors  
 * TODO: move into own files (.hpp and .cpp)
 */
class Sensors {
  public:
  /**
   * Status RGB led
   */
  static void set_rgb_led_color(int red, int green, int blue) {
    analogWrite(RED_PIN, red);
    analogWrite(GREEN_PIN, green);
    analogWrite(BLUE_PIN, blue);
  }

  /**
   * Magnetic door sensor
   */
  static bool is_door_closed() {
    return DOOR_STATUS = !(bool)digitalRead(DOOR_PIN);
  }
  static bool is_door_open() {
    return !Sensors::is_door_closed();
  }
  static bool is_door_changed() {
    int previous_door_status = DOOR_STATUS;
    int current_door_status = Sensors::is_door_closed();
    if (previous_door_status != current_door_status) { DOOR_STATUS_CHANGED = true; }
    return DOOR_STATUS_CHANGED;
  }
  static void reset_door_status() {
    DOOR_STATUS_CHANGED = false;
  }

  /**
   * Light sensor
   */
  static const int light_on_threshold = 1000;
  static bool is_light_on() {
    return analogRead(LIGHT_PIN) > Sensors::light_on_threshold;
  }
  static bool is_light_off() {
    return !Sensors::is_light_on();
  }

  /**
   * Motion sensor
   */
  static bool is_person_on_toilet() {
    return true;
    // return digitalRead(MOTION_PIN);
  }
  static bool is_person_not_on_toilet() {
    return !Sensors::is_person_on_toilet();
  }

  /**
   * Buttons
   */
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
    PREVIOUS_ACTION = millis();
  }

  /**
   * Air refreshener
   */
  static int amount_of_air_refreshener_sprays_left;
  static void spray_air_refreshener() {
    Sensors::amount_of_air_refreshener_sprays_left--;
    Sensors::set_rgb_led_color(255, 0, 0);
    delay(500);
    Sensors::set_rgb_led_color(255, 255, 255);
    delay(500);
  }
};
// setup amount of sprays left
int Sensors::amount_of_air_refreshener_sprays_left = 2400;

/**
 * functions
 */
void switch_status(int new_state) {
  /** prevent double switching */
  if (current_state == new_state) { return; }

  previous_state = current_state;
  current_state = new_state;
  /** switch led color */
  switch(current_state) {
    case state::NOT_IN_USE: { Sensors::set_rgb_led_color(255, 0, 0); Serial.println("status switched to: NOT_IN_USE."); Sensors::reset_time_passed(); break; }
    case state::ANALYSING: { Sensors::set_rgb_led_color(0, 255, 0); Serial.println("status switched to: ANALYSING."); Sensors::reset_time_passed(); break; }
    case state::IN_USE_NUMBER1: { Sensors::set_rgb_led_color(0, 0, 255); Serial.println("status switched to: IN_USE_NUMBER1."); Sensors::reset_time_passed(); break; }
    case state::IN_USE_NUMBER2: { Sensors::set_rgb_led_color(255, 0, 255); Serial.println("status switched to: IN_USE_NUMBER2."); Sensors::reset_time_passed(); break; }
    case state::IN_USE_CLEANING: { Sensors::set_rgb_led_color(255, 255, 255); Serial.println("status switched to: IN_USE_CLEANING."); Sensors::reset_time_passed(); break; }
    case state::SPRAYING: { Sensors::set_rgb_led_color(255, 255, 255); Serial.println("status switched to: SPRAYING."); Sensors::reset_time_passed(); break; }
    case state::MENU_ACTIVE: { Sensors::set_rgb_led_color(0, 0, 0); Serial.println("status switched to: MENU_ACTIVE."); Sensors::reset_time_passed(); break; }
  }
}

/**
 * setup
 */
void setup()
{
  // setup leds and serial
  Serial.begin(9600);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(DOOR_PIN, INPUT_PULLUP);
  pinMode(LIGHT_PIN, INPUT);
  pinMode(MOTION_PIN, INPUT);
  pinMode(BUTTON_SPRAY, INPUT_PULLUP);
  pinMode(BUTTON_ANALYSE, INPUT_PULLUP);
  pinMode(BUTTON_MENU, INPUT_PULLUP);
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
  if (Sensors::is_button_pressed(BUTTON_MENU)) { switch_status(state::MENU_ACTIVE); }

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
      if ( Sensors::is_time_passed(3500) ) { switch_status(state::ANALYSING); }
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

      /**
       * check if done
       */
      if (
        Sensors::is_time_passed(2500) &&
        Sensors::is_door_changed() &&
        Sensors::is_light_off()
      ) {
        Sensors::reset_door_status();
        switch_status(state::SPRAYING);
      }

      /**
       * if it takes longer, it's probably a number 2
       */
      if (
        Sensors::is_time_passed(5000)
      ) {
        switch_status(state::IN_USE_NUMBER2);
      }

      break;
    }

    /**
     * Number 2
     * @state-change to spray
     */
    case state::IN_USE_NUMBER2:
    {
      /**
       * check if done
       */
      if (
        Sensors::is_door_changed() &&
        Sensors::is_light_off()
      ) {
        Sensors::reset_door_status();
        switch_status(state::SPRAYING);
      }
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
      // show that in spraying state
      delay(500);

      /**
       *  Spray once
       *  @when - TODO: button press
       *  @when - previous state is number 1
       */
      if (previous_state == state::IN_USE_NUMBER1 || previous_state == state::ANALYSING) {
        Serial.println("SPRAYED ONCE!");
        Sensors::spray_air_refreshener();
        switch_status(state::ANALYSING);
        break;
      }

      /**
       *  Second spray
       *  @when - previous state is number 2
       */

      if (previous_state == state::IN_USE_NUMBER2) {
        // TODO: SPRAYING TWICE
        Serial.println("SPRAYED TWICE!");
        Sensors::spray_air_refreshener();
        Sensors::spray_air_refreshener();
        switch_status(state::ANALYSING);
      }
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