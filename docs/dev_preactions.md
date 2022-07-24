The current (6/6/2021) code uses flags to determine what to do next. For instance, generating mobs, generating intents, drawings cards, etc.

Consider adding a list of actions which would do this instead. If that list is empty, then the player must make a choice. Ending the turn would populate this list again.

This list could also be used to seamlessly implement drawing cards as a result of card actions.

Possible actions:
* Generate mobs (done once at start of battle)
* Generate mobs intents
* Draw X cards (for start of turn as well as for card actions)
* Discard X cards

I implemented this into the `pending_action` list.
