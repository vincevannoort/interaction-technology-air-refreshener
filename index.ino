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
     * @state-change to analysing
     */
    case state.NOT_IN_USE: {
      // functions
      break;
    }

    /**
     * Analysing
     * @state-change to number 1
     * @state-change to number 2
     * @state-change to cleaning
     */
    case state.ANALYSING:
    {
      // functions
      break;
    }

    /**
     * Number 1
     * @state-change to spray
     */
    case state.IN_USE_NUMBER1:
    {
      // functions
      break;
    }

    /**
     * Number 2
     * @state-change to spray
     */
    case state.IN_USE_NUMBER2:
    {
      // functions
      break;
    }

    /**
     * Cleaning
     * @state-change to not analysing
     */
    case state.IN_USE_CLEANING:
    {
      // functions
      break;
    }

    /**
     * Spray shot
     * @state-change to not in use
     */
    case state.TRIGGERED_SPRAY_SHOT:
    {
      // functions
      break;
    }

    /**
     * Menu active
     * @state-change to analysing
     * @state-change to not in use
     */
    case state.MENU_ACTIVE:
    {
      // functions
      break;
    }
  }
}