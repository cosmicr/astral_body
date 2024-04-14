#ifndef TIMER_H
#define TIMER_H

// Inline assembly function to read the jiffy clock
uint16_t read_jiffy_clock();

// Start the stopwatch
void start_stopwatch();

// End the stopwatch and print the elapsed time
void end_stopwatch();

#endif // TIMER_H