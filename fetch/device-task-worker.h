#pragma once
#include "stdafx.h"
#include "device-task.h"
#include "types.h"

DeviceTask* Worker_Create_Task_Averager_f32(Device *d, unsigned int ntimes);
DeviceTask* Worker_Create_Task_Caster(Device *d, Basic_Type_ID source_type, Basic_Type_ID dest_type);