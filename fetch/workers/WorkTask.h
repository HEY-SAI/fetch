/*
 * WorkTask.h
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
/* WorkTask
 * UpdateableWorkTask
 *
 *      WorkTasks are simplified tasks that are used as template parameters
 *      to WorkAgents.  Together WorkAgents and WorkTasks are used to define
 *      transformation pipelines for messages.
 *
 *      WorkAgents and WorkTasks don't require the full state management
 *      afforded by the Agent/Task model.  They play a specialized role.
 *
 *      Usually work tasks don't require any initialization of resources.
 *      Exceptions to this rule would be, for example, tasks that need to
 *      add static data to a buffer that will be used during the run.
 *
 * UpdateableWorkTask
 *
 *      These are used with WorkAgents where the parameters could be adjusted
 *      on the fly by the application.  They should be used with appropriate
 *      WorkTask children.
 *
 * OneToOneWorkTask
 *
 *      These are tasks who define a function, work, that maps a message to a
 *      message.  That is, for every element,e, popped from the source queue, a
 *      single result, work(e), is pushed onto the result queue.
 *
 *      This is a pretty common form, so the run() loop is already written.
 *      Subclasses just need to define the work() function.
 *
 *      See FrameCaster.h for an example.
 */
#pragma once

#include "WorkAgent.h"
#include "../task.h"
#include "../frame.h"

namespace fetch
{

  namespace task
  {

    class WorkTask : public fetch::Task
    { public:
        unsigned int config(WorkAgent *d) {}
    };

    class UpdateableWorkTask : public fetch::UpdateableTask
    { public:
        virtual unsigned int config(WorkAgent *d) {}
        virtual unsigned int update(WorkAgent *d) {}
    };

    class OneToOneWorkTask : public WorkTask
    { public:
                unsigned int run(WorkAgent *d);
        virtual unsigned int work(Message *dst, Message *src);
    };

  }
}
