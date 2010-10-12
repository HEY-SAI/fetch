/*
 * DAQ.h
 *
 *  Created on: Oct 10, 2010
 *      Author: Nathan Clack <clackn@janelia.hhmi.org>
 */
 /*
 * Copyright 2010 Howard Hughes Medical Institute.
 * All rights reserved.
 * Use is subject to Janelia Farm Research Campus Software Copyright 1.1
 * license terms (http://license.janelia.org/license/jfrc_copyright_1_1.html).
 */

#pragma once
#include "agent.h"
#include "daq.pb.h"
#include "object.h"
#include "DAQChannel.h"

namespace fetch
{

  namespace device
  {

    class IDAQ
    {
    public:
      virtual int waitForDone(DWORD timeout_ms=INFINITE) = 0;

      virtual void setupCLK(float64 nrecords, float64 record_frequency_Hz) = 0;
      virtual void setupAO(float64 nrecords, float64 record_frequency_Hz) = 0;
      virtual void setupAOChannels(float64 nrecords, 
                           float64 record_frequency_Hz,
                           float64 vmin,
                           float64 vmax,
                           IDAQChannel **channels,
                           int nchannels) = 0;

      virtual void writeAO(float64 *data);

      virtual int32 startAO() = 0;
      virtual int32 startCLK() = 0;
      virtual int32 stopAO() = 0;
      virtual int32 stopCLK() = 0;

      virtual float64 computeSampleFrequency( float64 nrecords, float64 record_frequency_Hz ) = 0;
      virtual int32   samplesPerRecordAO() = 0;
    };

    template<class T>
    class DAQBase:public IDAQ,public IConfigurableDevice<T>
    {
    public:
      DAQBase(Agent *agent)             :IConfigurableDevice(agent) {}
      DAQBase(Agent *agent, Config* cfg):IConfigurableDevice(agent,cfg) {}
    };

    class NationalInstrumentsDAQ : public DAQBase<cfg::device::NationalInstrumentsDAQ>
    {
      NIDAQChannel _clk,_ao;
    public:
      NationalInstrumentsDAQ(Agent *agent);
      NationalInstrumentsDAQ(Agent *agent, Config *cfg);

      virtual unsigned int attach();
      virtual unsigned int detach();

      int waitForDone(DWORD timeout_ms=INFINITE);

      void setupCLK(float64 nrecords, float64 record_frequency_Hz);
      void setupAO(float64 nrecords, float64 record_frequency_Hz);
      void setupAOChannels(float64 nrecords, float64 record_frequency_Hz, float64 vmin, float64 vmax, IDAQChannel **channels, int nchannels);
           
      void writeAO(float64 *data);

      int32 startAO()  { return DAQmxStartTask(_ao.daqtask);}
      int32 startCLK() { return DAQmxStartTask(_clk.daqtask);}
      int32 stopAO()  { return DAQmxStopTask(_ao.daqtask);}
      int32 stopCLK() { return DAQmxStopTask(_clk.daqtask);}

      float64 computeSampleFrequency( float64 nrecords, float64 record_frequency_Hz );
      int32   samplesPerRecordAO() {return _config->ao_samples_per_waveform();}
    protected:
      void registerDoneEvent(void);      
    public:
      HANDLE _notify_done;
    };

    class SimulatedDAQ : public DAQBase<cfg::device::SimulatedDAQ>
    {
    public:
      SimulatedDAQ(Agent *agent);
      SimulatedDAQ(Agent *agent, Config *cfg);

      virtual unsigned int attach() {return 0;}
      virtual unsigned int detach() {return 0;}

      int waitForDone(DWORD timeout_ms=INFINITE) {return 0;}

      void setupCLK(float64 nrecords, float64 record_frequency_Hz) {}
      void setupAO(float64 nrecords, float64 record_frequency_Hz)  {}
      void setupAOChannels(float64 nrecords, float64 record_frequency_Hz, float64 vmin, float64 vmax, IDAQChannel **channels, int nchannels) {}

      void writeAO(float64 *data) {}

      int32 startAO()  { return 0;}
      int32 startCLK() { return 0;}
      int32 stopAO()  { return 0;}
      int32 stopCLK() { return 0;}

      float64 computeSampleFrequency( float64 nrecords, float64 record_frequency_Hz ) {return _config->sample_frequency_hz();}
      int32   samplesPerRecordAO() {return _config->samples_per_record();}
    };
   
   class DAQ:public DAQBase<cfg::device::DAQ>
   {
     NationalInstrumentsDAQ *_nidaq;
     SimulatedDAQ           *_simulated;
     IDevice *_idevice;
     IDAQ    *_idaq;
   public:
     DAQ(Agent *agent);
     DAQ(Agent *agent, Config *cfg);
     ~DAQ();

     void setKind(Config::DAQKind kind);
     void _set_config( Config IN *cfg );
     void _set_config( const Config &cfg );

     virtual unsigned int attach() {return _idevice->attach();}
     virtual unsigned int detach() {return _idevice->detach();}

     int waitForDone(DWORD timeout_ms=INFINITE) {return _idaq->waitForDone(timeout_ms);}

     void setupCLK(float64 nrecords, float64 record_frequency_Hz) {_idaq->setupCLK(nrecords,record_frequency_Hz);}
     void setupAO(float64 nrecords, float64 record_frequency_Hz)  {_idaq->setupAO (nrecords,record_frequency_Hz);}
     void setupAOChannels(float64 nrecords, float64 record_frequency_Hz, float64 vmin, float64 vmax, IDAQChannel **channels, int nchannels) 
     {_idaq->setupAOChannels(nrecords,record_frequency_Hz,vmin,vmax,channels,nchannels);}

     void writeAO(float64 *data) {_idaq->writeAO(data);}

     int32 startAO()  { return _idaq->startAO();}
     int32 startCLK() { return _idaq->startCLK();}
     int32 stopAO()   { return _idaq->stopAO();}
     int32 stopCLK()  { return _idaq->stopCLK();}

     float64 computeSampleFrequency( float64 nrecords, float64 record_frequency_Hz ) {return _idaq->computeSampleFrequency(nrecords,record_frequency_Hz);};
     int32   samplesPerRecordAO() {return _idaq->samplesPerRecordAO();}
   };

   //end namespace fetch::device
  }
}

