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



#include <math.h>
#include <float.h>


// Ignora l'assenza delle schede
#ifdef DEBUG
#define IGNORE_MISSING_BOARD
#endif



// Numero di errori di lettura consecutivi tollerati
#define MAX_OVER_STROKE_ERROR   7


#define MODBUS_SLEEP_INTERCALL_MS  (30*1000)

int process_actuators_initialize(int32_t doHoming) {
    int retVal = 0;
    uint32_t i;

    for (i = 0; i < machine.num_actuator; i++) {
        LP_ACTUATOR pActuator = (LP_ACTUATOR) & machine.actuator[i];
        retVal = process_actuator_initialize((void *) pActuator, doHoming);
        if (retVal < 0)
            break;
    }

    return retVal;
}

int process_actuator_initialize(void *pvActuator, int32_t doHoming) {
    int retVal = 0;

    if (pvActuator) {
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pvActuator;
        char str[256];

        if (pActuator->step == STEP_UNINITIALIZED || pActuator->step == STEP_STOPPED || pActuator->step == STEP_ERROR) {

            pActuator->error = 0;
            pActuator->outStrokeError = 0;

                    
            if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN || pActuator->protocol == MODBUS_AC_SERVO_DELTA) {

                if (pActuator->pSerialSlot) {
                    SerialSlot *pSerialSlot = (SerialSlot*) pActuator->pSerialSlot;
                    
                    // Imposta la richiesta di setup
                    pSerialSlot->setupRequest = 1;

                    snprintf(App.Msg, App.MsgSize, "[%s SER: Entering %s ssetup requent (step:%s)%s]\n", (char*) ANSI_COLOR_YELLOW, pActuator->name, (char*)get_actuator_step((void*)pActuator), (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);



                } else {
                    // Interfaccia assente : disabilita il protocollo in simulazione
                    if (App.SimulateMode) {
                        pActuator->protocol = PROTOCOL_NONE;
                        snprintf(str, sizeof (str), "Actuator %s has no slot!", pActuator->name);
                        if (generate_alarm((char*) str, 6200, 0, (int) ALARM_WARNING, 0+1) < 0) {
                        }
                    } else {
                        snprintf(str, sizeof (str), "Actuator %s has no slot!", pActuator->name);
                        if (generate_alarm((char*) str, 6200, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                        }
                        return -1;
                    }
                }

            } else if (pActuator->protocol == CANOPEN_AC_SERVO_DELTA) {

                if (pActuator->pCANSlot) {
                    CANSlot *pCANSlot = (CANSlot*) pActuator->pCANSlot;

                    // Imposta la richiesta di setup
                    pCANSlot->setupRequest = 1;
                    pCANSlot->fullCMDFeedbackStartTime = 0;
                    pCANSlot->fullCMDFeedbackError = 0;

                    snprintf(App.Msg, App.MsgSize, "[%s CAN: Entering %s setup requent (step:%s)%s]\n", (char*) ANSI_COLOR_YELLOW, pActuator->name, (char*)get_actuator_step((void*)pActuator), (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);

                    // Necessario l'azzeramento ?
                    if (doHoming) {
                        if (!pActuator->homingDone) {
                            // Imposta la richiesta di homing
                            pActuator->homingRequest = 1;
                            pCANSlot->homingRequest = 1;
                        }
                    }
                    
                } else {
                    if (App.SimulateMode) {
                        pActuator->protocol = PROTOCOL_NONE;
                        snprintf(str, sizeof (str), "Actuator %s has no slot!", pActuator->name);
                        if (generate_alarm((char*) str, 6200, 0, (int) ALARM_WARNING, 0+1) < 0) {
                        }
                    } else {
                        snprintf(str, sizeof (str), "Actuator %s has no slot!", pActuator->name);
                        if (generate_alarm((char*) str, 6200, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                        }
                        return -1;
                    }
                }

            } else if (pActuator->protocol == VIRTUAL_AC_SERVO) {
                pActuator->step = STEP_READY;
                pActuator->homingDone = -1;

            } else if (pActuator->protocol == MONOSTABLE_PN_VALVE) {
                pActuator->step = STEP_INITIALIZED;
                pActuator->homingDone = -1;

            } else if (pActuator->protocol == BISTABLE_PN_VALVE) {
                pActuator->step = STEP_INITIALIZED;
                pActuator->homingDone = -1;

            } else {
                if (App.SimulateMode)
                    pActuator->step = STEP_INITIALIZED;
                    pActuator->homingDone = -1;
            }

        } else if (pActuator->step == STEP_READY) {

        }
    }

    return retVal;
}

int process_actuators_terminate(void) {
    int retVal = 0;
    uint32_t i;

    for (i = 0; i < machine.num_actuator; i++) {
        LP_ACTUATOR pActuator = (LP_ACTUATOR) & machine.actuator[i];
        retVal = process_actuator_terminate((void *) pActuator);
        if (retVal < 0)
            break;
    }

    return retVal;
}

int process_actuator_terminate(void *pvActuator) {
    int retVal = 0;

    if (pvActuator) {
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pvActuator;

        if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN || pActuator->protocol == MODBUS_AC_SERVO_DELTA) {
            
            pActuator->step = STEP_STOPPED;
                    
            if (pActuator->pSerialSlot) {
                SerialSlot *pSerialSlot = (SerialSlot*) pActuator->pSerialSlot;
                pSerialSlot->stopRequest = 1;
                retVal++;
            } else {
            }
        } else if (pActuator->protocol == CANOPEN_AC_SERVO_DELTA) {
            
            pActuator->step = STEP_STOPPED;
            
            if (pActuator->pCANSlot) {
                CANSlot *pCANSlot = (CANSlot*) pActuator->pCANSlot;
                pCANSlot->stopRequest = 1;
                retVal++;
            } else {
            }

        } else if (pActuator->protocol == PROTOCOL_NONE) {
            pActuator->step = STEP_READY;
        } else if (pActuator->protocol == VIRTUAL_AC_SERVO) {
            pActuator->step = STEP_READY;
        } else if (pActuator->protocol == MONOSTABLE_PN_VALVE) {
            pActuator->step = STEP_READY;
        } else if (pActuator->protocol == BISTABLE_PN_VALVE) {
            pActuator->step = STEP_READY;
        } else {
        }
    }

    return retVal;
}





int process_actuator_homing_request(void *pvActuator) {
    int retVal = 0;

    if (pvActuator) {
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pvActuator;
        char str[256];

        if (pActuator->step == STEP_UNINITIALIZED || pActuator->step == STEP_STOPPED || pActuator->step == STEP_ERROR) {

            // Necessario setup
            if (process_actuator_initialize( (void *)pvActuator, true) < 0) {
                return -1;
            } else {            
                return 0;
            }
        
        } else if (pActuator->step == STEP_READY) {

            if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN || pActuator->protocol == MODBUS_AC_SERVO_DELTA) {

                if (pActuator->pSerialSlot) {
                    SerialSlot *pSerialSlot = (SerialSlot*) pActuator->pSerialSlot;
                    
                    if (!pActuator->homingDone) {
                        // Imposta la richiesta di homing
                        pActuator->homingRequest = 1;
                        pSerialSlot->homingRequest = 1;
                        return 1;
                    } else {
                        return 0;
                    }
                    
                } else {
                    return -1;
                }

            } else if (pActuator->protocol == CANOPEN_AC_SERVO_DELTA) {
                if (pActuator->pCANSlot) {
                    CANSlot *pCANSlot = (CANSlot*) pActuator->pCANSlot;

                    if (!pActuator->homingDone) {
                        // Imposta la richiesta di homing
                        pActuator->homingRequest = 1;
                        pCANSlot->homingRequest = 1;
                        return 1;
                    } else {
                        return 0;
                    }
                } else {
                    return -1;
                }

            } else if (pActuator->protocol == VIRTUAL_AC_SERVO) {
                pActuator->step = STEP_READY;
                pActuator->homingDone = -1;
                return 1;

            } else if (pActuator->protocol == MONOSTABLE_PN_VALVE) {
                pActuator->step = STEP_INITIALIZED;
                pActuator->homingDone = -1;
                return 1;

            } else if (pActuator->protocol == BISTABLE_PN_VALVE) {
                pActuator->step = STEP_INITIALIZED;
                pActuator->homingDone = -1;
                return 1;

            } else {
                pActuator->step = STEP_INITIALIZED;
                pActuator->homingDone = -1;
                return 1;
            }
        }
    }

    return retVal;
}




///////////////////////////////////////////////////////////////////////
// N.B.: Restituisce i Pulse/Turn sullo spazio 0...pulsePerTurn
//
int actuator_position_to_encoder(void *pvActuator, float rpos, int32_t *pTargetTurnsPPT, int32_t *pTargetPulsesPPT) {
    if (pvActuator) {
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pvActuator;
        float rtargetTurns = 0.0f;
        float fractPart = modff( ( rpos + pActuator->workingOffset ) / (pActuator->cam_ratio > 0.0000001f ? pActuator->cam_ratio : 1.0f), &rtargetTurns);
        int32_t targetPulses = (int32_t) ((fractPart) * pActuator->pulsesPerTurn);

               
        if (pTargetTurnsPPT) {
            pTargetTurnsPPT[0] = (int32_t) rtargetTurns + pActuator->homingTurnsPPT;
        }
        
        if (pTargetPulsesPPT) {
            pTargetPulsesPPT[0] = (int32_t) targetPulses + pActuator->homingPulsesPPT;
            if (*pTargetPulsesPPT >= pActuator->pulsesPerTurn) {
                // Rotazione su pulsesPerTurn
                pTargetTurnsPPT[0] = pTargetTurnsPPT[0] + pTargetPulsesPPT[0] / (int32_t)pActuator->pulsesPerTurn;
                pTargetPulsesPPT[0] = pTargetPulsesPPT[0] % (int32_t)pActuator->pulsesPerTurn;
            }
        }

        return 1;
    }
    return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// N.B.: Sul MODBUS LICHIAN la rotazione non avviene al numero di pulsazioni per gito ma su overflow dei 16bit
//
int actuator_encoder_to_position(void *pvActuator, int32_t TurnsPPO, int32_t PulsesPPO, float *rpos) {
    if (pvActuator) {
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pvActuator;
        if (rpos) {
            float pulsesOverflow = (float)pActuator->pulsesOverflow;
            int32_t homingPulsesPPO = 0, homingTurnsPPO = 0;
            
            actuator_ppt_to_ppo (pvActuator, pActuator->homingTurnsPPT, pActuator->homingPulsesPPT, &homingTurnsPPO, &homingPulsesPPO);
                    
            *rpos = (float)(TurnsPPO * pulsesOverflow + PulsesPPO - homingPulsesPPO - homingTurnsPPO * pulsesOverflow) / (float) pActuator->pulsesPerTurn * pActuator->cam_ratio - pActuator->workingOffset;
            
            if (*rpos > 1000.0f) {
                int lb = 1;
            } else if (*rpos < -10.0f) {
                int lb = 1;
            }
        }

        return 1;
    }
    return 0;
}

// Pulse per turn -->  Pulse per Overflow
int actuator_ppt_to_ppo(void *pvActuator, int32_t turnsPPT, int32_t pulsesPPT, int32_t *tpurnsPPO, int32_t *pulsesPPO) {
    if (pvActuator) {
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pvActuator;
        int32_t totPulses = pulsesPPT + turnsPPT * (int32_t)pActuator->pulsesPerTurn;
        *pulsesPPO = totPulses % (int32_t)pActuator->pulsesOverflow;
        *tpurnsPPO = totPulses / (int32_t)pActuator->pulsesOverflow;
    }    
}
     
// Pulse per Overflow (es.:0..65535)  --> Pulse per turn (es.:0..10000)
int actuator_ppo_to_ppt(void *pvActuator, int32_t turnsPPO, int32_t pulsesPPO, int32_t *turnsPPT, int32_t *pulsesPPT) {
    if (pvActuator) {
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pvActuator;
        int32_t totPulses = pulsesPPO + turnsPPO * (int32_t)pActuator->pulsesOverflow;
        *pulsesPPT = totPulses % (int32_t)pActuator->pulsesPerTurn;
        *turnsPPT = totPulses / (int32_t)pActuator->pulsesPerTurn;
    }    
}


int actuator_add_pulse_ppt(void *pvActuator, int32_t turnsPPT, int32_t pulsesPPT, int32_t *turnsOutPPT, int32_t *pulsesOutPPT, int Mode) {
    if (pvActuator) {
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pvActuator;            
        int32_t pulsesLimit = (int32_t)pActuator->pulsesPerTurn;
        
        *turnsOutPPT = *turnsOutPPT + turnsPPT;
        *pulsesOutPPT = *pulsesOutPPT + pulsesPPT;
        if (*pulsesOutPPT > pulsesLimit) {
            *turnsOutPPT += (*pulsesOutPPT / pulsesLimit);
            *pulsesOutPPT = (*pulsesOutPPT % pulsesLimit);
        }        
        return 1;
    }
}



///////////////////////////////////////////////////////////////////////
// N.B.: 
//  Mode & 0    ->  Restituisce i Pulse/Turn sullo spazio 0...pulsePerTurn
//  Mode & 1    ->  Restituisce i Pulse/Turn sullo spazio 0...pulseOverflow (16bit)
//  Mode & 2    ->  Overflow a 16 bit per i giri negativi
//  Mode & 4    ->  Overflow a 32 bit per i giri negativi
//
int actuator_delta_pulse(void *pvActuator, int32_t turns2, int32_t turns, int32_t pulses2, int32_t pulses, int32_t *turnsOut, int32_t *pulsesOut, int Mode) {
    if (pvActuator) {
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pvActuator;            
        int32_t pulsesOverflow = (int32_t)pActuator->pulsesPerTurn;

        
        if (Mode & 1) {
            // Il contatore ruota su overflow dei 16bit
            pulsesOverflow = (int32_t)pActuator->pulsesOverflow;
        }
        
        *turnsOut = turns2 - turns;
        *pulsesOut = pulses2 - pulses;

        
        if (*turnsOut < 0) {
            if (Mode & 2) {
                // Overflow a 16 bit per i giri negativi (16bit)
                *turnsOut = 0xFFFF - *turnsOut;
            } else if (Mode & 4) {
                // Overflow a 32 bit per i giri negativi
                *turnsOut = 0xFFFFFFFF - *turnsOut;
            }
        }

        
        if (*pulsesOut > pulsesOverflow) {
            *turnsOut += *pulsesOut / (int32_t)pulsesOverflow;
            *pulsesOut = *pulsesOut % (int32_t)pulsesOverflow;
        } else if (*pulsesOut < 0) {
            *turnsOut -= (abs(*pulsesOut) / (int32_t)pulsesOverflow +1);
            *pulsesOut = (*pulsesOut % (int32_t)pulsesOverflow) + (int32_t)pulsesOverflow;
        }
    }
}


int actuator_set_working_offset(void *pvActuator, float workingOffset) {
    if (pvActuator) {
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pvActuator;
        if (pActuator->step == STEP_READY || pActuator->step == STEP_STOPPED) {
            if (fabs(workingOffset) <= EPSILON) {
                pActuator->start_rpos += pActuator->workingOffset;
                pActuator->end_rpos += pActuator->workingOffset;
                pActuator->cur_rpos += pActuator->workingOffset;
                pActuator->workingOffset = 0;
            } else {
                if (workingOffset <= pActuator->end_rpos && workingOffset >= pActuator->start_rpos ) {
                    pActuator->workingOffset += workingOffset;
                    pActuator->start_rpos -= workingOffset;
                    pActuator->end_rpos -= workingOffset;
                    pActuator->cur_rpos -= workingOffset;
                    return 1;
                }
            }
        }
    }
    
    return 0;
}


int actuator_handle_read_position(void *pvActuator, float newPosition, bool initialRead) {
    if (pvActuator) {
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pvActuator;

        if (fabs(pActuator->cur_rpos - newPosition) > 0.01) {
            if (initialRead) {
                snprintf(App.Msg, App.MsgSize, "[%s %s Initial POS %0.3f->%0.3f%s]", (char*) ANSI_COLOR_GREEN, pActuator->name, pActuator->cur_rpos, newPosition, (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);
            }
            pActuator->cur_rpos = newPosition;
        } else {
            if (initialRead) {
                snprintf(App.Msg, App.MsgSize, "[%s %s Initial POS %0.3f%s]", (char*) ANSI_COLOR_GREEN, pActuator->name, pActuator->cur_rpos, (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);
            }
        }
             
        float end_rpos_toll = 0.01f, start_rpos_toll = 0.01f;
        if (machine.status == AUTOMATIC) {
            start_rpos_toll = pActuator->start_rpos_toll;
            end_rpos_toll = pActuator->end_rpos_toll;
        } else {            
        }

        if (fabs(pActuator->cur_rpos - pActuator->end_rpos) <= end_rpos_toll) {
            pActuator->position = ON;
            if (initialRead) {
                snprintf(App.Msg, App.MsgSize, "[%s %s Initial POS=ON %s]", (char*) ANSI_COLOR_GREEN, pActuator->name, (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);
            }
        } else if (fabs(pActuator->cur_rpos - pActuator->start_rpos) <= start_rpos_toll) {
            pActuator->position = OFF;
            if (initialRead) {
                snprintf(App.Msg, App.MsgSize, "[%s %s Initial POS=OFF %s]", (char*) ANSI_COLOR_GREEN, pActuator->name, (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);
            }
        } else if (pActuator->cur_rpos < pActuator->start_rpos - start_rpos_toll) {
            // Understroke
            // pActuator->position = OFF;
            pActuator->position = UNDERSTROKE;
            if (initialRead) {
                snprintf(App.Msg, App.MsgSize, "[%s %s Initial POS=Understroke OFF %s]", (char*) ANSI_COLOR_RED, pActuator->name, (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);
            }
        } else if (pActuator->cur_rpos > pActuator->end_rpos + end_rpos_toll) {
            // Overstroke
            // pActuator->position = ON;
            pActuator->position = OVERSTROKE;
            if (initialRead) {
                snprintf(App.Msg, App.MsgSize, "[%s %s Initial POS=Overstroke ON %s]", (char*) ANSI_COLOR_RED, pActuator->name, (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);
            }
        } else {
            // Posizione intermedia
            pActuator->position = INDETERMINATE;
            if (initialRead) {
                snprintf(App.Msg, App.MsgSize, "[%s %s Initial POS INDETERMINATE %0.3f%s]", (char*) ANSI_COLOR_YELLOW, pActuator->name, pActuator->cur_rpos, (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);
            }
        }
        return 1;
    }
    
    return 0;
}


int actuator_speed_to_linear(void *pvActuator, float value, float *newValue) {
    if (pvActuator) {
        if (newValue) {
            LP_ACTUATOR pActuator = (LP_ACTUATOR) pvActuator;
            *newValue = value / 60.0f * pActuator->cam_ratio;
            return 1;
        }
    }
    return 0;
}

int actuator_linear_to_speed(void *pvActuator, float value, float *newValue) {
    if (pvActuator) {
        if (newValue) {
            LP_ACTUATOR pActuator = (LP_ACTUATOR) pvActuator;
            *newValue = value * 60.0f / pActuator->cam_ratio;
            return 1;
        }
    }
    return 0;
}


char *get_actuator_step(void *pvActuator) {

    if (pvActuator) {
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pvActuator;

        if (pActuator->protocol == PROTOCOL_NONE || pActuator->protocol == VIRTUAL_AC_SERVO) {
        
            switch (pActuator->step) {
                case STEP_UNINITIALIZED:
                    return (char*) "(*)UNINIT";
                    break;
                case STEP_INITIALIZED:
                    return (char*) "(*)INIT";
                    break;
                case STEP_READY:
                    return (char*) "(*)READY";
                    break;
                case STEP_SEND_CMD:
                    return (char*) "(*)SEND CMD";
                    break;
                case STEP_MOVING:
                    return (char*) "(*)MOVING";
                    break;
                case STEP_DONE:
                    return (char*) "(*)DONE";
                    break;
                case STEP_ERROR:
                    return (char*) "(*)ERROR";
                    break;
                case STEP_STOPPED:
                    return (char*) "(*)STOPPED";
                    break;
                case STEP_SEND_HOMING:
                    return (char*) "(*)SEND HOMING";
                    break;
                case STEP_HOMING:
                    return (char*) "(*)HOMING";
                    break;                
                case MAX_STEP:
                    return (char*) "(*)MAX STEP";
                    break;
            }
            
        } else {

            switch (pActuator->step) {
                case STEP_UNINITIALIZED:
                    return (char*) "UNINIT";
                    break;
                case STEP_INITIALIZED:
                    return (char*) "INITI";
                    break;
                case STEP_READY:
                    return (char*) "READY";
                    break;
                case STEP_SEND_CMD:
                    return (char*) "SEND CMD";
                    break;
                case STEP_MOVING:
                    return (char*) "MOVING";
                    break;
                case STEP_DONE:
                    return (char*) "DONE";
                    break;
                case STEP_ERROR:
                    return (char*) "ERROR";
                    break;
                case STEP_STOPPED:
                    return (char*) "STOPPED";
                    break;
                case STEP_SEND_HOMING:
                    return (char*) "SEND HOMING";
                    break;
                case STEP_HOMING:
                    return (char*) "HOMING";
                    break;                
                case MAX_STEP:
                    return (char*) "MAX STEP";
                    break;
            }
        }
    }
    return (char*) "";
}


char *get_actuator_pos_desc(void *pvActuator, int position) {

    if (pvActuator) {
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pvActuator;

        switch (position) {
            case ON:
                return (char*) pActuator->pos_name[1];
                break;
            case OFF:
                return (char*) pActuator->pos_name[0];
                break;
            case USER_POSITION:
                return (char*) "USER";
                break;                
            case INTERPOLATE_POSITION:
                return (char*) "INTERPL";
                break;                
            case INDETERMINATE:
                return (char*) "...";
                break;
            case OVERSTROKE:
                return (char*) "OVER";
                break;
            case UNDERSTROKE:
                return (char*) "UNDER";
                break;
            default:
                return (char*) "[???]";
                break;
        }
    }
    return (char*) "";
}




char *get_actuator_driver_status(void *pvActuator) {

    if (pvActuator) {
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pvActuator;

        pActuator->driverStatusDesc[0] = 0;

        if (pActuator->homingDone) {
            strncat((char*) pActuator->driverStatusDesc, (char*) "H", (sizeof (pActuator->driverStatusDesc) - 4 - strlen((char*) pActuator->driverStatusDesc)));
        } else if (pActuator->homingRequest) {
            strncat((char*) pActuator->driverStatusDesc, (char*) "h", (sizeof (pActuator->driverStatusDesc) - 4 - strlen((char*) pActuator->driverStatusDesc)));            
        } else {
            strncat((char*) pActuator->driverStatusDesc, (char*) "-", (sizeof (pActuator->driverStatusDesc) - 4 - strlen((char*) pActuator->driverStatusDesc)));            
        }
        

        if (pActuator->driverStatus & BIT1 && pActuator->driverStatus & BIT11) {
            strncat((char*) pActuator->driverStatusDesc, (char*) "C", (sizeof (pActuator->driverStatusDesc) - 4 - strlen((char*) pActuator->driverStatusDesc)));
        } else if (pActuator->driverStatus & BIT1 && !(pActuator->driverStatus & BIT11)) {
            strncat((char*) pActuator->driverStatusDesc, (char*) "c", (sizeof (pActuator->driverStatusDesc) - 4 - strlen((char*) pActuator->driverStatusDesc)));
        } else if (!(pActuator->driverStatus & BIT1) && pActuator->driverStatus & BIT11) {
            strncat((char*) pActuator->driverStatusDesc, (char*) ".", (sizeof (pActuator->driverStatusDesc) - 4 - strlen((char*) pActuator->driverStatusDesc)));
        } else {
            strncat((char*) pActuator->driverStatusDesc, (char*) "-", (sizeof (pActuator->driverStatusDesc) - 4 - strlen((char*) pActuator->driverStatusDesc)));
        }

        
        if (pActuator->driverStatus & BIT2 && pActuator->driverStatus & BIT12) {
            strncat((char*) pActuator->driverStatusDesc, (char*) "T", (sizeof (pActuator->driverStatusDesc) - 4 - strlen((char*) pActuator->driverStatusDesc)));
        } else if (pActuator->driverStatus & BIT2 && !(pActuator->driverStatus & BIT12)) {
            strncat((char*) pActuator->driverStatusDesc, (char*) "t", (sizeof (pActuator->driverStatusDesc) - 4 - strlen((char*) pActuator->driverStatusDesc)));
        } else if (!(pActuator->driverStatus & BIT2) && pActuator->driverStatus & BIT12) {
            strncat((char*) pActuator->driverStatusDesc, (char*) ".", (sizeof (pActuator->driverStatusDesc) - 4 - strlen((char*) pActuator->driverStatusDesc)));
        } else {
            strncat((char*) pActuator->driverStatusDesc, (char*) "-", (sizeof (pActuator->driverStatusDesc) - 4 - strlen((char*) pActuator->driverStatusDesc)));
        }


        
        if (pActuator->driverStatus & BIT3 && pActuator->driverStatus & BIT13) {
            strncat((char*) pActuator->driverStatusDesc, (char*) "S", (sizeof (pActuator->driverStatusDesc) - 4 - strlen((char*) pActuator->driverStatusDesc)));
        } else if (pActuator->driverStatus & BIT3 && !(pActuator->driverStatus & BIT13)) {
            strncat((char*) pActuator->driverStatusDesc, (char*) "s", (sizeof (pActuator->driverStatusDesc) - 4 - strlen((char*) pActuator->driverStatusDesc)));
        } else if (!(pActuator->driverStatus & BIT3) && pActuator->driverStatus & BIT13) {
            strncat((char*) pActuator->driverStatusDesc, (char*) ".", (sizeof (pActuator->driverStatusDesc) - 4 - strlen((char*) pActuator->driverStatusDesc)));
        } else {
            strncat((char*) pActuator->driverStatusDesc, (char*) "-", (sizeof (pActuator->driverStatusDesc) - 4 - strlen((char*) pActuator->driverStatusDesc)));
        }


        if (pActuator->driverStatus & BIT4 && pActuator->driverStatus & BIT14) {
            strncat((char*) pActuator->driverStatusDesc, (char*) "A", (sizeof (pActuator->driverStatusDesc) - 4 - strlen((char*) pActuator->driverStatusDesc)));
        } else if (pActuator->driverStatus & BIT4 && !(pActuator->driverStatus & BIT14)) {
            strncat((char*) pActuator->driverStatusDesc, (char*) "a", (sizeof (pActuator->driverStatusDesc) - 4 - strlen((char*) pActuator->driverStatusDesc)));
        } else if (!(pActuator->driverStatus & BIT4) && pActuator->driverStatus & BIT14) {
            strncat((char*) pActuator->driverStatusDesc, (char*) ".", (sizeof (pActuator->driverStatusDesc) - 4 - strlen((char*) pActuator->driverStatusDesc)));
        } else {
            strncat((char*) pActuator->driverStatusDesc, (char*) "-", (sizeof (pActuator->driverStatusDesc) - 4 - strlen((char*) pActuator->driverStatusDesc)));
        }

        if (pActuator->driverStatus & BIT5 && pActuator->driverStatus & BIT15) {
            strncat((char*) pActuator->driverStatusDesc, (char*) "D", (sizeof (pActuator->driverStatusDesc) - 4 - strlen((char*) pActuator->driverStatusDesc)));
        } else if (pActuator->driverStatus & BIT5 && !(pActuator->driverStatus & BIT15)) {
            strncat((char*) pActuator->driverStatusDesc, (char*) "d", (sizeof (pActuator->driverStatusDesc) - 4 - strlen((char*) pActuator->driverStatusDesc)));
        } else if (!(pActuator->driverStatus & BIT5) && pActuator->driverStatus & BIT15) {
            strncat((char*) pActuator->driverStatusDesc, (char*) ".", (sizeof (pActuator->driverStatusDesc) - 4 - strlen((char*) pActuator->driverStatusDesc)));
        } else {
            strncat((char*) pActuator->driverStatusDesc, (char*) "-", (sizeof (pActuator->driverStatusDesc) - 4 - strlen((char*) pActuator->driverStatusDesc)));
        }
        

        return pActuator->driverStatusDesc;
    }

    return (char*)"";
}

void do_acctuator_mirror() {
    for (int i = 0; i < machine.num_actuator; i++) {
        LP_ACTUATOR pActuator = (LP_ACTUATOR) & machine.actuator[i];
        if (!pActuator->actuator_mirror)
            pActuator->actuator_mirror = (void*) calloc(sizeof (machine.actuator[0]), 1);
        memcpy((void*) pActuator->actuator_mirror, (void*) machine.actuator, sizeof (machine.actuator[0]));
    }
}

void update_actuator_virtual_pos(void *pvActuator) {
    if (pvActuator) {
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pvActuator;
        if (pActuator->cur_rpos > pActuator->end_rpos * 1.000001) {
            pActuator->cur_vpos = 1001.0f;
        } else if (pActuator->cur_rpos < pActuator->start_rpos * 0.999999) {
            pActuator->cur_vpos = -1.0f;
        } else {
            pActuator->cur_vpos = pActuator->cur_rpos / (pActuator->end_rpos - pActuator->start_rpos) * 1000.0f;
        }
    }
}



// Inposta l'uscita sulla sheda di IO ausiliaria (comando il movimento del China Motor)

int actuator_set_aux_io(void *pvActuator, int newValue1, int newValue2) {
    int retVal = 0;
    if (pvActuator) {
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pvActuator;
        uint32_t i;
        if (pActuator->auxBoardId && (pActuator->auxDO1 || pActuator->auxDO2)) {
            for (i = 0; i < machine.numIOBoardSlots; i++) {
                if (machine.ioBoardSlots[i].id == pActuator->auxBoardId) {
                    if ((int) pActuator->auxDO1 > 0 && pActuator->auxDO1 <= machine.ioBoardSlots[i].numDigitalOUT) {
                        if (machine.ioBoardSlots[i].digitalOUT[pActuator->auxDO1 - 1] != newValue1) {
                            machine.ioBoardSlots[i].digitalOUT[pActuator->auxDO1 - 1] = newValue1;
                            // snprintf(App.Msg, App.MsgSize, "[%sActuator auxDO1:%d->%d%s]\n", (char*) ANSI_COLOR_GREEN, pActuator->auxDO1-1, newValue1, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);
                            retVal = 1;
                        }
                    }
                    if ((int) pActuator->auxDO2 > 0 && pActuator->auxDO2 <= machine.ioBoardSlots[i].numDigitalOUT) {
                        if (machine.ioBoardSlots[i].digitalOUT[pActuator->auxDO2 - 1] != newValue2) {
                            machine.ioBoardSlots[i].digitalOUT[pActuator->auxDO2 - 1] = newValue2;
                            // snprintf(App.Msg, App.MsgSize, "[%sActuator auxDO2:%d->%d%s]\n", (char*) ANSI_COLOR_GREEN, pActuator->auxDO2-1, newValue2, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);
                            retVal = 1;
                        }
                    }
                }
            }
        }
    }
    return retVal;
}


/////////////////////////////////////////////////
// Chiamata durante l'esecuzione del movimento
//

int process_actuators_move_loop(void) {
    uint32_t i;
    char str[256];

    for (i = 0; i < machine.num_actuator; i++) {
        LP_ACTUATOR pActuator = (LP_ACTUATOR) & machine.actuator[i];

        ////////////////////////
        // calcolo distanza
        //
        pActuator->dist = pActuator->target_rpos - pActuator->cur_rpos;


        
        if (pActuator->step == STEP_UNINITIALIZED) {
            // A carico della routine di rispristino

        } else if (pActuator->step == STEP_INITIALIZED) {
            // Inizializzazione ulteriore...
            pActuator->step = STEP_READY;

        } else if (pActuator->step == STEP_READY) {
            // In standby

        } else if (pActuator->step == STEP_SEND_HOMING) {
            
            // Invio comando Homing
            pActuator->homingRequest = 1;
            pActuator->error = 0;
            pActuator->outStrokeError = 0;
            pActuator->driverStatus = 0;
                // BIT1 ->  Comando avvio ricevuto

            pActuator->readCounter = 0;
            pActuator->positionReached = 0;
            

            ///////////////////////////////////////////////////////////
            // Invio comando alla sequenza di gestione della scheda
            //
            if (handle_actuator_send_move_cmd(&machine.actuator[i], 0 + 0) < 0) {
            }
            
            
        } else if (pActuator->step == STEP_HOMING) {
            // In Homing


            
        } else if (pActuator->step == STEP_SEND_CMD) {

            // snprintf(App.Msg, App.MsgSize, "[%sActuator start time:%d%s]\n", (char*) ANSI_COLOR_GREEN, pActuator->start_time, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);

            pActuator->error = 0;
            pActuator->outStrokeError = 0;
            pActuator->driverStatus = 0;
                // BIT1 ->  Comando avvio ricevuto

            if (pActuator->position != pActuator->target_position) {

                pActuator->start_time = pActuator->start_time1 = pActuator->start_time2 = pActuator->start_time3 = xTaskGetTickCount();
                pActuator->CheckFeedbackTimeout = true;
                
                if (pActuator->target_position == ON) {
                } else if (pActuator->target_position == OFF) {
                } else if (pActuator->target_position == USER_POSITION) {
                    // Imposta il timeout
                    float speed_rpm = 0.0f;
                    if (pActuator->target_rpos > pActuator->cur_rpos) {
                        speed_rpm = pActuator->speed_auto1;
                    } else {
                        speed_rpm = pActuator->speed_auto2;
                    }
                    pActuator->timeout3_ms = get_timeout_by_stroke ( (void**)pActuator, fabs(pActuator->target_rpos - pActuator->cur_rpos), speed_rpm);
                    
                } else if (pActuator->target_position == INTERPOLATE_POSITION) {
                    // Non arresta il comando se il feddback è interrotto
                    pActuator->CheckFeedbackTimeout = false;
                    
                    // Imposta il timeout
                    // pActuator->timeout3_ms = ...
                } else {
                }
                
                pActuator->readCounter = 0;
                pActuator->positionReached = 0;

                ///////////////////////////////////////////////////////////
                // Invio comando alla sequenza di gestione della scheda
                //
                if (handle_actuator_send_move_cmd(&machine.actuator[i], 0 + 0) < 0) {
                }
                
            } else {
                pActuator->step = STEP_READY;
            }



        } else if (pActuator->step == STEP_MOVING) {
            ///////////////////////////////////////////////////////////
            // comando in esecuzione
            //

            if (pActuator->readCounter) {
                //
                // almeno una lettura eseguita ...
                //

                float end_rpos_toll = 0.10f, start_rpos_toll = 0.10f;
                
                

                if (machine.status == AUTOMATIC) {
                    start_rpos_toll = pActuator->start_rpos_toll;
                    end_rpos_toll = pActuator->end_rpos_toll;
                } else {            
                }
                
                if (pActuator->target_position == OUTSIDE) {

                    if IS_ACTUATOR_LINKED(i) {
                        // collegato alla scheda : lettura del movimento
                    } else {
                        // Mosso da simulate_actuators_move()
                    }


        
                    if (pActuator->cur_rpos >= pActuator->end_rpos - end_rpos_toll && 
                            pActuator->cur_rpos <= pActuator->end_rpos + end_rpos_toll) {

                        /////////////////////////////////////
                        // Assegnamento movimento eseguito
                        //
                        handle_actuator_position_reached ( (void*)pActuator );


                    } else if (pActuator->cur_rpos > (pActuator->end_rpos+end_rpos_toll) ) {
                        ////////////////////////////
                        // extra corsa avanti
                        //
                        pActuator->outStrokeError++;
                        if (pActuator->outStrokeError >= MAX_OVER_STROKE_ERROR) {
                            snprintf(str, sizeof (str), "Actuator %s overstroke (side:%d Position:%0.3f/%0.3f - DriverPos.:%d - %d.%d (%derr)"
                                    ,pActuator->name, machine.serialSlots[i].runningCommand
                                    ,pActuator->cur_rpos, pActuator->end_rpos
                                    ,pActuator->driverPosition, pActuator->driverTurns, pActuator->driverPulses
                                    ,pActuator->outStrokeError
                                    );
                            if (machine.status == AUTOMATIC) {
                                if (generate_alarm((char*) str, 6031, 0, (int) ALARM_ERROR, 0+1) < 0) {
                                }
                                // Asse in stato di errore
                                pActuator->step = STEP_ERROR;
                            } else {
                                if (generate_alarm((char*) str, 6031, 0, (int) ALARM_WARNING, 0+1) < 0) {
                                }
                                pActuator->step = STEP_READY;
                            }
                        } else {
                            snprintf(App.Msg, App.MsgSize, "[%sO%0.3fmm%s]", (char*) ANSI_COLOR_RED, pActuator->cur_rpos-pActuator->end_rpos, (char*) ANSI_COLOR_RESET);
                            vDisplayMessage(App.Msg);
                        }
                        
                    } else if (pActuator->cur_rpos < (pActuator->start_rpos-start_rpos_toll) ) {
                        ////////////////////////////
                        // extra corsa indietro
                        //
                        pActuator->outStrokeError++;
                        if (pActuator->outStrokeError >= MAX_OVER_STROKE_ERROR) {
                            snprintf(str, sizeof (str), "Actuator %s understroke (side:%d Position:%0.3f/%0.3f - DriverPos.:%d - %d.%d (%derr)"
                                    ,pActuator->name, machine.serialSlots[i].runningCommand
                                    ,pActuator->cur_rpos, pActuator->end_rpos
                                    ,pActuator->driverPosition, pActuator->driverTurns, pActuator->driverPulses
                                    ,pActuator->outStrokeError
                                    );
                            if (machine.status == AUTOMATIC) {
                                if (generate_alarm((char*) str, 6032, 0, (int) ALARM_ERROR, 0+1) < 0) {
                                }
                                // Asse in stato di errore
                                pActuator->step = STEP_ERROR;
                            } else {
                                if (generate_alarm((char*) str, 6031, 0, (int) ALARM_WARNING, 0+1) < 0) {
                                }
                                pActuator->step = STEP_READY;
                            }                            
                        } else {
                            snprintf(App.Msg, App.MsgSize, "[%sO%0.3fmm%s]", (char*) ANSI_COLOR_RED, pActuator->cur_rpos-pActuator->end_rpos, (char*) ANSI_COLOR_RESET);
                            vDisplayMessage(App.Msg);
                        }

                    } else {
                        // Posizone indeterminata
                        pActuator->position = INDETERMINATE;
                        // Reset errore lettura posizionamento
                        pActuator->outStrokeError = 0;
                    }



                    ///////////////////////////////                    
                    if (pActuator->start_time3 == 0) {
                        if ( fabs(pActuator->cur_rpos - pActuator->end_rpos) <= 0.05) {
                            pActuator->start_time3 = GLCurTimeMs;
                        }
                    }                    
                    

                } else if (pActuator->target_position == INSIDE) {

                    if IS_ACTUATOR_LINKED(i) {
                        // collegato alla scheda : lettura del movimento
                    } else {
                        // Mosso da simulate_actuators_move()
                    }

                    if ( pActuator->cur_rpos <= pActuator->start_rpos + start_rpos_toll && 
                            pActuator->cur_rpos >= pActuator->start_rpos - start_rpos_toll) {

                        /////////////////////////////////////
                        // Assegnamento movimento eseguito
                        //
                        handle_actuator_position_reached ( (void*)pActuator );

                    } else if ( pActuator->cur_rpos < (pActuator->start_rpos - start_rpos_toll) ) {
                        //////////////////////////
                        // extra corsa indietro
                        //
                        pActuator->outStrokeError++;
                        if (pActuator->outStrokeError >= MAX_OVER_STROKE_ERROR) {
                            snprintf(str, sizeof (str), "Actuator %s understroke (side:%d Position:%0.3f/%0.3f - DriverPos.:%d-%d.%d",
                                    pActuator->name, machine.serialSlots[i].runningCommand,
                                    pActuator->cur_rpos, pActuator->start_rpos,
                                    machine.serialSlots[i].driverPosition, machine.serialSlots[i].driverTurns, machine.serialSlots[i].driverPulses
                                    );
                            if (machine.status == AUTOMATIC) {
                                if (generate_alarm((char*) str, 6032, 0, (int) ALARM_ERROR, 0+1) < 0) {
                                }
                                // Asse in stato di errore
                                pActuator->step = STEP_ERROR;
                            } else {
                                if (generate_alarm((char*) str, 6031, 0, (int) ALARM_WARNING, 0+1) < 0) {
                                }                                
                                pActuator->step = STEP_READY;
                            }
                        } else {
                            snprintf(App.Msg, App.MsgSize, "[%sU%0.3fmm%s]", (char*) ANSI_COLOR_RED, pActuator->cur_rpos-pActuator->start_rpos, (char*) ANSI_COLOR_RESET);
                            vDisplayMessage(App.Msg);
                            pActuator->step = STEP_READY;
                        }

                    } else if (pActuator->cur_rpos > (pActuator->end_rpos+end_rpos_toll) ) {
                        ////////////////////////////
                        // extra corsa avanti
                        //
                        pActuator->outStrokeError++;
                        if (pActuator->outStrokeError >= MAX_OVER_STROKE_ERROR) {
                            snprintf(str, sizeof (str), "Actuator %s overstroke (side:%d Position:%0.3f/%0.3f - DriverPos.:%d - %d.%d (%derr)"
                                    ,pActuator->name, machine.serialSlots[i].runningCommand
                                    ,pActuator->cur_rpos, pActuator->end_rpos
                                    ,pActuator->driverPosition, pActuator->driverTurns, pActuator->driverPulses
                                    ,pActuator->outStrokeError
                                    );
                            if (machine.status == AUTOMATIC) {
                                if (generate_alarm((char*) str, 6031, 0, (int) ALARM_ERROR, 0+1) < 0) {
                                }
                                // Asse in stato di errore
                                pActuator->step = STEP_ERROR;
                            } else {
                                if (generate_alarm((char*) str, 6031, 0, (int) ALARM_WARNING, 0+1) < 0) {
                                }                                
                                pActuator->step = STEP_READY;
                            }
                        } else {
                            snprintf(App.Msg, App.MsgSize, "[%sO%0.3fmm%s]", (char*) ANSI_COLOR_RED, pActuator->cur_rpos-pActuator->end_rpos, (char*) ANSI_COLOR_RESET);
                            vDisplayMessage(App.Msg);
                        }
                        
                    } else {
                        // Posizone indeterminata
                        pActuator->position = INDETERMINATE;
                        // Reset errore lettura posizionamento
                        pActuator->outStrokeError = 0;
                    } 
                    
                } else if (pActuator->target_position == USER_POSITION || pActuator->target_position == INTERPOLATE_POSITION) {
                    float target_rpos_toll = 0.10f;

                    target_rpos_toll = MAX(pActuator->start_rpos_toll, pActuator->end_rpos_toll);

                    if (target_rpos_toll <= 0.0f)
                        target_rpos_toll = machine.App.Epsilon;
                    
                    if IS_ACTUATOR_LINKED(i) {
                        // collegato alla scheda : lettura del movimento
                    } else {
                        // Mosso da simulate_actuators_move()
                    }

                    if ( fabs(pActuator->cur_rpos - pActuator->target_rpos) < target_rpos_toll ) {
                        

                        /////////////////////////////////////
                        // Assegnamento movimento eseguito
                        //
                        handle_actuator_position_reached ( (void*)pActuator );

                        
                    } else {
                        // Posizone indeterminata
                        pActuator->position = INDETERMINATE;
                        // Reset errore lettura posizionamento
                        pActuator->outStrokeError = 0;
                    } 
                }       
            }


        } else if (pActuator->step == STEP_DONE) {
            pActuator->step == STEP_READY;
        }
    }

    return 1;
}


int32_t handle_actuator_position_reached ( void *pvActuator ) {
    if (pvActuator) {
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pvActuator;

        
        pActuator->position = pActuator->target_position;
        pActuator->positionReached = 1;
        pActuator->step = STEP_READY;

        pActuator->start_time2 = GLCurTimeMs;
        pActuator->start_time3 = GLCurTimeMs;

        if (pActuator->target_position == OUTSIDE) {

            pActuator->time_ms11 = xrt_set_delta_time(pActuator->start_time1, pActuator->start_time);
            pActuator->time_ms12 = xrt_set_delta_time(pActuator->start_time2, pActuator->start_time1);
            pActuator->time_ms13 = xrt_set_delta_time(pActuator->start_time3, pActuator->start_time2);

            pActuator->time_ms1 = xrt_set_delta_time(GLCurTimeMs, pActuator->start_time);

        } else if (pActuator->target_position == INSIDE) {
            

            pActuator->time_ms21 = xrt_set_delta_time(pActuator->start_time1, pActuator->start_time);
            pActuator->time_ms22 = xrt_set_delta_time(pActuator->start_time2, pActuator->start_time1);
            pActuator->time_ms23 = xrt_set_delta_time(pActuator->start_time3, pActuator->start_time2);

            pActuator->time_ms2 = xrt_set_delta_time(GLCurTimeMs, pActuator->start_time);
            
        } else if (pActuator->target_position == USER_POSITION || pActuator->target_position == INTERPOLATE_POSITION) {
            

            pActuator->time_ms21 = xrt_set_delta_time(pActuator->start_time1, pActuator->start_time);
            pActuator->time_ms22 = xrt_set_delta_time(pActuator->start_time2, pActuator->start_time1);
            pActuator->time_ms23 = xrt_set_delta_time(pActuator->start_time3, pActuator->start_time2);

            pActuator->time_ms2 = xrt_set_delta_time(GLCurTimeMs, pActuator->start_time);
        }
        
        return 1;
    }
    
    return -1;
}


float get_timeout_by_stroke ( void *pvActuator, float stroke_mm, float speed_rpm ) {
    if (pvActuator) {
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pvActuator;
        return stroke_mm / (float)(speed_rpm * pActuator->cam_ratio / 60.0f) * 1.10f * 1000.0f + 1500.0f;
    }
    return 0.0f;
}


//////////////////////////////////////////////////////////
//  Utility dell'attuatore : impopsta in pendingCommand
//
int handle_set_as_pending_command(void *pvActuator, bool *okToNextStep) {
    int retVal = 0;

    if (pvActuator && okToNextStep) {
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pvActuator;
        SerialSlot *pSerialSlot = (SerialSlot*) pActuator->pSerialSlot;
        CANSlot *pCANSlot = (CANSlot*) pActuator->pCANSlot;
        char str[256];

        if (pActuator->target_position == OFF) {
            // Movimento indietro 
            pActuator->step = STEP_MOVING;
            // Set Comando in attesa di esecuzione
            if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN || pActuator->protocol == MODBUS_AC_SERVO_DELTA) {
                pSerialSlot->pendingCommand = -1;
                *okToNextStep = true;
            } else if (pActuator->protocol == CANOPEN_AC_SERVO_DELTA) {
                pCANSlot->pendingCommand = -1;
                *okToNextStep = true;
            }
            
        } else if (pActuator->target_position == ON) {
            // Movimento avanti : 
            pActuator->step = STEP_MOVING;
            // Set Comando in attesa di esecuzione
            if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN || pActuator->protocol == MODBUS_AC_SERVO_DELTA) {
                pSerialSlot->pendingCommand = 1;
                *okToNextStep = true;
            } else if (pActuator->protocol == CANOPEN_AC_SERVO_DELTA) {
                pCANSlot->pendingCommand = 1;
                *okToNextStep = true;
            }            

        } else if (pActuator->target_position == USER_POSITION) {
            if (pActuator->target_rpos <= pActuator->end_rpos && pActuator->target_rpos >= pActuator->start_rpos) {
                pActuator->step = STEP_MOVING;
                // Set Comando in attesa di esecuzione
                if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN || pActuator->protocol == MODBUS_AC_SERVO_DELTA) {
                    pSerialSlot->pendingCommand = 2;
                    *okToNextStep = true;
                } else if (pActuator->protocol == CANOPEN_AC_SERVO_DELTA) {
                    pCANSlot->pendingCommand = 2;
                    *okToNextStep = true;
                }
            } else {
                snprintf(str, sizeof (str), "Actuator %s : target position (%0.3f) out of range", pActuator->name, pActuator->target_rpos);
                if (generate_alarm((char*) str, 6021, 0, (int) ALARM_ERROR, 0+1) < 0) {
                }
                return -1;
            }                                    
        } else if (pActuator->target_position == INTERPOLATE_POSITION) {
            if (pActuator->target_rpos <= pActuator->end_rpos && pActuator->target_rpos >= pActuator->start_rpos) {
                pActuator->step = STEP_MOVING;
                // Set Comando in attesa di esecuzione
                if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN || pActuator->protocol == MODBUS_AC_SERVO_DELTA) {
                    // N.B.: Non possibile in Modbus
                    // pSerialSlot->pendingCommand = ...;
                    // *okToNextStep = true;
                    *okToNextStep = false;
                } else if (pActuator->protocol == CANOPEN_AC_SERVO_DELTA) {
                    pCANSlot->pendingCommand = 11;
                    *okToNextStep = true;
                }
            } else {
                snprintf(str, sizeof (str), "Actuator %s : target position (%0.3f) out of range", pActuator->name, pActuator->target_rpos);
                if (generate_alarm((char*) str, 6021, 0, (int) ALARM_ERROR, 0+1) < 0) {
                }
                return -1;
            }                                    
        } else {
            snprintf(str, sizeof (str), "Actuator %s unknown target position:%d", pActuator->name, pActuator->target_position);
            if (generate_alarm((char*) str, 6021, 0, (int) ALARM_ERROR, 0+1) < 0) {
            }
            return -1;
        }
        
        
        return 1;
    }
    
    return -1;
}

/////////////////////////////////////////////////
//  Passa il comando all driver dell'attuatore
//

int handle_actuator_send_move_cmd(void *pvActuator, int Mode) {
    int retVal = 0;

    if (pvActuator) {
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pvActuator;
        SerialSlot *pSerialSlot = (SerialSlot*) pActuator->pSerialSlot;
        CANSlot *pCANSlot = (CANSlot*) pActuator->pCANSlot;
        char str[256];

        
        if (pActuator->step == STEP_SEND_CMD || pActuator->step == STEP_SEND_HOMING) {

            if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN || pActuator->protocol == MODBUS_AC_SERVO_DELTA) {
                bool okToNextStep = false;

                if (pSerialSlot) {
                    
                    if (pActuator->boardId != pSerialSlot->boardId || pActuator->stationId != pSerialSlot->stationId) {
                        // Scheda non collegata
                        pActuator->step = STEP_ERROR;
                        pActuator->error = 3;

                        snprintf(str, sizeof (str), "Actuator %s board mislinked!", pActuator->name);
                        if (generate_alarm((char*) str, 6020, 0, (int) ALARM_ERROR, 0+1) < 0) {
                        }

                    } else {
                        
                        if (pActuator->step == STEP_SEND_CMD) {                                
                            if (!pActuator->homingDone) {
                                pActuator->step = STEP_READY;
                                snprintf(str, sizeof (str), "Actuator %s need homing", pActuator->name);
                                if (generate_alarm((char*) str, 6021, 0, (int) ALARM_WARNING, 0+1) < 0) {
                                }
                            return -1;                                
                            } else {
                                // Imposta il pendingCommand dell' interfaccia
                                handle_set_as_pending_command (pvActuator, &okToNextStep);
                            }
                            
                        } else if (pActuator->step == STEP_SEND_HOMING) {

                            if (pActuator->homingRequest == 0 || pActuator->homingRequest == 1) {
                                snprintf(App.Msg, App.MsgSize, "[%s CMD Homing on %s (step:%s)%s]\n", (char*) ANSI_COLOR_YELLOW, pActuator->name, (char*)get_actuator_step((void*)pActuator), (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);
                                pActuator->homingDone = false;
                                pActuator->homingRequest = 1;
                                pSerialSlot->homingRequest = 1;
                                pActuator->step = STEP_HOMING;
                                okToNextStep = 2;
                                
                            } else if (pActuator->homingRequest == 2) {
                                // Homing in corso
                                okToNextStep = 3;
                                snprintf(str, sizeof (str), "Actuator %s has homing in progress...", pActuator->name);
                                if (generate_alarm((char*) str, 6022, 0, (int) ALARM_WARNING, 0+1) < 0) {
                                }
                            } else {                                
                            }
                        }
                    }

                } else {
                    ///////////////////////////
                    // Mancanza interfaccia
                    //
                    if (App.SimulateMode) {
                        // Imposta il pendingCommand dell' interfaccia
                        handle_set_as_pending_command (pvActuator, &okToNextStep);

                    } else {
                        ////////////////////////////////////////////////
                        // Intercettato dalle routine di avviamento ??
                        // Errore
                        // 
                        pActuator->step = STEP_ERROR;
                        pActuator->error = 2;
                    }
                }


                if (okToNextStep == 1) {
                    // Comando inoltrato                                        
                } else if (okToNextStep == 2) {
                } else if (okToNextStep == 3) {
                } else {
                    snprintf(str, sizeof (str), "MODBUS Actuator %s has no slot!", pActuator->name);
                    if (generate_alarm((char*) str, 6022, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                    }
                }


            } else if (pActuator->protocol == CANOPEN_AC_SERVO_DELTA) {
                bool okToNextStep = false;

                if (pCANSlot) {
                    if (pActuator->boardId != pCANSlot->boardId || (pActuator->stationId != pCANSlot->stationId && pCANSlot->stationId != -1)) {
                        // Scheda non collegata
                        pActuator->step = STEP_ERROR;
                        pActuator->error = 3;

                        snprintf(str, sizeof (str), "Actuator %s board mislinked!", pActuator->name);
                        if (generate_alarm((char*) str, 6020, 0, (int) ALARM_ERROR, 0+1) < 0) {
                        }

                    } else {
                        if (pActuator->step == STEP_SEND_CMD) {
                            
                            if (!pActuator->homingDone) {
                                pActuator->step = STEP_READY;
                                snprintf(str, sizeof (str), "Actuator %s need homing", pActuator->name);
                                if (generate_alarm((char*) str, 6021, 0, (int) ALARM_WARNING, 0+1) < 0) {
                                }                                
                            return -1;

                            } else {                            
                                // Imposta il pendingCommand dell' interfaccia
                                handle_set_as_pending_command (pvActuator, &okToNextStep);
                            }
                            
                        } else if (pActuator->step == STEP_SEND_HOMING) {    
                            if (pActuator->homingRequest == 0 || pActuator->homingRequest == 1) {
                                snprintf(App.Msg, App.MsgSize, "[%s CMD Homing on %s (step:%s)%s]\n", (char*) ANSI_COLOR_YELLOW, pActuator->name, (char*)get_actuator_step((void*)pActuator), (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);                                
                                pActuator->homingDone = false;
                                pActuator->homingRequest = 1;
                                pCANSlot->homingRequest = 1;
                                pActuator->step = STEP_HOMING;
                                okToNextStep = 2;
                            } else if (pActuator->homingRequest == 2) {
                                // Homing in corso
                                okToNextStep = 3;
                                snprintf(str, sizeof (str), "Actuator %s has homing in progress...", pActuator->name);
                                if (generate_alarm((char*) str, 6023, 0, (int) ALARM_WARNING, 0+1) < 0) {
                                }
                            } else {                                
                            }
                        }
                    }
                    
                } else {
                    
                    ///////////////////////////
                    // Mancanza interfaccia
                    //
                    if (App.SimulateMode) {
                        // Imposta il pendingCommand dell' interfaccia
                        handle_set_as_pending_command (pvActuator, &okToNextStep);

                    } else {
                        ////////////////////////////////////////////////
                        // Intercettato dalle routine di avviamento ??
                        // Errore
                        // 
                        pActuator->step = STEP_ERROR;
                        pActuator->error = 2;
                    }
                }


                if (okToNextStep == 1) {
                    // Comando inoltrato
                } else if (okToNextStep == 2) {
                } else if (okToNextStep == 3) {
                } else {
                    snprintf(str, sizeof (str), "CANBUS Actuator %s has no slot!", pActuator->name);
                    if (generate_alarm((char*) str, 6022, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                    }
                }


            } else if (pActuator->protocol == VIRTUAL_AC_SERVO) {
                pActuator->step = STEP_MOVING;

            } else if (pActuator->protocol == BISTABLE_PN_VALVE) {


                if (pActuator->boardId) {
                    if (pActuator->IOindex1) {
                    }
                    if (pActuator->IOindex2) {
                    }


                    if (pActuator->position == 0) {
                        // Movimento indietro
                    } else if (pActuator->position == 1) {
                        // Movimento avanti
                    }

                    pActuator->step = STEP_MOVING;

                } else {
                    // Interfaccia non registrata
                    if (App.SimulateMode) {
                        pActuator->step = STEP_MOVING;
                    } else {
                        pActuator->step = STEP_ERROR;
                        pActuator->error = 2;
                    }
                }





            } else if (pActuator->protocol == MONOSTABLE_PN_VALVE) {

                if (pActuator->boardId) {
                    if (pActuator->IOindex1) {
                    }
                    if (pActuator->IOindex2) {
                    }

                    if (pActuator->position == 0) {
                        // Movimento indietro
                    } else if (pActuator->position == 1) {
                        // Movimento avanti
                    }

                    pActuator->step = STEP_MOVING;

                } else {
                    // Interfaccia non registrata
                    if (App.SimulateMode) {
                        pActuator->step = STEP_MOVING;
                    } else {
                        pActuator->step = STEP_ERROR;
                        pActuator->error = 2;
                    }
                }


            } else {
                // Nessun protocollo
                if (App.SimulateMode) {
                    pActuator->step = STEP_MOVING;
                } else {
                    snprintf(str, sizeof (str), "Actuator %s no interface!", pActuator->name);
                    if (generate_alarm((char*) str, 6023, 0, (int) ALARM_ERROR, 0+1) < 0) {
                    }
                    pActuator->step = STEP_ERROR;
                    pActuator->error = 3;
                }
            }


        } else if (pActuator->step == STEP_MOVING) {


        } else {
        }
    }

    return retVal;
}









int get_actuator_control_mode(void *pvActuator) {
    int retVal = -1;
    if (pvActuator) {
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pvActuator;
        SerialSlot *pSerialSlot = (SerialSlot *) NULL;
        CANSlot *pCANSlot = (CANSlot *) NULL;

        if (pActuator->pSerialSlot) {
            pSerialSlot = (SerialSlot*) pActuator->pSerialSlot;
            if (pActuator->boardId != pSerialSlot->boardId || pActuator->stationId != pSerialSlot->stationId) {
            } else {
                retVal = pSerialSlot->controlMode1B - 1;
            }
        } else if (pActuator->pCANSlot) {
            pCANSlot = (CANSlot*) pActuator->pCANSlot;
            if (pActuator->boardId != pCANSlot->boardId || (pActuator->stationId != pCANSlot->stationId && pCANSlot->stationId != -1)) {
            } else {
                retVal = pCANSlot->controlMode1B - 1;
            }
        }
    }
    return retVal;
}


///////////////////////////////////////////////////////////////////////
//
// N.B.:    Nel driver la posizione va da 0..PulsePerOverflow (16bt)
//          Nella logica da da 0..PulsePerTurn (10000)
//
int handle_actuator_position_mode(void *pvActuator,
        int32_t targetTurns, int32_t targetPulses, int32_t speed_rpm, int32_t acc_rpms2_1, int32_t dec_ms,
        int32_t targetTurns2, int32_t targetPulses2, int32_t speed_rpm2, int32_t acc_rpms2_2, int32_t dec_ms2,
        int32_t homingTurns, int32_t homingPulses , int32_t homing_speed_rpm) {

    int retVal = 0;
    char hHigh = 0, bLow = 0;
    int rc;
    int addr = 0;
    int newValue = 0;
    uint16_t setValue = 0;
    int pause_sec = 0;


    if (pvActuator) {
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pvActuator;
        SerialSlot *pSerialSlot = (SerialSlot *) NULL;
        CANSlot *pCANSlot = (CANSlot *) NULL;

        if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN || pActuator->protocol == MODBUS_AC_SERVO_DELTA) {

            if (pActuator->pSerialSlot) {
                pSerialSlot = (SerialSlot*) pActuator->pSerialSlot;

                if (pActuator->boardId != pSerialSlot->boardId || pActuator->stationId != pSerialSlot->stationId) {
                    snprintf(App.Msg, App.MsgSize, "[%s wrong serial board link %d<>%d  %s]\n", (char*) ANSI_COLOR_RED, pActuator->boardId, pSerialSlot->boardId, (char*) ANSI_COLOR_RESET);
                    vDisplayMessage(App.Msg);
                    return -1;
                }

                snprintf(App.Msg, App.MsgSize, "[%s Setting MODBUS  AC Servo #%d parameters {pos:%d.%d - speed:%d - acc:%d}-{pos:%d.%d - speed:%d - acc:%d} %s]\n"
                        , (char*) ANSI_COLOR_YELLOW
                        , pSerialSlot->boardId
                        , targetTurns, targetPulses, speed_rpm, acc_rpms2_1
                        , targetTurns2, targetPulses2, speed_rpm2, acc_rpms2_2
                        , (char*) ANSI_COLOR_RESET
                        );
                vDisplayMessage(App.Msg);


                if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN) {

                    //////////////////////////////////////////////////
                    // Modalita funzionamento (Position Control)
                    //
                    pSerialSlot->controlMode1B = 1 + 1;

                    
                    usleep(15 * 1000);
                    
                    // Scrittura : Pr1.00 = 1 (position control mode)
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 1), 00, 1, true, 2, App.DebugModbusParam) < 0) {
                        retVal = -1; // return -1;
                    }
    
                    // Scrittura : Pr1.22 (source pos = inernal pos settings)
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 1), 22, 1, true, 2, App.DebugModbusParam) < 0) {
                        retVal = -1; // return -1;
                    }
    
                    // Scrittura : Pr1.24 (undocumented : single pos = 2)
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 1), 24, 2, true, 2, App.DebugModbusParam) < 0) {
                        retVal = -1; // return -1;
                    }
                    
                    // Scrittura : Pr1.29 - acceleration (?)
                    int32_t acc_ms = (int32_t) ((3000.0f - 0.0f) / 9.55f / acc_rpms2_1 * 1000.0f);
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 1), 39, acc_ms, true, 2, App.DebugModbusParam) < 0) {
                        retVal = -1; // return -1;
                    }

                    /*
                    ///////////////////////////////////////////////
                    // Scrittura : Pr8.00
                    //  Pr8.00 Internal SON instruction:
                    //      0: Invalid internal servo ON, automatic clearance after the power is applied again.
                    //      1: Internal servo on.
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 8), 00 + 0, 0, true, 2, App.DebugModbusParam) < 0) {
                        // return -1;
                    }
                    */

                    usleep(15 * 1000);
                    
                    
                    //////////////////////////////////////////////////
                    // 1° Scrittura : Pr2.?0 - Posizione andata
                    //
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 2), 00 + 0, targetTurns, true, 2, App.DebugModbusParam) < 0) {
                        retVal = -1; // return -1;
                    }
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 2), 01 + 0, targetPulses, true, 2, App.DebugModbusParam) < 0) {
                        retVal = -1; // return -1;
                    }
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 2), 02 + 0, speed_rpm, true, 2, App.DebugModbusParam) < 0) {
                        retVal = -1; // return -1;
                    }
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 2), 03 + 0, pause_sec, true, 2, App.DebugModbusParam) < 0) {
                        retVal = -1; // return -1;
                    }


                    usleep(15 * 1000);


                    //////////////////////////////////////////////
                    // 2° Scrittura : Pr2.?1 - Posizione ritorno
                    //
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 2), 4 + 0, targetTurns2, true, 2, App.DebugModbusParam) < 0) {
                        retVal = -1; // return -1;
                    }
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 2), 5 + 0, targetPulses2, true, 2, App.DebugModbusParam) < 0) {
                        retVal = -1; // return -1;
                    }
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 2), 6 + 0, speed_rpm2, true, 2, App.DebugModbusParam) < 0) {
                        retVal = -1; // return -1;
                    }
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 2), 7 + 0, pause_sec, true, 2, App.DebugModbusParam) < 0) {
                        retVal = -1; // return -1;
                    }

                    usleep(15 * 1000);

                    ////////////////////////////////////////////////////
                    // 3° Scrittura : Pr2.xx - Posizione neutra
                    //
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 2), 8 + 0, 0/*targetTurns2*/, true, 2, App.DebugModbusParam) < 0) {
                        retVal = -1; // return -1;
                    }
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 2), 9 + 0, 0/*targetPulses2*/, true, 2, App.DebugModbusParam) < 0) {
                        retVal = -1; // return -1;
                    }
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 2), 10 + 0, 10/*speed_rpm2*/, true, 2, App.DebugModbusParam) < 0) {
                        retVal = -1; // return -1;
                    }
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 2), 11 + 0, 0/*pause_sec*/, true, 2, App.DebugModbusParam) < 0) {
                        retVal = -1; // return -1;
                    }


                    usleep(15 * 1000);

                    ////////////////////////////////////////////////////
                    // 4° Scrittura : Pr2.xx - Posizione HOMING
                    //
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 2), 12 + 0, homingTurns, true, 2, App.DebugModbusParam) < 0) {
                        retVal = -1; // return -1;
                    }
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 2), 13 + 0, homingPulses, true, 2, App.DebugModbusParam) < 0) {
                        retVal = -1; // return -1;
                    }
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 2), 14 + 0, homing_speed_rpm, true, 2, App.DebugModbusParam) < 0) {
                        retVal = -1; // return -1;
                    }
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 2), 15 + 0, 0/*pause_sec*/, true, 2, App.DebugModbusParam) < 0) {
                        retVal = -1; // return -1;
                    }


                    


                } else if (pActuator->protocol == MODBUS_AC_SERVO_DELTA) {

                    // Modalita funzionamento (Position Control)
                    pSerialSlot->controlMode1B = 1 + 1;

                    // Scrittura : Pr1.01 = 1 ( PR Position control mode. The command is from internal signal. Execution of 64 positions is via DI signals (POS0 ~ POS5). A variety of homing control is also provided.)
                    // N.B.: Non modificabile a Runtime : richiede il riavvio del driver
                    /*
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, 1, 01, 1, false, 2) < 0) {
                        // return -1;
                    }
                     */



                    /*
                    // P2-11=108 CTRG Command trigged
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, 2, 11, 108, true, 2) < 0) {
                        // return -1;
                    }
                    // P2-12=111 POS0 Position command selection
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, 2, 12, 111, true, 2) < 0) {
                        // return -1;
                    }
                    // P2-13=112 POS1 Position command selection
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, 2, 13, 112, true, 2) < 0) {
                        // return -1;
                    }
                    // P2-14=102 ARST Reset
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, 2, 14, 102, true, 2) < 0) {
                        // return -1;
                    }
                    // P2-15=0 Disabled This DI function is disabled
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, 2, 15, 0, true, 2) < 0) {
                        // return -1;
                    }
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, 2, 16, 0, true, 2) < 0) {
                        // return -1;
                    }
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, 2, 17, 0, true, 2) < 0) {
                        // return -1;
                    }
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, 2, 36, 0, false, 2) < 0) {
                        // return -1;
                    }
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, 2, 37, 0, false, 2) < 0) {
                        // return -1;
                    }
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, 2, 38, 0, false, 2) < 0) {
                        // return -1;
                    }
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, 2, 39, 0, false, 2) < 0) {
                        // return -1;
                    }
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, 2, 40, 0, false, 2) < 0) {
                        // return -1;
                    }
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, 2, 41, 0, false, 2) < 0) {
                        // return -1;
                    }
                     */

                    
                    
                    // Posizione di andata
                    // P6 - 02 PDEF1 Definition of Path 1
                    // PR Definition: P6-02 ~ P7-27, (64 BIT), total 63 groups (2N)
                    //  OPT                         TYPE
                    // Bit7 Bit6    Bit5    Bit4    Bit3 ~ Bit0
                    // -    UNIT    AUTO    INS     1: Constant speed control
                    //                              2: Single positioning control. Motor stops when positioning is completed.
                    // CMD          OVLP    INS     3: Auto positioning control. Motor goes to next dedicated path when positioning is completed.
                    // -    -       -       INS     7: Jump to the dedicated path.
                    // -    -       AUTO    INS     8: Write the specified parameter to the dedicated path.

                    usleep(MODBUS_SLEEP_INTERCALL_MS);
                    int32_t definitionOfPath = (0 * 1 + 1 * 2 + 0 * 4 + 0 * 8) + (0 * 16 + 0 * 32 + 0 * 64 + 0 * 128);
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, 6, 02, definitionOfPath, true, 4, App.DebugModbusParam) < 0) {
                        retVal = -1; // return -1;
                    }
                    usleep(20 * 1000);
                    // P6 - 03 PDAT1 Data of Path 1
                    int32_t dataOfPath = targetTurns * pActuator->pulsesPerTurn + targetPulses;
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, 6, 03, dataOfPath, true, 4, App.DebugModbusParam) < 0) {
                        retVal = -1; // return -1;
                    }
                    usleep(20 * 1000);
                    // Target speed: P5.60 ~ P5.75 (Moving Speed Setting of Position 0 ~ 15), total 16 groups
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, 5, 60, speed_rpm, true, 2, App.DebugModbusParam) < 0) {
                        retVal = -1; // return -1;
                    }
                    usleep(20 * 1000);
                    int32_t acc_ms = (int32_t) ((3000.0f - 0.0f) / 9.55f / acc_rpms2_1 * 1000.0f);
                    // Accel / Decel time: P5.20 ~ P5.35 (Accel / Decel Time 0 ~ 15), total 16 parameters
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, 5, 20, acc_ms, true, 2, App.DebugModbusParam) < 0) {
                        retVal = -1; // return -1;
                    }
                    usleep(20 * 1000);
                    // Delay time: P5.40 ~ P5.55 (Delay Time 0 ~ 15), total 16 groups
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, 5, 40, pause_sec, true, 2, App.DebugModbusParam) < 0) {
                        retVal = -1; // return -1;
                    }



                    // Posizione ritorno
                    // P6 - 04 PDEF2 Definition of Path 2
                    // P6 - 05 PDAT2 Data of Path 2
                    int32_t definitionOfPath2 = (0 * 1 + 1 * 2 + 0 * 4 + 0 * 8) + (0 * 16 + 0 * 32 + 0 * 64 + 0 * 128);
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, 6, 04, definitionOfPath2, true, 4, App.DebugModbusParam) < 0) {
                        retVal = -1; // return -1;
                    }
                    usleep(20 * 1000);
                    // P6 - 03 PDAT1 Data of Path 1
                    int32_t dataOfPath2 = targetTurns2 * pActuator->pulsesPerTurn + targetPulses2;
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, 6, 05, dataOfPath2, true, 4, App.DebugModbusParam) < 0) {
                        retVal = -1; // return -1;
                    }
                    usleep(20 * 1000);
                    // Target speed: P5.60 ~ P5.75 (Moving Speed Setting of Position 0 ~ 15), total 16 groups
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, 5, 61, speed_rpm2, true, 2, App.DebugModbusParam) < 0) {
                        retVal = -1; // return -1;
                    }

                    usleep(20 * 1000);
                    int32_t acc_ms2 = (int32_t) ((3000.0f - 0.0f) / 9.55f / acc_rpms2_2 * 1000.0f);
                    // Accel / Decel time: P5.20 ~ P5.35 (Accel / Decel Time 0 ~ 15), total 16 parameters
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, 5, 21, acc_ms2, true, 2, App.DebugModbusParam) < 0) {
                        retVal = -1; // return -1;
                    }
                    usleep(20 * 1000);
                    // Delay time: P5.40 ~ P5.55 (Delay Time 0 ~ 15), total 16 groups
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, 5, 41, pause_sec, true, 2, App.DebugModbusParam) < 0) {
                        retVal = -1; // return -1;
                    }

                }


            } else {
                return -1;
            }


        } else if (pActuator->protocol == MODBUS_AC_SERVO_DELTA) {


        } else if (pActuator->protocol == CANOPEN_AC_SERVO_DELTA) {

            if (pActuator->pCANSlot) {
                pCANSlot = (CANSlot*) pActuator->pCANSlot;

                if (pActuator->boardId != pCANSlot->boardId || (pActuator->stationId != pCANSlot->stationId && pCANSlot->stationId != -1)) {
                    snprintf(App.Msg, App.MsgSize, "[%s wrong canbus board link %d<>%d  %s]\n", (char*) ANSI_COLOR_RED, pActuator->boardId, pCANSlot->boardId, (char*) ANSI_COLOR_RESET);
                    vDisplayMessage(App.Msg);
                    return -1;
                }

                snprintf(App.Msg, App.MsgSize, "[%s Setting CANBUS AC Servo #%d parameters {pos:%d - speed:%d - acc:%d}-{pos:%d - speed:%d - acc:%d} %s]\n"
                        , (char*) ANSI_COLOR_YELLOW
                        , pCANSlot->boardId
                        , targetTurns, speed_rpm, acc_rpms2_1
                        , targetTurns2, speed_rpm2, acc_rpms2_2
                        , (char*) ANSI_COLOR_RESET
                        );
                vDisplayMessage(App.Msg);


                pCANSlot->controlMode1B = 1 + 1;


                ///////////////////////////////////////////////////
                // Scrittura : Mode of operations:6060h : INTEGER8
                // 0:Reserved      1:Profile position mode     3:Profile velocity mode     4:Profile torque mode       6:Homing mode       7:Interpolation position mode                
                //
                int8_t controlMode = 1;
                if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x6060, (int16_t) 0, (int8_t *) & controlMode, (int8_t) 1, true) < 0) {
                    /////////////////////////
                    // Generazione Allarme
                    //
                    if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                    }
                }


                /*
                 * ****************************************
                 * 
                 * Competenza del comando non del setup
                 * 
                 * ****************************************
                 * 
                 * 
                 */



            } else {
                return -1;
            }

        } else {
            return -1;
        }
    }

    return retVal;
}




int handle_actuator_speed_mode(void *pvActuator, int speed) {
    int retVal = 0;


    if (pvActuator) {
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pvActuator;
        SerialSlot *pSerialSlot = (SerialSlot *) NULL;
        CANSlot *pCANSlot = (CANSlot *) NULL;

        if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN) {

            if (pActuator->pSerialSlot) {
                pSerialSlot = (SerialSlot*) pActuator->pSerialSlot;

                if (pActuator->boardId != pSerialSlot->boardId || pActuator->stationId != pSerialSlot->stationId) {
                    snprintf(App.Msg, App.MsgSize, "[%s wrong board link %d<>%d  %s]\n", (char*) ANSI_COLOR_RED, pActuator->boardId, pSerialSlot->boardId, (char*) ANSI_COLOR_RESET);
                    vDisplayMessage(App.Msg);
                    return -1;
                }

                snprintf(App.Msg, App.MsgSize, "[%s Setting MODBUS AC Servo #%d SPEED parameters %s]\n", (char*) ANSI_COLOR_YELLOW, pSerialSlot->boardId, (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);


                pSerialSlot->controlMode1B = 0 + 1;

                ///////////////////////////////////////////////
                // Scrittura : Pr1.00
                //


                if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 1), 0, 0, false, 2, App.DebugModbusParam) < 0) {
                    // return -1;
                }


                ///////////////////////////////////////////////
                // Scrittura : Pr1.28
                //
                if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 1), 28, 4, false, 2, App.DebugModbusParam) < 0) {
                    // return -1;
                }




                ///////////////////////////////////////////////
                // Scrittura : Pr1.29
                //
                if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 1), 29, 2, false, 2, App.DebugModbusParam) < 0) {
                    // return -1;
                }

                ///////////////////////////////////////////////
                // Scrittura : Pr3.00
                //
                if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 3), 0, speed, false, 2, App.DebugModbusParam) < 0) {
                    // return -1;
                }


            } else {
                return -1;
            }

        } else if (pActuator->protocol == MODBUS_AC_SERVO_DELTA) {

            return -1;



        } else if (pActuator->protocol == CANOPEN_AC_SERVO_DELTA) {

            if (pActuator->pCANSlot) {
                pCANSlot = (CANSlot*) pActuator->pCANSlot;

                if (pActuator->boardId != pCANSlot->boardId || (pActuator->stationId != pCANSlot->stationId && pCANSlot->stationId != -1)) {
                    snprintf(App.Msg, App.MsgSize, "[%s wrong board link %d<>%d  %s]\n", (char*) ANSI_COLOR_RED, pActuator->boardId, pCANSlot->boardId, (char*) ANSI_COLOR_RESET);
                    vDisplayMessage(App.Msg);
                    return -1;
                }

                // snprintf(App.Msg, App.MsgSize, "[%s Setting CANBUS AC Servo SEED #%d parameters %s]\n", (char*) ANSI_COLOR_YELLOW, pCANSlot->boardId, (char*) ANSI_COLOR_RESET);
                // vDisplayMessage(App.Msg);


           
                if (speed != 0) {
            
                    if (pCANSlot->controlMode1B != 1 + 3 ) {
                        
                        pCANSlot->controlMode1B = 1 + 3;
                        
                        /////////////////////////
                        // Avvio motore
                        //
                        if (handle_actuator_servo_on((void *) pActuator) < 0) {
                            // return -1;
                            if (generate_alarm((char*) "CANBUS: handle_actuator_start() Error", 6101, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                            }
                            return -1;
                        }


                        if (handle_actuator_prepare_for_run((void*) pActuator, 999) < 0) {
                            // Generazione Allarme
                            if (generate_alarm((char*) "CAN Start AC Servo Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                            }
                            return -1;
                        }
                        
                        usleep(2000);

                        ///////////////////////////////////////////////////
                        // Scrittura : Mode of operations:6060h : INTEGER8
                        // 0:Reserved      1:Profile position mode     3:Profile velocity mode     4:Profile torque mode       6:Homing mode       7:Interpolation position mode                
                        //
                        int8_t controlMode = 3;
                        if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x6060, (int16_t) 0, (int8_t *) & controlMode, (int8_t) 1, true) < 0) {
                            // Generazione Allarme
                            if (generate_alarm((char*) "CAN Unable to set 0x6060", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                            }
                            return -1;
                        }            
                        
                        
                        usleep(2000);

                        uint32_t rated_acc_ms = (uint32_t) (200);
                        if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x6083, (int16_t) 0, (int8_t *) & rated_acc_ms, (int8_t)sizeof (rated_acc_ms), true) < 0) {
                            // Generazione Allarme
                            if (generate_alarm((char*) "CAN Unable to set 0x6083", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                            }
                            return -1;
                        }            

                        usleep(2000);
                        
                        if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x6084, (int16_t) 0, (int8_t *) & rated_acc_ms, (int8_t)sizeof (rated_acc_ms), true) < 0) {
                            // Generazione Allarme
                            if (generate_alarm((char*) "CAN Unable to set 0x6084", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                            }
                            return -1;
                        }            

                        usleep(2000);

                        // 0x607F Max profile velocity Unit:0.1rpm
                        int32_t cspeed = (uint32_t) ((float) speed * 10.0f );
                        if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x60FF, (int16_t) 0, (int8_t *) & cspeed, (int8_t) sizeof (cspeed), true) < 0) {
                            // Generazione Allarme
                            if (generate_alarm((char*) "CAN Unable to set 0x60FF", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                            }
                        }
                        
                        usleep(2000);
                        
                        int16_t controlWord = 1*1 + 1*2 + 1*4 + 1*8;
                        if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x6040, (int16_t) 0, (int8_t *) & controlWord, (int8_t)sizeof (controlWord), true) < 0) {
                            // Generazione Allarme
                            if (generate_alarm((char*) "CAN Unable to set 0x6040", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                            }
                            return -1;
                        }


                        pActuator->target_rpos = (int32_t)cspeed > 0 ? pActuator->end_rpos : pActuator->start_rpos;
                        pActuator->step = STEP_MOVING;
                    }
                    
                } else {
                    
                    // Quick stop
                    int16_t controlWord = 1*1 + 1*2 + 0*4 + 1*8;
                    if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x6040, (int16_t) 0, (int8_t *) & controlWord, (int8_t) sizeof (controlWord), true) < 0) {
                        // Generazione Allarme
                        if (generate_alarm((char*) "CAN Unable to set 0x607F", 0x6040, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                        }
                    }
                    
                    ///////////////////////////////////////////////////
                    // Scrittura : Mode of operations:6060h : INTEGER8
                    // 0:Reserved      1:Profile position mode     3:Profile velocity mode     4:Profile torque mode       6:Homing mode       7:Interpolation position mode                
                    //
                    int8_t controlMode = 1;
                    if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x6060, (int16_t) 0, (int8_t *) & controlMode, (int8_t) 1, true) < 0) {
                        // Generazione Allarme
                        if (generate_alarm((char*) "CAN Unable to set 0x607F", 0x6040, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                        }
                    }
                    
                    if (pActuator->step == STEP_MOVING) {
                        pActuator->step = STEP_READY;
                    }
                    
                    pCANSlot->controlMode1B = 0 + 1;
                }
            }
            
        } else {
            return -1;
        }
    }

    return retVal;
}

int handle_actuator_servo_on(void *pvActuator) {
    return handle_actuator_sevo_on_off(pvActuator, 1);
}

int handle_actuator_servo_off(void *pvActuator) {
    return handle_actuator_sevo_on_off(pvActuator, 0);
}

int handle_actuator_servo_stop(void *pvActuator) {
    return handle_actuator_sevo_on_off(pvActuator, -1);
}



//////////////////////////////////////////
// Avvia o arresta il motore
//

int handle_actuator_sevo_on_off(void *pvActuator, int mode) {
    int retVal = 0;


    if (pvActuator) {
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pvActuator;
        SerialSlot *pSerialSlot = (SerialSlot *) NULL;
        CANSlot *pCANSlot = (CANSlot *) NULL;
    int16_t controlWord = 0;

        if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN || pActuator->protocol == MODBUS_AC_SERVO_DELTA) {

            if (pActuator->pSerialSlot) {
                pSerialSlot = (SerialSlot*) pActuator->pSerialSlot;


                if (pActuator->boardId != pSerialSlot->boardId || pActuator->stationId != pSerialSlot->stationId) {
                    snprintf(App.Msg, App.MsgSize, "[%s wrong board link %d<>%d  %s]\n", (char*) ANSI_COLOR_RED, pActuator->boardId, pSerialSlot->boardId, (char*) ANSI_COLOR_RESET);
                    vDisplayMessage(App.Msg);
                    return -1;
                }

                if (mode) {
                    snprintf(App.Msg, App.MsgSize, "[%s Starting AC Servo SER#%d %s", (char*) ANSI_COLOR_YELLOW, pSerialSlot->boardId, (char*) ANSI_COLOR_RESET);
                    vDisplayMessage(App.Msg);
                } else {
                    snprintf(App.Msg, App.MsgSize, "[%s Stopping AC Servo SER#%d %s", (char*) ANSI_COLOR_YELLOW, pSerialSlot->boardId, (char*) ANSI_COLOR_RESET);
                    vDisplayMessage(App.Msg);
                }


                if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN) {

                    ///////////////////////////////////////////////
                    // Scrittura : Pr6.01 -> 26int = 38hex
                    //
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 6), 01, mode, true, 2, App.DebugModbusParam) < 0) {
                    } else {
                        // Perdita della continuita' con l'encoder
                        if (mode == 0) {
                            snprintf(App.Msg, App.MsgSize, "[%s Servo OFF : Lost encoder reference%s]\n", (char*) ANSI_COLOR_YELLOW, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);
                            pActuator->homingDone = 0;
                            pActuator->homingTurnsPPT = 0;
                            pActuator->homingPulsesPPT = 0;
                        }
                        retVal = 1;
                    }
                    
                    

                } else if (pActuator->protocol == MODBUS_AC_SERVO_DELTA) {

                    if (mode) {
                        if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, 2, 30, 1, true, 2, App.DebugModbusParam) < 0) {
                            retVal = -1;
                        } else {
                            retVal = 1;
                        }
                        
                    } else {
                        if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, 2, 30, /*(1*1 + 2*0 + 4*1)*/0, true, 2, App.DebugModbusParam) < 0) {
                            retVal = -1;
                        } else {
                            retVal = 1;
                        }
                    }
                }

                snprintf(App.Msg, App.MsgSize, "%s Done %s]\n", (char*) ANSI_COLOR_YELLOW, (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);


            } else {
                return -1;
            }



        } else if (pActuator->protocol == CANOPEN_AC_SERVO_DELTA) {
            int16_t controlWord = 0;

            if (pActuator->pCANSlot) {
                pCANSlot = (CANSlot*) pActuator->pCANSlot;

                if (pActuator->boardId != pCANSlot->boardId || (pActuator->stationId != pCANSlot->stationId && pCANSlot->stationId != -1)) {
                    snprintf(App.Msg, App.MsgSize, "[%s wrong board link %d<>%d  %s]\n", (char*) ANSI_COLOR_RED, pActuator->boardId, pCANSlot->boardId, (char*) ANSI_COLOR_RESET);
                    vDisplayMessage(App.Msg);
                    return -1;
                }

                if (mode) {
                    snprintf(App.Msg, App.MsgSize, "[%s Starting AC Servo CAN#%d %s", (char*) ANSI_COLOR_YELLOW, pCANSlot->boardId, (char*) ANSI_COLOR_RESET);
                    vDisplayMessage(App.Msg);
                } else {
                    snprintf(App.Msg, App.MsgSize, "[%s Stopping AC Servo CAN#%d %s", (char*) ANSI_COLOR_YELLOW, pCANSlot->boardId, (char*) ANSI_COLOR_RESET);
                    vDisplayMessage(App.Msg);
                }



                ///////////////////////////////////////////////
                // Scrittura : 6040h Control Word
                //
                //  Bit Definition
                //  ... (bit15~9)       N/A
                //  256(bit8)           Halt
                //  128(bit7)           Fault reset 
                //
                //  Operation mode specific 
                //  64(bit6)    abs(0)/rel(1) N/A N/A N/A N/A
                //  32(bit5)    Change set immediately
                //  16(bit4)    New set-point (positive trigger) 
                //
                //  Bit Definition
                //  15~9    N/A
                //  8       Halt
                //  7       Fault reset 
                //  6~4     Operation mode specific 
                //  3       Enable operation
                //  2       Quick stop
                //  1       Enable voltage    
                //  0       Switch on
                //  
                    

                
                if (App.CanOpenVersion == 0x0C) {
                    // If P1-01 = 0x0C, user need to set 6040h to 0x0006->0x0007->0x000F for Servo On step by step.                    
                    if (mode) {
                        controlWord = 0x0006;
                        if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x6040, (int16_t) 0, (int8_t *) & controlWord, (int8_t) sizeof (controlWord), true) < 0) {
                            /////////////////////////
                            // Generazione Allarme
                            //
                            if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                            }
                        }
                        
                        usleep(10*1000);

                        controlWord = 0x0007;
                        if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x6040, (int16_t) 0, (int8_t *) & controlWord, (int8_t) sizeof (controlWord), true) < 0) {
                            /////////////////////////
                            // Generazione Allarme
                            //
                            if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                            }
                        }

                        usleep(10*1000);

                        controlWord = 0x000F;
                        if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x6040, (int16_t) 0, (int8_t *) & controlWord, (int8_t) sizeof (controlWord), true) < 0) {
                            /////////////////////////
                            // Generazione Allarme
                            //
                            if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                            }
                        }

                        snprintf(App.Msg, App.MsgSize, "%s Done %s]\n", (char*) ANSI_COLOR_YELLOW, (char*) ANSI_COLOR_RESET);
                        vDisplayMessage(App.Msg);
                        
                    } else {
                        
                        controlWord = 0*1 + 0*2 + 1*4 + 0*8 + (0*16 + 0*32 + 1*64) + 1*128 + 1*256; // OK 
                        if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x6040, (int16_t) 0, (int8_t *) & controlWord, (int8_t) sizeof (controlWord), true) < 0) {
                            /////////////////////////
                            // Generazione Allarme
                            //
                            if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                            }
                        }
                    }
                    
                } else {
                    //////////////////////////////////////////////////////////////////////////////////////
                    // If P1-01 = 0x0B, user could set 6040h to 0x000F for ServoOn immediately.
                    //
                    
                    if (mode > 0) {
                        controlWord = 1*1 + 1*2 + 1*4 + 1*8 + (0*16 + 0*32 + 0*64) + 0*128 + 0*256; // Servo On
                    } else if (mode == 0) {
                        controlWord = 0*1 + 0*2 + 0*4 + 0*8 + (0*16 + 0*32 + 0*64) + 0*128 + 0*256; // Servo Off e Quick Stop
                    } else if (mode < 0) {
                        controlWord = 1*1 + 1*2 + 0*4 + 1*8 + (0*16 + 0*32 + 0*64) + 0*128 + 0*256; // Servo On e Quick Stop
                    }

                    if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x6040, (int16_t) 0, (int8_t *) & controlWord, (int8_t) sizeof (controlWord), true) < 0) {
                        /////////////////////////
                        // Generazione Allarme
                        //
                        if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                        }
                    }

                    snprintf(App.Msg, App.MsgSize, "%s Done %s]\n", (char*) ANSI_COLOR_YELLOW, (char*) ANSI_COLOR_RESET);
                    vDisplayMessage(App.Msg);
                }

            }

        } else {
            return -1;
        }
    }

    return retVal;
}




///////////////////////////////////////////////////////////////////
//
//  N.B.: OBSOLETA : utilizzo della modalità asincrona
//
//  Avvia il motore usando la tabella dei posizionamenti :
//      Imposta la posizione e in tabella attiva il servo motore
//      position_in_table = 1..16
//

int handle_actuator_prepare_for_run(void *pvActuator, int position_in_table) {
    int retVal = 0;


    if (pvActuator) {
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pvActuator;
        SerialSlot *pSerialSlot = (SerialSlot *) NULL;
        CANSlot *pCANSlot = (CANSlot *) NULL;
        uint16_t bit_pos = 0;

        if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN) {

            if (pActuator->pSerialSlot) {
                pSerialSlot = (SerialSlot*) pActuator->pSerialSlot;


                if (pActuator->boardId != pSerialSlot->boardId || pActuator->stationId != pSerialSlot->stationId) {
                    snprintf(App.Msg, App.MsgSize, "[%s wrong board link %d<>%d  %s]\n", (char*) ANSI_COLOR_RED, pActuator->boardId, pSerialSlot->boardId, (char*) ANSI_COLOR_RESET);
                    vDisplayMessage(App.Msg);
                    return -1;
                }


                bit_pos = modbus_get_bits_by_pable_pos( (void *)pvActuator, position_in_table);

                // snprintf(App.Msg, App.MsgSize, "[%s Running AC Servo #%d %s]\n", (char*) ANSI_COLOR_YELLOW, pSerialSlot->boardId, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);


                ///////////////////////////////////////////////
                // Scrittura : Pr6.01 -> 26int = 38hex
                //
                if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 6), 01, bit_pos, false, 2, App.DebugModbusParam) < 0) {
                    return -1;
                }
                
                if (!(bit_pos & 1)) {
                    // Perdita della continuita' con l'encoder
                    pActuator->homingDone = 0;
                    pActuator->homingTurnsPPT = 0;
                    pActuator->homingPulsesPPT = 0;
                }

            } else {
                return -1;
            }



        } else if (pActuator->protocol == MODBUS_AC_SERVO_DELTA) {

            ////////////////////////////////////////////////////////////////////////////////////
            // Scrittura : Pr5.07 -> 1..63  (PRCM Trigger Position Command (PR mode only)
            //
            if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, 5, 07, position_in_table, true, 2, App.DebugModbusParam) < 0) {
                return -1;
            }



        } else if (pActuator->protocol == CANOPEN_AC_SERVO_DELTA) {

            if (pActuator->pCANSlot) {
                pCANSlot = (CANSlot*) pActuator->pCANSlot;

                if (pActuator->boardId != pCANSlot->boardId || (pActuator->stationId != pCANSlot->stationId && pCANSlot->stationId != -1)) {
                    snprintf(App.Msg, App.MsgSize, "[%s wrong board link %d<>%d  %s]\n", (char*) ANSI_COLOR_RED, pActuator->boardId, pCANSlot->boardId, (char*) ANSI_COLOR_RESET);
                    vDisplayMessage(App.Msg);
                    return -1;
                }



                // snprintf(App.Msg, App.MsgSize, "[%s Running AC Servo #%d %s]\n", (char*) ANSI_COLOR_YELLOW, pCANSlot->boardId, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);


                //////////////////////////////////////////////////
                // Scrittura : 6040h Control Word : UNSIGNED16
                //
                //  Bit Definition
                //  ... (bit15~9)       N/A
                //  256(bit8)           Halt
                //  128(bit7)           Fault reset 
                //
                //  Operation mode specific 
                //  64(bit6)    abs(0)/rel(1) N/A N/A N/A N/A
                //  32(bit5)    Change set immediately
                //  16(bit4)    New set-point (positive trigger) 
                //
                //  8(bit3)             Enable operation
                //  4(bit2)             Quick stop
                //  2(bit1)             Enable voltage    
                //  1(bit0)             Switch on
                //                  
                int16_t controlWord = 0 + 0;

                // int16_t controlWord = position_in_table>=0 ? (1*1 + 1*2 + 1*4 + 1*8 + (1*16 + 1*32 + 0*64) + 0*128 + 0*256) : (0); // OK
                // int16_t controlWord = position_in_table>=0 ? (1*1 + 1*2 + 1*4 + 1*8 + (1*16 + 0*32 + 0*64) + 0*128 + 0*256) : (0); // OK
                // int16_t controlWord = position_in_table>=0 ? (1*1 + 1*2 + 1*4 + 1*8 + (1*16 + 1*32 + 0*64) + 1*128 + 0*256) : (0); // OK
                // int16_t controlWord = position_in_table>=0 ? (1*1 + 1*2 + 0*4 + 1*8 + (1*16 + 1*32 + 0*64) + 1*128 + 0*256) : (0); // OK
                // int16_t controlWord = position_in_table>=0 ? (1*1 + 1*2 + 1*4 + 1*8 + (1*16 + 1*32 + 1*64) + 0*128 + 0*256) : (0); // NO
                // int16_t controlWord = position_in_table>=0 ? (1*1 + 1*2 + 1*4 + 1*8 + (0*16 + 0*32 + 0*64) + 0*128 + 0*256) : (0); // NO
                // int16_t controlWord = position_in_table>=0 ? (1*1 + 1*2 + 0*4 + 1*8 + (1*16 + 1*32 + 0*64) + 1*128 + 1*256) : (0); // NO
                // int16_t controlWord = position_in_table>=0 ? (1*1 + 1*2 + 1*4 + 1*8 + (1*16 + 1*32 + 0*64) + 1*128 + 1*256) : (0); // NO
                if (position_in_table >= 0) {
                    controlWord = (1*1 + 1*2 + 1*4 + 1*8 + (1*16 + 1*32 + 1*64) + 0*128 + 0*256); // OK
                    
                } else if (position_in_table == CONTROL_WORD_RESET) {
                    // N.B.: Il quick stop con l'anticipo arresta il motore prima di raggiungere la pos. reale
                    controlWord = (1*1 + 1*2 + 1*4 + 1*8 + (0*16 + 0*32 + 0*64) + 1*128 + 1*256); // OK
                
                } else if (position_in_table == CONTROL_WORD_QUICK_STOP) {
                    // N.B.: Il quick stop con l'anticipo arresta il motore prima di raggiungere la pos. reale                    
                    controlWord = (1*1 + 1*2 + 0*4 + 1*8 + (0*16 + 0*32 + 0*64) + 1*128 + 1*256); // OK
                
                } else {
                    return -1;
                }


                if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x6040, (int16_t) 0, (int8_t *) & controlWord, (int8_t) sizeof (controlWord), true) < 0) {
                    /////////////////////////
                    // Generazione Allarme
                    //
                    if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                    }
                }
            }


        } else {
            return -1;
        }
    }

    return retVal;
}




int handle_actuator_reset_pos_table(void *pvActuator, int mode) {

    if (pvActuator) {
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pvActuator;
        SerialSlot *pSerialSlot = (SerialSlot *) NULL;
        CANSlot *pCANSlot = (CANSlot *) NULL;

        if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN) {

            if (pActuator->pSerialSlot) {
                pSerialSlot = (SerialSlot*) pActuator->pSerialSlot;


                if (pActuator->boardId != pSerialSlot->boardId || pActuator->stationId != pSerialSlot->stationId) {
                    snprintf(App.Msg, App.MsgSize, "[%s wrong board link %d<>%d  %s]\n", (char*) ANSI_COLOR_RED, pActuator->boardId, pSerialSlot->boardId, (char*) ANSI_COLOR_RESET);
                    vDisplayMessage(App.Msg);
                    return -1;
                }


                snprintf(App.Msg, App.MsgSize, "[%s Resetting Position Table on AC Servo #%d %s]\n", (char*) ANSI_COLOR_YELLOW, pSerialSlot->boardId, (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);


                for (int j = 0; j < 16; j++) {

                    ///////////////////////////////////////////////
                    // Scrittura : Pr2.?0 - Posizione (turns)
                    //
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 2), 00 + j * 4, 0, false, 2, App.DebugModbusParam) < 0) {
                        // return -1;
                    }


                    ///////////////////////////////////////////////
                    // Scrittura : Pr2.?1 - Posizione (pulses)
                    //
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 2), 01 + j * 4, 0, false, 2, App.DebugModbusParam) < 0) {
                        // return -1;
                    }

                    ///////////////////////////////////////////////
                    // Scrittura : Pr2.?2 - Speed
                    //
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 2), 03 + j * 4, 0, false, 2, App.DebugModbusParam) < 0) {
                        // return -1;
                    }

                    ///////////////////////////////////////////////
                    // Scrittura : Pr2.?2 - Pause
                    //
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 2), 03 + j * 4, 0, false, 2, App.DebugModbusParam) < 0) {
                        // return -1;
                    }
                }

            } else {
                return -1;
            }


        } else if (pActuator->protocol == MODBUS_AC_SERVO_DELTA) {


        } else if (pActuator->protocol == CANOPEN_AC_SERVO_DELTA) {

            if (pActuator->pCANSlot) {
                pCANSlot = (CANSlot*) pActuator->pCANSlot;

                if (pActuator->boardId != pCANSlot->boardId || (pActuator->stationId != pCANSlot->stationId && pCANSlot->stationId != -1)) {
                    snprintf(App.Msg, App.MsgSize, "[%s wrong board link %d<>%d  %s]\n", (char*) ANSI_COLOR_RED, pActuator->boardId, pCANSlot->boardId, (char*) ANSI_COLOR_RESET);
                    vDisplayMessage(App.Msg);
                    return -1;
                }

                snprintf(App.Msg, App.MsgSize, "[%s Resetting Position Table on AC Servo #%d %s]\n", (char*) ANSI_COLOR_YELLOW, pCANSlot->boardId, (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);


                for (int j = 0; j < 16; j++) {

                    ///////////////////////////////////////////////
                    // Scrittura : ...
                    //

                    // ...
                }
            }

        } else {
            return -1;
        }
    }

    return 1;
}
