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
atomic<int> running;
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
mutex mtx, mtx1, mtx2, mtx3;
std::condition_variable cv1, cv2, cv3;
int *ready;
mutex all_ready_mutex;
int all_ready = 0;
int *seat, *sleepers;

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
    ch = (std::atomic_flag*)malloc((nplayers - 1) * sizeof(std::atomic_flag));
    isalive = (std::atomic<bool>*)malloc(nplayers * sizeof(bool));
    chair_array = (std::atomic<bool>*)malloc((nplayers - 1) * sizeof(bool));
    sl = (int*)malloc(nplayers * sizeof(int));
    for(int i = 0;i < nplayers - 1;i++) {
        chair_array[i] = false;
    }
    for(int i = 0;i < nplayers;i++) {
        isalive[i] = true;
    }

    num_chairs = nplayers - 1;
//    n_searching = 0;

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
    cout <<"Musical Chairs: "<< nplayers <<" player game with "<< nplayers-1 << " laps.\n";
    string inst; int lap_no=1;
    cout << mtx.try_lock();
    std::unique_lock<std::mutex> all_ready_lck(all_ready_mutex);
    
    while(!all_ready) {
        cv1.wait(all_ready_lck);
    }
    
    while(nplayers > 1) {
        
        running = nplayers;
        getline(cin, inst);

        if(inst.compare("lap_start") == 0) {
            //l_e_mutex.unlock();
            for(int i = 0;i < nplayers - 1;i++) {
                chair_array[i] = false;
            }
            std::unique_lock<std::mutex> lk3(mtx3);
            for (int i = 0; i < nplayers; i++)
            {
                ready[i] = 1;
                sleepers[i] = 0;
            }
            lk3.unlock();
            cv3.notify_all();
            cout << "======= lap# "<< lap_no << " =======\n";

            for (int i = 0; i <nplayers; i++) {
                sl[i] = 0;
            }

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

        if(inst.compare("music_start") == 0) {

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
            mtx.unlock();
            getline(cin, inst);
        }

        if(inst.compare("lap_stop") == 0) {
            std::unique_lock<std::mutex> lk2(mtx2);
            while (running != 0)
                cv2.wait(lk2);
            mtx.lock();

            cout << "**********************\n";
        }
        lap_no++;

    }

    if(nplayers == 1) {
        for(int i = 0;i < nplayers;i++) {
            if (isalive[i] == true) {
                cout << "Winner :" << i << "\n";
                return;
            }
        }
    }

	return;
}
void player_main(int plid)
{
    bool alive = true;
//    cout << alive << "\n";
    while(true) {
        
        std::unique_lock<std::mutex> lk3(mtx3);
        while (!ready[plid]) cv3.wait(lk3);
        lk3.unlock();
        
        srand(time(0));
        
        mtx.lock();
        if (sleepers[plid] != 0) {
            mtx.unlock();
            this_thread::sleep_for(chrono::microseconds(sl[plid]));
        }
        else mtx.unlock();
        bool got_chair = false;
        
        int i = rand() % num_chairs;
        int j = i;
        do{
            if (!ch[j].test_and_set()) {
                if(!chair_array[j]) {
                    chair_array[j] = true;
                    cout << plid << " got chair: " << j << "\n";
                    got_chair = true;
                }
                ch[j].clear();
            }
            if (got_chair) break;
            else j = (j + 1) % num_chairs;
        }while(j != i);
        
        ready[plid] = 0;
        running -= 1;
        
        if(!got_chair) {
            alive = false;
            cout << plid << " died \n";
            isalive[plid] = false;
            return;
        } else {
            if(nplayers == 2)  {
                cout << plid << " won \n";
            }
        }
        end_count += 1;
    }
	return;
}

unsigned long long musical_chairs()
{
	auto t1 = chrono::steady_clock::now();
    ready = new int[nplayers];
    sleepers = new int[nplayers];

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
    std::unique_lock<std::mutex> all_ready_lck(all_ready_mutex);
    all_ready = 1;
    all_ready_lck.unlock();
    cv1.notify_one();
    umpire.join();
    /* Add your code here */

	auto t2 = chrono::steady_clock::now();

	auto d1 = chrono::duration_cast<chrono::microseconds>(t2 - t1);

	return d1.count();
}
