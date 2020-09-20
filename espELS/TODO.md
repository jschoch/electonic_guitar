make enum for modes
show which buttons are pressed top right on screen
define a default feed mode startup and a start button to turn feeding on
startup screen should allow up/down to select the mode (feed, thread);

mock up fake lathe


when feed mode works:
set initial conditions for limit to be 10mm from starting tool position.
try to figure out why delta lags but only by a bit




# OLD
1. need a counter on the encoder for position 0,  movement should only begin when enc position is at 0 to sync all movements to the same spindle possition.

2.  need to add limit buttons.

3.  constrain movement based on limits.

4.  add jog buttons like russian ELS.  feed/thread until limit.
