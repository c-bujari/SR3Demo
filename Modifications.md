Additions to the TCB:
+ 

New functions:
+ BaseType_t xTaskVirtCreate (TaskFunction_t pxTaskCode,
    							const char * const pcName,
								const configSTACK_DEPTH_TYPE usStackDepth,
								void * const pvParameters,
								UBaseType_t uxPriority,
								TaskHandle_t * const pxCreatedTask,
								TickType_t xPeriod,
								CritType_t ucCrit,
								float xShiftScaler
								);
  --- Code starts line 880 ---
  Almost identical to xTaskPeriodicCreate(), but accepts two additional parameters related to
  virtual deadlines.

Modified functions:
+ static void prvInitialiseNewTask( TaskFunction_t pxTaskCode,
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
  								  ) PRIVILEGED_FUNCTION;
  --- Definition on line 601, code starts line  ---
  Functionality is almost the same as original, but now takes additional virtual deadline parameters.

CURRENTLY UNUSED ADDITIONS:

Functions:
+ void prvAddTaskToCritList( TCB_t * pxTCB ) --- starts on line 394
  This was intended to be add a task to its correct criticality-specific list.
  Unfortunately there appears to be an error when using vListInsert() like this,
  but the functionality is not really needed anyway so this has been left unused.

Misc.:
+ List_t pxHiCritTasks and List_t pxLoCritTasks --- starts on line 388
  I originally added these in with the thought that they could be used to
  iterate through just High or just Low criticality tasks if needed.
  This has not come up in practice, but I have left them in in case they
  become useful later.