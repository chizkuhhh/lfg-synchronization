#include "lfg.h"
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <random>

using namespace std;

int main() {
    cout << "PS2 - LFG Synchronization" << endl;

    int n, t, h, d, t1, t2;
    cout << "Input (in the format n t h d t1 t2): ";
    cin >> n >> t >> h >> d >> t1 >> t2;

    cout << "\nSetup complete!" << endl;
    cout << "Instances: " << n << ", Tanks: " << t << ", Healers: " << h << ", DPS: " << d << endl;

    vector<Instance> instances(n);

    // print instance statuses
    printInstanceStatus(instances);

    // actually run the synchronization simulation
    cout << "\nStarting LFG Queueing...\n";
    // beginLFGQueue(n, t, h, d, t1, t2);
    beginLFGQueueBonus(n, t, h, d, t1, t2);
}