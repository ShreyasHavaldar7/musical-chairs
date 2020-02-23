/*
 * Program: Musical chairs game with n players and m intervals.
 * Authors: Shreyas Jayant Havaldar, Vedant Singh
 * Roll# : CS18BTECH11042, CS18BTECH11047 
 */

#include <stdlib.h>  /* for exit, atoi */
#include <iostream>  /* for fprintf */
#include <errno.h>   /* for error code eg. E2BIG */
#include <getopt.h>  /* for getopt */
#include <assert.h>  /* for assert */
#include <chrono>    /* for timers */
#include <mutex>     /* for use of mutex locks */
#include <condition_variable> /* for use of condition variables */
#include <sstream> /* for use of stringstream for managing input read */
#include <thread> /* for use of threads */
#include <atomic> /* for use of atomic flag */

using namespace std;

int running;
bool all_create = false; // stores whether all the threads have been created or not
int winner; // stores the player who won the game
int all_thread;
int *sl; // stores the sleep intervals corresponding to all players
std::mutex control_mtx, ready_mtx, player_kick, music_start, value; // mutexes used to prevent synchronization issues
std::condition_variable ready_cv, player_kick_cv, music_start_cv; // Condition variables used to ensure proper order of execution
std::atomic_flag *ch; // Atomic flag array to store whether a chair has been acquired by a player or not
bool *chair_array;
int nplayers;
int total;

void usage(int argc, char *argv[]);
unsigned long long musical_chairs(int nplayers);

int main(int argc, char *argv[])
{
    int c;
    while (1)
    {
        int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        static struct option long_options[] = {
            {"help", no_argument, 0, 'h'},
            {"nplayers", required_argument, 0, '1'},
            {0, 0, 0, 0}};

        c = getopt_long(argc, argv, "h1:", long_options, &option_index);
        if (c == -1)
            break;

        switch (c)
        {
        case 0:
            std::cerr << "option " << long_options[option_index].name;
            if (optarg)
                std::cerr << " with arg " << optarg << endl;
            break;

        case '1':
            nplayers = atoi(optarg);
            break;

        case 'h':
        case '?':
            usage(argc, argv);

        default:
            std::cerr << "?? getopt returned character code 0%o ??n" << c << endl;
            usage(argc, argv);
        }
    }

    if (optind != argc)
    {
        std::cerr << "Unexpected arguments.\n";
        usage(argc, argv);
    }

    if (nplayers <= 0)
    {
        std::cerr << "Invalid nplayers argument." << endl;
        return EXIT_FAILURE;
    }
    
    total = nplayers;
    // Allocating space in the memory for the arrays
    ch = (std::atomic_flag*)malloc((nplayers - 1) * sizeof(std::atomic_flag));
    chair_array = (bool*)malloc((nplayers - 1) * sizeof(bool));
    sl = (int*)malloc(nplayers * sizeof(int));
    
    unsigned long long game_time;
    
    game_time = musical_chairs(nplayers);

    std::cout << "Time taken for the game: " << game_time << " us" << std::endl;

    exit(EXIT_SUCCESS);
}

void usage(int argc, char *argv[])
{
    std::cerr << "Usage:\n";
    std::cerr << argv[0] << "--nplayers <n>" << endl;
    exit(EXIT_FAILURE);
}

void umpire_main() {
    
    std::string str;
    control_mtx.lock(); // Acquires mutex to prevent players from advancing
    running = nplayers;
    
    std::unique_lock<std::mutex> is_all_thread(ready_mtx);
    ready_cv.wait(is_all_thread, []() {return all_create;});
    // Waits until all the player threads have been created
    
    if(nplayers == 1) winner = 0;
    
    while(nplayers > 1) { // Running loop until more than one player exists in the game
        
        getline(std::cin, str); // Reads input
        if(!str.compare("lap_start")) { // If input read tells us to start the lap
            std::cout << "======= lap# " << total - nplayers + 1 << " =======\n";
            running = nplayers;
            // Initialize the sleeping time of all the players to 0
            for(int i = 0;i < nplayers;i++) {
                sl[i] = 0;
            }
            // All the chairs are set to free
            for(int i = 0;i < nplayers - 1;i++) {
                chair_array[i] = false;
            }
        }
        
        getline(std::cin, str);
        stringstream S(str);
        std::string s;
        getline(S, s, ' ');
        
        if(s.compare("player_sleep") == 0) {
            int p_id;

            do {
                getline(S, s, ' ');
                p_id = stoi(s);
                getline(S, s, ' ');
                sl[p_id] = stoi(s);

                getline(cin, str);
                S.str(str);
                S.clear();
                getline(S, s, ' ');
            } while (s.compare("player_sleep") == 0);
        }
        // To store the values of sleep in microseconds for each player in array until there is an input corresponding to player_sleep
        
        if(str.compare("music_start") == 0) {
        
            std::unique_lock<std::mutex> is_music_start(music_start);
            is_music_start.unlock();
            music_start_cv.notify_all();
            // Notifies all the player threads waiting for the music to start
            getline(cin, str);
            S.clear();
            S.str(str);
            getline(S, s, ' ');

            if(s.compare("umpire_sleep") == 0) {
            
				// The umpire sleeps if the input reads umpire_sleep
                getline(S, s, ' ');
                this_thread::sleep_for(chrono::microseconds(stoi(s)));
                getline(cin, str);
            }
        }
        
        if(str.compare("music_stop") == 0) {
        	// If the music stops, releases the control to allow players to choose the chairs
            control_mtx.unlock();
            getline(cin, str);
        }

        if(str.compare("lap_stop") == 0) {
            std::unique_lock<std::mutex> is_player_kicked(player_kick);
            player_kick_cv.wait(is_player_kicked, []() {return !running;});
            // Waits until a player fails to acquire a chair, and then acquires the control to move on to the next lap or print ehichplayer won
            control_mtx.lock();

            cout << "**********************\n";
        }
        
        nplayers --;
    }
    cout << "Winner is " << winner << "\n";
    control_mtx.unlock();
    return;
}

void player_main(int plid) {

    if(nplayers == 1) return;
    
    // Infinite loop only to be exited when the player is unable to acquire any chair
    while(true) {
        bool got_chair = false;// Initially chair isn't acquired by the player
        
        std::unique_lock<std::mutex> is_music_start(music_start);
        music_start_cv.wait(is_music_start);
        is_music_start.unlock();
        // Waits on the condition variable until the umpire reads music_start and unlocks it
        this_thread::sleep_for(chrono::microseconds(sl[plid]));
        //Sleeps for the duration specified in the input
        control_mtx.lock();
        control_mtx.unlock();
        
        //Each player may try to acquire any random chair
        int i = rand() % (nplayers - 1);
        int j = i;
        
        do{
            if (!ch[j].test_and_set()) { /* Checks if the atomic flag corresponding to that chair is free ie. no other player is trying to acquire the chair at the instant */
                if(!chair_array[j]) { // If the chair is free, the player acquires it
                    chair_array[j] = true;
                    got_chair = true;
                }
                ch[j].clear(); // clears the atomic flag corresponding to that chair
            }
            
            if (got_chair) {
            	break;
            } // Player will not keep trying to acquire the chair further
             
            else j = (j + 1) % (nplayers - 1); // Player tries to acquire the next chair
            
        }while(j != i);
        
        std::unique_lock<std::mutex> running_change(value);
        running -= 1;
        running_change.unlock();
        
        if(!got_chair) { // If the player fails to acquire a chair even after trying on all possible ones, it must be kicked from the game
            cout << plid << " could not get chair\n";
            player_kick_cv.notify_one(); // Notifies the umpire about the player getting kicked
            return;
        } else {
            if (nplayers == 2) { // If it is the last lap, the player remaining is the winner
                winner = plid;
                return;
            }
        }
    }
}

unsigned long long musical_chairs(int nplayers)
{
    auto t1 = chrono::steady_clock::now();
    cout << "Musical Chairs: " << nplayers << " player game with " << nplayers - 1 << " laps.\n";

	// Creation of umpire thread		
    thread umpire;
    umpire = thread (umpire_main);

	// Creation of nplayers player threads
    thread players[nplayers];
    for (int i=0; i<nplayers; i++) {
        players[i] = thread(player_main, i);
    }
    // All the threads have been created
    all_create = true;
    std::unique_lock<std::mutex> is_all_thread(ready_mtx);
    is_all_thread.unlock();
    ready_cv.notify_one();
    //Notifies the umpire that all the player threads are ready
    
    for (int i=0; i<nplayers; i++) {
        players[i].join();
    };
    
    umpire.join();

    auto t2 = chrono::steady_clock::now();

    auto d1 = chrono::duration_cast<chrono::microseconds>(t2 - t1);

    return d1.count();
}


