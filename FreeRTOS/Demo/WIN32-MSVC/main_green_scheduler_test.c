/*
 * FreeRTOS Green Scheduler Test Demo
 * 
 * This demo creates multiple tasks with different green classes and deadlines
 * to test and demonstrate the energy-aware scheduling features.
 * 
 * Tasks created:
 * - CriticalTask: High priority, runs frequently (simulates real-time task)
 * - NormalTask: Medium priority, moderate workload
 * - DeferrableTask: Low priority with deadline (can be delayed to save energy)
 * - MonitorTask: Prints energy statistics periodically
 */

/* Standard includes. */
#include <stdio.h>
#include <conio.h>
#include <time.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

/*-----------------------------------------------------------*/
/* Energy Log File */
/*-----------------------------------------------------------*/
#define ENERGY_LOG_FILE    "energy_log.csv"
static FILE *pxEnergyLogFile = NULL;

/*-----------------------------------------------------------*/
/* Task Priorities */
/*-----------------------------------------------------------*/
#define CRITICAL_TASK_PRIORITY      ( tskIDLE_PRIORITY + 5 )
#define NORMAL_TASK_PRIORITY        ( tskIDLE_PRIORITY + 3 )
#define DEFERRABLE_TASK_PRIORITY    ( tskIDLE_PRIORITY + 1 )
#define MONITOR_TASK_PRIORITY       ( tskIDLE_PRIORITY + 4 )


/*-----------------------------------------------------------*/
/* Task Periods (in milliseconds) */
/*-----------------------------------------------------------*/
#define CRITICAL_TASK_PERIOD_MS     pdMS_TO_TICKS( 100 )
#define NORMAL_TASK_PERIOD_MS       pdMS_TO_TICKS( 250 )
#define DEFERRABLE_TASK_PERIOD_MS   pdMS_TO_TICKS( 500 )
#define MONITOR_TASK_PERIOD_MS      pdMS_TO_TICKS( 5000 )  /* Print stats every 5 seconds */

/*-----------------------------------------------------------*/
/* Task Handles */
/*-----------------------------------------------------------*/
static TaskHandle_t xCriticalTaskHandle = NULL;
static TaskHandle_t xNormalTaskHandle = NULL;
static TaskHandle_t xDeferrableTaskHandle = NULL;
static TaskHandle_t xMonitorTaskHandle = NULL;

/*-----------------------------------------------------------*/
/* Task counters for workload simulation */
/*-----------------------------------------------------------*/
static volatile uint32_t ulCriticalTaskCounter = 0;
static volatile uint32_t ulNormalTaskCounter = 0;
static volatile uint32_t ulDeferrableTaskCounter = 0;

/*-----------------------------------------------------------*/
/* Task Implementations */
/*-----------------------------------------------------------*/

/* Critical Task: High priority, runs at full frequency */
static void prvCriticalTask( void *pvParameters )
{
    TickType_t xLastWakeTime;
    volatile uint32_t ulWorkCounter;
    
    ( void ) pvParameters;
    
    printf( "[CRITICAL] Task started - Class: CRITICAL, Freq: HIGH\r\n" );
    
    xLastWakeTime = xTaskGetTickCount();
    
    for( ;; )
    {
        /* Simulate CPU-intensive work */
        for( ulWorkCounter = 0; ulWorkCounter < 1000; ulWorkCounter++ )
        {
            /* Busy work */
        }
        
        ulCriticalTaskCounter++;
        
        /* Run periodically */
        vTaskDelayUntil( &xLastWakeTime, CRITICAL_TASK_PERIOD_MS );
    }
}

/* Normal Task: Medium priority, moderate workload */
static void prvNormalTask( void *pvParameters )
{
    TickType_t xLastWakeTime;
    volatile uint32_t ulWorkCounter;
    
    ( void ) pvParameters;
    
    printf( "[NORMAL] Task started - Class: NORMAL, Freq: MEDIUM\r\n" );
    
    xLastWakeTime = xTaskGetTickCount();
    
    for( ;; )
    {
        /* Simulate moderate work */
        for( ulWorkCounter = 0; ulWorkCounter < 500; ulWorkCounter++ )
        {
            /* Busy work */
        }
        
        ulNormalTaskCounter++;
        
        /* Run periodically */
        vTaskDelayUntil( &xLastWakeTime, NORMAL_TASK_PERIOD_MS );
    }
}

/* Deferrable Task: Low priority, has deadline, can run at low frequency */
static void prvDeferrableTask( void *pvParameters )
{
    TickType_t xLastWakeTime;
    volatile uint32_t ulWorkCounter;
    
    ( void ) pvParameters;
    
    printf( "[DEFERRABLE] Task started - Class: DEFERRABLE, Freq: LOW\r\n" );
    
    /* Set a deadline for this task (1 second from now) */
    #if ( configUSE_GREEN_SCHEDULER == 1 )
    vTaskSetDeadline( NULL, pdMS_TO_TICKS( 1000 ) );
    #endif
    
    xLastWakeTime = xTaskGetTickCount();
    
    for( ;; )
    {
        /* Simulate light work (can be run at low frequency) */
        for( ulWorkCounter = 0; ulWorkCounter < 200; ulWorkCounter++ )
        {
            /* Busy work */
        }
        
        ulDeferrableTaskCounter++;
        
        /* Update deadline for next iteration */
        #if ( configUSE_GREEN_SCHEDULER == 1 )
        vTaskSetDeadline( NULL, pdMS_TO_TICKS( 1000 ) );
        #endif
        
        /* Run periodically */
        vTaskDelayUntil( &xLastWakeTime, DEFERRABLE_TASK_PERIOD_MS );
    }
}

/* Monitor Task: Prints energy statistics and logs to file */
static void prvMonitorTask( void *pvParameters )
{
    TickType_t xLastWakeTime;
    static char pcStatsBuffer[ 1024 ];
    uint32_t ulTotalEnergy;
    static uint32_t ulLastTotalEnergy = 0;
    uint32_t ulEnergyDelta;
    static uint32_t ulReportCount = 0;
    time_t xCurrentTime;
    
    ( void ) pvParameters;
    
    /* Open energy log file */
    pxEnergyLogFile = fopen( ENERGY_LOG_FILE, "w" );
    if( pxEnergyLogFile != NULL )
    {
        fprintf( pxEnergyLogFile, "Report,Tick,TotalEnergy,EnergyDelta,CriticalIter,NormalIter,DeferrableIter\n" );
        fflush( pxEnergyLogFile );
        printf( "Energy log file created: %s\r\n", ENERGY_LOG_FILE );
    }
    
    printf( "\r\n" );
    printf( "========================================\r\n" );
    printf( "  GREEN SCHEDULER TEST DEMO STARTED\r\n" );
    printf( "========================================\r\n" );
    printf( "Tasks created with different energy classes.\r\n" );
    printf( "Energy stats will be printed every 5 seconds.\r\n" );
    printf( "Energy data logged to: %s\r\n", ENERGY_LOG_FILE );
    printf( "========================================\r\n\r\n" );
    
    xLastWakeTime = xTaskGetTickCount();
    
    for( ;; )
    {
        /* Wait for next reporting period */
        vTaskDelayUntil( &xLastWakeTime, MONITOR_TASK_PERIOD_MS );
        
        ulReportCount++;
        
        taskENTER_CRITICAL();
        {
            printf( "\r\n" );
            printf( "============ REPORT #%lu (Tick: %lu) ============\r\n", 
                    ( unsigned long ) ulReportCount, 
                    ( unsigned long ) xTaskGetTickCount() );
            
            /* Print task execution counts */
            printf( "\r\nTask Execution Counts:\r\n" );
            printf( "  Critical Task:   %lu iterations\r\n", ( unsigned long ) ulCriticalTaskCounter );
            printf( "  Normal Task:     %lu iterations\r\n", ( unsigned long ) ulNormalTaskCounter );
            printf( "  Deferrable Task: %lu iterations\r\n", ( unsigned long ) ulDeferrableTaskCounter );
            
            #if ( configUSE_GREEN_SCHEDULER == 1 )
            {
                /* Get total energy consumed */
                ulTotalEnergy = ulTaskGetTotalEnergyConsumed();
                ulEnergyDelta = ulTotalEnergy - ulLastTotalEnergy;
                ulLastTotalEnergy = ulTotalEnergy;
                
                printf( "\r\nEnergy Consumption:\r\n" );
                printf( "  Total Energy:    %lu units\r\n", ( unsigned long ) ulTotalEnergy );
                printf( "  Energy Delta:    %lu units (since last report)\r\n", ( unsigned long ) ulEnergyDelta );
                
                /* Print slack time for deferrable task */
                if( xDeferrableTaskHandle != NULL )
                {
                    TickType_t xSlack = xTaskGetSlackTime( xDeferrableTaskHandle );
                    printf( "  Deferrable Slack: %lu ticks\r\n", ( unsigned long ) xSlack );
                }
                
                /* Print detailed stats */
                printf( "\r\nDetailed Task Energy Stats:\r\n" );
                vTaskGetGreenSchedulerStats( pcStatsBuffer, sizeof( pcStatsBuffer ) );
                printf( "%s", pcStatsBuffer );
                
                /* Log to CSV file */
                if( pxEnergyLogFile != NULL )
                {
                    fprintf( pxEnergyLogFile, "%lu,%lu,%lu,%lu,%lu,%lu,%lu\n",
                        ( unsigned long ) ulReportCount,
                        ( unsigned long ) xTaskGetTickCount(),
                        ( unsigned long ) ulTotalEnergy,
                        ( unsigned long ) ulEnergyDelta,
                        ( unsigned long ) ulCriticalTaskCounter,
                        ( unsigned long ) ulNormalTaskCounter,
                        ( unsigned long ) ulDeferrableTaskCounter );
                    fflush( pxEnergyLogFile );
                }
            }
            #else
            {
                printf( "\r\n[Green Scheduler is DISABLED]\r\n" );
            }
            #endif
            
            printf( "================================================\r\n" );
        }
        taskEXIT_CRITICAL();
    }
    
    /* Should not reach here */
    vTaskDelete( NULL );
}

/*-----------------------------------------------------------*/
/* Main Entry Point */
/*-----------------------------------------------------------*/

void main_green_scheduler_test( void )
{
    printf( "\r\n" );
    printf( "================================================\r\n" );
    printf( "   FreeRTOS GREEN SCHEDULER TEST\r\n" );
    printf( "================================================\r\n" );
    printf( "Initializing tasks...\r\n\r\n" );
    
    /* Create Critical Task (High priority = GREEN_CLASS_CRITICAL) */
    xTaskCreate( prvCriticalTask,
                 "Critical",
                 configMINIMAL_STACK_SIZE * 2,
                 NULL,
                 CRITICAL_TASK_PRIORITY,
                 &xCriticalTaskHandle );
    
    /* Create Normal Task (Medium priority = GREEN_CLASS_NORMAL) */
    xTaskCreate( prvNormalTask,
                 "Normal",
                 configMINIMAL_STACK_SIZE * 2,
                 NULL,
                 NORMAL_TASK_PRIORITY,
                 &xNormalTaskHandle );
    
    /* Create Deferrable Task (Low priority = GREEN_CLASS_DEFERRABLE) */
    xTaskCreate( prvDeferrableTask,
                 "Deferrable",
                 configMINIMAL_STACK_SIZE * 2,
                 NULL,
                 DEFERRABLE_TASK_PRIORITY,
                 &xDeferrableTaskHandle );
    
    /* Create Monitor Task */
    xTaskCreate( prvMonitorTask,
                 "Monitor",
                 configMINIMAL_STACK_SIZE * 4,
                 NULL,
                 MONITOR_TASK_PRIORITY,
                 &xMonitorTaskHandle );
    
    /* Explicitly set green classes (in case priority-based assignment differs) */
    #if ( configUSE_GREEN_SCHEDULER == 1 )
    vTaskSetGreenClass( xCriticalTaskHandle, GREEN_CLASS_CRITICAL );
    vTaskSetGreenClass( xNormalTaskHandle, GREEN_CLASS_NORMAL );
    vTaskSetGreenClass( xDeferrableTaskHandle, GREEN_CLASS_DEFERRABLE );
    #endif
    
    printf( "All tasks created. Starting scheduler...\r\n\r\n" );
    
    /* Start the scheduler */
    vTaskStartScheduler();
    
    /* Should never reach here */
    for( ;; );
}
