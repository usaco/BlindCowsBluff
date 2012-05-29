(0) Call bot
* calls every time

(1) All-in bot
* goes all in every time

(2) Random bot
* given the scenario, will choose randomly between FOLD/CALL/WAGER (with weight given to the more conservative options)
* will never re-raise, and will never FOLD if no pending wager
* will bet a random amount between current wager and total pool

(3) Conservative bot
* given the number of active players, will determine a maximum valued "cutoff" card
  * if a card is bigger than that cutoff card, will FOLD (CALL if no pending wager)
  * if there is no card bigger than the cutoff card, will bet a small amount, or call a wager up to 2*(small amount)
