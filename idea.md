1. 1 semaphore for stopping all the player threads from going into their main function.
2. While a player is occupying a chair, a lock is acquired.
3. n chair locks.
4. Condition variable on starting music.
5. 
