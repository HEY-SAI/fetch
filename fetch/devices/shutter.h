/*
 * Shutter.h
 *
 *  Created on: Apr 19, 2010
 *      Author: Nathan Clack <clackn@janelia.hhmi.org>
 */
/*
 * Copyright 2010 Howard Hughes Medical Institute.
 * All rights reserved.
 * Use is subject to Janelia Farm Research Campus Software Copyright 1.1
 * license terms (http://license.janelia.org/license/jfrc_copyright_1_1.html).
 */

#pragma once
#include "NIDAQAgent.h"
#include "shutter.pb.h"

#define SHUTTER_DEFAULT_TIMEOUT           INFINITY
#define SHUTTER_MAX_CHAN_STRING                 32

//#define DEFAULT_SHUTTER_CHANNEL "/Dev1/port0/line8"
//#define DEFAULT_SHUTTER_OPEN                     0
//#define DEFAULT_SHUTTER_CLOSED                   1

#define SHUTTER_DEFAULT_OPEN_DELAY_MS           50

//#define SHUTTER_DEFAULT_CONFIG \
//        { DEFAULT_SHUTTER_CHANNEL,\
//          DEFAULT_SHUTTER_OPEN,\
//          DEFAULT_SHUTTER_CLOSED\
//        }

namespace fetch
{

  namespace device
  {

    class Shutter : public NIDAQAgent
    {
    public:
      typedef cfg::device::Shutter Config;

      Shutter();
      Shutter(Config *cfg);

      void Set          (u8 val);
      void Close        (void);
      void Open         (void);
      
      void Bind         (void);   // Binds the digital output channel to the daq task.

    public:
      Config *config;

    private:
      Config _default_config;
    };

  }

}
