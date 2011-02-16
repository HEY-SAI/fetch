/*
 * Digitizer.h
 *
 * Author: Nathan Clack <clackn@janelia.hhmi.org>
 *   Date: Apr 20, 2010
 */
/*
 * Copyright 2010 Howard Hughes Medical Institute.
 * All rights reserved.
 * Use is subject to Janelia Farm Research Campus Software Copyright 1.1
 * license terms (http://license.janelia.org/license/jfrc_copyright_1_1.html).
 */
/*
 * Notes
 * -----
 *
 * In niScope, a waveform is a single channel for a single record.  A
 * multi-record acquisition is used when one needs fast retriggering.  The data
 * in a record is formated:
 *
 *   [--- chan 0 ---] [--- chan 1 ---] ... [--- chan N ---] : record k
 *
 * For a multirecord acquisition:
 *
 *   [--- record 1 ---][--- record 2 ---] ...
 *
 * So, we have this identity:
 *
 *   #waveforms = #records x #channels
 */
#pragma once

#include "../agent.h"
#include "../util/util-niscope.h"
#include "digitizer.pb.h"
#include "object.h"
#include "types.h"

#define DIGITIZER_BUFFER_NUM_FRAMES       4        // must be a power of two
#define DIGITIZER_DEFAULT_TIMEOUT         INFINITE // ms

namespace fetch
{
  namespace device
  {
    // IDigitizer
    // ----------
    //
    // setup() - configures digitizer for triggered multirecord acquisition.  
    //           specialized a bit for video acquisition.
    // record_size() - returns the number of samples per record.  A record is 
    //                 typically the samples acquired from a single trigger for
    //                 a single channel.
    // nchan() - returns the number of channels acquired by a single acquisition
    //           according to the current digitizer configuration.
    //
    class IDigitizer
    {
    public:
      virtual void   setup(int nrecords, double record_frequency_Hz, double duty) = 0;
      virtual size_t record_size(double record_frequency_Hz, double duty) = 0;
      virtual size_t nchan() = 0;
    }; 

    template<class T>
    class DigitizerBase:public IDigitizer,public IConfigurableDevice<T>
    {
    public:
      DigitizerBase(Agent *agent) : IConfigurableDevice<T>(agent) {}
      DigitizerBase(Agent *agent, Config *cfg) :IConfigurableDevice<T>(agent,cfg) {}
    };


    //////////////////////////////////////////////////////////////////////////
    class NIScopeDigitizer : public DigitizerBase<cfg::device::NIScopeDigitizer>
    {
    public:
      typedef cfg::device::NIScopeDigitizer          Config;
      typedef cfg::device::NIScopeDigitizer_Channel  Channel_Config;

      NIScopeDigitizer(Agent *agent);      
      NIScopeDigitizer(Agent *agent, Config *cfg);
      ~NIScopeDigitizer();

      unsigned int on_attach(void);
      unsigned int on_detach(void);

      virtual void   setup(int nrecords, double record_frequency_Hz, double duty);
      virtual size_t record_size(double record_frequency_Hz, double duty);
      virtual size_t nchan() {return _config->nchannels();}
    public:
      ViSession _vi;

    private:
      void __common_setup();
    };

    //////////////////////////////////////////////////////////////////////////
    class AlazarDigitizer : public DigitizerBase<cfg::device::AlazarDigitizer>
    {
    public:
      AlazarDigitizer(Agent *agent)             : DigitizerBase<cfg::device::AlazarDigitizer>(agent) {}
      AlazarDigitizer(Agent *agent, Config *cfg): DigitizerBase<cfg::device::AlazarDigitizer>(agent,cfg) {}

      unsigned int on_attach() {return 0;}
      unsigned int on_detach() {return 0;}

      virtual void setup(int nrecords, double record_frequency_Hz, double duty);
      virtual size_t record_size(double record_frequency_Hz, double duty);
      virtual size_t nchan();

      double sample_rate();
    };

    ////////////////////////////////////////////////////////////////////////////////
    class SimulatedDigitizer : public DigitizerBase<cfg::device::SimulatedDigitizer>
    {
    public:
      SimulatedDigitizer(Agent *agent) : DigitizerBase<cfg::device::SimulatedDigitizer>(agent) {}
      SimulatedDigitizer(Agent *agent, Config *cfg): DigitizerBase<cfg::device::SimulatedDigitizer>(agent,cfg) {}

      unsigned int on_attach() {return 0;}
      unsigned int on_detach() {return 0;}

      virtual void setup(int nrecords, double record_frequency_Hz, double duty) {}
      size_t SimulatedDigitizer::record_size( double record_frequency_Hz, double duty );
      virtual size_t nchan() {return _config->nchan();}
    };

    ////////////////////////////////////////////////////////////
    class Digitizer:public DigitizerBase<cfg::device::Digitizer>
    {
    public:
      Digitizer(Agent *agent);
      Digitizer(Agent *agent, Config *cfg);
      ~Digitizer();

      virtual unsigned int on_attach();
      virtual unsigned int on_detach();

      void setKind(Config::DigitizerType kind);
      virtual void _set_config(Config IN *cfg);
      virtual void _set_config(const Config &cfg); // only updates the digitizer selected by cfg.kind().

      virtual void setup(int nrecords, double record_frequency_Hz, double duty) {_idigitizer->setup(nrecords,record_frequency_Hz,duty);}

      virtual size_t record_size(double record_frequency_Hz, double duty) {return _idigitizer->record_size(record_frequency_Hz,duty);}
      virtual size_t nchan() {return _idigitizer->nchan();}

      // XXX: These are pretty useless, consider deleting?
      virtual void set_config(const NIScopeDigitizer::Config &cfg);
      virtual void set_config(const AlazarDigitizer::Config &cfg);
      virtual void set_config(const SimulatedDigitizer::Config &cfg);
      virtual void set_config_nowait(const NIScopeDigitizer::Config &cfg);
      virtual void set_config_nowait(const AlazarDigitizer::Config &cfg);
      virtual void set_config_nowait(const SimulatedDigitizer::Config &cfg);

    public:
      NIScopeDigitizer     *_niscope;
      SimulatedDigitizer   *_simulated;
      AlazarDigitizer      *_alazar;
      IDevice              *_idevice;
      IDigitizer           *_idigitizer;
    };
  }
} // namespace fetch
