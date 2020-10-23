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






#define SIMULATE_GAP 25.0;

// Simula il movimento degli attuattori

static uint32_t GLSimulateSectTime = xTaskGetTickCount();

int simulate_actuators_move(void) {
    uint32_t i;
    char str[256];
    int retVal = 1;

    try {
            
        for (i = 0; i < machine.num_actuator; i++) {

            if (machine.actuator[i].step == STEP_SEND_CMD) {

            } else if (machine.actuator[i].step == STEP_MOVING) {
                int up_position = OUTSIDE;
                int down_position = INSIDE;



                //////////////////////////////////
                // Simulazione del movimento
                //
                if (machine.actuator[i].pSerialSlot) {
                    ////////////////////////////////////////////////////////////////////////
                    // Letta dall'attuatore attraverso la sequenza gestione della seriale
                    // ...
                } else if (machine.actuator[i].pIOSlot) {
                    ////////////////////////////////////////////////////////////////////////
                    // Letta dall'attuatore attraverso la scheda di I/O
                    // ...
                } else if (machine.actuator[i].pCANSlot) {
                    ////////////////////////////////////////////////////////////////////////
                    // Letta dall'attuatore attraverso la scheda di I/O
                    // ...

                } else {
                    bool simulateMovementCase = 0;

                    // Simulazione
                    if (App.SimulateMode) {
                        simulateMovementCase = 1;
                    } else if (machine.actuator[i].protocol == BISTABLE_PN_VALVE || machine.actuator[i].protocol == MONOSTABLE_PN_VALVE) {
                        simulateMovementCase = 1;
                    } else if (machine.actuator[i].protocol == ADC_INPUT) {
                        simulateMovementCase = 1;
                    } else if (machine.actuator[i].protocol == VARIABLE_ON_OFF) {
                        simulateMovementCase = 1;
                    } else if (machine.actuator[i].protocol == AC_INVERTER) {
                        simulateMovementCase = 1;
                    } else if (machine.actuator[i].protocol == ASYNC_MOTOTR) {
                        simulateMovementCase = 1;
                    } else if (machine.actuator[i].protocol == VIRTUAL_AC_SERVO) {
                        simulateMovementCase = 1;
                    } else {
                        simulateMovementCase = 0;
                    }
                    
                    if (simulateMovementCase == 1) {
                        if (machine.actuator[i].target_position == up_position) {
                            // machine.actuator[i].cur_rpos = machine.actuator[i].cur_rpos + machine.actuator[i].speed_auto1 * (1.0f + (machine.actuator[i].end_rpos - machine.actuator[i].cur_rpos) / machine.actuator[i].end_rpos);
                            machine.actuator[i].cur_rpos += machine.actuator[i].speed_auto1 * (float)(xTaskGetTickCount() - GLSimulateSectTime) / 1000.0f;
                            
                            // essendo una simulazione...
                            if (machine.actuator[i].cur_rpos > machine.actuator[i].end_rpos)
                                machine.actuator[i].cur_rpos = machine.actuator[i].end_rpos;

                        } else if (machine.actuator[i].target_position == down_position) {
                            // machine.actuator[i].cur_rpos = machine.actuator[i].cur_rpos - machine.actuator[i].speed_auto2 * (1.0f + (machine.actuator[i].cur_rpos) / (machine.actuator[i].end_rpos - machine.actuator[i].start_rpos));
                            machine.actuator[i].cur_rpos -= machine.actuator[i].speed_auto2 * (float)(xTaskGetTickCount() - GLSimulateSectTime) / 1000.0f;
                            
                            // essendo una simulazione...
                            if (machine.actuator[i].cur_rpos < machine.actuator[i].start_rpos)
                                machine.actuator[i].cur_rpos = machine.actuator[i].start_rpos;

                        } else if (machine.actuator[i].target_position == USER_POSITION || machine.actuator[i].target_position == INTERPOLATE_POSITION) {
                            float dir = 0.0f;
                            float speed = 0.0f;
                            
                            if (machine.actuator[i].cur_rpos < machine.actuator[i].target_rpos) 
                                dir = +1.0f;
                            else if (machine.actuator[i].cur_rpos > machine.actuator[i].target_rpos) 
                                dir = -1.0f;

                            if (machine.actuator[i].target_position == INTERPOLATE_POSITION) {
                                speed = machine.actuator[i].speed_lin_auto3;
                            } else {
                                if (dir > 0.0) {
                                    speed = machine.actuator[i].speed_lin_auto2;
                                } else {
                                    speed = machine.actuator[i].speed_lin_auto1;
                                }
                            }
                            
                            machine.actuator[i].cur_rpos += dir * speed * (float)(xTaskGetTickCount() - GLSimulateSectTime) / 1000.0f;
                            
                            // machine.actuator[i].cur_rpos += speed * (1.0f + (machine.actuator[i].cur_rpos) / (machine.actuator[i].end_rpos - machine.actuator[i].start_rpos)) * dir;
                            
                            if (fabs (machine.actuator[i].cur_rpos - machine.actuator[i].target_rpos) < 0.1) {
                                machine.actuator[i].cur_rpos = machine.actuator[i].target_rpos;
                            }
                            
                            if (dir > 0) {
                                if (machine.actuator[i].cur_rpos > machine.actuator[i].target_rpos)
                                    machine.actuator[i].cur_rpos = machine.actuator[i].target_rpos;
                            } else { 
                                if (machine.actuator[i].cur_rpos < machine.actuator[i].target_rpos)
                                    machine.actuator[i].cur_rpos = machine.actuator[i].target_rpos;
                            }                            
                        }


                        // posizione virtuale (0...1000)
                        update_actuator_virtual_pos(&machine.actuator[i]);

                        machine.actuator[i].readCounter++;
                        
                    } else if (simulateMovementCase == 2) {
                        // ...
                        
                    } else {
                        
                        snprintf(str, sizeof (str), "Actuator %s isn't link to any board\n", (char*) machine.actuator[i].name);
                        if (generate_alarm((char*) str, 9002, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                            retVal = -1;
                        }
                    }
                }
            }

#ifdef xBM_COMPILE
            
            // Simulazione
            if (App.SimulateMode) {
                if (machine.actuator[PRIMARY_AIR].position == ON) {
                    machine.actuator[BLOW_PRESSURE].cur_rpos += (machine.actuator[LINE_PRESSURE].cur_rpos - machine.actuator[BLOW_PRESSURE].cur_rpos) / 30000.0;
                    // posizione virtuale (0...1000)
                    update_actuator_virtual_pos(&machine.actuator[BLOW_PRESSURE]);
                } 

                if (machine.actuator[SECONDARY_AIR].position == ON) {
                    machine.actuator[BLOW_PRESSURE].cur_rpos += (machine.actuator[LINE_PRESSURE].cur_rpos - machine.actuator[BLOW_PRESSURE].cur_rpos) / 6000.0;
                    // posizione virtuale (0...1000)
                    update_actuator_virtual_pos(&machine.actuator[BLOW_PRESSURE]);
                }


                if (machine.actuator[DISCHARGE_AIR].position == OFF) {
                    if (machine.actuator[BLOW_PRESSURE].cur_rpos > 0.0)
                        machine.actuator[BLOW_PRESSURE].cur_rpos -= (machine.actuator[BLOW_PRESSURE].cur_rpos) * 0.00025;
                    // posizione virtuale (0...1000)
                    update_actuator_virtual_pos(&machine.actuator[BLOW_PRESSURE]);
                }
                if (machine.actuator[RECOVERY_AIR].position == OFF) {
                    if (machine.actuator[BLOW_PRESSURE].cur_rpos > 0.0)
                        machine.actuator[BLOW_PRESSURE].cur_rpos -= (machine.actuator[BLOW_PRESSURE].cur_rpos) * 0.00025;
                    // posizione virtuale (0...1000)
                    update_actuator_virtual_pos(&machine.actuator[BLOW_PRESSURE]);
                }
            }
#endif
            

        }


        GLSimulateSectTime = xTaskGetTickCount();
        

    } catch (std::exception& e) {
        // std::cerr << "Exception catched : " << e.what() << std::endl;
        //////////////////////////////////////
        // Generazione Warning
        //
        char msg[512];
        snprintf(msg, sizeof(msg), "simulate_actuators_move() :  Exception : %s", e.what());
        if (generate_alarm((char*) msg, 8888, 0, (int) ALARM_WARNING, 0+1) < 0) {
        } 

    } catch (...) {
        // std::cerr << "Exception catched : " << e.what() << std::endl;
        //////////////////////////////////////
        // Generazione Warning
        //
        char msg[512];
        snprintf(msg, sizeof(msg), "simulate_actuators_move() :  Unk Exception");
        if (generate_alarm((char*) msg, 8888, 0, (int) ALARM_WARNING, 0+1) < 0) {
        } 
    }   
    
    return retVal;
}

