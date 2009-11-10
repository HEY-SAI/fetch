#include <NIDAQmx.h>

#define NI6713_MAX_SOURCE_FREQUENCY 20000000 // Hz
#define NI6713_SLEW_RATE            20.0     // V/us
#define NI6713_SETTLING_TIME        3.0      // us

#define NI6713_MAX_UPDATE_RATE      1000000  // S/sec
#define NI6713_FIFO_BUFFER_SIZE     16384    // Samples

#define NI6713_MAX_VOLTS_OUT        10       // +/- V
#define NI6713_OUTPUT_IMPEDANCE     0.1      // ohms