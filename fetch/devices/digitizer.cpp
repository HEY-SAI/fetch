/*
 * Digitizer.cpp
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
#include "stdafx.h"
#include "../util/util-niscope.h"
#include "digitizer.h"
#include "../asynq.h"

#pragma warning(push)
#pragma warning(disable:4005)
#if 1
#define digitizer_debug(...) debug(__VA_ARGS__)
#else
#define digitizer_debug(...)
#endif
#pragma warning(pop)

#define CheckWarn( expression )  (niscope_chk( this->vi, expression, #expression, &warning ))
#define CheckPanic( expression ) (niscope_chk( this->vi, expression, #expression, &error   ))
#define ViErrChk( expression )    goto_if( CheckWarn(expression), Error )

namespace fetch
{
  namespace device 
  {
    Digitizer::Digitizer(void)
    { 
      // Setup output queues.
      // - Sizes determine initial allocations.
      // - out[0] receives raw data     from each niScope_Fetch call.
      // - out[1] receives the metadata from each niScope_Fetch call.
      //
      size_t Bpp    = 2, //bytes per pixel to initially allocated for
             nbuf[2] = {DIGITIZER_BUFFER_NUM_FRAMES,
                        DIGITIZER_BUFFER_NUM_FRAMES},
               sz[2] = { config.num_records * config.record_length * config.num_channels * Bpp,
                         config.num_records * sizeof(struct niScope_wfmInfo)};
      _alloc_qs( &this->out, 2, nbuf, sz );
    }

    Digitizer::Digitizer(size_t nbuf, size_t nbytes_per_frame, size_t nwfm)
    { size_t Bpp    = 2, //bytes per pixel to initially allocated for
             _nbuf[2] = {nbuf,nbuf},
               sz[2] = {nbytes_per_frame,
                        nwfm*sizeof(struct niScope_wfmInfo)};
      _alloc_qs( &this->out, 2, _nbuf, sz );
    }

  unsigned int
    Digitizer::detach(void)
    { ViStatus status = 1; //error
      
      digitizer_debug("Digitizer: Attempting to detach. vi: %d\r\n", this->vi);
      if( !this->disarm( DIGITIZER_DEFAULT_TIMEOUT ) )
        warning("Could not cleanly disarm digitizer.\r\n");

      this->lock();
      if(this->vi)
        ViErrChk(niScope_close(this->vi));  // Close the session

      status = 0;  // success
      digitizer_debug("Digitizer: Detached.\r\n");
    Error:  
      this->vi = 0;
      this->_is_available = 0;  
      this->unlock();
      return status;
    }

  unsigned int
    Digitizer::attach(void)
    { ViStatus status = VI_SUCCESS;
      this->lock();
      digitizer_debug("Digitizer: Attach\r\n");
      { ViSession *vi = &this->vi;

        if( (*vi) == NULL )
        { // Open the NI-SCOPE instrument handle
          status = CheckPanic (
            niScope_init (this->config.resource_name, 
                          NISCOPE_VAL_TRUE,  // ID Query
                          NISCOPE_VAL_FALSE, // Reset?
                          vi)                // Session
          );
        }
      }
      this->set_available();
      digitizer_debug("\tGot session %3d with status %d\n",this->vi,status);
      this->unlock();

      return status;
    }

  } // namespace fetch
} // namespace fetch