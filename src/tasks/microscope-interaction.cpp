/*
 * microscope-interaction.cpp
 *
 * Author: Nathan Clack <clackn@janelia.hhmi.org>
 *   Date: Apr 28, 2010
 */
/*
 * Copyright 2010 Howard Hughes Medical Institute.
 * All rights reserved.
 * Use is subject to Janelia Farm Research Campus Software Copyright 1.1
 * license terms (http://license.janelia.org/license/jfrc_copyright_1_1.html).
 */
#pragma once
#include "stdafx.h"
#include "task.h"
#include "tasks\Video.h"
#include "devices\microscope.h"
#include "microscope-interaction.h"

namespace fetch
{ namespace task
  { namespace microscope
    {   
      // Upcasting
      unsigned int Interaction::config(IDevice *d) {return config(dynamic_cast<device::Microscope*>(d));}
      unsigned int Interaction::run   (IDevice *d) {return run   (dynamic_cast<device::Microscope*>(d));}   
      
      
      //
      // Implementation
      //
      
      unsigned int Interaction::config(device::Microscope *dc)
      { 
        static task::scanner::Video<i16> focus;
      
        //Assemble pipeline here
	      IDevice *cur;
        cur =  dc->configPipeline();
	      cur =  dc->trash.apply(cur);
	      
        dc->__scan_agent.arm(&focus,&dc->scanner._scanner2d);
	      //dc->__scan_agent.arm_nowait(&focus,&dc->scanner._scanner2d,INFINITE);
	      
        return 1; //success
      }            

      static int _handle_wait_for_result(int n, DWORD result, const char *msg)
      {
        if(WAIT_OBJECT_0<=result && result<WAIT_OBJECT_0+n)
          return result - WAIT_OBJECT_0;        
        Guarded_Assert_WinErr( result != WAIT_FAILED );
        if(result == WAIT_TIMEOUT)
          warning("Wait timeout\r\n\t%s\r\n", msg);
        warning("Wait %d abandoned\r\n\t%s\r\n",result-WAIT_ABANDONED_0,msg);
        return -1;
      }

      unsigned int Interaction::run(device::Microscope *dc) 
      {
        unsigned int eflag = 0; // success

        Guarded_Assert(dc->__scan_agent.is_runnable());
        //Guarded_Assert(dc->__io_agent.is_running());

        eflag |= (dc->__scan_agent.run()!=1);

        { 
          HANDLE hs[] = {
            dc->__scan_agent._thread,
            dc->__self_agent._notify_stop
          };
          DWORD res;
          int   t;

          // wait for scan to complete (or cancel)
          res = WaitForMultipleObjects(2,hs,FALSE,INFINITE);
          t = _handle_wait_for_result(2,res,"Interaction::run - Wait for scanner to finish.");
          switch(t)
          { 
          case 0:       // in this case, the scanner thread stopped.  Nothing left to do.
            eflag |= 0; // success
            break; 
          case 1:       // in this case, the stop event triggered and must be propagated.
            eflag |= dc->__scan_agent.stop() != 1;
            break;
          default:      // in this case, there was a timeout or abandoned wait
            eflag |= 1; //failure              
          }

        }
        dc->__scan_agent.disarm();
        return eflag;
      }
    }
  }
}
