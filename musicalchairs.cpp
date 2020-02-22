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
#include <sstream>
#include <thread>
using namespace std;

int nplayers;
std::atomic<int> ready_count;
std::atomic<int> end_count;
std::atomic<int> dead;
int player_count=0;
int num_chairs;

mutex music_start, music_end, player, creation, mus, count_mutex;
mutex l_s_mutex, m_s_mutex, m_e_mutex, l_e_mutex;
unique_lock<mutex> l_s_lck(l_s_mutex);
unique_lock<mutex> m_s_lck(m_s_mutex);
unique_lock<mutex> m_e_lck(m_e_mutex);
unique_lock<mutex> l_e_lck(l_e_mutex);
condition_variable l_s, m_s, m_e, l_e;

std::atomic<bool> *chair_array;
std::atomic<bool> *isalive;
int *s;

mutex *chair;

void usage(int argc, char *argv[]);
unsigned long long musical_chairs();

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

    chair = (mutex*)malloc((nplayers - 1) * sizeof(mutex));
    isalive = (std::atomic<bool>*)malloc(nplayers * sizeof(bool));
    chair_array = (std::atomic<bool>*)malloc((nplayers - 1) * sizeof(bool));
    s = (int*)malloc(nplayers * sizeof(int));
    for(int i = 0;i < nplayers - 1;i++) {
        chair_array[i] = false;
    }
    for(int i = 0;i < nplayers;i++) {
        isalive[i] = true;
    }

    num_chairs = nplayers - 1;

    unsigned long long game_time;
	game_time = musical_chairs();

    cout << "Time taken for the game: " << game_time << " us" << endl;

    exit(EXIT_SUCCESS);
}

void usage(int argc, char *argv[])
{
    cerr << "Usage:\n";
    cerr << argv[0] << "--nplayers <n>" << endl;
    exit(EXIT_FAILURE);
}

void umpire_main()
{
    // [0 : lap start, 1: lap end, 2 : mus start, 3:mus end]
    // l_s, l_e, m_s, m_e
    string inst; int lap_no=1;
    while(nplayers > 1) {
        getline(cin, inst);
        //std::unique_lock<std::mutex> m_s_lock(m_s_mutex, std::defer_lock);

        if(inst.compare("lap_start") == 0) {
            cout << "======= lap# "<< lap_no << " =======\n";

            for (int i = 0; i <nplayers; i++) {
                s[i] = 0;
            }
            for(int i = 0;i < nplayers-1;i++) {
                chair_array[i] = true;
            }


            m_s_mutex.lock();
            //m_s_lock.lock();
            while(ready_count < nplayers);
        }

        getline(cin, inst);
        stringstream S(inst);
        string s;
        getline(S, s, ' ');

        if(s.compare("player_sleep") == 0) {
            int p_id;

            do {
                getline(S, s, ' ');
                p_id = stoi(s);
                getline(S, s, ' ');
                s[p_id] = stoi(s);

                getline(cin, inst);
                S.str(inst);
                S.clear();
                getline(S, s, ' ');
            } while (s.compare("player_sleep") == 0);
        }

        if(inst.compare("music_start") == 0) {

            cout << "unlocked \n";
            m_s_mutex.unlock();
            cout << "unlocked \n";
            getline(cin, inst);
            S.clear();
            S.str(inst);
            getline(S, s, ' ');

            if(s.compare("umpire_sleep") == 0) {

                getline(S, s, ' ');
                this_thread::sleep_for(chrono::microseconds(stoi(s)));
                getline(cin, inst);
            }
        }

        if(inst.compare("music_stop") == 0) {

            l_e.wait(l_e_lck);
            m_e.notify_all();
            //cout << dead;
            nplayers--;
            num_chairs--;
            for(int i = 0;i < nplayers - 1;i++) {
                chair_array[i] = false;
            }
            getline(cin, inst);
        }

        if(inst.compare("lap_stop") == 0) {
            l_s.wait(l_s_lck);
            while(nplayers > end_count);
            ready_count = 0;
            l_e.notify_all();
            cout << "**********************\n";
        }
        lap_no++;

    }

    if(nplayers == 1) {
        for(int i = 0;i < nplayers;i++) {
            if (isalive[i] == true) {
                cout << "Winner :" << i << "\n";
                break;
            }
        }
    }

	return;
}

void player_main(int plid)
{
    bool alive = true;
    while(alive) {
        ready_count += 1;

        //std::unique_lock<std::mutex> m_s_lock(m_s_mutex, std::defer_lock);
        cout << "waiting for music start \n";
        m_s_mutex.lock();
        cout << "lock acq \n";
        m_s_mutex.unlock();
        cout << "music started \n";
        this_thread::sleep_for(chrono::microseconds(s[plid]));
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
            isalive[plid] = false;
        }
    }
	return;
}

unsigned long long musical_chairs()
{
	auto t1 = chrono::steady_clock::now();

    thread umpire;
    umpire = thread (umpire_main);

	// Spawn umpire thread.
    /* Add your code here */
    thread players[nplayers];
    for (int i=0; i<nplayers; i++) {
        players[i] = thread(player_main, i);
    }
    for (int i=0; i<nplayers; i++) {
        players[i].join();
    };
	// Spawn n player threads.
    umpire.join();
    /* Add your code here */

	auto t2 = chrono::steady_clock::now();

	auto d1 = chrono::duration_cast<chrono::microseconds>(t2 - t1);

	return d1.count();
}
