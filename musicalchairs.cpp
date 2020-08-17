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

std::atomic<int> running; // Maintains the no of players trying to acquire the chair at that moment
bool all_create = false; // stores whether all the threads have been created or not
std::atomic<int> winner; // stores the player who won the game
std::atomic<int> *sl; // stores the sleep intervals corresponding to all players
std::mutex control_mtx, ready_mtx, player_kick, music_start; // mutexes used to prevent synchronization issues
std::condition_variable ready_cv, player_kick_cv, music_start_cv; // Condition variables used to ensure proper order of execution
std::atomic_flag *ch; // Atomic flag array to store whether a chair is trying to be acquired by a player or not
std::atomic<bool> *chair_array; // Atomic bool to store whether a chair is acquired or not
int nplayers; // total no of players at the starting of the game
int total; // total no of players in a particular lap
std::atomic<int> *next_lap_ready; // stores whether the game can proceed to next lap

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
    chair_array = (std::atomic<bool>*)malloc((nplayers - 1) * sizeof(std::atomic<bool>));
    sl = (std::atomic<int>*)malloc(nplayers * sizeof(std::atomic<int>));
    next_lap_ready = (std::atomic<int>*)malloc(nplayers * sizeof(std::atomic<int>));
    
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
    
    if(total == 1) winner = 0;
    
    while(total > 1) { // Running loop until more than one player exists in the game
        
        getline(std::cin, str);
        if(str=="lap_start") { // Reads input
            std::cout << "======= lap# " << nplayers - total + 1 << " =======\n"; // If input read tells us to start the lap
            cout.flush();
            running = total;
            // Initialize the sleeping time of all the players to 0
            for(int i = 0;i < nplayers;i++) {
                next_lap_ready[i] = 1;
                sl[i] = 0;
            }
            // All the chairs are set to free
            for(int i = 0;i < total - 1;i++) {
                chair_array[i] = false;
            }
        }
        
        getline(std::cin, str);
        stringstream S(str);
        std::string s;
        getline(S, s, ' ');
        
        if(s=="player_sleep") {
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
            } while (s=="player_sleep");
        }         // To store the values of sleep in microseconds for each player in array until there is an input corresponding to player_sleep
        
        if(str=="music_start") {
            std::unique_lock<std::mutex> is_music_start(music_start);
            is_music_start.unlock();
            music_start_cv.notify_all();
            // Notifies all the player threads waiting for the music to start
            getline(cin, str);
            S.clear();
            S.str(str);
            getline(S, s, ' ');

            if(s=="umpire_sleep") {

                getline(S, s, ' ');
                this_thread::sleep_for(chrono::microseconds(stoi(s))); // The umpire sleeps if the input reads umpire_sleep
                getline(cin, str);
            }
        }
        
        if(str=="music_stop") {
            control_mtx.unlock(); // If the music stops, releases the control to allow players to choose the chairs
            getline(cin, str);
        }

        if(str=="lap_stop") {
            std::unique_lock<std::mutex> is_player_kicked(player_kick);
            player_kick_cv.wait(is_player_kicked, []() {return !running;});
            /* Waits until one player fails to acquire a chair and all the others have acquired one, and then acquires the control to move on to the next lap or print which player won*/
            control_mtx.lock();

            cout << "**********************\n";
        }
        total --; // Decreasing the no of players still in the game
    }
    cout << "Winner is " << winner << "\n"; // Printing the winner, releasing control and returning
    cout.flush();
    control_mtx.unlock();
    return;
}

void player_main(int plid) {
    if(total == 1) return;

    // Infinite loop only to be exited when the player is unable to acquire any chair    
    while(true) {
        bool got_chair = false; // Initially chair isn't acquired by the player
        
        std::unique_lock<std::mutex> is_music_start(music_start);
        while(next_lap_ready[plid] == 0) music_start_cv.wait(is_music_start);
        is_music_start.unlock();
        // Waits on the condition variable until the umpire reads music_start and unlocks it
        
        this_thread::sleep_for(chrono::microseconds(sl[plid]));
        //Sleeps for the duration specified in the input
        
        control_mtx.lock();
        control_mtx.unlock();
        
        int i = rand() % (total - 1); //Each player may try to acquire any random chair
        int j = i;
        do{
            if (!ch[j].test_and_set()) { /* Checks if the atomic flag corresponding to that chair is free ie. no other player is trying to acquire the chair at the instant */
                if(!chair_array[j]) { // If the chair is free, the player acquires it
                    chair_array[j] = true;
                    got_chair = true;
                }
                ch[j].clear(); // clears the atomic flag corresponding to that chair
            }
            
            if (got_chair) break; // Player will not keep trying to acquire the chair further
            else j = (j + 1) % (total - 1); // It will otherwise try to acquire the next chair
            
        } while(j != i);

        control_mtx.lock();        
        next_lap_ready[plid] = 0;

        if(!got_chair) { // If the player fails to acquire a chair even after trying on all possible ones, it must be kicked from the game
            cout << plid << " could not get chair\n";
            cout.flush();
            
            std::unique_lock<std::mutex> is_player_kicked(player_kick);
            running--;
            is_player_kicked.unlock();
            
            player_kick_cv.notify_one();
                        control_mtx.unlock(); // Notifies the umpire about the player getting kicked and returns
            return;
        } else {
            if (total == 2) { // If it is the last lap, the player remaining is the winner
                winner = plid;
                running--;
                            control_mtx.unlock();
                return;
            }
            std::unique_lock<std::mutex> is_player_kicked(player_kick);
            running--;
            is_player_kicked.unlock();
                        control_mtx.unlock();
        }
    }
}

unsigned long long musical_chairs(int nplayers)
{
    auto t1 = chrono::steady_clock::now();
    cout << "Musical Chairs: " << nplayers << " player game with " << nplayers - 1 << " laps.\n";
    cout.flush();

    thread umpire; // Creation of umpire thread
    umpire = thread (umpire_main);

    thread players[nplayers]; // Creation of nplayers player threads
    for (int i=0; i<nplayers; i++) {
        players[i] = thread(player_main, i);
    }
    all_create = true; // All the threads have been created
    std::unique_lock<std::mutex> is_all_thread(ready_mtx);
    is_all_thread.unlock();
    ready_cv.notify_one();
    
    for (int i=0; i<nplayers; i++) { // Joining the threads
        players[i].join();
    };
    
    umpire.join();

    auto t2 = chrono::steady_clock::now();

    auto d1 = chrono::duration_cast<chrono::microseconds>(t2 - t1);

    return d1.count();
}
