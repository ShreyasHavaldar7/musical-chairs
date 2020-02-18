/*
 * Program: Musical chairs game with n players and m intervals.
 * Authors: Vedant Singh, Shreyas Jayant Havaldar 
 * Roll# : CS18BTECH11047, CS18BTECH11042
 */

#include <stdlib.h>  /* for exit, atoi */
#include <iostream>  /* for fprintf */
#include <errno.h>   /* for error code eg. E2BIG */
#include <getopt.h>  /* for getopt */
#include <assert.h>  /* for assert */
#include <chrono>	/* for timers */
#include <mutex>
using namespace std;

int nplayers;
int dead = 0;
int player_count=0;
int num_chairs;

mutex music_start, music_end, player;

mutex *chair;

void usage(int argc, char *argv[]);
unsigned long long musical_chairs(int nplayers);

using namespace std;

int main(int argc, char *argv[])
{
    int c;
	//int nplayers = 0;

    while (1) {
        int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        static struct option long_options[] = {
            {"help",            no_argument,        0, 'h'},
            {"nplayers",         required_argument,    0, '1'},
            {0,        0,            0,  0 }
        };

        c = getopt_long(argc, argv, "h1:", long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
        case 0:
            cerr << "option " << long_options[option_index].name;
            if (optarg)
                cerr << " with arg " << optarg << endl;
            break;

        case '1':
            nplayers = atoi(optarg);
            break;

        case 'h':
        case '?':
            usage(argc, argv);

        default:
            cerr << "?? getopt returned character code 0%o ??n" << c << endl;
            usage(argc, argv);
        }
    }

    if (optind != argc) {
        cerr << "Unexpected arguments.\n";
        usage(argc, argv);
    }


	if (nplayers == 0) {
		cerr << "Invalid nplayers argument." << endl;
		return EXIT_FAILURE;
	}

    chair = (mutex*)malloc(nplayers * sizeof(mutex));
    num_chairs = nplayers - 1;

    unsigned long long game_time;
	game_time = musical_chairs(nplayers);

    cout << "Time taken for the game: " << game_time << " us" << endl;

    exit(EXIT_SUCCESS);
}

void usage(int argc, char *argv[])
{
    cerr << "Usage:\n";
    cerr << argv[0] << "--nplayers <n>" << endl;
    exit(EXIT_FAILURE);
}

void umpire_main(int nplayers)
{
    int instr;

    if(instr == 1) {
        music_end.lock();
        music_start.unlock();
    }
    if(instr == 2) {
        music_end.unlock();
        music_start.lock();
    }

	return;
}

void player_main(int plid)
{
    sleep(s[plid]);
    player.lock();
    player_count++;
    if(player_count == 0) music.lock();
    player.unlock();
    bool alive = false;
    int i = rand() % num_chairs;
    int j = i;

    do{
        if (chair[j].try_lock()) {
            if(c[j] != -1) {
                c[j] = plid;
                alive = true;
            }
            chair[j].unlock();
        }
        j = (j + 1) % num_chairs;
    }while(j != i);

    if (alive == false) dead = plid;
    player.lock();
    player_count--;
    if(player_count == 0) music.unlock();
    player.unlock();
	return;
}

unsigned long long musical_chairs(int nplayers)
{
	auto t1 = chrono::steady_clock::now();

	// Spawn umpire thread.
    /* Add your code here */

	// Spawn n player threads.
    /* Add your code here */

	auto t2 = chrono::steady_clock::now();

	auto d1 = chrono::duration_cast<chrono::microseconds>(t2 - t1);

	return d1.count();
}
