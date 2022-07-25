The previous optimizations for handling collections of cards helped quite a bit, but I notice there is still a lot of time spend managing decks by adding/removing cards.

In the EndTurn node, almost all time is spent in `CardCollectionPtr` routines. We can probably improve this significantly.

Oops, I'm calling stuff when I don't need to (`burn_count = 0`):
![](.dev_deck_speed_improvement_images/removing_zero_burn_cards.png)

I can probably do this way more efficiently by avoid converting beween CardCollection and CardCollectionPtr except at the start and end of the function, and only if needed.
![](.dev_deck_speed_improvement_images/inefficient_card_calculations.png)

I did this, and the result was a 50% increase in speed. The standard Nob fight dropped from about 3.0 seconds to 1.5 seconds.

The new bottlenecks are FindPlayerChoices, EndTurn, DrawCards. Probably not too much we can do here, although DrawCards can be improved by using `CardCollectionPtr` instead of `CardCollection`.
![](.dev_deck_speed_improvement_images/new_slow_functions.png)

Reworked selection of cards to draw to get time down to about 1.3 sec.

## Cache

Maybe it makes sense to cache the results of drawing cards. We spend about 8% of our time here.

![](.dev_deck_speed_improvement_images/draw_card_profile_amount.png)

This would dramatically speed up. Cache would be built off of CardCollectionPtr and number of cards to draw.

Yep, this sped up the overall by 5%, and reduced the time we spend in `DrawCards` quite a bit.

![](.dev_deck_speed_improvement_images/draw_card_optimization_results.png)