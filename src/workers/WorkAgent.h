/*
 * WorkAgent.h
 *
 *  Created on: Apr 22, 2010
 *      Author: Nathan Clack <clackn@janelia.hhmi.org>
 */
/*
 * Copyright 2010 Howard Hughes Medical Institute.
 * All rights reserved.
 * Use is subject to Janelia Farm Research Campus Software Copyright 1.1
 * license terms (http://license.janelia.org/license/jfrc_copyright_1_1.html).
 */
/*
 * WorkAgent<TWorkTask,TConfig=void*>
 * ---------------------------------
 *
 * TWorkTask
 *  - must be a child of class Task.
 *  - must implement alloc_output_queues(Agent*)
 *
 * Notes
 * -----
 * Privately inherits Agent.  The idea is that state manipulation is delegated
 * to construction/destruction.  There doesn't need to be any outside access
 * to state manipulation functions.
 *
 *
 */
#pragma once

#include "agent.h"

#define WORKER_DEFAULT_TIMEOUT INFINITE

namespace fetch
{
  /*
   * Example:
   *
   * device::Scanner2D src;
   * WorkAgent<task::FrameAverager,int> step1(&src,  // producer Agent
   *                                             0,  // x so that step1->in[0] is src->out[x]
   *                                             4); // times to average
   *
   *
   * ...or...
   *
   * device::Scanner2D src;
   * WorkAgent<task::FrameAverager,int> step1(2);     // times to average   
   * WorkAgent<task::PixelWiseAverager,int> step2(4); // times to average
   * Agent *last = step2.apply(step1.apply(&src,0));
   */
  template<typename TWorkTask, typename TConfig=void*>
  class WorkAgent : public IConfigurableDevice<TConfig>
  { 
  public:
    typedef TWorkTask TaskType;

    WorkAgent();                                           // Will configure only.  Use apply() to connect, arm, and run.  config is set to TConfig().
    WorkAgent(TConfig *config);                            // Will configure only.  Use apply() to connect, arm, and run.
    WorkAgent(IDevice *source, int ichan, TConfig *config);  // Will connect, configure, arm, and run

    WorkAgent<TWorkTask,TConfig>* apply(IDevice *source, int ichan=0); // returns <this>

    unsigned int attach(void);
    unsigned int detach(void);

  public: //data
    TWorkTask __task_instance;
    Agent     __agent_instance;
  };

  ////////////////////////////////////////////////////////////////////////////
  //
  // Implementation
  //
  ////////////////////////////////////////////////////////////////////////////
  
  template<typename TWorkTask,typename TConfig>
  WorkAgent<TWorkTask,TConfig>::WorkAgent()
    :IConfigurableDevice<TConfig>(&__agent_instance)
    ,__agent_instance(NULL)    
  {
    __agent_instance._owner = this;
    _agent->attach(); 
  }

  template<typename TWorkTask,typename TConfig>
  WorkAgent<TWorkTask,TConfig>::WorkAgent(TConfig *config)
    :IConfigurableDevice<TConfig>(&__agent_instance,config)
    ,__agent_instance(NULL)
  { 
    __agent_instance._owner = this;
    _agent->attach(); 
  }

  template<typename TWorkTask,typename TConfig>
  WorkAgent<TWorkTask,TConfig>::WorkAgent(IDevice *source, int ichan, TConfig *config)
    :IConfigurableDevice<TConfig>(&__agent_instance,config)
    ,__agent_instance(this)
  { 
    _agent->attach();
    apply(source,ichan);
  }
  
  template<typename TWorkTask,typename TConfig>
  unsigned int WorkAgent<TWorkTask,TConfig>::
  attach(void) 
  {
    return 0; /*0 success, 1 failure*/
  }
  
  template<typename TWorkTask,typename TConfig>
  unsigned int WorkAgent<TWorkTask,TConfig>::
  detach(void) 
  { debug("Attempting WorkAgent::detach() for task at 0x%p.\r\n",&this->__task_instance);    
    return 0; /*0 success, 1 failure*/
  }
  
  template<typename TWorkTask,typename TConfig>
  WorkAgent<TWorkTask,TConfig>*
  WorkAgent<TWorkTask,TConfig>::apply(IDevice *source, int ichan)
  { 
    connect(this,ichan,source,ichan);
    if( _out==NULL )
      __task_instance.alloc_output_queues(this);// Task must implement this.  Must connect() first.  WorkTask has a default impl. that assumes in[0]->out[0].  These should handle pre-existing queues (by freecycling).
    Guarded_Assert(_agent->disarm(WORKER_DEFAULT_TIMEOUT));
    Guarded_Assert(_agent->arm(&__task_instance,this,WORKER_DEFAULT_TIMEOUT));
    Guarded_Assert(_agent->run());
    return this;
  }

}
