#include "lfg.h"
#include <iostream>
#include <vector>

using namespace std;

// global synchronization variables
mutex mtx;
condition_variable cv;
int active_instances = 0;
bool generator_done = false;

void printInstanceStatus(const vector<Instance>& instances) {
    cout << "\nCurrent instance status:\n";
    for (size_t i = 0; i < instances.size(); i++) {
        cout << "Instance " << i + 1 << ": " << (instances[i].active ? "active" : "empty") << endl;
    }
    cout << endl;
}

// run executed by each party thread
void dungeonRun(int id, int t1, int t2, vector<Instance>& instances) {
    // simulate a random dungeon duration between t1 and t2
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(t1, t2);
    int duration = dist(gen);

    this_thread::sleep_for(chrono::seconds(duration));

    {
        lock_guard<mutex> lock(mtx);
        instances[id].active = false;
        instances[id].parties_served++;
        instances[id].total_time += duration;
        cout << "Party finished instance " << id + 1 << " in " << duration << "s\n";
        printInstanceStatus(instances);
        active_instances--;
    }
    cv.notify_all(); // notify waiting threads that an instance is free
}

// regular or static version (all players arrive at the same time, no new joins)
void beginLFGQueue(int n, int t, int h, int d, int t1, int t2) {
    vector<Instance> instances(n);
    vector<thread> threads;

    int possible_parties = min({t, h, d / 3});
    cout << "Total parties that can form: " << possible_parties << endl;

    printInstanceStatus(instances);

    for (int i = 0; i < possible_parties; i++) {
        unique_lock<mutex> lock(mtx);
        cv.wait(lock, [&]() {
            return active_instances < n;
        });

        // find any empty instance slot
        int id = -1;    // placeholder
        for (int j = 0; j < n; j++) {
            if (!instances[j].active) {
                id = j;
                instances[j].active = true;
                break;
            }
        }

        if (id != -1) {
            active_instances++;
            cout << "Party entered instance " << id + 1 << endl;
            printInstanceStatus(instances);
            threads.emplace_back(dungeonRun, id, t1, t2, ref(instances));
        }
    }

    // wait for all dungeons to finish
    for (auto& t : threads) t.join();

    cout << "\nAll parties finished!\n\nFinal instance summary:\n";
    for (size_t i = 0; i < instances.size(); i++) {
        cout << "Instance " << i + 1
            << " | Parties served: " << instances[i].parties_served
            << " | Total time: " << instances[i].total_time << "s" << endl;
    }
};

// bonus functions
// dynamically add new players while simulation is running
void generatePlayers(int& t, int& h, int& d, std::condition_variable& cv, std::mutex& mtx) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> roleDist(1, 3);
    uniform_int_distribution<> delayDist(2, 5);     // seconds between new joins

    // randomly add 10 players
    for (int i = 0; i < 10; i++) {
        this_thread::sleep_for(chrono::seconds(delayDist(gen)));

        lock_guard<mutex> lock(mtx);
        int role = roleDist(gen);

        if (role == 1) {
            t++;
            cout << "New TANK joined! Total tanks available: " << t << endl;
        } else if (role ==2) {
            h++;
            cout << "New HEALER joined! Total healers available: " << h << endl;
        } else {
            d++;
            cout << "New DPS joined! Total DPS available: " << d << endl;
        }

        cv.notify_all();
    }

    cout << "Player generator has finished adding players.";
    generator_done = true;
    cv.notify_all();    // wake up main thread
}

void beginLFGQueueBonus(int n, int& t, int& h, int& d, int t1, int t2) {
    vector<Instance> instances(n);
    vector<thread> threads;

    printInstanceStatus(instances);

    // start background thread to add new players
    thread generator(generatePlayers, ref(t), ref(h), ref(d), ref(cv), ref(mtx));

    int parties_formed = 0;

    while (true) {
        unique_lock<mutex> lock(mtx);

        // wait until there's enough players to make a party and for a free instance or wake every 1 second to check if it can still make parties
        cv.wait_for(lock, chrono::seconds(1), [&]() {
            return ((t >= 1 && h >= 1 && d >= 3 && active_instances < n) || generator_done);
        }); 

        // if generator's done and no new part can form, break
        if (generator_done && (t < 1 || h < 1 || d < 3) && active_instances == 0)
            break;

        // double-check player counts before forming a party
        if (!(t >= 1 && h >= 1 && d >= 3 && active_instances < n)) {
            continue;
        }

        // form a party
        t--;
        h--;
        d -= 3;

        int id = -1;
        for (int j = 0; j < n; j++) {
            if (!instances[j].active) {
                id = j;
                instances[j].active = true;
                break;
            }
        }

        if (id != -1) {
            active_instances++;
            parties_formed++;
            cout << "Party #" << parties_formed << " entered instance " << id + 1
                 << " (T:" << t << ", H:" << h << ", D:" << d << ")\n";
            printInstanceStatus(instances);
            threads.emplace_back(dungeonRun, id, t1, t2, ref(instances));
        }

        lock.unlock();
    }

    // join all threads
    generator.join();
    for (auto& th : threads)
        if (th.joinable())
            th.join();

    cout << "\nAll parties finished!\n\nFinal instance summary:\n";
    for (size_t i = 0; i < instances.size(); i++) {
        cout << "Instance " << i + 1
             << " | Parties served: " << instances[i].parties_served
             << " | Total time: " << instances[i].total_time << "s" << endl;
    }
};