#pragma once

#include <ctime>

// Example usage:
//   Stopwatch watch;
//   ...
//   std::cout << "Elapsed time is " << watch.GetTime() << " seconds.\n";

// A Stopwatch is a class for keeping track of elapsed time.
struct Stopwatch {
    std::clock_t start;
    void Reset() {
        start = clock();
    }
    Stopwatch() {
        Reset();
    }
    double GetTime() const {
        return ((double) clock() - start) / CLOCKS_PER_SEC;
    }
    operator double() const {
        return GetTime();
    }

};
