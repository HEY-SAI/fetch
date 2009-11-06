#pragma once

#include "util-niscope.h"

#define NI5105_MAX_NUM_CHANNELS 8
#define NI5105_MAX_SAMPLE_RATE  60000000.0

#define DIGITIZER_MAX_NUM_CHANNELS NI5105_MAX_NUM_CHANNELS
#define DIGITIZER_MAX_SAMPLE_RATE  NI5105_MAX_SAMPLE_RATE

typedef struct _digitizer 
{ ViSession vi;
} Digitizer;

typedef struct _digitizer_channel_config
{ ViChar    *name;     // Null terminated string with channel syntax: e.g. "0-2,7"
  ViReal64   range;    // Volts peak to peak
  ViInt32    coupling; // Specifies how to couple the input signal. Refer to NISCOPE_ATTR_VERTICAL_COUPLING for more information.
  ViBoolean  enabled;  // Specifies whether the channel is enabled for acquisition. Refer to NISCOPE_ATTR_CHANNEL_ENABLED for more information.
} Digitizer_Channel_Config;

#define DIGITIZER_CHANNEL_CONFIG_EMPTY ((Digitizer_Channel_Config){NULL,0.0,NISCOPE_VAL_DC,VI_FALSE})

typedef struct _digitizer_config
{ ViChar                  *resource_name;      // NI device name: e.g. "Dev6"
  ViReal64                 sample_rate;        // samples/second
  ViInt32                  record_length;      // samples per scan
  ViReal64                 reference_position; // as a percentage
  Digitizer_Channel_Config channels[DIGITIZER_MAX_NUM_CHANNELS];
} Digitizer_Config;

#define DIGITIZER_CONFIG_EMPTY \
  ((Digitizer_Config){NULL,\
                      0.0,\
                        0,\
                      0.0,\
                      {DIGITIZER_CHANNEL_CONFIG_EMPTY,\
                       DIGITIZER_CHANNEL_CONFIG_EMPTY,\
                       DIGITIZER_CHANNEL_CONFIG_EMPTY,\
                       DIGITIZER_CHANNEL_CONFIG_EMPTY,\
                       DIGITIZER_CHANNEL_CONFIG_EMPTY,\
                       DIGITIZER_CHANNEL_CONFIG_EMPTY,\
                       DIGITIZER_CHANNEL_CONFIG_EMPTY,\
                       DIGITIZER_CHANNEL_CONFIG_EMPTY}\
                      })

#define DIGITIZER_CONFIG_DEFAULT \
  ((Digitizer_Config){"Dev6\0",\
                       DIGITIZER_MAX_SAMPLE_RATE,\
                       1024,\
                       0.0,\
                       {{"0\0",2.0,NISCOPE_VAL_DC,VI_TRUE},\
                        {"1\0",2.0,NISCOPE_VAL_DC,VI_TRUE},\
                        {"2\0",2.0,NISCOPE_VAL_DC,VI_TRUE},\
                        {"3\0",2.0,NISCOPE_VAL_DC,VI_TRUE},\
                        {"4\0",2.0,NISCOPE_VAL_DC,VI_FALSE},\
                        {"5\0",2.0,NISCOPE_VAL_DC,VI_FALSE},\
                        {"6\0",2.0,NISCOPE_VAL_DC,VI_FALSE},\
                        {"7\0",2.0,NISCOPE_VAL_DC,VI_FALSE}}\
                       })

void         Digitizer_Init   (void);
unsigned int Digitizer_Close  (void);
unsigned int Digitizer_Off    (void);
unsigned int Digitizer_Hold   (void);