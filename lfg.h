#ifndef LFG_H
#define LFG_H

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <random>
#include <algorithm>


struct Instance {
    bool active = false;        // is there a party inside?
    int parties_served = 0;     // how many parties have used this instance
    int total_time = 0;         // total dungeon time served (s)
};

void printInstanceStatus(const std::vector<Instance>& instances);

void beginLFGQueue(int n, int t, int h, int d, int t1, int t2);

// bonus functions
void generatePlayers(int& t, int& h, int& d, std::condition_variable& cv, std::mutex& mtx);

void beginLFGQueueBonus(int n, int& t, int& h, int& d, int t1, int t2);
#endif