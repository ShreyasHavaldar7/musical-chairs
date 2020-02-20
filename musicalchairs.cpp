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
#include <condition_variable>
using namespace std;

int nplayers;
int ready = 0, fuck = 0;
int dead;
int player_count=0;
int num_chairs;

mutex music_start, music_end, player, creation, mus, count_mutex;
mutex l_s_mutex, m_s_mutex, m_e_mutex, l_e_mutex;
unique_lock<mutex> l_s_lck(l_s_mutex);
unique_lock<mutex> m_s_lck(m_s_mutex);
unique_lock<mutex> m_e_lck(m_e_mutex);
unique_lock<mutex> l_e_lck(l_e_mutex);
condition_variable l_s, m_s, m_e, l_e;
bool *chair_array;

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
    chair_array = (bool*)malloc((nplayers - 1) * sizeof(bool));
    for(int i = 0;i < nplayers - 1;i++) {
        chair_array[i] = false;
    }

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
    // [0 : lap start, 1: lap end, 2 : mus start, 3:mus end]
    // l_s, l_e, m_s, m_e
    int instr;
    while(nplayers >= 1) {
        cin >> instr;
        if(instr == 0) {
            m_s.wait(m_s_lck);
            while(ready < nplayers);
            l_s.notify_all();
        }

        if(instr == 2) {
            m_e.wait(m_e_lck);
            m_s.notify_all();
        }

        if(instr == 3) {
            l_e.wait(l_e_lck);
            m_e.notify_all();
            cout << dead;
            nplayers--;
            num_chairs--;
            for(int i = 0;i < nplayers - 1;i++) {
                chair_array[i] = false;
            }
        }

        if(instr == 1) {
            l_s.wait(l_s_lck);
            while(nplayers > fuck);
            ready = 0;
            l_e.notify_all();
            fuck = 0;
        }

    }

    if(nplayers == 1) // print winner 
	return;
}

void player_main(int plid)
{
    bool alive = true;
    while(alive) {
        count_mutex.lock();
        ready++;
        count_mutex.unlock();
        l_s.wait(l_s_lck);
        m_s.wait(m_s_lck);
        //sleep(s[plid]);
        m_e.wait(m_e_lck);
        int i = rand() % num_chairs;
        int j = i;

        do{
            if (chair[j].try_lock()) {
                if(chair_array[j] == false) {
                    chair_array[j] = true;
                    alive = true;
                    break;
                }
                chair[j].unlock();
            }
            j = (j + 1) % num_chairs;
        }while(j != i);

        if (j == i) {
            alive = false;
            dead = plid;
        }
        count_mutex.lock();
        fuck++;
        count_mutex.unlock();
        l_e.wait(l_e_lck);
    }
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
