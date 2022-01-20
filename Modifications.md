# Summary of major kernel modifications

### New Global definitions
+ `configUSE_EDFVD_SCHEDULER  1`- Like with `configUSE_EDF_SCHEDULER`, this enables/disables much of the added functionality needed for virtual deadlines.
+ `typedef uint8_t CritType_t;` - Defined in FreeRTOSConfig.h, provides a standard 8-bit type to use for criticality.
+ `#define HI_CRIT  1`		- Simple definition to represent high criticality
+ `#define LO_CRIT  0`		- Simple definition to represent low criticality

### TCB Additions:
+ `CritType_t ucTaskCrit` - Value should be either HI_CRIT or LO_CRIT. This determines if the task's *type* is high-criticality or low-criticality.
+ `CritType_t ucCurrCrit` - Value should be either HI_CRIT or LO_CRIT. This represents the task's *current* criticality setting. While this could be done globally since criticality should always be the same for all tasks, for the sake of fitting into FreeRTOS conventions it made more sense to use a per-task variable in the TCB. 
+ `BaseType_t xLoPeriod`  - Value can be any integer (number of bits determined by port settings). This is a pre-calculated number to be copied into xTaskPeriod when the task is in the low criticality mode (For low-criticality tasks, this is the same as xHiPeriod).
+ `BaseType_t xHiPeriod`  - Value can be any integer (number of bits determined by port settings). This is a pre-calculated number to be copied into xTaskPeriod when the task is in the high criticality mode (For low-criticality tasks, this is the same as xLoPeriod).
+ 
### New functions:
+ `BaseType_t xTaskVirtCreate (TaskFunction_t pxTaskCode,
    							const char * const pcName,
								const configSTACK_DEPTH_TYPE usStackDepth,
								void * const pvParameters,
								UBaseType_t uxPriority,
								TaskHandle_t * const pxCreatedTask,
								TickType_t xPeriod,
								CritType_t ucCrit,
								float xShiftScaler
								);` - Written @ line 880 of `tasks.c`.
  Almost identical to xTaskPeriodicCreate(), but accepts two additional parameters related to
  virtual deadlines. Would additionally handle adding tasks to respective critlists, but that functionality is disabled as explained later.

+ `void prvTaskShiftHi( TaskHandle_t xTask, CritType_t ucNewCrit);` - defined @ line 1900, written @ line 1960 of `tasks.c`.
   Short function to perform criticality shifts on high criticality tasks.
   
+ `void prvTaskShiftLo( TaskHandle_t xTask, CritType_t ucNewCrit);` - defined @ line 1901, written @ line 1975 of `tasks.c`.
   Short function to perform criticality shifts on low criticality tasks.
   
+ `void vListShift( List_t * pxList, CritType_t ucNewCrit );` - written @ line 1905 of `tasks.c`.
   Handle the shifting of an entire list. Currently unused due to issues with creating criticality-specific lists, but still potentially useful.
   
+ `xCritShiftFromISR( TaskHandle_t xTaskToShift, CritType_t ucNewCrit );` - written @ line 2362 of `tasks.c`.
   Heavily work-in-progress, intended to enable criticality shifts in an ISR-safe manner for manual triggering.
   Shifting of high-criticality tasks will hopefully be simple but currently appears to do nothing.
   Shifting of low-criticality tasks is definitely possible going from high -> low, as this is essentially the same as `xTaskResumeFromISR()`. However, suspending tasks from an ISR is highly discouraged, may need alternative approach.
   

### Modified functions:
+ `static void prvInitialiseNewTask( TaskFunction_t pxTaskCode,
                                  const char * const pcName, /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
                                  const uint32_t ulStackDepth,
                                  void * const pvParameters,
                                  UBaseType_t uxPriority,
                                  TaskHandle_t * const pxCreatedTask,
                                  TCB_t * pxNewTCB,
                                  const MemoryRegion_t * const xRegions,
                                  TickType_t xPeriod,
 				  CritType_t ucTaskCrit,
  				  float xShiftScaler
  				  ) PRIVILEGED_FUNCTION;`
  Defined on line 601 of tasks.c, function code found on line 1154.
  Functionality is almost the same as original, but now takes additional virtual deadline parameters and ensures they are assigned to their location in the TCB.

#### Deprecated/Unused:

##### TCB Additions:
+ `BaseType_t xScaleDiv` - this can be any integer. This was used in an effort to have a variable more logically consistent with the SR3 concept, which would be multiplied by xTaskPeriod to perform a criticality shift instead of just copying pre-calculated amounts. However, there really is no benefit to performing criticality shifts like this, so it makes more sense to use the static variables. This will be removed at a later date.

##### New Functions:
+ `void prvAddTaskToCritList( TCB_t * pxTCB )` --- starts on line 394 of `tasks.c`.
  This was intended to be add a task to its correct criticality-specific list.
  Unfortunately there appears to be an error when using vListInsert() like this,
  but the functionality is not really needed anyway so this has been left unused.
  


##### Misc.:
+ `List_t pxHiCritTasks and List_t pxLoCritTasks` --- starts on line 388 of `tasks.c`.
  I originally added these in with the thought that they could be used to
  iterate through just High or just Low criticality tasks if needed.
  This has not come up in practice, but I have left them in in case they
  become useful later.
