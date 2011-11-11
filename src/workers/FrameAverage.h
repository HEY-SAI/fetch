/*
 * FrameAverage.h
 *
 *  Created on: Apr 22, 2010
 *      Author: Nathan Clack <clackn@janelia.hhmi.org>
 */

#ifndef FRAMEAVERAGE_H_
#define FRAMEAVERAGE_H_

#include "WorkAgent.h"
#include "WorkTask.h"
#include "workers.pb.h"

namespace fetch
{

  namespace task
  {
   
    class FrameAverage : public WorkTask
    { public:
        unsigned int run(IDevice* dc);
    };
  }
  bool operator==(const cfg::worker::FrameAverage& a, const cfg::worker::FrameAverage& b);
  bool operator!=(const cfg::worker::FrameAverage& a, const cfg::worker::FrameAverage& b);
  namespace worker
  {
    typedef WorkAgent<task::FrameAverage,cfg::worker::FrameAverage> FrameAverageAgent;
  }

}

#endif /* FRAMEAVERAGE_H_ */
