/*
 * Vibratome.h
 *
 *  Created on: Apr 19, 2011
 *      Author: Nathan Clack <clackn@janelia.hhmi.org>
 */
/*
 * Copyright 2011 Howard Hughes Medical Institute.
 * All rights reserved.
 * Use is subject to Janelia Farm Research Campus Software Copyright 1.1
 * license terms (http://license.janelia.org/license/jfrc_copyright_1_1.html).
 */
 
/* Class Vibratome
 * ===============
 *
 * Overview
 * --------
 * This object tracks the state associated with a Vibratome controller.
 *
 * The current hardware supports only amplitude control; frequency is fixed.
 * The controller interfaces with the PC via a serial port.  It supports the
 * following commands:
 *
 *    START       - starts wave generation.
 *    STOP        - stops wave generation.
 *    AMP         - queries for the amplitude.  Returns a string of the form
 *                  "Amplitude X" where X is a number from 0-255.
 *    AMP <x>     - sets the amplitude. <x> should be a value from 0-255.
 *
 * Currently, the serial port communication is implemented using the native
 * win32 API.
 *
 */

/*
 * Notes
 * =====
 * Serial communication edge cases.
 * 1. Device gets disconnected (stops responding).
 * 2. Synchronous or Asynchronous reads.
 */

#pragma once

#include "vibratome.pb.h"
#include "object.h"
#include "agent.h"

namespace fetch
{

  namespace device
  {

    class IVibratome
    {
    public:
      virtual int isValidAmplitude(int val)      = 0;
      virtual int setAmplitude(int val)          = 0; 
      virtual int setAmplitudeNoWait(int val)    = 0;
      virtual int getAmplitude_ControllerUnits() = 0;
      virtual double getAmplitude_mm()           = 0;

      virtual int start() = 0;
      virtual int stop()  = 0;
    };

    template<class T>
    class VibratomeBase:public IVibratome, public IConfigurableDevice<T>
    {
    public:
      VibratomeBase(Agent *agent)              : IConfigurableDevice<T>(agent) {}
      VibratomeBase(Agent *agent, Config *cfg) : IConfigurableDevice<T>(agent,cfg) {}
    };

    //
    // SerialControlVibratome
    //

    class SerialControlVibratome:public VibratomeBase<cfg::device::SerialControlVibratome>
    {    
      HANDLE _h;
    public:

      SerialControlVibratome(Agent *agent);
      SerialControlVibratome(Agent *agent, Config *cfg);

      virtual ~SerialControlVibratome();

      virtual unsigned int on_attach(); // open  handle to the com port
      virtual unsigned int on_detach(); // close handle to the com port

      virtual int isValidAmplitude(int val) {return (val>=0)&&(val<=255);}
      virtual int setAmplitude(int val);
      virtual int setAmplitudeNoWait(int val);
      virtual int getAmplitude_ControllerUnits();
      virtual double getAmplitude_mm();

      virtual int start();
      virtual int stop();

    private:
      unsigned int _close();

      // implementation of the serial interface
      // these return 1 on success, 0 otherwise
      int _write(const char* buf, int n);
      int _query(const char* cmd, int ncmd, const char *resp, int *nresp);
      int AMP(int val);        
      int qAMP();
      int START();
      int STOP();
    };   

class SimulatedVibratome:public VibratomeBase<cfg::device::SimulatedVibratome>
    {
      int    _amp;
      double _unit2mm;
    public:
      SimulatedVibratome(Agent *agent)              : VibratomeBase<cfg::device::SimulatedVibratome>(agent),    _amp(0), _unit2mm(1.0/255.0) {}
      SimulatedVibratome(Agent *agent, Config *cfg) : VibratomeBase<cfg::device::SimulatedVibratome>(agent,cfg), _amp(0), _unit2mm(1.0/255.0) {}

      virtual unsigned int on_attach() {return 0;}
      virtual unsigned int on_detach() {return 0;}

      virtual int isValidAmplitude(int val)      {return (val>=0 && val<=255); }
      virtual int setAmplitude(int val)          {_amp=val; return 0; }
      virtual int setAmplitudeNoWait(int val)    {_amp=val; return 0; }
      virtual int getAmplitude_ControllerUnits() {return _amp;}      
      virtual double getAmplitude_mm()           {return _amp*_unit2mm;}

      virtual int start()                        {return 0;} 
      virtual int stop()                         {return 0;} 
    };
    
    class Vibratome:public VibratomeBase<cfg::device::Vibratome>
    {
      SerialControlVibratome *_serial;
      SimulatedVibratome     *_simulated;
                                                              
      IDevice          *_idevice;
      IVibratome       *_ivibratome;
    public:
      Vibratome(Agent *agent);
      Vibratome(Agent *agent, Config *cfg);
      ~Vibratome();

      virtual unsigned int on_attach();
      virtual unsigned int on_detach();

      void setKind(Config::VibratomeType kind);
      void _set_config( Config IN *cfg );
      void _set_config( const Config &cfg );

      virtual int isValidAmplitude(int val)      {return _ivibratome->isValidAmplitude(val);}
      virtual int setAmplitude(int val)          {return _ivibratome->setAmplitude(val);}      
      virtual int setAmplitudeNoWait(int val)    {return _ivibratome->setAmplitudeNoWait(val);}      
      virtual int getAmplitude_ControllerUnits() {return _ivibratome->getAmplitude_ControllerUnits();}
      virtual double getAmplitude_mm()           {return _ivibratome->getAmplitude_mm();}

      virtual int start()                        {return _ivibratome->start();}
      virtual int stop()                         {return _ivibratome->stop();}
    };

  } // end namespace device
}   // end namespace fetch