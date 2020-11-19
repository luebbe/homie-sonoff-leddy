# homie-sonoff-leddy

A simple homie based software for a Sonoff S20 that sets the four states of an aquarium led light.
If the light is turned off and on within five seconds, it steps to the next state.

States are:

- off
- sunny
- sunnyblue
- blue

Turning the light off can always be done. Setting the light to a particular color can
only be achieved by turning it off and on for a certain number of times. Since there
is no feedback, we try to maintain the state internally.

If the internal state gets out of sync with the physical light,
set the state to off for at least five seconds.
