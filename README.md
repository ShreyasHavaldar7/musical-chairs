HW Programming Assignment 2 - Musical Chairs
CS18BTECH11042 | CS18BTECH11047
Note: Please refer to the program comments for details.


List of files submitted:
* musicalchairs.cpp
* report.pdf
* readme.txt


Compilation:

Please compile the file matmul.c using flags pthread and lm as follows:

g++ musicalchairs.cpp -O2 -std=c++11 -pthread        


Running:

Run the ./a.out (or the executable file produced) providing the required arguments:

* np (No. of players playing the game)


Note: It is assumed that the no of laps in the game is one less than the number of players.

Sample Outputs:

* input.txt:

lap_start

music_start

music_stop

lap_stop

lap_start

music_start

music_stop

lap_stop

lap_start

music_start

music_stop

lap_stop


COMMAND:
./a.out --np 4 < input.txt

OUTPUT:
Musical Chairs: 4 player game with 3 laps.

======= lap# 1 =======

0 could not get chair

**********************
======= lap# 2 =======

3 could not get chair

**********************
======= lap# 3 =======

1 could not get chair

**********************

Winner is 2

Time taken for the game: 496 us

* input.txt:

lap_start

music_start

umpire_sleep 200

music_stop

lap_stop

lap_start

music_start

umpire_sleep 200000

music_stop

lap_stop

lap_start

music_start

umpire_sleep 800000

music_stop

lap_stop

COMMAND:

./a.out --np 4 < input.txt

OUTPUT:

Musical Chairs: 4 player game with 3 laps.

======= lap# 1 =======

0 could not get chair

**********************

======= lap# 2 =======

3 could not get chair

**********************

======= lap# 3 =======

1 could not get chair

**********************

Winner is 2

Time taken for the game: 1002809 us


* input.txt:
lap_start
player_sleep 0 1000
player_sleep 1 2000
player_sleep 2 3000
player_sleep 3 4000
music_start
umpire_sleep 200
music_stop
lap_stop
lap_start
player_sleep 0 1000
player_sleep 1 2000
player_sleep 2 3000
music_start
umpire_sleep 200000
music_stop
lap_stop
lap_start
player_sleep 0 1000
player_sleep 1 2000
music_start
umpire_sleep 800000
music_stop
lap_stop


COMMAND:
./a.out --np 4 < input.txt


OUTPUT:
Musical Chairs: 4 player game with 3 laps.
======= lap# 1 =======
3 could not get chair
**********************
======= lap# 2 =======
2 could not get chair
**********************
======= lap# 3 =======
1 could not get chair
**********************
Winner is 0
Time taken for the game: 1006189 us


* input.txt:
lap_start
music_start
music_stop
lap_stop


COMMAND:
./a.out --np 2 < input.txt


OUTPUT:
Musical Chairs: 2 player game with 1 laps.
======= lap# 1 =======
1 could not get chair
**********************
Winner is 0
Time taken for the game: 407 us


* input.txt:
lap_start
music_start
music_stop
lap_stop


COMMAND:
./a.out --np 1 < input.txt


OUTPUT:
Musical Chairs: 1 player game with 0 laps.
Winner is 0
Time taken for the game: 202 us


* input.txt:
lap_start
player_sleep 0 1000
player_sleep 1 2000
player_sleep 2 3000
music_start
umpire_sleep 400
music_stop
lap_stop
lap_start
player_sleep 0 1000
player_sleep 1 2000
music_start
umpire_sleep 300
music_stop
lap_stop


COMMAND:
./a.out --np 3 < input.txt


OUTPUT:
Musical Chairs: 3 player game with 2 laps.
======= lap# 1 =======
2 could not get chair
**********************
======= lap# 2 =======
1 could not get chair
**********************
Winner is 0
Time taken for the game: 6433 us


* input.txt:
lap_start
music_start
music_stop
lap_stop
lap_start
music_start
music_stop
lap_stop
lap_start
music_start
music_stop
lap_stop
lap_start
music_start
music_stop
lap_stop
lap_start
music_start
music_stop
lap_stop
lap_start
music_start
music_stop
lap_stop
lap_start
music_start
music_stop
lap_stop
lap_start
music_start
music_stop
lap_stop
lap_start
music_start
music_stop
lap_stop
lap_start
music_start
music_stop
lap_stop
lap_start
music_start
music_stop
lap_stop
lap_start
music_start
music_stop
lap_stop
lap_start
music_start
music_stop
lap_stop




COMMAND:
./a.out --np 14 < input.txt

OUTPUT:
Musical Chairs: 14 player game with 13 laps.
======= lap# 1 =======
11 could not get chair
**********************
======= lap# 2 =======
6 could not get chair
**********************
======= lap# 3 =======
7 could not get chair
**********************
======= lap# 4 =======
2 could not get chair
**********************
======= lap# 5 =======
8 could not get chair
**********************
======= lap# 6 =======
12 could not get chair
**********************
======= lap# 7 =======
0 could not get chair
**********************
======= lap# 8 =======
10 could not get chair
**********************
======= lap# 9 =======
9 could not get chair
**********************
======= lap# 10 =======
1 could not get chair
**********************
======= lap# 11 =======
3 could not get chair
**********************
======= lap# 12 =======
13 could not get chair
**********************
======= lap# 13 =======
5 could not get chair
**********************
Winner is 4
Time taken for the game: 1506 us

