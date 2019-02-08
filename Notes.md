#  basic how it works

several multi-select switches control state which drives the controller.  

the joystick is used to tell the controller to engage with the work.  The joystick is modal, operation os based on the current mode.

It appears that a manual mode exists for operation with a single stepper.  This is "manual" mode which drives the "Z" stepper motor based on the encoder position.

In "manual threading mode" the x motor will attempt to be in the "right" position based on the spindle encoder position

in "ext threading mode" it appear that the X is also driven until a depth is achieved.  Left/right movement is driven by some limit parameters.  


## It appears you need to run the spreadsheet to calculate the tables used to calculate drive position

`
Do you need to drive all the threads and cone yourself? Or am I wrong?

In all versions of the thread and the cone it is necessary to calculate and drive yourself,

a calculator that is in the archive with firmware,

A calculator that is laid out separately - under the old versions.
`

#  bare minimum to get something working

An encoder is required to determine spindle position

one stepper motor is required to drive the Z axis.

A "joystick" must be in place to tell the controller to feed.

you need to have one mode and one sub-mode pin pulled low

you can get away without a LCD or using the menu mode if you select the right state in the firmware defaults.

it needs left and right limit buttons.  these are actual momentary switches to set the limit to the currently jogged position.  they are not limit switches in the sense that they can overcome position errors. 

the limit buttons needed pullups.

# unknown

is there a homing routine?

it is not clear how the "tachometer" works, it appears to be an external display







