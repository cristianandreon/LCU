//============================================================================
// Name        : ControlUnit.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Entry point, C++, Ansi-style
//============================================================================

#define EXTERN


/////////////////////////
// RT Kernel includes
//
#ifdef WATCOM
#include "FreeRTOS.h"
#include "task.h"
#else
#include "./../RTLinux/RTLinux.h"
#endif







// Avviso sonoro errore
#define ERROR_SOUND \
    for (int __i=0; __i<15000; __i++) {\
    	/*sound (250+__i/10); */ \
        }\
    /* nosound() */;






// Modalita denug
// #define DEBUG_PRINTF




#define ENABLE_RTCOMM
#define ENABLE_HTTP_SERVICE


// #define DELETE_TASKS
// #define TERMINATE_AS_DEBUG


#define DEBUG_RT_TASK



// Test
// #define STATIC_ALLOCATE_TASK



// Colori printf NON suportati
// #define KRED  ""





// The tasks as described in the comments at the top of this file.
static void *prvLogic(void *pvParameters);
static void *prvComm(void *pvParameters);
static void *prvHTTPServices(void *pvParameters);
static void *prvWatchDog(void *pvParameters);





// Visualizzazione messaggi

void vDisplayMessage(const char * const msg) {
    // disable_ints();
    // vTaskSuspendAll();
    fprintf(stderr, (char*) msg, 0);
    fflush(stderr);

    addToConsole((char*) msg);

    // xTaskResumeAll();
    // enable_ints();
}



// Test startup
void test_routine();



///////////////////////////////////////
// Routine terminazione programma
//

void terminate_program(bool bExit) {

    fprintf(stderr, "\n[XP] Program terminating\n");
    fflush(stdout);

    // Mette in stop i servo motori
    process_actuators_terminate();

    fprintf(stderr, "Stopping AC Servo(s)...\n");
    fflush(stdout);

    usleep(1000 * 1000);

    fprintf(stderr, "Stopping Communications...\n");
    fflush(stdout);

    // chiusura comunicazioni
    dataExchangeClose();

    // Chisura servizio HTTTP
    HTTPServerClose();

    uint32_t t0 = xTaskGetTickCount();

    if (App.RTCRunning || App.HTTPRunning || App.SerialRunning || App.CANRunning) {
        while (App.RTCRunning && App.HTTPRunning) {
            if (xTaskGetTickCount() - t0 > 10 * 1000) {
                break;
            }
        }
    }

    if (App.RTCRunning || App.HTTPRunning || App.SerialRunning || App.CANRunning) {
        fprintf(stderr, "\n[XP] RTC:%d - HTTP:%d - SER:%d - CAN:%d - Force closing...\n", App.RTCRunning, App.HTTPRunning,  App.SerialRunning,  App.CANRunning);
        fflush(stdout);
        // causa l'uscita dal display di info utili
        dataExchangeDump(App.Msg, App.MsgSize);
    }


    ////////////////////////////////////
    // Scrittura impostazoni persistenti
    //
    fprintf(stderr, "\n[XP] Writing settigns...");
    if (write_settings() < 0) {
    } else {
        fprintf(stderr, "%sOK%s\n", (char*) ANSI_COLOR_GREEN, (char*) ANSI_COLOR_RESET);
    }


    if (bExit)
        exit(1);
}

void my_handler(int s) {
    App.TerminateProgram = 1;
}













////////////////////////////////////
// Funzione avvio dei task
//

#define PRINT_VERSION() \
    snprintf(App.Msg, App.MsgSize, "%s[XP] xProject RealTime v%d.%d - 32bit - %s\nCopyright 2017 xRT by Cristian Andreon. All rights reserved\n", (char*)ANSI_COLOR_RESET, App.MajVer, App.MinVer, __DATE__ ); vDisplayMessage(App.Msg);\
    snprintf(App.Msg, App.MsgSize, "     %s %s%s%s\n", App.KernelString , (App.DebugMode?(char*)ANSI_COLOR_YELLOW:(char*)""), App.DebugMode?"[DEBUG MODE]":"", (App.DebugMode?(char*)ANSI_COLOR_RESET:(char*)"")); vDisplayMessage(App.Msg);


int main(int argc, char *argv[]) {
    int res = 0;


    memset(&App, 0, sizeof(App));

    App.RTRun = 1;
    App.RTCommRun = 1;
    App.SimulateMode = 1;
    

    ////////////
    // DEBUG
    // 
    App.DebugMode = true;
    
    // test_routine ();
    
    
    App.ExecFilePath[0] = 0;
    if (argv && argv[0] && argc >= 1) {
        COPY_ARRAY(App.ExecFilePath, argv[0]);
    }

    App.MsgSize = 1024;
    App.Msg = (char*) calloc(App.MsgSize, 1);
    App.TrackPacketRecived = 0;

    
    ////////////////////////////////////
    // Versione Firmware / programma
    //
    App.MajVer = 2;
    App.MinVer = 17;


    App.HTTPRunLoop = 1;
    App.HTTPIDLECounter = 0;

    App.HTTPLListeningPort = (uint16_t)8073;
        
    App.UIRunLoop = 1;

    
    // Buffer UI
    App.UIRecvBufferSize = (uint32_t) RECV_BUFFER_SIZE;
    App.UIRecvBuffer = (uint8_t *) calloc(App.UIRecvBufferSize, 1);

    App.UISendBufferSize = 32000;
    App.UISendBuffer = (uint8_t*) calloc(App.UISendBufferSize + 1, 1);

    
    // Param Seriale
    App.SERRunLoop = 1;
    App.SERReadPosIDLE = true;
            
    // Param CANBUS
    App.CANRunLoop = 1;    
    App.CANReadPosIDLE = true;

    // Attesa ciclo di logica
    App.LogicWaitUSec = 700; 
    App.LogicMinTimeNanoSec = 1000000;
    
    
    // Gestione del comnado di avvio in pacchetto unico ETH
    App.CanBusUseFullCommand = true;
    App.CanBusFullCmdFeedbackTimeoutMS = 50;
    App.CanBusFullCmdMacAttemps = 3;
    
    App.CanBusStreamTimeoutMS = CANBUS_STREAM_TIMEOUT_MS;

    // Versione del CanOpen
    App.CanOpenVersion = 0x0B;
    
    
    App.RowSep[0] = '\1';
    App.RowSep[1] = '\0';

    
    // Inizializzazione modulo runtime linux
    xrt_init();


    // Inizializzazione buffer della console
    setupConsole();

    

    vDisplayMessage("\r\n\r\n");
    
#ifdef xBM_COMPILE
    snprintf(App.Msg, App.MsgSize, "%s[%sxBM%s : Blow moulding Machine - Written by Cristian Andreon. ]\n", (char*)ANSI_COLOR_RESET, (char*)ANSI_COLOR_BLUE, (char*)ANSI_COLOR_RESET); 
    vDisplayMessage(App.Msg);
#elif xCNC_COMPILE
    snprintf(App.Msg, App.MsgSize, "%s[%sxCNC%s : GCode to Machine - Machine Written by Cristian Andreon. ]\n", (char*)ANSI_COLOR_RESET, (char*)ANSI_COLOR_BLUE, (char*)ANSI_COLOR_RESET); 
    vDisplayMessage(App.Msg);
#endif
    
    PRINT_VERSION();

    
    
    


    //////////////////////////////////
    // Aggancia il segnale CRTL-C
    //
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);




    ///////////////////////////////////////////////////
    // Inizializza il gestione della logica macchina
    //
    if ((res = logic_init()) < 0) {
        fprintf(stderr, "[XP] !logic_init:%d\n", res); fflush(stderr);
    }



    ////////////////////////////////////
    // Lettura impostazoni persistenti
    //
    fprintf(stderr, "\n[XP] Reading settigns...");
    if (read_settings() < 0) {
    } else {
        fprintf(stderr, "%sOK%s\n", (char*) ANSI_COLOR_GREEN, (char*) ANSI_COLOR_RESET);  fflush(stderr);
    }


    
    
    

    ///////////////////////////////////////////////////
    // Post Inizializza il gestione della logica macchina
    //
    if ((res = logic_post_init()) < 0) {
        fprintf(stderr, "[XP] !logic_post_init:%d\n", res); fflush(stderr);
    }

    
    float x = sin( 0.35 / 180.0f * PIGRECO );
    x = sin( 0.35 );
    x = cos( 0.35 / 180.0f * PIGRECO );
    x = cos( 0.35 );
    x = x * 1.0f;
    
   /*
    */
            
    /*
     * TEST calcolo lunghezza archi
    float arcLen = cal_arc_length( 91.0 * PIGRECO / 180.0f, 90.0 * PIGRECO / 180.0f, 60.0f, 0);
    int32_t PrecisionNSteps = arcLen / 0.05f;
    fprintf(stderr, "[XP] !arcLen:%0.3f - steps:%d\n", arcLen, PrecisionNSteps); fflush(stderr);
    */
                                    
    
    /*
    // Inizializza la variabile dipendente dal ciclo automatico
    format_time_run(machine.statistic.machine_running, (char*)machine.statistic.machine_running_string, sizeof(machine.statistic.machine_running_string));
    format_time_run(machine.rt_statistic.machine_running, (char*)machine.rt_statistic.machine_running_string, sizeof(machine.rt_statistic.machine_running_string));
    */

    
    
    
    /* TEST Conversione mm/gradi <-> Unita' encoder
     * 
    LP_ACTUATOR pActuator = &machine.actuator[MOLD];
    int32_t targetTurnsPPT, targetPulsesPPT, targetTurnsPPT2, targetPulses2PPT, targetTurnsPPTForw, targetPulsesForw, offsetTurnsPPT, offsetPulsesPPT, driverTurnsPPT = 0, driverPulsesPPT = 0;


    

    pActuator->homing_offset_mm = 17.15f;
    actuator_position_to_encoder( (void *)pActuator, pActuator->homing_offset_mm, &offsetTurnsPPT, &offsetPulsesPPT);
                                       

    pActuator->homingPulsesPPT = driverPulsesPPT;
    pActuator->homingTurnsPPT = driverTurnsPPT;
    pActuator->homingDone = true;           

    // Aggiunta alle pulsazioni correnti dell'offset
    actuator_add_pulse_ppt( (void *)pActuator, offsetTurnsPPT, offsetPulsesPPT, &pActuator->homingTurnsPPT, &pActuator->homingPulsesPPT, 0+0);
                    


    float newPosition = 0.0f;                    
    actuator_encoder_to_position((void *) pActuator, (int32_t) driverTurnsPPT, (int32_t) driverPulsesPPT, &newPosition);
    actuator_handle_read_position((void *) pActuator, newPosition, false);

                    
                    
                    
    pActuator->homingTurnsPPT = 1;
    pActuator->homingPulsesPPT = 4;

    
    actuator_encoder_to_position((void *) pActuator, (int32_t) driverTurnsPPT, (int32_t) driverPulsesPPT, &newPosition);
    
    
    // Unità encoder inizio e fine
    actuator_position_to_encoder( pActuator, (pActuator->start_rpos), &targetTurnsPPT, &targetPulsesPPT);
    actuator_position_to_encoder( pActuator, (pActuator->end_rpos), &targetTurnsPPT2, &targetPulses2PPT);
            
    // Corsa in avanti in unità encoder
    actuator_delta_pulse(pActuator, targetTurnsPPT2, targetTurnsPPT, targetPulses2PPT, targetPulsesPPT, &targetTurnsPPTForw, &targetPulsesForw, 0+0);

    fprintf(stderr, "[To Forward pulse : %d.%d\n", targetTurnsPPTForw, targetPulsesForw); fflush(stderr);

    
    */


    
/*
    {   int32_t targetTurnsPPT = 0, targetPulses = 0, targetTurnsPPT2 = 0, targetPulses2PPT = 0;

        machine.actuator[STRETCH].cur_rpos = 0.06;
        
        actuator_position_to_encoder(&machine.actuator[STRETCH], machine.actuator[STRETCH].end_rpos, &targetTurnsPPT, &targetPulses);                   
        actuator_position_to_encoder(&machine.actuator[STRETCH], machine.actuator[STRETCH].cur_rpos, &targetTurnsPPT2, &targetPulses2PPT);
        
        int32_t cposition = (targetTurnsPPT - targetTurnsPPT2) * (int32_t)machine.actuator[STRETCH].pulsesPerTurn + (targetPulses - targetPulses2PPT);

        machine.actuator[STRETCH].cur_rpos = 0.03;
        
        actuator_position_to_encoder(&machine.actuator[STRETCH], machine.actuator[STRETCH].end_rpos, &targetTurnsPPT, &targetPulses);                   
        actuator_position_to_encoder(&machine.actuator[STRETCH], machine.actuator[STRETCH].cur_rpos, &targetTurnsPPT2, &targetPulses2PPT);
        
        cposition = (targetTurnsPPT - targetTurnsPPT2) * machine.actuator[STRETCH].pulsesPerTurn + (targetPulses - targetPulses2PPT);
        
        machine.actuator[STRETCH].cur_rpos = -0.03;
        
        actuator_position_to_encoder(&machine.actuator[STRETCH], machine.actuator[STRETCH].end_rpos, &targetTurnsPPT, &targetPulses);                   
        actuator_position_to_encoder(&machine.actuator[STRETCH], machine.actuator[STRETCH].cur_rpos, &targetTurnsPPT2, &targetPulses2PPT);
        
        cposition = (targetTurnsPPT - targetTurnsPPT2) * (int32_t)machine.actuator[STRETCH].pulsesPerTurn + (targetPulses - targetPulses2PPT);
        
        cposition = 0;
        
       }
*/
    
    
    // Test
    // format_time_run( (uint32_t) 3600*24*33, App.Msg, App.MsgSize);




    

    ///////////////////////////////////////////////////////////////////
    // Avvio del task HTTP Services
    //  N.B.: Avvio prima dell'inizializzazione
    //          Per stabilire un canale per la disgnostica
    //      La funzione di INIT pone lo stato (App.HTTPServerState = STATE_INIT)
    //      Abilitanto il servizio HTTP
    //

    
    

    
#ifdef ENABLE_HTTP_SERVICE


    /////////////////////////////////////////////////////////////////////////////////
    //
    // Modulo HTTP Server
    //
    // Mode & 1 ->  Debug Mode
    //
    if (HTTPServerInit(0+0) <= 0) {
        fprintf(stderr, "\n[!HTTPServer]\n");
        ERROR_SOUND
    } else {    
        if (xrt_pthread_create(&App.HTTPServicesThreadId, NULL, &prvHTTPServices, NULL) == 0) {
        } else {
            ERROR_SOUND
            fprintf(stderr, "[!HTTP...Continue?\n"); fflush(stderr);
            if (getch() == 'y') {
            } else {
                xrt_reboot();
                return -1;
            }
        }
    }

#endif


    
    
    
    
    
    ////////////////////////////////////////////
    // Inizializza il gestione comunicazioni       
    // Mode & 1 ->  Debug Mode
    //
    
#ifdef ENABLE_RTCOMM
    
    App.RTCommInit = dataExchangeInit(0 + 1);
    if (App.RTCommInit < 0) {
        fprintf(stderr, "[RTC] Init error:%d\n", (int) App.RTCommInit);  fflush(stderr);
        // return (int)-1;
    }
    
#endif




    ///////////////////////////////////////////////////////////////
    // Inizializza la cache della variabili (risporta alla UI)
    //
    // fprintf (stderr, "[XP] initVarToCache...\n"); fflush(stderr);
    if (initVarToCache() < 0) {
        fprintf(stderr, "[XP] !initVarToCache");
        return (int) - 1;
    }




    // Start the tasks and timer running.
    // printf ("[XP] Start Scheduler [timer size:%d]...\n", sizeof(portTickType) ); fflush(stdout);

    ////////////////////////////////////
    // Avvio del task principale
    //


prvLogicStart:

    //
    if (xrt_pthread_create(&App.LogicThreadId, NULL, &prvLogic, NULL) == 0) {
    } else {
        ERROR_SOUND
        fprintf(stderr, "[!RT...Continue?\n");
        fflush(stderr);
        if (getch() == 'y') {
        } else {
            xrt_reboot();
            return -1;
        }
    }


    ///////////////////////////////////////////////
    // Avvio del task gestione comunicazione
    //

prvCommStart:

    if (xrt_pthread_create(&App.CommThreadId, NULL, &prvComm, NULL) == 0) {
    } else {
        ERROR_SOUND
        fprintf(stderr, "[!RTC...Continue?\n");
        fflush(stderr);
        if (getch() == 'y') {
        } else {
            xrt_reboot();
            return -1;
        }
    }






#ifdef ENABLE_WATCHDOG

    ///////////////////////////////////////////////
    // Avvio del task WatchDog
    //
    if (xrt_pthread_create(&App.WatchDogThreadId, NULL, &prvWatchDog, NULL) == 0) {
    } else {
        ERROR_SOUND
        fprintf(stderr, "[!WD...Continue?\n");
        fflush(stderr);
        if (getch() == 'y') {
        } else {
            reboot();
            return -1;
        }
    }

#endif


        




    if (App.RTKernelOK) {
        App.CheckRTTickts = 1;
    }

    uint32_t lt = xTaskGetTickCount(), max_err = 0, err = 0;
    struct timespec t, rem;

    t.tv_sec = 0;
    t.tv_nsec = 500 * 1000;


    while ((App.RTRun && App.RTCommRun) && !App.TerminateProgram) {



        lt = xTaskGetTickCount();

        int res = nanosleep(&t, NULL);

        // Test del Real-Time

#ifdef DEBUG

        err = xTaskGetTickCount() - lt;

        if (App.CheckRTTickts) {
            if (App.Initialized) {
                if (err > 1) {
                    if (err > max_err) {
                        max_err = err;
                    }
                    // fprintf(stderr, "\n%s[XP] RealTime Error <%u.%d>!%s\n", ANSI_COLOR_RED, err, max_err, ANSI_COLOR_RESET); fflush(stdout);
                }
            }
        }
#endif

    }



    terminate_program(true);


    return (int) 1;
}





#define SET_PRIORITY


extern uint16_t GLMStatIOTime;
extern uint16_t GLMaxIOTime;


static void *prvLogic(void *pvParameters) {

    portTickType tOut = 0, maxtOut = 0, xLastTime = xTaskGetTickCount(), xExpectedWakeTime = xLastTime + configTICK_RATE_HZ;
    TickType_t xLastWakeTime = xLastTime + configTICK_RATE_HZ;

    uint32_t LogicErr = 0, LogicExec = 0;


    snprintf(App.Msg, App.MsgSize, "[RT] Logic %dHz %dms\n", (int) configTICK_RATE_HZ, (int) (1000 / configTICK_RATE_HZ));
    vDisplayMessage(App.Msg);


    
    
#ifdef SET_PRIORITY
    struct sched_param param;
    int s, policy;

    policy = SCHED_FIFO; // SCHED_RR; // SCHED_FIFO;
    param.__sched_priority = 51; // 49
    s = pthread_setschedparam(pthread_self(), policy, &param);
    if (s != 0)
        perror("prvLogic_!setschedparam:");

    pthread_getschedparam(pthread_self(), &policy, &param);
    // printf("Priority of the thread: %d, current policy is: %d and should be %d", param.sched_priority, policy, SCHED_FIFO);
#endif


    /* Lock memory */
    if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1) {
        perror("mlockall failed");
    }

    /* Pre-fault our stack */
    stack_prefault();

    struct timespec t, ct, ct2;
    

    t.tv_sec = 0;
    t.tv_nsec = App.LogicWaitUSec * 1000;


    while (App.RTRun) {

        nanosleep(&t, NULL);

        try {

            // Wait for the next cycle.
            switch (myTaskDelayUntil(&xLastWakeTime, 1, &tOut)) {
                case TIMER_RUN:
                    break;
                case TIMER_DONE:
                    if (App.Initialized) {
                        
                        clock_gettime(CLOCK_MONOTONIC, &ct);
                        LogicExec++;
                        if (logic_loop(0 + 0) < 0) {
                        }
                        clock_gettime(CLOCK_MONOTONIC, &ct2);
                        
                        if (machine.status >= INITIALIZED) {
                            if (ct2.tv_nsec > ct.tv_nsec) {
                                App.LogicLastTimeNanoSec = (int32_t)(ct2.tv_nsec - ct.tv_nsec);
                                if (App.LogicLastTimeNanoSec < 1000000) {

                                    if (App.LogicLastTimeNanoSec < App.LogicMinTimeNanoSec && App.LogicLastTimeNanoSec > 1000) {
                                        App.LogicMinTimeNanoSec = App.LogicLastTimeNanoSec;
                                    }
                                    if (App.LogicLastTimeNanoSec > App.LogicMaxTimeNanoSec) {
                                        App.LogicMaxTimeNanoSec = App.LogicLastTimeNanoSec;
                                    }
                                    if (App.LogicLastTimeNanoSec / 1000 > (1000 - App.LogicWaitUSec)) {
                                        App.LogicWaitUSec = 1000 - App.LogicLastTimeNanoSec / 1000 - 100;
                                        if ((int32_t)App.LogicWaitUSec < 1)
                                            App.LogicWaitUSec = 0;
                                    }
                                } else {
                                    if (machine.status >= AUTOMATIC) {
                                        if (App.CheckRTTickts) LogicErr++;
                                        GLLogicErr++;
                                    }
                                }
                            } else {
                                // skip
                            }
                        }
                    }
                    break;
                    
                case TIMER_OUT:
                    if (tOut > 1) {
                        if (machine.status >= AUTOMATIC) {
                            if (App.CheckRTTickts) LogicErr++;
                            GLLogicErr++;
                            if (tOut > maxtOut) maxtOut = tOut;
                        }
                    }
                    break;
            }


            // Cadenziatore a 1 sec
            switch (myTaskDelayUntil(&xExpectedWakeTime, (1000 / portTICK_RATE_MS), &tOut)) {
                case TIMER_RUN:
                    break;
                case TIMER_DONE:
#ifdef DEBUG_RT_TASK
                    if (LogicErr) {
                        if (machine.status >= INITIALIZED) {
                            snprintf(App.Msg, App.MsgSize, "%sR%dE%d-%d%s", ANSI_COLOR_RED, GLMStatIOTime, LogicErr, maxtOut, ANSI_COLOR_RESET);
                            vDisplayMessage(App.Msg);
                            if (maxtOut > GLmaxtOut) GLmaxtOut = maxtOut;
                        }
                        LogicErr = 0;
                        LogicExec = 0;
                        maxtOut = 0;
                    } else {
                        // To stdout
                        snprintf(App.Msg, App.MsgSize, "R%d", GLMStatIOTime);
                        fprintf(stdout, "%s", App.Msg);
                    }
                    GLMStatIOTime = 0;
#endif

                    // Statistiche IO/sec
                    if (GLIODataPerSec > GLIOMaxDataPerSec)
                        GLIOMaxDataPerSec = GLIODataPerSec;
                    if (GLIODataPerSec < GLIOMinDataPerSec && GLIODataPerSec > 0)
                        GLIOMinDataPerSec = GLIODataPerSec;

                    GLIOLastDataPerSec = GLIODataPerSec;
                    GLIODataPerSec = 0;

                    // Statistiche UI/sec
                    if (GLUIDataPerSec > GLUIMaxDataPerSec)
                        GLUIMaxDataPerSec = GLUIDataPerSec;
                    if (GLUIDataPerSec < GLUIMinDataPerSec && GLUIDataPerSec > 0)
                        GLUIMinDataPerSec = GLUIDataPerSec;
                    GLUILastDataPerSec = GLUIDataPerSec;
                    GLUIDataPerSec = 0;
                    break;

                case TIMER_OUT:
                    if (!App.TerminateProgram) {
                        if (machine.status >= INITIALIZED) {
                            snprintf(App.Msg, App.MsgSize, "%s!R%dE%d-%d%s", ANSI_COLOR_RED, GLMStatIOTime, LogicErr, maxtOut, ANSI_COLOR_RESET);
                            vDisplayMessage(App.Msg);
                        }
                    }
                    LogicErr = 0;
                    LogicExec = 0;
                    maxtOut = 0;
                    // snprintf(App.Msg,App.MsgSize, "[R>%u]", tOut );
                    // To stderr
                    break;
            }


        } catch (int) {
        }

    }


    return NULL;
}

static void *prvComm(void *pvParameters) {
    portTickType tOut = 0, xExpectedWakeTime = xTaskGetTickCount() + (3000 / portTICK_RATE_MS);

    snprintf(App.Msg, App.MsgSize, "[RTC] UI/IO Comm %uTK/loop\n", (3000 / portTICK_RATE_MS));
    vDisplayMessage(App.Msg);

    struct sched_param param;
    int s, policy;


#ifdef SET_PRIORITY
    policy = SCHED_RR; // SCHED_RR; // SCHED_FIFO;
    param.__sched_priority = 48;
    s = pthread_setschedparam(pthread_self(), policy, &param);
    if (s != 0)
        perror("prvComm! setschedparam:");

    pthread_getschedparam(pthread_self(), &policy, &param);
    // printf("Priority of the thread: %d, current policy is: %d and should be %d", param.sched_priority, policy, SCHED_FIFO);
#endif




    while (App.RTCommRun) {

        switch (myTaskDelayUntil(&xExpectedWakeTime, (3000 / portTICK_RATE_MS), &tOut)) {
            case TIMER_RUN:
                break;
            case TIMER_DONE:
                // snprintf(msg,msg_size, "[RTC]\n");
                // vDisplayMessage(msg);
                //////////////////////////////////
                // Loop gestione comunicazioni       
                // Mode & 1 ->  Debug Mode
                // 
                if (App.RTCommInit > 0) {
                    if (dataExchangeLoop(0 + 0) >= 0) {
                        break;
                    }
                } else {
                    snprintf(App.Msg, App.MsgSize, "[!C]");
                    vDisplayMessage(App.Msg);
                }
                break;
            case TIMER_OUT:
                if (!App.TerminateProgram) {
                    snprintf(App.Msg, App.MsgSize, "[RTC>%u]", tOut);
                    vDisplayMessage(App.Msg);
                }
                break;
        }

    }

    return NULL;
}

static void *prvHTTPServices(void *pvParameters) {
    portTickType xHTTPLoop = 100 / portTICK_RATE_MS;
    portTickType tOut = 0, xExpectedWakeTime = xTaskGetTickCount() + xHTTPLoop;

    // snprintf(msg, msg_size, "[HTTP]\n", xHTTPLoop );
    // vDisplayMessage(msg);


    struct sched_param param;
    int s, policy;

#ifdef SET_PRIORITY
    policy = SCHED_RR; // SCHED_RR; // SCHED_FIFO;
    param.__sched_priority = 30;
    s = pthread_setschedparam(pthread_self(), policy, &param);
    if (s != 0)
        perror("prvHTTPServices !setschedparam:");

    pthread_getschedparam(pthread_self(), &policy, &param);
    // printf("Priority of the thread: %d, current policy is: %d and should be %d", param.sched_priority, policy, SCHED_FIFO);
#endif


    for (;;) {


        switch (myTaskDelayUntil(&xExpectedWakeTime, xHTTPLoop, &tOut)) {

            case TIMER_RUN:
                break;

            case TIMER_OUT:
            case TIMER_DONE:
                /////////////////////////////////////////////////////////
                // Servizio WEB SERVER
                //
                // ...
                if (App.RTHttpInit > 0) {
                    // Mode & 2 ->  Modalit� ritorno
                    // Mode & 1 ->  Modalit� debug
                    HTTPServerLoop(0 + 0);
                }
                break;
        }
        
        // Imposta il timer a valori più consoni
        xHTTPLoop = 10 * 1000 / portTICK_RATE_MS;
    }

    return NULL;
}

static void *prvWatchDog(void *pvParameters) {
    portTickType xWDLoop = 5000 / portTICK_RATE_MS;
    portTickType tOut = 0, xExpectedWakeTime = xTaskGetTickCount() + xWDLoop;

    int WDState = 1;
    size_t msg_size = 256;
    char *msg = (char *) malloc(msg_size);



    snprintf(msg, msg_size, "[WD] WatchDog %uTK/loop\n", xWDLoop);
    vDisplayMessage(msg);


    struct sched_param param;
    int s, policy;

#ifdef SET_PRIORITY
    policy = SCHED_RR; // SCHED_RR; // SCHED_FIFO;
    param.__sched_priority = 10;
    s = pthread_setschedparam(pthread_self(), policy, &param);
    if (s != 0)
        perror("prvWatchDog!setschedparam:");

    pthread_getschedparam(pthread_self(), &policy, &param);
    // printf("Priority of the thread: %d, current policy is: %d and should be %d", param.sched_priority, policy, SCHED_FIFO);
#endif


    for (;;) {


        // snprintf(msg, msg_size, "[WD#1:%lu]", xNextWakeTime); vDisplayMessage(msg);
        // vTaskDelayUntil( &xNextWakeTime, xBlockTime );

        switch (myTaskDelayUntil(&xExpectedWakeTime, xWDLoop, &tOut)) {

            case TIMER_RUN:
                break;

            case TIMER_DONE:
                //////////////////////////////////
                // Loop gestione WatchDog
                // 
                //
                if (WDState == 1) {

                    snprintf(msg, msg_size, "\033[32;3m[WD]\033[0m");
                    vDisplayMessage(msg);

#ifdef TERMINATE_AS_DEBUG
                    WDState++;
#else
#endif

                    // drawInfo( NULL, msg, NULL );

                    // Avio controllo dei tick
                    App.CheckRTTickts = 1;

                } else if (WDState == 2) {


                    taskENTER_CRITICAL();
                    {
                        xWDLoop = 3000 / portTICK_RATE_MS;
                        WDState++;

                    }
                    taskEXIT_CRITICAL();

                } else if (WDState == 3) {

                    snprintf(msg, msg_size, "[WD] Ending sheduler...\n");
                    vDisplayMessage(msg);
                    // vTaskEndScheduler();

                    // return;
                    WDState++;

                } else if (WDState == 4) {
                }
                break;

            case TIMER_OUT:
                snprintf(msg, msg_size, "\033[32;4m[WD>%u]\033[32;0m", tOut);
                vDisplayMessage(msg);
                // drawInfo( NULL, msg, NULL );
                break;
        }






        //////////////////////////////////////////////
        // Lettura Comandi da tastiera (DEBUG)
        // 
        //

        if (WDState == 1) {
            process_keyboard_input(msg, msg_size);
        }
    }

    return NULL;
}

void handle_help(char *msg, size_t msg_size) {

    taskENTER_CRITICAL();
    {

        vDisplayMessage("\n");

        PRINT_VERSION()

        snprintf(msg, msg_size, "[T=Trace][+/-= Test CMD][X=eXit][ESC(x3)=Reboot][C=Reset RTComm][0=Reser IO][*=test servo]\n");
        vDisplayMessage(msg);

        unsigned int ims = 1000 / configTICK_RATE_HZ;
        snprintf(msg, msg_size, "[RT] Loop %dHz %dms\n", (int) configTICK_RATE_HZ, (int) ims);
        vDisplayMessage(msg);

        snprintf(msg, msg_size, "[RTC at %d.%d.%d.%d:%d/%d][UI tout:%d][IO tout:%d][Stk:%u][MaxIO:%d]\n"
                , (int) App.MyIpAddr[0], (int) App.MyIpAddr[1], (int) App.MyIpAddr[2], (int) App.MyIpAddr[3], (int) GLListeningPort, (int) GLIOIpAddrPort
                , (int) machine.ui_timeout_count, (int) machine.io_timeout_count, (int) App.StackOverlow, (int) GLMaxIOTime);
        vDisplayMessage(msg);



        dataExchangeDumpIO(msg, msg_size);
        
        dataExchangeDumpSCR(msg, msg_size);

        dataExchangeDumpSerial(msg, msg_size);

        dataExchangeDumpCAN(msg, msg_size);



        // Stato UI e HTTP
        dataExchangeDump(msg, msg_size);

        // Tabella ARP
        // Arp::dumpTable();

        // Stato dei Task
        xprojctTaskGetRunTimeStats();


        /*
        Sequenza Macchina (Logica)
        snprintf( (char*)msg, msg_size, "(%d)[%d][%d][%d](%d)(%d)(%d)",
            (int)machine.sequence,
            (int)machine.App.mold_sequence, (int)machine.App.transferitor_sequence, (int)machine.App.load_sequence,
            (int)machine.App.manip_pref_sequence, (int)machine.App.pit_lock_sequence, (int)machine.App.owens_sequence);
        vDisplayMessage(msg);
         */


        snprintf(msg, msg_size, "\n");
        vDisplayMessage(msg);
    }
    taskEXIT_CRITICAL();

}




#define MIN_KEYBOARD_PROCESS_TIME_MS    100

void process_keyboard_input(char *msg, uint32_t msg_size) {


    if (xTaskGetTickCount() - App.KeyboardxTime < MIN_KEYBOARD_PROCESS_TIME_MS) {
        // 
        //  N.B.: Se disturbato (tipo dalla pressione contunua del tasto 'h') il processo dei pacchetti causa il crash :
        //  Probabile causa il driver della scheda di rete
        //  Verificato il blocco appena avviato nonostante il filtro temporare : problema di contemporaneit� non di velocit�
        //  probabile causa : interrupt di rete / stampa testo su display
        //

    } else {

        App.KeyboardxTime = xTaskGetTickCount();

        char c = 0;

        if ((c = _bios_keybrd(1)) != -1) {
            // Remove the key from the input buffer
            // _getch();

            // snprintf(msg, msg_size, "[WD] %d\n", (int)c);
            // vDisplayMessage(msg);

            if ((c == 'i') || (c == 'I')) {
                // Infp ?



            } else if ((c == '0') || (c == '0')) {

                // Elimina il file deglio IO
                dataEchangeResetIO();


            } else if ((c == '+') || (c == '-')) {
                ////////////////////////////////
                // Test Command
                //



            } else if ((c == 'A') || (c == 'a')) {
                ////////////////////////////////
                // Automatic Mode
                //
                taskENTER_CRITICAL();
                {
                    PUT_AUTOMATIC();
                }
                taskEXIT_CRITICAL();


            } else if ((c == 'M') || (c == 'm')) {
                ////////////////////////////////
                // Manual Mode
                //
                taskENTER_CRITICAL();
                {
                    PUT_MANUAL();
                }
                taskEXIT_CRITICAL();


            } else if ((c == 'S') || (c == 's')) {
                ////////////////////////////////
                // Start cyle
                //
                taskENTER_CRITICAL();
                {
                    PUT_START_CYCLE();
                }
                taskEXIT_CRITICAL();


            } else if ((c == 'P') || (c == 'p')) {
                ////////////////////////////////
                // Stop cyle
                //
                taskENTER_CRITICAL();
                {
                    PUT_STOP_CYCLE();
                }
                taskEXIT_CRITICAL();


            } else if ((c == 'E') || (c == 'e')) {
                ////////////////////////////////
                // Emergency
                //
                taskENTER_CRITICAL();
                {
                    PUT_EMERGENCY();
                }
                taskEXIT_CRITICAL();


            } else if ((c == 'R') || (c == 'r')) {
                ////////////////////////////////
                // Reset allarmi
                //
                taskENTER_CRITICAL();
                {
                    PUT_RESET_ALARMS();
                }
                taskEXIT_CRITICAL();





#ifdef xBM_COMPILE
                
            } else if ((c == 'B')) {
                ////////////////////////////////
                // Blow ON/OFF
                //
                taskENTER_CRITICAL();
                {
                    PUT_BLOW_ON();
                }
                taskEXIT_CRITICAL();


            } else if ((c == 'b')) {
                ////////////////////////////////
                // Blow ON/OFF
                //                    
                taskENTER_CRITICAL();
                {
                    PUT_BLOW_OFF();
                }
                taskEXIT_CRITICAL();


            } else if ((c == 'L')) {
                //////////////////
                // Load ON/OFF
                //
                taskENTER_CRITICAL();
                {
                    PUT_LOAD_ON();
                }
                taskEXIT_CRITICAL();


            } else if ((c == 'l')) {
                /////////////////
                // Load ON/OFF
                //
                taskENTER_CRITICAL();
                {
                    PUT_LOAD_OFF();
                }
                taskEXIT_CRITICAL();
                
#endif

            } else if ((c == 'W')) {
                //////////////////
                // Power ON/OFF
                //
                taskENTER_CRITICAL();
                {
                    PUT_POWER_ON();
                }
                taskEXIT_CRITICAL();


            } else if ((c == 'w')) {
                /////////////////
                // Power ON/OFF
                //
                taskENTER_CRITICAL();
                {
                    PUT_POWER_OFF();
                }
                taskEXIT_CRITICAL();



            } else if ((c == 't') || (c == 'T')) {
                ////////////////////////////////
                // Trace
                //
                taskENTER_CRITICAL();
                {
                    App.TrackPacketRecived++;
                    if (App.TrackPacketRecived >= 5)
                        App.TrackPacketRecived = 0;
                    snprintf(msg, msg_size, "[XP - Trace:%d]", App.TrackPacketRecived);
                    vDisplayMessage(msg);
                }
                taskEXIT_CRITICAL();

            } else if ((c == 'c') || (c == 'C')) {
                ////////////////////////////////
                // Resert Task comunicazioni
                //
                taskENTER_CRITICAL();
                {
                    dataExchangeReset();
                }
                taskEXIT_CRITICAL();



            } else if ((c == '*') || (c == '*')) {
                GLTestSerial = 2;

            } else if ((c == '/') || (c == '/')) {
                GLTestSerial = 3;

            } else if ((c == '.') || (c == '.')) {
                GLTestSerial = 1;
                

                /*
                char str[256];
                snprintf(str, sizeof (str), "[SER#%d] HOMING FEEDBACK XXX Timeout", 1);
                if (generate_alarm((char*) str, 6007, 0, (int32_t) ALARM_ERROR, 0+1) < 0) {
                }
                 * */
                
            } else if ((c == 'h') || (c == 'H')) {
                ////////////////////////////////
                // Help
                //
                handle_help(msg, msg_size);


            } else if ((c == 'x') || (c == 'X')) {
                //////////////////
                // Exit
                //

                App.TerminateProgram = 1;

                return;



            } else if ((c == 27)) {
                ///////////////////
                // [ESC] Reboot
                //
                App.escCounter++;
                if (App.escCounter >= 3) {
                    xrt_reboot();
                }

            }
        }
        
        if (c != 27) {
            App.escCounter = 0;
        }
    }
}





















////////////////////////////////////
// Varibili a carico del sistema
//

int system_get_value(uint8_t *var_name, uint8_t *out_str, uint32_t out_str_size, int Mode) {
    int retVal = 1;
    int cacheVarId = 0;
    uint32_t str_size = 256;
    char *str = (char *) calloc(str_size, 1);



    // if (Mode & 1) printf("[STORE::%s]", var_name); fflush (stdout);

    if (out_str)
        *out_str = 0;

    if (var_name) {

        if (Mode & 2) {
            // Modalit� non supportata : la chiamante ha il compito ri registrare i nomi nella cache

        } else {
            if (strcmpi((char*) var_name, "sys.prgVer") == 0) {
                if (Mode & 1) {
                    // Risoluzione nome variabile in ID (e messa in cache della risposta)
                    if (addVarToCache((void*) &App.MinVer, TYPE_INT, &cacheVarId) > 0) {
                        snprintf((char*) str, str_size, "%d", (int) cacheVarId);
                    } else {
                        snprintf((char*) out_str, str_size, "!");
                    }
                } else {
                    snprintf((char*) str, str_size, "sys.prgVer=%d", App.MinVer);
                }

            } else if (strcmpi((char*) var_name, "sys.console") == 0) {
                if (Mode & 1) {
                    // Risoluzione nome variabile in ID (e messa in cache della risposta)
                    if (addVarToCache((void*) NULL, TYPE_CONSOLE, &cacheVarId) > 0) {
                        snprintf((char*) str, str_size, "%d", (int) cacheVarId);
                    } else {
                        snprintf((char*) str, str_size, "!");
                    }
                } else {
                    snprintf((char*) str, str_size, "sys.console=%s", (char*) "...");
                }

            } else if (strcmpi((char*) var_name, "sys.tick") == 0) {
                if (Mode & 1) {
                    // Risoluzione nome variabile in ID (e messa in cache della risposta)
                    if (addVarToCache((void*) NULL, TYPE_SYS_TICK, &cacheVarId) > 0) {
                        snprintf((char*) str, str_size, "%d", (int) cacheVarId);
                    } else {
                        snprintf((char*) str, str_size, "!");
                    }
                } else {
                    snprintf((char*) str, str_size, "sys.tick=%u", (uint32_t) (xTaskGetTickCount()));
                }

            } else if (strcmpi((char*) var_name, "sys.time") == 0) {
                if (Mode & 1) {
                    // Risoluzione nome variabile in ID (e messa in cache della risposta)
                    if (addVarToCache((void*) NULL, TYPE_SYS_TIME, &cacheVarId) > 0) {
                        snprintf((char*) str, str_size, "%d", (int) cacheVarId);
                        // fprintf(stdout, "[STORE OK:%s]\n", str); fflush (stdout);
                    } else {
                        snprintf((char*) str, str_size, "!");
                    }
                } else {
                    snprintf((char*) str, str_size, "sys.time=%0.3f", (float) (xTaskGetTickCount() * portTICK_RATE_MS) / 1000.0f);
                }

            } else if (strcmpi((char*) var_name, "sys.mac_addr") == 0) {
                if (Mode & 1) {
                    // Risoluzione nome variabile in ID (e messa in cache della risposta)
                    if (addVarToCache((void*) NULL, TYPE_MAC_ADDR, &cacheVarId) > 0) {
                        snprintf((char*) str, str_size, "%d", (int) cacheVarId);
                        // fprintf(stdout, "[STORE OK:%s]\n", str); fflush (stdout);
                    } else {
                        snprintf((char*) str, str_size, "!");
                    }
                } else {
                    snprintf((char*) str, str_size, "sys.mac_addr=%0.3f", (float) (xTaskGetTickCount() * portTICK_RATE_MS) / 1000.0f);
                }

            } else {
                retVal = -1;
            }
        }


        if (out_str) {
            // if (Mode & 1) fprintf(stdout, "[STORING:%s]\n", str);
            strncat((char*) out_str, str, (size_t) out_str_size - strlen(str));
            // if (Mode & 1) fprintf(stdout, "[STORED:%s-%d]\n", out_str, (int)*out_str_size); fflush (stdout);
        }
    }

    if (str)
        free(str);

    return retVal;
}








///////////////////////////////////////////
// Eliminazione caratteri non UTF8
//

BOOL invalidChar(char c) {
    return !(c >= 0 && c < 128);
}

char *sanitizeString(char *str) {
    int i = 0;
    while (str[i]) {
        if (invalidChar(str[i])) {
            str[i] = '?';
        }
        i++;
    }
    return (char*) str;
}

int checkString(char *str) {
    int i = 0;
    while (str[i]) {
        if (invalidChar(str[i])) {
#ifdef DEBUG_PRINTF
            snprintf(App.Msg, App.MsgSize, "{%s}", str);
            vDisplayMessage(App.Msg);
#else
            return 0;
#endif
        }
        i++;
    }

    return 1;
}

void rt_printf(char *format, ...) {
    char *__p_msg = (char *) malloc(512);
    if (__p_msg) {
        /*
        va_list args;
        va_start (args, format);
        snprintf (__p_msg, 512, format, args);
        va_end (args);
        vDisplayMessage(__p_msg);
         */
        free(__p_msg);
    }
}









///////////////////////////////////////////
// TEST ROUTINES
//

void test_routine() {
    /*
    char str[128];
    uint16_t str_size = sizeof(str);
    uint8_t *var_name = (uint8_t *)"!mac.prgVer;mac.stat;ma:c.stat_msg;!mac.nAct;act.mold.pos;sys.time";

    uint8_t App.sendBuffer[512];    
    uint16_t nSend = sizeof(App.sendBuffer);





    ////////////////////////////////////
    // Test protocollo WebSocket
    //
    // Stringa PAYLOAD : !mac.prgVer;mac.stat;mac.stat_msg;mac.nAct;act.mold.pos;sys.time
    //
    uint8_t App.recvBuffer[] = {    0x81, 0xc0, 0x5c, 0x35, 0x65, 0xbe,
                                0x7d,0x58,0x04,0xdd,0x72,0x45,0x17,0xd9,0x0a,0x50,0x17,0x85,0x31,0x54,0x06,0x90,0x2f,0x41,0x04,0xca,0x67,0x58,0x04,0xdd,0x72,0x46,0x11,0xdf,0x28,0x6a,0x08,0xcd,0x3b,0x0e,0x08,0xdf,0x3f,0x1b,0x0b,0xff,0x3f,0x41,0x5e,0xdf,0x3f,0x41,0x4b,0xd3,0x33,0x59,0x01,0x90,0x2c,0x5a,0x16,0x85,0x2f,0x4c,0x16,0x90,0x28,0x5c,0x08,0xdb
                                };
                                
    uint16_t nReciv = sizeof(App.recvBuffer);
    uint16_t indexRecvBuffer = 0;
    uint16_t out_str_size = sizeof(str);

    // ( uint8_t *App.recvBuffer, uint16_t nReciv, uint16_t *indexRecvBuffer, uint8_t *out_str, uint16_t *out_str_size );
    handle_websocket_data ( App.recvBuffer, nReciv, &indexRecvBuffer, (uint8_t*)str, &out_str_size );


     */







    /* TEST Handshake WebSocket
    //  Risposta Protocollo websocket da testare a 
        "I1jGRgrbWxONeAO9Thawew==" -> "-mzqT\r\n"
     */

    /*
        {   uint8_t *App.recvBuffer = (uint8_t *)"Sec-WebSocket-Key: I1jGRgrbWxONeAO9Thawew==\r\n";
            uint16_t nReciv = strlen((char*)App.recvBuffer);

            out_str_size = sizeof(str);
            handle_websocket_handshake ( App.recvBuffer, nReciv, (uint8_t *)str, &out_str_size );
        }
     */


    ///////////////////////////////////////////////
    // test routine valorizzazione variabili
    //
    // snprintf( (char*)str, sizeof(str), "!mac.prgVer;mac.stat_msg;mac");
    /*
    snprintf( (char*)str, sizeof(str), "%s", var_name);
    nReciv = strlen(str);
    handle_xproject_command((uint8_t*)&str[1], nReciv-1, App.sendBuffer, &nSend);
     */



    /*
    machine.status_message[0] = 'x';
    machine.status_message[1] = 0;

    snprintf( (char*)str, sizeof(str), "mac.prgVer=%d", (int)machine.version);
    snprintf( (char*)str, sizeof(str), "mac.stat_msg=%s", (char*)machine.status_message);

        if (sprintf(str, "[logic_get_value()-var_name:%d]\n", sizeof(str_size),0+0) < 0) {
            // printf("snprintf Error"); fflush (stdout);
        } else {
            fflush (stdout);
        }
        if (sprintf(str, "[logic_get_value()-var_name:%s]\n", var_name) < 0) {
            // printf("snprintf Error"); fflush (stdout);
        } else {
            fflush (stdout);
        }


    logic_get_value ( (uint8_t *)"mac.prgVer;mac", (uint8_t *)str, &str_size, 0+0 );
     */


}

