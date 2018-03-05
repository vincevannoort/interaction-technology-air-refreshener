#include <EEPROM.h>
#include <OneWire.h>
#include <LiquidCrystal.h>
#include <DallasTemperature.h>

bool DEBUG = true;

/**
 * State
 */
enum State {
  SETUP,
  NOT_IN_USE,
  IN_USE_NUMBER1,
  IN_USE_NUMBER2,
  IN_USE_CLEANING,
  SPRAYING,
  MENU_ACTIVE
};

enum Pins {
  RED_PIN = 11,
  GREEN_PIN = 10,
  BLUE_PIN = 9,
  DOOR_PIN = 7,
  LIGHT_PIN = 0,
  MOTION_PIN = 8,
  TEMPERATURE_BUS = 12,
  BUTTON_PREVIOUS = 4,
  BUTTON_NEXT = 3,
  BUTTON_MENU = 2,
  LCD_RS = 5,
  LCD_EN = 6,
  LCD_D4 = A5,
  LCD_D5 = A4,
  LCD_D6 = A3,
  LCD_D7 = A2
};

enum Menu {
  START,
  SPRAYS,
  DELAY,
  RESET,
  END
};

OneWire one_wire_bus(TEMPERATURE_BUS);
DallasTemperature temperature_sensor(&one_wire_bus);
int lcd_update_count = 0;
LiquidCrystal lcd(Pins::LCD_RS, Pins::LCD_EN, Pins::LCD_D4, Pins::LCD_D5, Pins::LCD_D6, Pins::LCD_D7);

enum Variables {
  MAX_TIME_NUMBER1 = 30000,
  TIME_BEFORE_MEASURING_AFTER_DOOR_CHANGE = 7500,
  LIGHT_THRESHOLD = 970,
  INITIAL_EXTRA_SPRAY_DELAY = 500,
  INITIAL_NUMBER_OF_SPRAYS = 2400
};

int previous_state = State::SETUP;
int current_state = State::SETUP;
int current_menu_state = Menu::START;

/**
 * EEPROM ADDRESSES
 */
// addresses
int eea_sprays = 0;
int eea_delay = 0;
// data
int dee_sprays = 0;
int dee_delay = 0;

int DOOR_STATUS = 0;
bool DOOR_STATUS_CHANGED = false;
int PREVIOUS_ACTION = 0;
int CURRENT_ACTION = millis();

/**
 * Sensors  
 */
class Sensors {
  public:
  /**
   * Status RGB led
   */
  static void set_rgb_led_color(int red, int green, int blue) {
    analogWrite(Pins::RED_PIN, red);
    analogWrite(Pins::GREEN_PIN, green);
    analogWrite(Pins::BLUE_PIN, blue);
  }

  /**
   * Magnetic door sensor
   * DOOR_STATUS -> true = CLOSED
   * DOOR_STATUS -> false = OPEN
   */
  static bool get_door_status() {
    int previous_door_status = DOOR_STATUS;
    int current_door_status = !(bool)digitalRead(Pins::DOOR_PIN);
    if (previous_door_status != current_door_status) { DOOR_STATUS_CHANGED = true; }
    return DOOR_STATUS = !(bool)digitalRead(Pins::DOOR_PIN);
  }
  static bool is_door_closed() {
    return Sensors::get_door_status();
  }
  static bool is_door_open() {
    return !Sensors::is_door_closed();
  }
  static int is_time_passed_since_door_change(int interval_time) {
    return Sensors::is_time_passed(interval_time);
  }
  static bool is_door_changed() {
    Sensors::get_door_status();
    return DOOR_STATUS_CHANGED;
  }
  static void reset_door_status() {
    DOOR_STATUS_CHANGED = false;
  }

  /**
   * Light sensor
   */
  static const int light_on_threshold = Variables::LIGHT_THRESHOLD;
  static int get_light_value() {
    return analogRead(Pins::LIGHT_PIN);
  }
  static bool is_light_on() {
    return analogRead(Pins::LIGHT_PIN) > Sensors::light_on_threshold;
  }
  static bool is_light_off() {
    return !Sensors::is_light_on();
  }

  /**
   * Motion sensor
   */
  static bool is_movement_detected() {
    return digitalRead(Pins::MOTION_PIN);
  }
  static bool is_no_movement_detected() {
    return !Sensors::is_movement_detected();
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
   * Temperature sensors
   */
  static float get_temperature() {
    temperature_sensor.requestTemperatures();
    return temperature_sensor.getTempCByIndex(0);
  }

  /**
   * Display
   */
  static void display_temperature_and_shots() {
    if (lcd_update_count++ % 2000 == 0) {
      // temperature
      lcd.setCursor(1, 0);
      lcd.print(" Temp: ");
      // lcd.print(Sensors::get_temperature(), DEC);
      lcd.setCursor(12, 0);
      lcd.print((char)223);
      lcd.setCursor(13, 0);
      lcd.print("C   ");

      // shots
      lcd.setCursor(1, 1);
      lcd.print(" Shots: ");
      lcd.print(Sensors::amount_of_air_refreshener_sprays_left);
    }
  }

  static void display_menu() {
    lcd.setCursor(1, 0);
    switch(current_menu_state) {
      case Menu::START: {
        lcd.print("Menu: Start");
        break;
      }
      case Menu::SPRAYS: {
        lcd.print("Menu: Sprays");
        break;
      }
      case Menu::DELAY: {
        lcd.print("Menu: Delay");
        break;
      }
      case Menu::RESET: {
        lcd.print("Menu: Reset");
        break;
      }
    }
  }

  static void next_menu() {
    current_menu_state = ++current_menu_state % 5;
    lcd.clear();
    delay(500);
  }

  /**
   * Air refreshener
   */
  static int amount_of_air_refreshener_sprays_left;
  static int delay_before_spraying;
  static void spray_air_refreshener() {
    Sensors::amount_of_air_refreshener_sprays_left--;
    EEPROM.put(eea_sprays, Sensors::amount_of_air_refreshener_sprays_left);
    Sensors::set_rgb_led_color(255, 0, 0);
    delay(500);
    Sensors::set_rgb_led_color(255, 255, 255);
    delay(500);
  }

  /**
   * Status
   */
  static void switch_status(int new_state) {
    /** prevent double switching */
    if (current_state == new_state) { return; }

    previous_state = current_state;
    current_state = new_state;
    /** switch led color */
    switch(current_state) {
      case State::NOT_IN_USE: { Sensors::set_rgb_led_color(0, 255, 0); Serial.println("status switched to: NOT_IN_USE."); break; }
      case State::IN_USE_NUMBER1: { Sensors::set_rgb_led_color(0, 0, 255); Serial.println("status switched to: IN_USE_NUMBER1."); break; }
      case State::IN_USE_NUMBER2: { Sensors::set_rgb_led_color(255, 0, 255); Serial.println("status switched to: IN_USE_NUMBER2."); break; }
      case State::IN_USE_CLEANING: { Sensors::set_rgb_led_color(255, 255, 0); Serial.println("status switched to: IN_USE_CLEANING."); break; }
      case State::SPRAYING: { Sensors::set_rgb_led_color(255, 255, 255); Serial.println("status switched to: SPRAYING."); break; }
      case State::MENU_ACTIVE: { Sensors::set_rgb_led_color(0, 0, 0); Serial.println("status switched to: MENU_ACTIVE."); break; }
    }
    Sensors::reset_time_passed();
  }

  /**
   * Debug information
   */
  static void print_debug_information_to_serial() {
    Serial.println("DEBUG INFORMATION:");
    Serial.print("LIGHT value: "); Serial.print(Sensors::get_light_value()); Serial.print(" threshold: "); Serial.print(Sensors::light_on_threshold); Serial.println();
    Serial.print("DOOR value: "); Serial.print(Sensors::get_door_status()); Serial.println();
    Serial.print("MOTION value: "); Serial.print(Sensors::is_movement_detected()); Serial.println();
    Serial.print("TEMPERATURE value: "); Serial.print(Sensors::get_temperature()); Serial.println();
    Serial.println();
  }

};
// setup amount of sprays left
int Sensors::amount_of_air_refreshener_sprays_left = Variables::INITIAL_NUMBER_OF_SPRAYS;
int Sensors::delay_before_spraying = Variables::INITIAL_EXTRA_SPRAY_DELAY;

/**
 * setup
 */
void setup()
{
  // setup leds and serial
  Serial.begin(9600);
  pinMode(Pins::RED_PIN, OUTPUT);
  pinMode(Pins::GREEN_PIN, OUTPUT);
  pinMode(Pins::BLUE_PIN, OUTPUT);
  pinMode(Pins::DOOR_PIN, INPUT_PULLUP);
  pinMode(Pins::LIGHT_PIN, INPUT);
  pinMode(Pins::MOTION_PIN, INPUT);
  pinMode(Pins::BUTTON_PREVIOUS, INPUT_PULLUP);
  pinMode(Pins::BUTTON_NEXT, INPUT_PULLUP);
  pinMode(Pins::BUTTON_MENU, INPUT_PULLUP);
  temperature_sensor.begin();
  lcd.begin(16, 2);

  // get values from EEPROM
  EEPROM.get(eea_sprays, dee_sprays);
  EEPROM.get(eea_delay, dee_delay);
  Sensors::amount_of_air_refreshener_sprays_left = dee_sprays;
  Sensors::delay_before_spraying = dee_delay;

  // switch to begin status
  Sensors::switch_status(State::NOT_IN_USE);
}

/**
 * loop
 */
void loop()
{
  /**
   * Buttons
   */
  if (current_state != State::MENU_ACTIVE) {
    if (Sensors::is_button_pressed(Pins::BUTTON_PREVIOUS)) { Sensors::switch_status(State::SPRAYING); }
    if (Sensors::is_button_pressed(Pins::BUTTON_NEXT)) { Sensors::switch_status(State::NOT_IN_USE); }
    if (Sensors::is_button_pressed(Pins::BUTTON_MENU)) { Sensors::switch_status(State::MENU_ACTIVE); }
    Sensors::display_temperature_and_shots();
    // Sensors::print_debug_information_to_serial();
  }

  /**
   * States
   */
  switch(current_state) {

    /**
     * NOT_IN_USE
     * @state-change to number 1
     * @state-change to number 2
     * @state-change to cleaning
     */
    case State::NOT_IN_USE:
    {
      /**
       * Reset time when door changed
       */
      if ( Sensors::is_door_changed() ) { 
        Sensors::reset_time_passed(); 
        Sensors::reset_door_status(); 
      }

      /**
       * Number one requires:
       * - door to be shut
       * - light to be on
       * - is movement detected
       */
      if (
        Sensors::is_door_closed() &&
        Sensors::is_light_on() &&
        Sensors::is_movement_detected() &&
        Sensors::is_time_passed_since_door_change(Variables::TIME_BEFORE_MEASURING_AFTER_DOOR_CHANGE)
      ) {
        Sensors::reset_door_status();
        Sensors::switch_status(State::IN_USE_NUMBER1);
      }

      /**
       * Cleaning requires:
       * - door to be open
       * - light to be on
       * - is movement detected
       */
      if (
        Sensors::is_door_open() &&
        Sensors::is_light_on() &&
        Sensors::is_movement_detected() && 
        Sensors::is_time_passed_since_door_change(Variables::TIME_BEFORE_MEASURING_AFTER_DOOR_CHANGE)
      ) {
        Sensors::reset_door_status();
        Sensors::switch_status(State::IN_USE_CLEANING);
      }

      break;
    }

    /**
     * Number 1
     * @state-change to spray
     */
    case State::IN_USE_NUMBER1:
    {

      /**
       * check if done
       */
      if (
        Sensors::is_door_changed() &&
        Sensors::is_light_off()
      ) {
        Sensors::reset_door_status();
        Sensors::switch_status(State::SPRAYING);
      }

      /**
       * if it takes longer, it's probably a number 2
       */
      if (
        Sensors::is_time_passed(Variables::MAX_TIME_NUMBER1)
      ) {
        Sensors::switch_status(State::IN_USE_NUMBER2);
      }

      break;
    }

    /**
     * Number 2
     * @state-change to spray
     */
    case State::IN_USE_NUMBER2:
    {
      /**
       * check if done
       */
      if (
        Sensors::is_door_changed() &&
        Sensors::is_light_off()
      ) {
        Sensors::reset_door_status();
        Sensors::switch_status(State::SPRAYING);
      }
      break;
    }

    /**
     * Cleaning
     * @state-change to not NOT_IN_USE
     */
    case State::IN_USE_CLEANING:
    {
      /**
       * check if done
       */
      if (
        Sensors::is_door_changed() &&
        Sensors::is_light_off()
      ) {
        Sensors::reset_door_status();
        Sensors::switch_status(State::NOT_IN_USE);
      }
      break;
    }

    /**
     * Spray shot
     * @state-change to not in use
     */
    case State::SPRAYING:
    {
      /**
       * show that in spraying state
       * using delay justified since, it shouldnt be able to exit the spraying state without finishing, since that could leave the motor on
       */
      delay(Sensors::delay_before_spraying);

      /**
       *  Spray once
       *  @when - TODO: button press
       *  @when - previous state is number 1
       */
      if (previous_state == State::IN_USE_NUMBER1 || previous_state == State::NOT_IN_USE || previous_state == State::NOT_IN_USE) {
        Sensors::spray_air_refreshener();
        Sensors::switch_status(State::NOT_IN_USE);
        break;
      }

      /**
       *  Second spray
       *  @when - previous state is number 2
       */

      if (previous_state == State::IN_USE_NUMBER2) {
        Sensors::spray_air_refreshener();
        Sensors::spray_air_refreshener();
        Sensors::switch_status(State::NOT_IN_USE);
      }
      break;
    }

    /**
     * Menu active
     * @state-change to NOT_IN_USE
     * @state-change to not in use
     */
    case State::MENU_ACTIVE:
    {
      // display menu
      Sensors::display_menu();

      // previous
      // if (Sensors::is_button_pressed(Pins::BUTTON_NEXT)) { Sensors::switch_status(State::NOT_IN_USE); }

      // next
      // if (Sensors::is_button_pressed(Pins::BUTTON_NEXT)) { Sensors::switch_status(State::NOT_IN_USE); }
      
      switch(current_menu_state) {
        case Menu::START: {
          break;
        }
        case Menu::SPRAYS: {
          break;
        }
        case Menu::DELAY: {
          break;
        }
        case Menu::RESET: {
          if (Sensors::is_button_pressed(Pins::BUTTON_PREVIOUS) && Sensors::is_button_pressed(Pins::BUTTON_NEXT)) {
            EEPROM.put(eea_delay, Variables::INITIAL_EXTRA_SPRAY_DELAY);
            EEPROM.put(eea_sprays, Variables::INITIAL_NUMBER_OF_SPRAYS);
            Sensors::amount_of_air_refreshener_sprays_left = Variables::INITIAL_NUMBER_OF_SPRAYS;
            Sensors::delay_before_spraying = Variables::INITIAL_EXTRA_SPRAY_DELAY;
            Sensors::switch_status(State::NOT_IN_USE);
            delay(1000);
          }
          break;
        }
      }

      // back to menu
      if (Sensors::is_button_pressed(Pins::BUTTON_MENU)) {
        Sensors::next_menu();
        if (current_menu_state == Menu::END) {
          Sensors::switch_status(State::NOT_IN_USE);
        }
      }
      break;
    }

  }
}