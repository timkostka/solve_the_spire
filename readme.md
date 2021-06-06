# Solving the Spire

How hard is it to choose the objectively optimal choice in a given battle within Slay the Spire?

For easy game states, there is often one optimal solution which leaves very little decision-making to be made. For example, if one can block all the incoming attack damage and still damage enemies, that is often (but not always) the best choice. Sometimes, there are states in which you can trade off life for extra damage. Can we create a program which explores these game states to come up with the optimal choice for a given situation? Further, can we find out before the battle has even started how much life we expect to lose after the battle is complete?

Also, how complex are the game trees for typical battles? Can we even store the entire game tree in memory on a typical computer? It is expected that fighting a Cultist with a starting deck is something we can expect to solve and that fighting the Heart is something we do not expect to solve due to the sheer number of possible game states. How complex of a battle can we fully solve?

Those are some questions we set out to answer.

Note that we are not simply designing an AI which attempts to make the best choice for the cards currently in hand. Rather, we are solving the complete set of possible game states in a given battle and finding the chain of decisions which optimize that fight.

# What is optimal play?

We define optimal play as making decisions which minimize the expected amount of life lost after the battle is complete. This definition is equivalent to making decisions which maximize the expected amount of HP left at the end of battle.

In mathematical terms, we define our objective function as the expected amount of HP left at the end of the battle and make decisions which maximize this objective.

## Example 1

For example, if we are in a spot where we can kill a mob without taking damage, that is the optimal decision (barring any sort of life gain options).

As another example, consider the following two options:

1. A play which results in a 100% chance of losing 2 life over the course of the battle.

2. A play which results in a 10% chance of losing 10 life, and 90% chance of losing 0 life.

The first example has an expected life loss of 2. The second example has an expected life loss of 0.1 * 10 = 1. Therefore, the second option is the right choice between these two possibilities.

## Example 2

Keep in mind that this definition may trade a higher chance of dying for the possibility of ending the battle at higher life. For example, consider the following:

1. A play which results in a 100% chance of ending the battle with 20 life.

2. A play which results in a 90% change of ending at 30 life, and a 10% chance to die (end battle at 0 life).

The second option has an expected end of battle hp of 0.9 * 30 = 27, which is better than the first option, even though there is an increased risk of dying.

If two decisions end up with the same expected life loss, we choose one of them arbitrarily.

## Side quests

For the moment, we do not consider side quests such as Hand of Greed, Ritual Dagger, Genetic Algorithm, or Lesson Learned.

Life gain side quests such as Bandage are considered.

# Game tree

To simulate a battle, we store game states within a tree structure with links between each node. To progress from one node to another, the game state is advanced by something like drawing cards, playing a card, ending the turn, or generating enemy intents. Broadly, these connections can be categorized into player decisions and automatic events.

An automatic event is something the player has no agency in, such as generating monster intents or drawing cards.

A player decision is a choice that the player actively makes, such as playing a card or ending the turn. These are the choices we are tasked with optimizing.

## Drawing cards

When drawing cards, there are often many combinations of cards that can be drawn. We must store each possible combination along with how probable it is. For a starting Ironclad deck (5xStrike, 4xDefend, 1xBash), these are the possible 5-card hands that may be drawn, along with the probability of each:

* (0.40%) 5xStrike
* (7.95%) 4xStrike, Defend
* (1.98%) 4xStrike, Bash
* (23.8%) 3xStrike, 2xDefend
* (15.9%) 3xStrike, Defend, Bash
* (15.9%) 2xStrike, 3xDefend
* (23.8%) 2xStrike, 2xDefend, Bash
* (1.98%) Strike, 4xDefend
* (7.95%) Strike, 3xDefend, Bash
* (0.40%) 4xDefend, Bash

# Random decisions

There are many things in Slay the Spire where randomness comes into play. When multiple scenarios are possible, we consider each to be equally likely. For example, when drawing a card from your deck, each individual card is equally likely to be drawn. When generating mob HPs at the start of the battle, each HP within the range is equally likely to be generated.

The exception to this is for generating enemy intents. These follow a known probability of choosing each intent and not all options may be equally likely.

Although the Slay the Spire game has game seeds which allow one to know the outcomes of these random choices for a given seed, we do not utilize this capability.
