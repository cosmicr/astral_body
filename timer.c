#include <stdint.h>
#include <stdio.h>

// TODO: integrate timer into game loop

// Global variables to hold the jiffy clock values
static uint8_t jiffy_high, jiffy_mid, jiffy_low;

// Function to read the jiffy clock and return a 24-bit value
uint32_t read_jiffy_clock()
{
    uint32_t jiffy_value;

    __asm__("jsr RDTIM");
    __asm__("sty %v", jiffy_high);
    __asm__("stx %v", jiffy_mid);
    __asm__("sta %v", jiffy_low);

    // Combine the values into a 24-bit number
    jiffy_value = ((uint32_t)jiffy_high << 16) | ((uint32_t)jiffy_mid << 8) | jiffy_low;

    return jiffy_value;
}

// Function to calculate elapsed time in a human-readable format
void print_elapsed_time(uint32_t elapsed_jiffies)
{
    uint32_t seconds = elapsed_jiffies / 60;
    uint32_t minutes = seconds / 60;
    uint32_t milliseconds = (elapsed_jiffies % 60) * 1000 / 60;
    seconds %= 60; // Remaining seconds after minutes

    printf("%lu min, %lu sec, %lu ms (%lu jiffies tot)\n",
           minutes, seconds, milliseconds, elapsed_jiffies);
}

// Global variable for the stopwatch
static uint32_t start_time = 0;

// Start the stopwatch
void start_stopwatch()
{
    start_time = read_jiffy_clock();
}

// End the stopwatch and print the elapsed time
void end_stopwatch()
{
    uint32_t end_time = read_jiffy_clock();
    uint32_t elapsed_time = end_time - start_time;

    print_elapsed_time(elapsed_time);
}

