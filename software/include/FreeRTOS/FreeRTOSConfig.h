#define DUMMY_PLATFORM

#define configUSE_PREEMPTION 	 1
#define configUSE_IDLE_HOOK 	 0
#define configMAX_PRIORITIES 	 ((unsigned portBASE_TYPE )5)
#define configUSE_TICK_HOOK 	  0
#define configCPU_CLOCK_HZ       ((unsigned long) 100000000)    // 100MHz
#define configTICK_RATE_HZ       ((portTickType) 1000)      // 1 Tick every 1ms
#define configMINIMAL_STACK_SIZE ((unsigned short) 80)
#define configTOTAL_HEAP_SIZE    ((size_t) ( 19 * 1024 ))
#define configMAX_TASK_NAME_LEN  (12)
#define configUSE_TRACE_FACILITY 1
#define configUSE_16_BIT_TICKS   0
#define configIDLE_SHOULD_YIELD  0
#define configUSE_CO_ROUTINES    0
#define configUSE_MUTEXES      	 1

#define configMAX_CO_ROUTINE_PRIORITIES (2)

#define configUSE_COUNTING_SEMAPHORES   1
#define configUSE_ALTERNATIVE_API     0
#define configCHECK_FOR_STACK_OVERFLOW  0
#define configUSE_RECURSIVE_MUTEXES    1
#define configQUEUE_REGISTRY_SIZE    10
#define configGENERATE_RUN_TIME_STATS  0

#define INCLUDE_vTaskPrioritySet 1
#define INCLUDE_uxTaskPriorityGet 1
#define INCLUDE_vTaskDelete 1
#define INCLUDE_vTaskCleanUpResources 0
#define INCLUDE_vTaskSuspend 1
#define INCLUDE_vTaskDelayUntil 1
#define INCLUDE_vTaskDelay 1
#define INCLUDE_uxTaskGetStackHighWaterMark  1

/* Use the system definition, if there is one */
#ifdef __NVIC_PRIO_BITS
  #define configPRIO_BITS       __NVIC_PRIO_BITS
#else
  #define configPRIO_BITS       5        /* 32 priority levels */
#endif

/* The lowest priority. */
#define configKERNEL_INTERRUPT_PRIORITY   ( 31 << (8 - configPRIO_BITS) ) //248
/* Priority 5, or 160 as only the top three bits are implemented. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY   ( 5 << (8 - configPRIO_BITS) )
