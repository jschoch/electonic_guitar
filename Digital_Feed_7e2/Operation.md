On a tip in the rm found that there is no LCD and menu buttons on the diagram,

I will supplement the scheme later, but who needs it, it is googled by "LCD KEYPAD SHIELD".

 

Major changes in the 5th version:

* Significantly increased the ceiling of the spindle revolutions in the "Feed" mode.

* Appeared cross feed (with all the possibilities of the longitudinal).

* Acceleration / deceleration is implemented at large feed steps.

* A new mode "Cone" has appeared (for now only in one direction, we will see what happens next).

* In the "Thread" mode, it was possible to go to the right for the set right limit (for fitting the part) and return with the synchronization preserved.

* Changed the mode "Divider", more precisely implemented "division calculator."

 

Operating procedure

Modes are still switched with the "Select" button in a circle: Feed => Cone => Thread => Divider

 

Feed

In principle, nothing has changed for the user,

Use Up-Down buttons to change the feed pitch, you can change both on the move and with the joystick in neutral position,

4-D direction of the joystick,

4-D directions of fast movement on the joystick and the accelerated button,

4-limit buttons, limits are set only when the joystick is in the neutral position.

A small school was eliminated: if we stopped in the acceleration / deceleration zone and we need to get to the nearest limit, then in the new version we will crawl to the nearest limit at the lowest feed.

 

Cone

So far, only in one direction, the thin end of the spindle (we will see later).

Use Up-Down buttons to change the feed pitch, you can change on the go.

Use the Right-Left Buttons (with the joystick in a neutral position) select the cone.

You can set (right-left) or use previously set limits.

Front-back limits (if they were previously set) are ignored, but their position in memory is saved.

Cutting is turned on with the Joystick Right-Left.

By the joystick button, the accelerated movement works, the trajectory - the selected cone.

Joystick Forward-Back is not involved yet (let's see what we have to hang on it).

 

Thread

Use the Up-Down buttons to select the thread pitch, can be changed only when the joystick is in the neutral position,

4-D joystick threading directions, (yes, yes, self-indulgence, but cross-threading :))

4-limit buttons, limits are set only when the joystick is in the neutral position.

When pressing the joystick button, the right stop is ignored, you can arbitrarily go after it,

to restore synchronization, it is enough to get to the left abutment, we continue threading as usual.

 

Divider

Use the Up-Down buttons to set the number of division teeth (maximum 255) displayed in the first line of the indicator.

Right-Left buttons move to the next-previous "tooth", the current "tooth" is displayed in the second line of the indicator.

The first line of the indicator displays the required angle for the current “tooth”.

The second line of the indicator displays the actual spindle angle.

All that is required is to turn the spindle to match the readings.

(about a dozen times did the markup, it seemed quite convenient,

There is no reset of the angle, but you can hang it on the joystick button, so that with complex markings you will not lose the number of teeth when reset with reset).

 
Work with auto threading:

1. We grind the blank to the size according to GOST using any method that is familiar to you.

2. If you have not previously stood, set the right and left support to the desired thread length

3. In the thread mode, use the Up / Down buttons to select the required step.

4. Use the Right / Left Buttons to select "Outdoor" Ext, or "Internal" Int, (in the middle of the "Manual" Man)

5. Using the example of the right thread, direct spindle revolutions, cut to the chuck:

before starting the carriage should stand on the right stop,

switched the joystick to the left, the process went, the transverse stops were lit,

transverse stops went out - the process is completed.

6. After completing the cycle, the cross feed takes the starting position,

if the material is plasticine, then you can switch to the "Feed" mode and walk along the peaks,

for CT45 and D16T analogues this is not required.

+ Automatic multipass feed

For all of the following combinations:

Use the Up / Down buttons to set the amount of removal,

Right / Left buttons set the number of passes

 

1. external groove, direction to chuck ,

in the "Manual" mode, set the right + left limits,

move the instrument to the right limit,

switch "Sub Mode" to "Ext",

switch the joystick "Left".

 

2. external groove, direction from chuck

in the "Manual" mode, set the right + left limits,

move the instrument to the left limit,

switch "Sub Mode" to "Ext",

switch the joystick "Right".

 

3. internal boring, direction to chuck

in the "Manual" mode, set the right + left limits,

move the instrument to the right limit,

switch "Sub-mode" to "Int",

switch the joystick "Left".

 

4. internal boring, direction from chuck

in the "Manual" mode, set the right + left limits,

move the instrument to the left limit,

switch "Sub-mode" to "Int",

switch the joystick "Right".

 

5. facing, direction to spindle axis

In the "Manual" mode, set the front + rear limits,

move the instrument to the back limit

switch "Sub Mode" to "Ext",

switch the joystick forward.

 

6. facing, direction from spindle axis

In the "Manual" mode, set the front + rear limits,

move the instrument to the front limit,

switch "Sub Mode" to "Ext",

switch the joystick back.

 

Longitudinal turning, 3 passes, eat 0.2mm

https://youtu.be/Iebd0okzRyI

 

Face-up, 5 passes, eat 1.0mm

https://youtu.be/Qz68fPM3Iq4

 

Bonding to facing, 3 passes, offset 3.0mm, edge 1.5mm,

need to think and refine

https://youtu.be/cIbWgN8mywU

 

+ Rough Sphere

tool - detachable cutter with straight RK

Use the Up / Down buttons to set the diameter of the ball.

Use the Left and Right buttons to set the diameter of the under-cut leg,

with the Up / Down buttons pressed on the "Sel" button, set the width of the river. tool,

with the "Sel" button pressed with the Right / Left buttons, set the offset in Z, the step width.

 

Operating procedure:

1. machined disc in diameter ++, ("++" should be <REBOUND_X)

2. installed a cutting cutter on the right edge of the ingot machined 1-2-3mm with measurements in the required diameter, put back support,

3. torcanuli / dusted the workpiece with a return to the back stop, put the right stop,

4. switch the joystick to the left.


Updated to version 7c,

1. added asynchronous feed

. fixed a small jamb on the cross-feed (probably, no one would ever get out, nevertheless)

. fixed a small jamb on "Auto Thread"

. the jamb "on the accelerated feed variable shifter" was corrected (there was a motor disruption earlier)

. something else changed upon request
