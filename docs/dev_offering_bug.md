Playing Offering isn't quite working right. This is from a completed tree:

```
  |           |           |           |           |         |   +-Game(solved, obj=16, play Defend, turn=6, p=1.51e-08, hp=16/80, energy=2, hand={4 cards: Strike, 2xDefend, Offering}, mob0=(Gremlin Nob, 11hp, Rush, 3xStr, 1xVuln, 3xEnrage))
  |           |           |           |           |         |     +-Game(solved, obj=16, play Offering, turn=6, p=1.51e-08, hp=10/80, to_draw=3, hand={3 cards: Strike, 2xDefend}, mob0=(Gremlin Nob, 11hp, Rush, 6xStr, 1xVuln, 3xEnrage))
  |           |           |           |           |         |       +-Game(solved, obj=16, turn=6, p=1.51e-08, hp=10/80, to_draw=3, hand={2 cards: 2xDefend}, mob0=(Gremlin Nob, 2hp, Rush, 6xStr, 1xVuln, 3xEnrage))
  |           |           |           |           |         |         +-Game(solved, obj=16, turn=6, p=1.51e-08, hp=10/80, to_draw=2, hand={3 cards: 3xDefend}, mob0=(Gremlin Nob, 2hp, Rush, 6xStr, 1xVuln, 3xEnrage))
  |           |           |           |           |         |           +-Game(solved, obj=16, turn=6, p=1.51e-08, hp=10/80, to_draw=2, hand={3 cards: 3xDefend}, mob0=(Gremlin Nob, 2hp, Rush, 6xStr, 1xVuln, 3xEnrage))
  |           |           |           |           |         |             +-Game(solved, obj=16, turn=6, p=7.21e-09, hp=10/80, energy=3, hand={5 cards: 2xStrike, 3xDefend}, mob0=(Gremlin Nob, 2hp, Rush, 6xStr, 1xVuln, 3xEnrage))
  |           |           |           |           |         |             | +-Game(solved, obj=16, done, play Strike, turn=6, p=7.21e-09, hp=16/80, energy=2, hand={4 cards: Strike, 3xDefend})
  |           |           |           |           |         |             +-Game(solved, obj=16, turn=6, p=3.61e-09, hp=10/80, energy=3, hand={5 cards: Strike, 4xDefend}, mob0=(Gremlin Nob, 2hp, Rush, 6xStr, 1xVuln, 3xEnrage))
  |           |           |           |           |         |             | +-Game(solved, obj=16, done, play Strike, turn=6, p=3.61e-09, hp=16/80, energy=2, hand={4 cards: 4xDefend})
  |           |           |           |           |         |             +-Game(solved, obj=16, turn=6, p=3.61e-09, hp=10/80, energy=3, hand={5 cards: Strike, 3xDefend, Bash}, mob0=(Gremlin Nob, 2hp, Rush, 6xStr, 1xVuln, 3xEnrage))
  |           |           |           |           |         |             | +-Game(solved, obj=16, done, play Strike, turn=6, p=3.61e-09, hp=16/80, energy=2, hand={4 cards: 3xDefend, Bash})
  |           |           |           |           |         |             +-Game(solved, obj=16, turn=6, p=7.21e-10, hp=10/80, energy=3, hand={5 cards: 4xDefend, Bash}, mob0=(Gremlin Nob, 2hp, Rush, 6xStr, 1xVuln, 3xEnrage))
  |           |           |           |           |         |               +-Game(solved, obj=16, done, play Bash, turn=6, p=7.21e-10, hp=16/80, energy=1, hand={4 cards: 4xDefend})
```

It's possible the node after Offering is played is getting expanded within `FindPlayerChoices`.

