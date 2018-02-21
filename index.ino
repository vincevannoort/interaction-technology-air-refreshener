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
int initial_state = state.NOT_IN_USE;
int current_state = initial_state;

/**
 * setup
 */
void setup()
{

}

/**
 * loop
 */
void loop()
{
  switch(current_state) {
    /**
     * Not in use
     */
    case state.NOT_IN_USE: {
      // functions
      break;
    }

    /**
     * Analysing
     */
    case state.ANALYSING:
    {
      // functions
      break;
    }

    /**
     * Number 1
     */
    case state.IN_USE_NUMBER1:
    {
      // functions
      break;
    }

    /**
     * Number 2
     */
    case state.IN_USE_NUMBER2:
    {
      // functions
      break;
    }

    /**
     * Cleaning
     */
    case state.IN_USE_CLEANING:
    {
      // functions
      break;
    }

    /**
     * Spray shot
     */
    case state.TRIGGERED_SPRAY_SHOT:
    {
      // functions
      break;
    }

    /**
     * Menu active
     */
    case state.MENU_ACTIVE:
    {
      // functions
      break;
    }
  }
}