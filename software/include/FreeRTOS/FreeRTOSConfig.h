#define DUMMY_PLATFORM

#define configTICK_RATE_HZ 100

#define configMAX_CO_ROUTINE_PRIORITIES 10
#define configMAX_PRIORITIES 20

#define configUSE_PREEMPTION 1
#define configUSE_IDLE_HOOK 1
#define  configUSE_TICK_HOOK 1
#define  configUSE_CO_ROUTINES 1
#define  INCLUDE_vTaskPrioritySet 1
#define  INCLUDE_uxTaskPriorityGet 1
#define  INCLUDE_vTaskDelete 1
#define  INCLUDE_vTaskCleanUpResources 1
#define  INCLUDE_vTaskSuspend 1
#define  INCLUDE_vTaskDelayUntil 1
#define  INCLUDE_vTaskDelay 1
#define  configUSE_16_BIT_TICKS 0
