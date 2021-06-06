# What is implemented?

This does not implement the full possibility of game states primarily due to the computational complexity it would cause.

Implementing Frozen Eye would mean we need to track the order of cards in the draw pile. While this is a simple task, it would increse the number of game states beyond the resources we have to store information.


## Relics

Most relics which do not have state which persists between battles are implemented.

Relics which have a state which persists between battles, such as Pen Nib and Ink Bottle, are not implemented. This may be revisited in the future.
