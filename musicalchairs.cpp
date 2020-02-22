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
#include <atomic>
using namespace std;
int nplayers;
int fuck = 0;
std::atomic<int> ready_count;
std::atomic<int> end_count;
std::atomic<int> dead;
int player_count=0;
std::atomic<int> num_chairs;
int music_end = 0;
mutex music_start, player, creation, mus, count_mutex;
mutex l_s_mutex, m_s_mutex, m_e_mutex, l_e_mutex;
//unique_lock<mutex> l_s_lck(l_s_mutex);
//unique_lock<mutex> m_s_lck(m_s_mutex);
//unique_lock<mutex> m_e_lck(m_e_mutex);
//unique_lock<mutex> l_e_lck(l_e_mutex);
condition_variable l_s, m_s, m_e, l_e;
std::atomic<int> avail;
std::atomic<bool> *chair_array;
std::atomic<bool> *isalive;
int *sl;
std::atomic_flag *ch;
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
    //std::mutex xy[10];

    //for(int i = 0;i < 10;i++) {
        //cout <<  "before : " << xy[i].try_lock() << "\n";
        ////if(i % 2 == 0) xy[i].lock();
         //xy[i].unlock();
        //cout << "after : " << xy[i].try_lock() << "\n";
    //}
    //chair = (mutex*)malloc((nplayers - 1) * sizeof(mutex));
    //for(int i = 0;i < nplayers - 1;i++) {
        //cout <<  "before : " << chair[i].try_lock() << "\n";
        //chair[i].unlock();
        //cout << "after : " << chair[i].try_lock() << "\n";
    //}
    //for(int i = 0;i < nplayers - 1;i++) {
        //chair[i] = new mutex;
    //}
    //cout << typeof(chair) << "\n";
    ch = (std::atomic_flag*)malloc((nplayers - 1) * sizeof(std::atomic_flag));
    isalive = (std::atomic<bool>*)malloc(nplayers * sizeof(bool));
    chair_array = (std::atomic<bool>*)malloc((nplayers - 1) * sizeof(bool));
    sl = (int*)malloc(nplayers * sizeof(int));
    //for(int i = 0;i < nplayers - 1;i++) {
        //cout << "value : " << ch[i].test_and_set() << "\n";
        //cout << "value : " << ch[i].test_and_set() << "\n";
        //ch[i].clear();
        //cout << "value : " << ch[i].test_and_set() << "\n";

    //}
    for(int i = 0;i < nplayers - 1;i++) {
        chair_array[i] = false;
    }
    for(int i = 0;i < nplayers;i++) {
        isalive[i] = true;
    }

    num_chairs = nplayers - 1;

    unsigned long long game_time;
    ready_count = 0;
    end_count = 0;
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
    string inst; int lap_no=1;
    while(nplayers > 1) {
        m_s_mutex.lock();
        getline(cin, inst);

        if(inst.compare("lap_start") == 0) {
            //l_e_mutex.unlock();
            for(int i = 0;i < nplayers - 1;i++) {
                chair_array[i] = false;
            }
            cout << "======= lap# "<< lap_no << " =======\n";

            for (int i = 0; i <nplayers; i++) {
                sl[i] = 0;
            }
            //for(int i = 0;i < nplayers-1;i++) {
                //chair_array[i] = true;
            //}

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
                sl[p_id] = stoi(s);

                getline(cin, inst);
                S.str(inst);
                S.clear();
                getline(S, s, ' ');
            } while (s.compare("player_sleep") == 0);
        }

        //for(int i = 0;i < nplayers;i++) {
            //cout << sl[i] << " ";
        //}
        //cout << "\n";

        if(inst.compare("music_start") == 0) {

            m_s_mutex.unlock();
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
            {
                std::lock_guard<std::mutex> m_e_lck(m_e_mutex);
                music_end = 1;
            }
            m_e.notify_all();
            getline(cin, inst);
        }

        if(inst.compare("lap_stop") == 0) {
            //m_s_mutex.lock();
            while(nplayers + 1 > end_count) continue;
            ready_count = 0;
            end_count = 0;
            //l_e.notify_all();
            //cout << dead << "\n";
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
    bool got_chair = false;
    while(alive & !got_chair) {
        if(num_chairs == 0) return;
        srand(time(0));
        ready_count += 1;
        std::unique_lock<std::mutex> m_s_lock(m_s_mutex, std::defer_lock);
        m_s_mutex.lock();
        m_s_mutex.unlock();
        this_thread::sleep_for(chrono::microseconds(sl[plid]));
        std::unique_lock<std::mutex> m_e_lock(m_e_mutex, std::defer_lock);
        m_e_lock.lock();
        m_e.wait(m_e_lock, [](){return music_end;});
        int i = rand() % num_chairs;
        int j = i;
        do{
            //cout << ch[j].test_and_set();
            //ch[j].clear();
            if (!ch[j].test_and_set()) {
                if(!chair_array[j]) {
                    chair_array[j] = true;
                    cout << plid << " got chair: " << j << "\n";
                    got_chair = true;
                    //alive = true;
                }
                ch[j].clear();
            }
            if (got_chair) break;
            else j = (j + 1) % num_chairs;
        }while(j != i);
        if(!got_chair) {
            alive = false;
            cout << plid << " died \n";
            nplayers--;
            num_chairs--;
            isalive[plid] = false;
        }
        end_count += 1;
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
