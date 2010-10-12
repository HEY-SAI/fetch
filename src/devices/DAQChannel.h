/*
 * NIDAQChannel.h
 *
 *  Created on: Apr 20, 2010
 *      Author: Nathan Clack <clackn@janelia.hhmi.org>
 */
/*
 * Copyright 2010 Howard Hughes Medical Institute.
 * All rights reserved.
 * Use is subject to Janelia Farm Research Campus Software Copyright 1.1
 * license terms (http://license.janelia.org/license/jfrc_copyright_1_1.html).
 */
/*
 * Agent's that attach to NI-DAQmx resources through calls to:
 *
 *    DAQmxCreateTask
 *
 * should inherit from this class.  This class provides the attach/detach
 * functions required of the Agent interface.
 */

#pragma once

#include "../agent.h"
#include "../util/util-nidaqmx.h"

#define NIDAQAGENT_DEFAULT_TIMEOUT INFINITE

namespace fetch
{

  namespace device
  {
    class IDAQChannel
    {
    public:
      virtual char* name() = 0;
    };

    class NIDAQChannel : public IDAQChannel, public IConfigurableDevice<char*>
    {
    public:
      NIDAQChannel(Agent *agent, char *name);
      ~NIDAQChannel(void);

      unsigned int attach();
      unsigned int detach();
      
      virtual char* name() {return _daqtaskname;}
    public:
      TaskHandle daqtask;
      char _daqtaskname[128];      
    };

    class SimulatedDAQChannel:public IDAQChannel, public IConfigurableDevice<char*>
    { char _name[128];
    public:
      SimulatedDAQChannel(Agent *agent, char *name);
      unsigned int attach();
      unsigned int detach();

      virtual char* name() {return _name;}
    };
  }

}

