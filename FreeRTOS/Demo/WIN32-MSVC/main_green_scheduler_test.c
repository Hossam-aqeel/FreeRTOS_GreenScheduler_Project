/*
 * FreeRTOS Green Scheduler Test Demo
 * 
 * This demo creates multiple tasks with different green classes and deadlines
 * to test and demonstrate the energy-aware scheduling features.
 * 
 * Features demonstrated:
 * - Energy-aware task scheduling
 * - Configurable energy weights
 * - Thermal throttling simulation
 * - Battery level awareness
 * - C-State (idle power states) tracking
 * - Deadline management with EDF-like priority boost
 * - Baseline comparison mode
 * 
 * Tasks created:
 * - CriticalTask: High priority, runs frequently (simulates real-time task)
 * - NormalTask: Medium priority, moderate workload
 * - DeferrableTask: Low priority with deadline (can be delayed to save energy)
 * - MonitorTask: Prints energy statistics periodically
 */

/*-----------------------------------------------------------*/
/* Real Hardware Mode - Set to 1 to read actual laptop battery */
/*-----------------------------------------------------------*/
#define USE_REAL_HARDWARE_SENSORS   1

/* Standard includes. */
#include <stdio.h>
#include <conio.h>
#include <time.h>

/* Windows includes for real hardware access */
#include <windows.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

/*-----------------------------------------------------------*/
/* Real Hardware Sensor Functions (Windows API)              */
/*-----------------------------------------------------------*/
#if ( USE_REAL_HARDWARE_SENSORS == 1 )

/* Read real battery level from Windows */
static uint32_t ulGetRealBatteryLevel( void )
{
    SYSTEM_POWER_STATUS powerStatus;
    if( GetSystemPowerStatus( &powerStatus ) )
    {
        if( powerStatus.BatteryLifePercent != 255 )  /* 255 = unknown */
        {
            return ( uint32_t ) powerStatus.BatteryLifePercent;
        }
    }
    return 100;  /* Fallback for desktop PCs without battery */
}

/* Check if running on AC power or battery */
static BaseType_t xIsOnBatteryPower( void )
{
    SYSTEM_POWER_STATUS powerStatus;
    if( GetSystemPowerStatus( &powerStatus ) )
    {
        return ( powerStatus.ACLineStatus == 0 ) ? pdTRUE : pdFALSE;
    }
    return pdFALSE;
}

/* Get remaining battery time in seconds (-1 if unknown/AC) */
static int32_t lGetBatteryTimeRemaining( void )
{
    SYSTEM_POWER_STATUS powerStatus;
    if( GetSystemPowerStatus( &powerStatus ) )
    {
        if( powerStatus.BatteryLifeTime != 0xFFFFFFFF )
        {
            return ( int32_t ) powerStatus.BatteryLifeTime;
        }
    }
    return -1;  /* Unknown */
}

#endif /* USE_REAL_HARDWARE_SENSORS */

/*-----------------------------------------------------------*/
/* External Green Scheduler API declarations */
/*-----------------------------------------------------------*/
#if ( configUSE_GREEN_SCHEDULER == 1 )
extern void vTaskSetDeadline( TaskHandle_t xTask, TickType_t xDeadlineTicks );
extern TickType_t xTaskGetSlackTime( TaskHandle_t xTask );
extern void vTaskSetGreenClass( TaskHandle_t xTask, uint8_t ucClass );
extern void vTaskResetEnergyStats( TaskHandle_t xTask );
extern uint32_t ulTaskGetTotalEnergyConsumed( void );
extern void vTaskGetGreenSchedulerStats( char * pcWriteBuffer, size_t xBufferLength );
extern void vTaskSetPeriod( TaskHandle_t xTask, TickType_t xPeriodTicks );
extern uint32_t ulTaskGetDeadlineMisses( TaskHandle_t xTask );
extern uint32_t ulGetSimulatedTemperature( void );
extern uint32_t ulGetSimulatedBatteryLevel( void );
extern void vSetSimulatedBatteryLevel( uint32_t ulLevel );
extern uint8_t ucGetCurrentCState( void );
extern void vGetCStateStats( uint32_t *pulC0, uint32_t *pulC1, uint32_t *pulC2 );
extern uint32_t ulGetTotalSystemEnergy( void );
extern uint32_t ulGetTotalDeadlineMisses( void );
extern void vResetGreenSchedulerStats( void );
extern void vSetBaselineMode( BaseType_t xEnable );
extern BaseType_t xIsBaselineMode( void );
extern void vTaskGetEnhancedGreenStats( char * pcWriteBuffer, size_t xBufferLength );
#endif

/*-----------------------------------------------------------*/
/* Energy Log File */
/*-----------------------------------------------------------*/
#define ENERGY_LOG_FILE         "energy_log.csv"
#define ENHANCED_LOG_FILE       "enhanced_energy_log.csv"
static FILE *pxEnergyLogFile = NULL;
static FILE *pxEnhancedLogFile = NULL;

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
#define CRITICAL_TASK_PERIOD_MS     pdMS_TO_TICKS( 50 )    /* Faster = more CPU load */
#define NORMAL_TASK_PERIOD_MS       pdMS_TO_TICKS( 100 )
#define DEFERRABLE_TASK_PERIOD_MS   pdMS_TO_TICKS( 200 )
#define MONITOR_TASK_PERIOD_MS      pdMS_TO_TICKS( 3000 )  /* Print stats every 3 seconds */

/*-----------------------------------------------------------*/
/* Workload Intensity - Increase to stress test scheduler */
/*-----------------------------------------------------------*/
#define CRITICAL_WORK_ITERATIONS    50000   /* Heavy CPU work */
#define NORMAL_WORK_ITERATIONS      30000   /* Moderate work */
#define DEFERRABLE_WORK_ITERATIONS  20000   /* Light but noticeable */

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
    volatile double dResult = 1.0;  /* Use floating point for real CPU work */
    
    ( void ) pvParameters;
    
    printf( "[CRITICAL] Task started - Class: CRITICAL, Freq: HIGH\r\n" );
    printf( "           Workload: %d iterations per cycle\r\n", CRITICAL_WORK_ITERATIONS );
    
    xLastWakeTime = xTaskGetTickCount();
    
    for( ;; )
    {
        /* Simulate CPU-intensive work with real computation */
        for( ulWorkCounter = 0; ulWorkCounter < CRITICAL_WORK_ITERATIONS; ulWorkCounter++ )
        {
            /* Real floating-point work that compiler won't optimize away */
            dResult = dResult * 1.000001 + 0.000001;
            if( dResult > 1000000.0 ) dResult = 1.0;
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
    volatile double dResult = 1.0;
    
    ( void ) pvParameters;
    
    printf( "[NORMAL] Task started - Class: NORMAL, Freq: MEDIUM\r\n" );
    printf( "         Workload: %d iterations per cycle\r\n", NORMAL_WORK_ITERATIONS );
    
    xLastWakeTime = xTaskGetTickCount();
    
    for( ;; )
    {
        /* Simulate moderate work with real computation */
        for( ulWorkCounter = 0; ulWorkCounter < NORMAL_WORK_ITERATIONS; ulWorkCounter++ )
        {
            dResult = dResult * 1.000001 + 0.000001;
            if( dResult > 1000000.0 ) dResult = 1.0;
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
    volatile double dResult = 1.0;
    
    ( void ) pvParameters;
    
    printf( "[DEFERRABLE] Task started - Class: DEFERRABLE, Freq: LOW\r\n" );
    printf( "             Workload: %d iterations per cycle\r\n", DEFERRABLE_WORK_ITERATIONS );
    printf( "             Deadline: 300ms (tight!)\r\n" );
    
    /* Set a TIGHT deadline to trigger EDF priority boosting */
    #if ( configUSE_GREEN_SCHEDULER == 1 )
    vTaskSetDeadline( NULL, pdMS_TO_TICKS( 300 ) );  /* Tight deadline! */
    #endif
    
    xLastWakeTime = xTaskGetTickCount();
    
    for( ;; )
    {
        /* Simulate work that takes noticeable time */
        for( ulWorkCounter = 0; ulWorkCounter < DEFERRABLE_WORK_ITERATIONS; ulWorkCounter++ )
        {
            dResult = dResult * 1.000001 + 0.000001;
            if( dResult > 1000000.0 ) dResult = 1.0;
        }
        
        ulDeferrableTaskCounter++;
        
        /* Update deadline for next iteration - keep it tight */
        #if ( configUSE_GREEN_SCHEDULER == 1 )
        vTaskSetDeadline( NULL, pdMS_TO_TICKS( 300 ) );
        #endif
        
        /* Run periodically */
        vTaskDelayUntil( &xLastWakeTime, DEFERRABLE_TASK_PERIOD_MS );
    }
}

/* Monitor Task: Prints energy statistics and logs to file */
static void prvMonitorTask( void *pvParameters )
{
    TickType_t xLastWakeTime;
    static char pcStatsBuffer[ 2048 ];
    uint32_t ulTotalEnergy;
    static uint32_t ulLastTotalEnergy = 0;
    uint32_t ulEnergyDelta;
    static uint32_t ulReportCount = 0;
    
    ( void ) pvParameters;
    
    /* Open energy log file - basic */
    pxEnergyLogFile = fopen( ENERGY_LOG_FILE, "w" );
    if( pxEnergyLogFile != NULL )
    {
        fprintf( pxEnergyLogFile, "Report,Tick,TotalEnergy,EnergyDelta,CriticalIter,NormalIter,DeferrableIter\n" );
        fflush( pxEnergyLogFile );
        printf( "Basic energy log: %s\r\n", ENERGY_LOG_FILE );
    }
    
    /* Open enhanced log file */
    pxEnhancedLogFile = fopen( ENHANCED_LOG_FILE, "w" );
    if( pxEnhancedLogFile != NULL )
    {
        fprintf( pxEnhancedLogFile, "Report,Tick,SystemEnergy,Temperature,Battery,C0_Ticks,C1_Ticks,DeadlineMisses,CriticalIter,DeferrableIter\n" );
        fflush( pxEnhancedLogFile );
        printf( "Enhanced log: %s\r\n", ENHANCED_LOG_FILE );
    }
    
    printf( "\r\n" );
    printf( "========================================\r\n" );
    printf( "  GREEN SCHEDULER ENHANCED TEST DEMO\r\n" );
    printf( "========================================\r\n" );
    printf( "Features enabled:\r\n" );
    printf( "  - Configurable energy weights\r\n" );
    printf( "  - Thermal throttling simulation\r\n" );
    printf( "  - Battery level awareness\r\n" );
    printf( "  - C-State power states tracking\r\n" );
    printf( "  - Deadline management with EDF boost\r\n" );
    #if ( USE_REAL_HARDWARE_SENSORS == 1 )
    printf( "  - REAL HARDWARE SENSORS ENABLED!\r\n" );
    #endif
    printf( "========================================\r\n" );
    printf( "Energy weights: Low=%d, Med=%d, High=%d\r\n", 
            GREEN_ENERGY_WEIGHT_LOW, GREEN_ENERGY_WEIGHT_MED, GREEN_ENERGY_WEIGHT_HIGH );
    printf( "Thermal thresholds: Warn=%d, Crit=%d\r\n",
            GREEN_THERMAL_THRESHOLD_WARN, GREEN_THERMAL_THRESHOLD_CRIT );
    printf( "Battery thresholds: Low=%d%%, Crit=%d%%\r\n",
            GREEN_BATTERY_LOW_THRESHOLD, GREEN_BATTERY_CRITICAL_THRESHOLD );
    #if ( USE_REAL_HARDWARE_SENSORS == 1 )
    printf( "Real Battery:  %lu%%  (Power: %s)\r\n",
            ( unsigned long ) ulGetRealBatteryLevel(),
            xIsOnBatteryPower() ? "BATTERY" : "AC" );
    #endif
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
                    printf( "  Deadline Misses:  %lu\r\n", ( unsigned long ) ulTaskGetDeadlineMisses( xDeferrableTaskHandle ) );
                }
                
                /* Print thermal and battery status */
                printf( "\r\nSystem Status:\r\n" );
                printf( "  Temperature:   %lu / 100\r\n", ( unsigned long ) ulGetSimulatedTemperature() );
                printf( "  Battery Level: %lu%%\r\n", ( unsigned long ) ulGetSimulatedBatteryLevel() );
                #if ( USE_REAL_HARDWARE_SENSORS == 1 )
                {
                    uint32_t ulRealBattery = ulGetRealBatteryLevel();
                    printf( "  REAL Battery:  %lu%% (%s)\r\n", 
                            ( unsigned long ) ulRealBattery,
                            xIsOnBatteryPower() ? "BATTERY" : "AC POWER" );
                    
                    /* Sync real battery to scheduler so it affects task behavior! */
                    vSetSimulatedBatteryLevel( ulRealBattery );
                }
                #endif
                printf( "  C-State:       C%d\r\n", ucGetCurrentCState() );
                
                /* Print C-State stats */
                {
                    uint32_t ulC0, ulC1, ulC2;
                    vGetCStateStats( &ulC0, &ulC1, &ulC2 );
                    printf( "  Time in C0:    %lu ticks (active)\r\n", ( unsigned long ) ulC0 );
                    printf( "  Time in C1:    %lu ticks (idle)\r\n", ( unsigned long ) ulC1 );
                }
                
                /* Print detailed stats */
                printf( "\r\nDetailed Task Energy Stats:\r\n" );
                vTaskGetGreenSchedulerStats( pcStatsBuffer, sizeof( pcStatsBuffer ) );
                printf( "%s", pcStatsBuffer );
                
                /* Log to CSV file - basic */
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
                
                /* Log to enhanced CSV file */
                if( pxEnhancedLogFile != NULL )
                {
                    uint32_t ulC0, ulC1, ulC2;
                    vGetCStateStats( &ulC0, &ulC1, &ulC2 );
                    fprintf( pxEnhancedLogFile, "%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu\n",
                        ( unsigned long ) ulReportCount,
                        ( unsigned long ) xTaskGetTickCount(),
                        ( unsigned long ) ulGetTotalSystemEnergy(),
                        ( unsigned long ) ulGetSimulatedTemperature(),
                        ( unsigned long ) ulGetSimulatedBatteryLevel(),
                        ( unsigned long ) ulC0,
                        ( unsigned long ) ulC1,
                        ( unsigned long ) ulGetTotalDeadlineMisses(),
                        ( unsigned long ) ulCriticalTaskCounter,
                        ( unsigned long ) ulDeferrableTaskCounter );
                    fflush( pxEnhancedLogFile );
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
