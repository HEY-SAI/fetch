/*
 * Scanner2D.h
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
 /*
 * Scanner3D
 * ---------
 *
 * This class models a device capable of point scanned volume acquisition using
 * a resonant mirror, linear scan mirror, and a closed-loop piezo-drive
 * objective positioner.
 *
 * Acquisition using a resonant mirror requires synchronizing multiple devices.
 * Here, I've broken the operation of the scanner into many parts, each
 * represented by parent classes.  The parent classes are all eventually
 * virtually derived from the Agent class, so Agent state is shared among all
 * the parent classes.  The hierarchy looks like this:
 *
 * (Agent)-------|--------------|
 *           Digitizer      NIDAQAgent----|-------|--------------|-----------|
 *               |                     Pockels  Shutter  LinearScanMirror  ZPiezo
 *                \                      /       /              /           /
 *                 \____________________/_______/______________/           /
 *                           |                                            /
 *                        Scanner2D                                      /
 *                           | _________________________________________/
 *                           |/
 *                        Scanner3D
 *
 * Notes
 * -----
 *  - attach/detach just call Scanner2D's implementations.  Redefining them here
 *    just makes the call explicit (could implicitly do it with inheritence order, I think).
 *
 *  - writing enough data for a frame across three channels requires more than the 
 *    available memory on the DAQ.  However, we can transfer data faster than we use it.
 *    So we need to set this up to write the waveform out in pieces.
 *    Anyway, there are two relevant DAQmx properties, I think:
 *      DAQmx.Channel >> AO.DataXFerReqCond set    to  DAQmx_Val_OnBrdMemNotFull 
 *      DAQmx.Write   >> RegenMode                 to  DAQmx_Val_DoNotAllowRegen 
 */
#pragma once
#include "scanner2d.h"
#include "ZPiezo.h"

namespace fetch
{ namespace device
  {

  class Scanner3D :
    public Scanner2D,
    public ZPiezo
    {
    public:
               Scanner3D(void);
      virtual ~Scanner3D(void);
      
      unsigned int attach(void); // Returns 0 on success, 1 otherwise
      unsigned int detach(void); // Returns 0 on success, 1 otherwise
    };
  
  }
}