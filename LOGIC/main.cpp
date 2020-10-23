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


#include <exception>




// Debug
#ifdef _DEBUG_MODE
#endif


// Modalit� simulazione
#ifdef PLC_SIMULATOR
#define SIMULATE_MODE
#endif




/////////////////////////////////////////////
// Gestione logica macchina
//

int logic_loop(int Mode) {

    
    try {

    #ifdef SIMULATE_MODE
        if (machine.simulate_pause) {
            // Pausa
            SleepEx(70, TRUE);
        } else {

    #else
    #endif


            // Emergenza
            if (IS_IN_EMERGENCY_REQUEST()) {

                // loop emergenza e stato emergenza
                emergency();

            } else {

                // Richiesta Reset allarmi
                if (IS_RESET_ALARMS_REQUEST()) {
                    reset_alarms();
                }


                if (machine.status == EMERGENCY) {
                    ////////////////////////////////////
                    // Macchina in emergenza
                    //

                    // loop emergenza
                    emergency();

                    if (IS_START_CYCLE_REQUESTED()) {
                        // Reset allarmi ?
                    }


                } else if (machine.status == UNINITIALIZED) {
                    /////////////////////////////////////////
                    // Macchina da inizializzare
                    //



                } else if (machine.status == PENDING_INIT) {
                    /////////////////////////////////////////
                    // Attesa I/O e periferiche
                    //
                    if (App.Initialized) {
                        if (App.CANOK == INIT_AND_RUN && App.SEROK == INIT_AND_RUN && App.SCROK == INIT_AND_RUN && App.USBOK == INIT_AND_RUN && App.IOOK == INIT_AND_RUN) {
                            /////////////////////////////////////////////////////
                            // Tutte le schede inizializzate e funzionanti
                            //
                            machine.status = INITIALIZED;

                            on_machine_initialized();
                            
                            
                        } else {
                            strncpy(machine.status_message, "PENDING INIT", machine.status_message_size);
                            if (App.CANOK != INIT_AND_RUN) strncat(machine.status_message, "[CAN]", machine.status_message_size);
                            if (App.SEROK != INIT_AND_RUN) strncat(machine.status_message, "[SER]", machine.status_message_size);
                            if (App.IOOK != INIT_AND_RUN) strncat(machine.status_message, "[IO]", machine.status_message_size);
                            if (App.SCROK != INIT_AND_RUN) strncat(machine.status_message, "[SCR]", machine.status_message_size);
                            if (App.USBOK != INIT_AND_RUN) strncat(machine.status_message, "[USB]", machine.status_message_size);
                        }
                    }

                    

                } else if (machine.status == INITIALIZED) {

                    ////////////////////////////////////////////////////////////////
                    // Macchina inizializzata : azioni di startup...
                    //
                    if (IS_IN_AUTOMATIC_REQUEST()) {

                        if (IS_START_CYCLE_REQUESTED()) {
                            ///////////////////////////////////////////////////
                            // Start ciclo : Esecuzione ripristino
                            //
                            machine.sequence = INIT_SEQUENCE;
                            machine.status = RECOVERING;

                            // Memorizza la richiesta partenza da fare alla fine del recovery
                            machine.start_after_recovery = TRUE;
                        }

                    } else if (IS_IN_MANUAL_REQUEST()) {

                        ///////////////////////
                        // Stato in Manuale
                        //
                        put_machine_in_manual_mode();


                        ////////////////////////////
                        // Modalita' debug
                        //
                        if (App.DebugMode)
                            PUT_POWER_ON();

                    }




    #ifdef SIMULATE_MODE
                    // Mode & 1     ->      Disegna lo sfondo e i titoli
                    draw_actuators_move(machine.hwnd, 0 + 1);
    #endif



                } else if (machine.status == RECOVERING) {
                    ///////////////////
                    // Rispristino
                    //
                    recover();

                    if (IS_IN_MANUAL_REQUEST()) {
                        // Passaggio in manuale : messa in stop della macchina
                        PUT_STOP_CYCLE();
                    }


                } else if (machine.status == READY) {

                    //////////////////////////////////////////////////////////////////
                    // Macchina pronta all' automatico o passo/passo
                    //
                    if (IS_IN_AUTOMATIC_REQUEST()) {

                        // Richiesta pendente di avvio
                        if (machine.start_after_recovery) {
                            machine.start_after_recovery = FALSE;
                            machine.sequence = INIT_SEQUENCE;
                            machine.status = AUTOMATIC;
                        } else {
                            if (IS_START_CYCLE_REQUESTED()) {
                                ////////////////////////
                                // Avvio ciclo
                                //
                                machine.sequence = INIT_SEQUENCE;
                                machine.status = AUTOMATIC;
                            }
                        }
                    } else if (IS_IN_MANUAL_REQUEST()) {
                        ///////////////////////
                        // Stato in Manuale
                        //
                        put_machine_in_manual_mode();
                    }


                } else if (machine.status == AUTOMATIC || machine.status == STEP_BY_STEP) {

                    ///////////////////////////////////////////////////
                    // Automatico - Passo/Passo
                    //
                    automatic(NULL, NULL);


                    if (IS_IN_MANUAL_REQUEST()) {
                        // Passaggio in manuale : messa in stop della macchina
                        PUT_STOP_CYCLE();
                    }


                } else if (machine.status == MANUAL) {
                    /////////////////
                    // Manuale
                    //
                    manual();
                }
            }




            // Controllo timeout dei movimenti
            if (check_timeouts() > 0) {
            }

            // Controllo contemporaneit� dei movimenti
            if (check_actuators_status() > 0) {
            }




            // Aggiorna la sezione temporale
            GLCurTimeMs = xTaskGetTickCount();



            if (App.SimulateMode) {
                ////////////////////////////////////////////////////////////
                // Esecuzione simulazione dei movimenti
                //
                if (simulate_actuators_move() < 0) {
                }
            }

            ///////////////////////////////////////////
            // Processa lo stato degli attuatori
            //
            process_actuators_move_loop();



            // Contatore cicli programma in ms
            machine.task_cycles++;


            if (machine.task_cycles % 1000 == 0) {
            
                if (machine.status == AUTOMATIC) {
                    machine.statistic.machine_running++;
                    machine.rt_statistic.machine_running++;
                    format_time_run(machine.statistic.machine_running, (char*)machine.statistic.machine_running_string, sizeof(machine.statistic.machine_running_string));
                    format_time_run(machine.rt_statistic.machine_running, (char*)machine.rt_statistic.machine_running_string, sizeof(machine.rt_statistic.machine_running_string));
                }
                
                machine.statistic.machine_elapsed++;
                machine.rt_statistic.machine_elapsed++;               
                format_time_run(machine.statistic.machine_elapsed, (char*)machine.statistic.machine_elapsed_string, sizeof(machine.statistic.machine_elapsed_string));
                format_time_run(machine.rt_statistic.machine_elapsed, (char*)machine.rt_statistic.machine_elapsed_string, sizeof(machine.rt_statistic.machine_elapsed_string));
            }                
            
    #ifdef SIMULATE_MODE
        }
    #endif


  
    } catch (std::exception& e) {
        // std::cerr << "Exception catched : " << e.what() << std::endl;
        //////////////////////////////////////
        // Generazione Warning
        //
        char msg[512];
        snprintf(msg, sizeof(msg), "logic_loop() :  Exception : %s", e.what());
        if (generate_alarm((char*) msg, 8888, 0, (int) ALARM_WARNING, 0+1) < 0) {
        } 

    } catch (...) {
        // std::cerr << "Exception catched : " << e.what() << std::endl;
        //////////////////////////////////////
        // Generazione Warning
        //
        char msg[512];
        snprintf(msg, sizeof(msg), "logic_loop() :  Unk Exception");
        if (generate_alarm((char*) msg, 8888, 0, (int) ALARM_WARNING, 0+1) < 0) {
        } 
    }   

    return 1;
}

    
                    
int IS_RESET_ALARMS_REQUEST(void) {
    if (machine.reset_alarms_request) {
        machine.reset_alarms_request = 0;
        return 1;
    }
    return 0;
}

// Funzione richiesta passaggio in emergenza

int IS_IN_EMERGENCY_REQUEST(void) {
    if (machine.emergency_request == 1) {
        strncpy(machine.status_message, "EMERGENCY REQUEST", machine.status_message_size);
        machine.emergency_request = 0;
        return 1;
    }
    return 0;
}


// Funzione richiesta passaggio in automatico

int IS_IN_AUTOMATIC_REQUEST(void) {
    if (machine.machine_mode_request == 1) {
        strncpy(machine.status_message, "AUTOMATIC REQUEST", machine.status_message_size);
        return 1;
    }
    return 0;
}


// Funzione interrogazione stato in manuale

int IS_IN_MANUAL_REQUEST(void) {
    if (machine.machine_mode_request == 0) {
        strncpy(machine.status_message, "MANUAL REQUEST", machine.status_message_size);
        return 1;
    }
    return 0;
}



// Funzione interrogazione arresto macchina (STOP CICLO)

int IS_STOP_CYCLE_REQUESTED(void) {
    if (machine.stop_request) {
        // machine.stop_request = 0;
        strncpy(machine.status_message, "STOP CYCLE REQUEST", machine.status_message_size);
        return 1;
    }
    return 0;
}


// Funzione interrogazione inizio funzionamento (START CICLO)

int IS_START_CYCLE_REQUESTED(void) {
    if (machine.start_request) {
        // machine.start_request = 0;
        strncpy(machine.status_message, "START CYCLE REQUEST", machine.status_message_size);
        return 1;
    }
    return 0;
}

int IS_SIMULATE_CYCLE_REQUESTED(void) {
    if (machine.simulate_request) {
        // machine.start_request = 0;
        strncpy(machine.status_message, "SIMULATE CYCLE REQUEST", machine.status_message_size);
        return 1;
    }
    return 0;
}

void PUT_AUTOMATIC() {
    if (machine.machine_mode_request != 1) {
        machine.machine_mode_request = 1;
        // Abbassa la richiesta di partenza
        machine.start_request = 0;
        machine.simulate_request = 0;
    }
}

void PUT_MANUAL() {
    machine.machine_mode_request = 0;
}

void PUT_POWER_ON() {
    if (!machine.power_on_request) {
        machine.power_on_request = TRUE;
        process_actuators_initialize(false);
    }
}

void PUT_POWER_OFF() {
    if (machine.power_on_request) {
        machine.power_on_request = FALSE;
        process_actuators_terminate();
    }
}


void PUT_START_CYCLE() {
    machine.start_request = 1;
    machine.stop_request = 0;
}

void PUT_STOP_CYCLE() {
    machine.start_request = 0;
    machine.stop_request = 1;
}

void PUT_SIMULATE_CYCLE() {
    machine.start_request = 0;
    machine.simulate_request = 1;
    machine.stop_request = 0;
}


void PUT_EMERGENCY() {
    machine.start_request = 0;
    machine.stop_request = 0;
    machine.emergency_request = 1;
    if (generate_alarm((char*) "Emergency by user request", 9999, 0, (int32_t) ALARM_ERROR, 0+1) < 0) {
    }
}

void PUT_RESET_ALARMS() {
    machine.reset_alarms_request = 1;
}





#ifdef xBM_COMPILE
    void PUT_LOAD_ON() {
        machine.App.load_on = TRUE;
    }

    void PUT_LOAD_OFF() {
        machine.App.load_on = FALSE;
    }

    void PUT_SINGLE_LOAD_ON() {
        machine.App.single_load_on = TRUE;
    }

    void PUT_SINGLE_LOAD_OFF() {
        machine.App.single_load_on = FALSE;
    }


    void PUT_HEAT_ON() {
        machine.App.heat_on = TRUE;
    }

    void PUT_HEAT_OFF() {
        machine.App.heat_on = FALSE;
    }

    void PUT_BLOW_ON() {
        machine.App.blow_on = TRUE;
    }

    void PUT_BLOW_OFF() {
        machine.App.blow_on = FALSE;
    }

    void PUT_INLINE_OUTPUT_ON() {
        machine.App.inline_output_on = TRUE;
    }

    void PUT_INLINE_OUTPUT_OFF() {
        machine.App.inline_output_on = FALSE;
    }
#elif xCNC_COMPILED
#endif




char *get_machine_status_desc() {
    switch (machine.status) {

        case UNINITIALIZED:
            return (char*) "UNINITIALIZED";
            break;

        case INITIALIZED:
            return (char*) "INITIALIZED";
            break;

        case RECOVERING:
            return (char*) "RECOVERING";
            break;

        case READY:
            return (char*) "READY";
            break;

        case MANUAL:
            return (char*) "MANUAL";
            break;

        case AUTOMATIC:
            return (char*) "AUTOMATIC";
            break;

        case STEP_BY_STEP:
            return (char*) "STEP BY STEP";
            break;

        case EMERGENCY:
            return (char*) "EMERGENCY";
            break;

        case MAX_MACHINE_STATUS:
            return (char*) "MAX STATUS";
            break;
            
        case PENDING_INIT:
            return (char*) "PENDING INIT";
            break;

        default:
            return (char*) "UNKNOWN";
            break;

    }
}