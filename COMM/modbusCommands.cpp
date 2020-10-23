//============================================================================
// Name        : modbusCommands.cpp
// Author      : Cristian Andreon
// Version     :
// Copyright   : Your copyright notice
// Description : Entry point, C++, Ansi-style
//============================================================================

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */


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

#define MAX_MODVAR_PULSES_VAR     20000

#define MAX_HOMING_POSITION_ERROR   0.05f



/*
#undef MODBUS_STREAM_TIMEOUT_MS
#define MODBUS_STREAM_TIMEOUT_MS   3500
*/

#define MAX_MODBUS_STREAM_ERROR 10




int32_t modbus_get_bits_by_pable_pos( void *pvActuator, int32_t position_in_table) {

    int32_t bit_pos = 0;

    switch (position_in_table) {
        case 1:
            bit_pos = 1 + (0 * 16 + 0 * 32 + 0 * 64 + 0 * 128) + 256;
            break;
        case 2:
            bit_pos = 1 + (1 * 16 + 0 * 32 + 0 * 64 + 0 * 128) + 256;
            break;
        case 3:
            bit_pos = 1 + (0 * 16 + 1 * 32 + 0 * 64 + 0 * 128) + 256;
            break;
        case 4:
            bit_pos = 1 + (1 * 16 + 1 * 32 + 0 * 64 + 0 * 128) + 256;
            break;
        case 5:
            bit_pos = 1 + (0 * 16 + 0 * 32 + 1 * 64 + 0 * 128) + 256;
            break;
        case 6:
            bit_pos = 1 + (1 * 16 + 0 * 32 + 1 * 64 + 0 * 128) + 256;
            break;
        case 7:
            bit_pos = 1 + (0 * 16 + 1 * 32 + 1 * 64 + 0 * 128) + 256;
            break;
        case 8:
            bit_pos = 1 + (1 * 16 + 1 * 32 + 1 * 64 + 0 * 128) + 256;
            break;
        default:
            bit_pos = 1 + (0 * 16 + 0 * 32 + 0 * 64 + 0 * 128) + 0;
            break;
    }

    return bit_pos;
}



int32_t modbus_param_update(modbus_t *ctx, int32_t hParam, int32_t lParam, int32_t newValue, bool check_mode, int32_t sizeOf, bool printParam) {
    int32_t retVal = 1;
    char hHigh = 0, bLow = 0;
    int32_t rc;
    int32_t addr = 0;
    int32_t purgeBytes = 0;
    int32_t iTry = 0, nTry = 5, iChk = 0, nChk = 5;
    uint16_t setValue = 0;


    ///////////////////////////////////////////////
    // Scrittura : Pr hParam.lParam
    //

    restart_func:
    
    
    // hHigh = HEX_TO_INT(2, hParam);
    hHigh = hParam;

    // bLow = HEX_TO_INT(lParam/10, lParam%10);
    addr = MAKEWORD(lParam, hHigh);

    
    if (sizeOf == 4) {
        uint16_t newusValue[2];
        memcpy(&newusValue, &newValue, 4);
        
        for (iTry=0; iTry<nTry; iTry++) {
            if ((rc = modbus_write_registers((modbus_t*) ctx, addr, 2, (uint16_t*) newusValue)) < 0) {
                if (check_mode) {
                    vDisplayMessage(".");
                    usleep(11 * 1000);
                    if ((purgeBytes = modbus_purge_comm((modbus_t*) ctx)) > 0) {
                        snprintf(App.Msg, App.MsgSize, "[%s%dbytes purged%s]\n", (char*) ANSI_COLOR_GREEN, purgeBytes, (char*) ANSI_COLOR_RESET);
                        vDisplayMessage(App.Msg);
                    }
                } else {
                    snprintf(App.Msg, App.MsgSize, "[%sPr%d.%d=%d:%s%s]", (char*) ANSI_COLOR_RED, (hParam > 32 ? hParam - 32 : hParam), lParam, newValue, (char*) modbus_strerror(errno), (char*) ANSI_COLOR_RESET);
                    vDisplayMessage(App.Msg);
                    retVal = -1;
                    goto end_func; // return -1;
                }
            } else {
                break;
            }
        }
        
    } else if (sizeOf == 2) {
        for (iTry=0; iTry<nTry; iTry++) {
            if ((rc = modbus_write_register((modbus_t*) ctx, addr, (int32_t) newValue)) < 0) {
                if (check_mode) {
                    vDisplayMessage(".");
                    usleep(11 * 1000);
                    if ((purgeBytes = modbus_purge_comm((modbus_t*) ctx)) > 0) {
                        snprintf(App.Msg, App.MsgSize, "[%s%dbytes purged%s]\n", (char*) ANSI_COLOR_GREEN, purgeBytes, (char*) ANSI_COLOR_RESET);
                        vDisplayMessage(App.Msg);
                    }
                } else {
                    snprintf(App.Msg, App.MsgSize, "[%sPr%d.%d=%d>%s%s]", (char*) ANSI_COLOR_RED, (hParam > 32 ? hParam - 32 : hParam), lParam, newValue, (char*) modbus_strerror(errno), (char*) ANSI_COLOR_RESET);
                    vDisplayMessage(App.Msg);
                    retVal = -1;
                    goto end_func; // return -1;
                }
            } else {
                break;
            }
        }
    } else {
        snprintf(App.Msg, App.MsgSize, "[%s modbus Pr%d.%d=%d write error : Invalid sizeOf %s]\n", (char*) ANSI_COLOR_RED, (hParam > 32 ? hParam - 32 : hParam), lParam, newValue, (char*) ANSI_COLOR_RESET);
        vDisplayMessage(App.Msg);
        return -1;
    }



    if (check_mode) {

        usleep(11 * 1000);

        // snprintf(App.Msg, App.MsgSize, "{1}"); vDisplayMessage(App.Msg);

        hHigh = hParam > 32 ? hParam - 32 : hParam;
        addr = MAKEWORD(lParam, hHigh);

        // snprintf(App.Msg, App.MsgSize, "{2}"); vDisplayMessage(App.Msg);

        if ((rc = modbus_read_registers((modbus_t*) ctx, addr, 1, (uint16_t *) & setValue)) < 0) {
            if (iChk < nChk) {
                iChk++;
                vDisplayMessage("<");
                if ((purgeBytes = modbus_purge_comm((modbus_t*) ctx)) > 0) {
                    snprintf(App.Msg, App.MsgSize, "[%s%dbytes purged%s]\n", (char*) ANSI_COLOR_YELLOW, purgeBytes, (char*) ANSI_COLOR_RESET);
                    vDisplayMessage(App.Msg);
                }
                goto restart_func;
            } else {
                snprintf(App.Msg, App.MsgSize, "[%s modbus Pr%d.%d read error : %s %s]\n", (char*) ANSI_COLOR_RED, (hParam > 32 ? hParam - 32 : hParam), lParam, (char*) modbus_strerror(errno), (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);
                return -1;
            }
        }

        // snprintf(App.Msg, App.MsgSize, "{3}"); vDisplayMessage(App.Msg);
        if (sizeOf == 4) {
            if ((uint32_t) setValue != (uint32_t) newValue) {
                if (iChk < nChk) {
                    iChk++;
                    vDisplayMessage("<");
                    goto restart_func;
                } else {
                    snprintf(App.Msg, App.MsgSize, "[%s modbus Pr%d.%d 32bit check error (%d/%d) %s]\n", (char*) ANSI_COLOR_RED, (hParam > 32 ? hParam - 32 : hParam), lParam, (uint16_t) newValue, (uint16_t) setValue, (char*) ANSI_COLOR_RESET);
                    vDisplayMessage(App.Msg);
                    return -1;
                }
            } else {
                if (printParam) {
                    snprintf(App.Msg, App.MsgSize, "[%sPr%d.%d=%d%s]", (char*) ANSI_COLOR_GREEN, (hParam > 32 ? hParam - 32 : hParam), lParam, newValue, (char*) ANSI_COLOR_RESET);
                    vDisplayMessage(App.Msg);
                }
            }
        } else if (sizeOf == 2) {
            if ((uint16_t) setValue != (uint16_t) newValue) {
                if (iChk < nChk) {
                    iChk++;
                    vDisplayMessage("<");
                    goto restart_func;
                } else {
                    snprintf(App.Msg, App.MsgSize, "[%s modbus Pr%d.%d 16bit check error (%d/%d) %s]\n", (char*) ANSI_COLOR_RED, (hParam > 32 ? hParam - 32 : hParam), lParam, (uint16_t) newValue, (uint16_t) setValue, (char*) ANSI_COLOR_RESET);
                    vDisplayMessage(App.Msg);
                    return -1;
                }
            } else {
                if (printParam) {
                    snprintf(App.Msg, App.MsgSize, "[%sPr%d.%d=%d%s]", (char*) ANSI_COLOR_GREEN, (hParam > 32 ? hParam - 32 : hParam), lParam, newValue, (char*) ANSI_COLOR_RESET);
                    vDisplayMessage(App.Msg);
                }
            }
        }
    } else {
        if (printParam) {
            snprintf(App.Msg, App.MsgSize, "[%sPr%d.%d=%d%s]", (char*) ANSI_COLOR_YELLOW, (hParam > 32 ? hParam - 32 : hParam), lParam, newValue, (char*) ANSI_COLOR_RESET);
            vDisplayMessage(App.Msg);
        }
    }

end_func:

    usleep(10 * 1000); // usleep(50*1000);

    if ((purgeBytes = modbus_purge_comm((modbus_t*) ctx)) > 0) {
        snprintf(App.Msg, App.MsgSize, "[%s%dbytes purged%s]\n", (char*) ANSI_COLOR_YELLOW, purgeBytes, (char*) ANSI_COLOR_RESET);
        vDisplayMessage(App.Msg);
    }

    return retVal;
}




int32_t modbus_check_pretimeout(void *pvSerialSlot) {
    if (pvSerialSlot) {
        SerialSlot *pSerialSlot = (SerialSlot *) pvSerialSlot;
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pSerialSlot->pActuator;
        char str[256];
        
        bool resendMsg = false;
        char hHigh = 0, bLow = 0;
        int32_t addr = 0;
        int32_t nb = 0;

        
        if (xTaskGetTickCount() - pSerialSlot->start_time > 15 && pSerialSlot->preTimeout == 0) {
            resendMsg = true;
            snprintf(str, sizeof (str), "%s[>]%s", (char*) ANSI_COLOR_YELLOW, (char*) ANSI_COLOR_RESET);
            vDisplayMessage(str);
        } else if (xTaskGetTickCount() - pSerialSlot->start_time > 30 && pSerialSlot->preTimeout == 1) {
            resendMsg = true;
            snprintf(str, sizeof (str), "%s[>>]%s", (char*) ANSI_COLOR_RED, (char*) ANSI_COLOR_RESET);
            vDisplayMessage(str);
        } else if (xTaskGetTickCount() - pSerialSlot->start_time > 30 && pSerialSlot->preTimeout == 2) {
            resendMsg = true;
            snprintf(str, sizeof (str), "%s[>>>]%s", (char*) ANSI_COLOR_MAGENTA, (char*) ANSI_COLOR_RESET);
            vDisplayMessage(str);
        } else  if (xTaskGetTickCount() - pSerialSlot->start_time > MODBUS_STREAM_PRE_TIMEOUT_MS && pSerialSlot->preTimeout == 3) {
            resendMsg = true;
        }

        if (resendMsg) {
            ////////////////////////////////////////
            // Pre-Timeout : reinvio richiesta
            //

            /////////////////////////
            // Generazione Warning
            //
            if (xTaskGetTickCount() - pSerialSlot->start_time > MODBUS_STREAM_PRE_TIMEOUT_MS && pSerialSlot->preTimeout == 3) {
                snprintf(str, sizeof (str), "[SER#%d] PreTimeout:resending request...data:%d/%d", pSerialSlot->stationId, modbus_data_available((modbus_t*) pSerialSlot->modbus_ctx) , pSerialSlot->waitingRespSize);
                if (generate_alarm((char*) str, 6009, 0, (int32_t) ALARM_WARNING, 0+1) < 0) {
                }
            }

            pSerialSlot->preTimeout++;

            

            /* xrt_modbus_read_registers_send((modbus_t*) pSerialSlot->modbus_ctx, addr, nb)*/
            
            if (send_msg((modbus_t*) pSerialSlot->modbus_ctx, (uint8_t*)xrt_modbus_get_last_req((modbus_t*) pSerialSlot->modbus_ctx), (int32_t)xrt_modbus_get_last_req_len((modbus_t*) pSerialSlot->modbus_ctx)) < 0) {
                /////////////////////////
                // Generazione Allarme
                //
                if (pSerialSlot->streamErrorCount > MAX_MODBUS_STREAM_ERROR) {
                    if (generate_alarm((char*) "[SER] PreTimeout Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0+1) < 0) {
                    }
                    // reinizializza il flusso
                    pSerialSlot->state = DEV_STATE_INIT;
                    return -1;
                }

                pSerialSlot->streamErrorCount++;
        
            } else {
                pSerialSlot->streamErrorCount = 0;
            }
        }


        return 1;
    }

    return 0;
}
            




            
//////////////////////////////////////////////
// Funzioni Homing
//

int32_t handle_modbus_homing_init(void *pvSerialSlot) {
    if (pvSerialSlot) {
        SerialSlot *pSerialSlot = (SerialSlot *) pvSerialSlot;
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pSerialSlot->pActuator;
        char str[256];

        if (pSerialSlot->homingRequest) {
            if (pActuator->homingTorqueMode) {
                pActuator->homingSeq = 1000;
            } else {
                pActuator->homingSeq = 0;
            }

            pActuator->homingRequest = 2;   // In corso
            pActuator->homingDone = 0;
            pActuator->homingPulsesPPT = 0;
            pActuator->homingTurnsPPT = 0;
            pActuator->homingZeroCounter = 0;
            pActuator->readPositionCounter = 0;
            pActuator->homingStartTime = xTaskGetTickCount();

            pSerialSlot->streamErrorCount = 0;
            pSerialSlot->readCount = 0;


            pSerialSlot->state = DEV_STATE_HOMING_SEND;
            return 1;
            
        } else {
            // Generazione Allarme        
            snprintf(str, sizeof (str), "[SER#%d] Invalid Homing request ", pSerialSlot->stationId);
            if (generate_alarm((char*) str, 6007, 0, (int32_t) ALARM_ERROR, 0+1) < 0) {
            }
            pSerialSlot->state = DEV_STATE_INIT;
            return 0;            
        }
    }

    
    return -1;
}



///////////////////////////////////
// Verifica ingresso digitale
// 
int32_t check_homing_trigger(void *pvSerialSlot) {
    if (pvSerialSlot) {
        SerialSlot *pSerialSlot = (SerialSlot *) pvSerialSlot;
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pSerialSlot->pActuator;
        if (pActuator->homingBoardId && pActuator->homingDI) {
            for (int32_t i = 0; i < machine.numIOBoardSlots; i++) {
                if (machine.ioBoardSlots[i].id == pActuator->homingBoardId) {
                    if ((int) pActuator->homingDI > 0 && pActuator->homingDI <= machine.ioBoardSlots[i].numDigitalOUT) {
                        if (machine.ioBoardSlots[i].digitalOUT[pActuator->homingDI - 1] == pActuator->homingDIvalue) {
                            // Segnale raggiunto : prossima sequenza
                            return 1;
                        } else {
                            return 0;
                        }
                    }
                }
            }
        }
    }
    return -1;
}




int32_t reenter_position_mode(void *pvSerialSlot) {
    if (pvSerialSlot) {
        SerialSlot *pSerialSlot = (SerialSlot *) pvSerialSlot;
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pSerialSlot->pActuator;
        bool bTurnOffandOnServo = false;
        char str[256];
        int res = 0;        
        
        // Non Necessario ma azzera la posizione
        if (bTurnOffandOnServo) {
            snprintf(App.Msg, App.MsgSize, "[%sHoming End : entering Servo OFF%s]\n", (char*) ANSI_COLOR_YELLOW, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);
            if (handle_actuator_servo_off((void *) pActuator) < 0) {
                // return -1;
                if (generate_alarm((char*) "MODBUS : handle_actuator_start() Error", 6101, 0, (int32_t) ALARM_FATAL_ERROR, 0+1) < 0) {
                }
                // Homing failed
                pActuator->homingSeq = 5000;
            }        
            usleep(10*1000);
        }
        
        snprintf(App.Msg, App.MsgSize, "[%sHoming End : entering Position Mode%s]\n", (char*) ANSI_COLOR_YELLOW, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);        
        if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 1), 00, 1, true, sizeof(int16_t), App.DebugModbusParam) < 0) {
            snprintf(str, sizeof (str), "[SER#%d] HOMING Error entering position Mode", pSerialSlot->stationId);
            if (generate_alarm((char*) str, 6009, 0, (int32_t) ALARM_ERROR, 0+1) <= 0) {
            }
        } else {
        }

        
        if (bTurnOffandOnServo) {
            usleep(10*1000);
            snprintf(App.Msg, App.MsgSize, "[%sHoming End : entering Servo ON%s]\n", (char*) ANSI_COLOR_YELLOW, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);
            if (handle_actuator_servo_on((void *) pActuator) < 0) {
                // return -1;
                if (generate_alarm((char*) "MODBUS : handle_actuator_servo_on() Error", 6101, 0, (int32_t) ALARM_FATAL_ERROR, 0+1) < 0) {
                }
                // Homing failed
                pActuator->homingSeq = 5000;
            }        
         }        
        
        return 1;
    }
}



int32_t handle_modbus_homing_send(void *pvSerialSlot) {
    if (pvSerialSlot) {
        SerialSlot *pSerialSlot = (SerialSlot *) pvSerialSlot;
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pSerialSlot->pActuator;
        char hHigh = 0, bLow = 0;
        int32_t addr = 0;
        int32_t nb = 0;
        char str[256];

        switch (pActuator->homingSeq) {
            
            case 0:                
                ////////////////////////
                // go to home : cmd
                //
                if (check_homing_trigger((void *)pvSerialSlot) == 1) {
                    pActuator->homingSeq = 100;
                    // Rimane sulla stessa seq. principale
                    return 1;                    
                } else {
                    handle_actuator_prepare_for_run (pvSerialSlot, AC_SERVO_HOMING_POSITION_IN_TABLE);
                    if (actuator_set_aux_io((void *) pActuator, (int32_t) SSR_STATE_OFF_AND_ON, (int32_t) 0) < 0) {
                    }
                }
                break;
                
            case 10: {
                //////////////////////////////////
                // go to home : send read req.
                //
                if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN) {
                    hHigh = 0, bLow = 0;
                    addr = MAKEWORD(bLow, hHigh);
                    nb = 6;
                } else if (pActuator->protocol == MODBUS_AC_SERVO_DELTA) {
                    hHigh = 5, bLow = 16;
                    addr = MAKEWORD(bLow, hHigh);
                    nb = 2;
                }                
                // Lunghezza risposta attesa
                pSerialSlot->waitingRespSize = 3 + 2 * nb + 2;
                if (xrt_modbus_read_registers_send((modbus_t*) pSerialSlot->modbus_ctx, addr, nb) < 0) {
                    pSerialSlot->streamErrorCount++;
                    if (pSerialSlot->streamErrorCount > MAX_MODBUS_STREAM_ERROR) {
                        // Generazione Allarme        
                        snprintf(str, sizeof (str), "[SER#%d] HOMING FEEDBACK Send Error", pSerialSlot->stationId);
                        if (generate_alarm((char*) str, 6007, 0, (int32_t) ALARM_FATAL_ERROR, 0+1) < 0) {
                        }
                        // Homing failed
                        pActuator->homingSeq = 5000;
                    }
                } else {
                    pSerialSlot->streamErrorCount++;                    
                }
                break;
            }
                
            case 100:
                // go to lost signal
                break;
                
            case 110:
                // go to lost signal :
                break;

                
            case 200:
                // go to zero encoder
                break;
                
                
            case 1000: {
                // Home by torque mode
                // Set the control method to the torque control mode (set Pr1.00 to 2), and select the torque instruction source（Pr4.00）.
                // Select the torque limitation source（Pr4.09）and the torque speed limitation instruction source（Pr4.02）.
                // the motor torque force and speed can be controlled by setting Pr4.05, Pr4.06, Pr4.11, Pr4.12.
                // The controller inputs servo-on signal (servo-on signal connected to servo CN3-5), to excite the motor.
                // The current motor torque force can be known by monitoring the load rate（Pr0.09）and the motor speed（Pr0.00）
                bool turnOffServo = false;
                if (turnOffServo) {
                    snprintf(App.Msg, App.MsgSize, "[%sEntering Servo OFF%s]\n", (char*) ANSI_COLOR_YELLOW, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);
                    if (handle_actuator_servo_off((void *) pActuator) < 0) {
                        // return -1;
                        if (generate_alarm((char*) "MODBUS : handle_actuator_servo_off() Error", 6101, 0, (int32_t) ALARM_FATAL_ERROR, 0+1) < 0) {
                        }
                        // Homing failed
                        pActuator->homingSeq = 5000;
                    } else {
                        snprintf(App.Msg, App.MsgSize, "[%sWaiting...%s]\n", (char*) ANSI_COLOR_YELLOW, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);
                        pActuator->homingSeq = 1001;
                        pActuator->homingStartTime = xTaskGetTickCount();
                        return 1;
                    }
                } else {
                    pActuator->homingSeq = 1001;
                    pActuator->homingStartTime = xTaskGetTickCount();
                    return 1;                    
                }
                break;
            }
            
            case 1001:
                // Attesa
                if (xTaskGetTickCount() - pActuator->homingStartTime >= 500) {
                    pActuator->homingSeq = 1002;
                    return 1;
                } else {
                    return 0;
                }
                break;
                
            case 1002:
                snprintf(App.Msg, App.MsgSize, "[%sStart Homing : entering Torque Mode%s]\n", (char*) ANSI_COLOR_YELLOW, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);

                if (handle_actuator_servo_off((void *) pActuator) < 0) {
                    // return -1;
                    if (generate_alarm((char*) "MODBUS : handle_actuator_servo_off() Error", 6101, 0, (int32_t) ALARM_FATAL_ERROR, 0+1) < 0) {
                    }
                    // Homing failed
                    pActuator->homingSeq = 5000;
                }        
                usleep(10*1000);
                
                //////////////////////////////////////////////////////////////////////////////
                // Creazione tabella posizionamento con la posizione di homing offset
                //
                handle_modbus_service_setup_II((void*)pSerialSlot);
                
                pSerialSlot->state = DEV_STATE_HOMING_SEND;
                
                if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 1), 00, 2, true, sizeof(int16_t), App.DebugModbusParam) < 0) {
                    // Homing failed
                    pActuator->homingSeq = 5000;
                } else {
                    
                    // Torque source : internal register (Pr4.05)
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 4), 00, 0, true, sizeof(int16_t), App.DebugModbusParam) < 0) {
                        pActuator->homingSeq = 5000;
                    } else {
                        // Speed source : internal register (Pr4.06)
                        if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 4), 02, 0, true, sizeof(int16_t), App.DebugModbusParam) < 0) {
                            pActuator->homingSeq = 5000;
                        } else {
                            // Torque limit (-300..0..+300.0%)
                            if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 4), 05, (int16_t)pActuator->homing_rated_torque, true, sizeof(int16_t), App.DebugModbusParam) < 0) {
                                pActuator->homingSeq = 5000;
                            } else {
                                // Speed limit
                                if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 4), 06, (int16_t)pActuator->homing_speed_rpm, true, sizeof(int16_t), App.DebugModbusParam) < 0) {
                                    pActuator->homingSeq = 5000;
                                } else {
                                }

                                
                                /*
                                // Natural rotation torque limit  (?)
                                if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 4), 11, (int16_t)pActuator->homing_rated_torque, true, sizeof(int16_t)) < 0) {
                                    pActuator->homingSeq = 5000;
                                } else {
                                }
                                
                                // Inversion torque limit (?)
                                if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 4), 12, (int16_t)pActuator->homing_rated_torque, true, sizeof(int16_t)) < 0) {
                                    pActuator->homingSeq = 5000;
                                } else {
                                }
                                */

                                int32_t maxTurnsPPT = 0, maxPulsesPPT = 0;
                                actuator_position_to_encoder( (void *)pActuator, pActuator->end_rpos * 1.10f, &maxTurnsPPT, &maxPulsesPPT);

                                maxTurnsPPT += 2;
                                
                                                    
                                // Max turn ( 0..20)
                                if (maxTurnsPPT > 20)
                                    maxTurnsPPT = 0;

                                /*
                                snprintf(App.Msg, App.MsgSize, "[%smaxTurns:%d%s]\n", (char*) ANSI_COLOR_YELLOW, maxTurns, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);

                                if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 4), 04, (int16_t)maxTurns, true, sizeof(int16_t)) < 0) {
                                    pActuator->homingSeq = 5000;
                                } else {
                                }
                                */
                                
                                
                                /* OK
                                snprintf(App.Msg, App.MsgSize, "[%stest param write%s]\n", (char*) ANSI_COLOR_YELLOW, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);        
                                if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 2), 01, 3, true, sizeof(int16_t)) < 0) {
                                    snprintf(str, sizeof (str), "[SER#%d] HOMING Error entering position Mode", pSerialSlot->stationId);
                                    vDisplayMessage(str);
                                } else {
                                }
                                 * */                                
                            }
                        }
                    }
                }
                
                snprintf(App.Msg, App.MsgSize, "[%sEntering Servo ON%s]\n", (char*) ANSI_COLOR_YELLOW, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);
                if (handle_actuator_servo_on((void *) pActuator) < 0) {
                    // return -1;
                    if (generate_alarm((char*) "MODBUS : handle_actuator_servo_off() Error", 6101, 0, (int32_t) ALARM_FATAL_ERROR, 0+1) < 0) {
                    }
                    // Homing failed
                    pActuator->homingSeq = 5000;
                    pSerialSlot->state = DEV_STATE_HOMING_DONE;
                } else {                
                    int32_t rc = modbus_purge_comm((modbus_t*) pSerialSlot->modbus_ctx);
                    if (rc) {
                        fprintf(stdout, "[purge comm serial board#%d - %d bytes]\n", pSerialSlot->boardId, rc);
                        fflush(stdout);
                    }
                    snprintf(App.Msg, App.MsgSize, "[%sWaiting...%s]\n", (char*) ANSI_COLOR_YELLOW, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);
                    pSerialSlot->state = DEV_STATE_HOMING_SEND;
                    pActuator->homingSeq = 1003;
                    pActuator->homingStartTime = xTaskGetTickCount();
                    pSerialSlot->streamErrorCount = 0;
                    return 1;
                }
                
                break;

                
            case 1003:
                // Attesa
                if (xTaskGetTickCount() - pActuator->homingStartTime >= 1500) {
                    pActuator->homingSeq = 1010;
                    return 1;
                } else {
                    return 0;
                }
                break;
                
                
            case 1010: {
                /////////////////////////////////////////
                // go to home : Torque Mode : read req.
                //
                char hHigh = 0, bLow = 0;
                int32_t addr = 0;
                int32_t nb = 0;
                char str[256];

                if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN) {
                    hHigh = 0, bLow = 0;
                    addr = MAKEWORD(bLow, hHigh);
                    nb = 6;
                } else if (pActuator->protocol == MODBUS_AC_SERVO_DELTA) {
                    hHigh = 5, bLow = 16;
                    addr = MAKEWORD(bLow, hHigh);
                    nb = 2;
                }

                // Lunghezza risposta attesa
                pSerialSlot->waitingRespSize = 3 + 2 * nb + 2;

                if (xrt_modbus_read_registers_send((modbus_t*) pSerialSlot->modbus_ctx, addr, nb) < 0) {
                
                    pSerialSlot->streamErrorCount++;
                    if (pSerialSlot->streamErrorCount > 10 /*MAX_MODBUS_STREAM_ERROR*/) {
                        // Generazione Allarme        
                        snprintf(str, sizeof (str), "[SER#%d] HOMING FEEDBACK Send Error (%d errs)", pSerialSlot->stationId, pSerialSlot->streamErrorCount);                        
                        if (generate_alarm((char*) str, 6007, 0, (int32_t) ALARM_FATAL_ERROR, 0+1) < 0) {
                        }

                        // Homing failed
                        pActuator->homingSeq = 5000;

                        int32_t rc = modbus_purge_comm((modbus_t*) pSerialSlot->modbus_ctx);
                        if (rc) {
                            fprintf(stdout, "[purge comm serial board#%d - %d bytes]\n", pSerialSlot->boardId, rc);
                            fflush(stdout);
                        }

                        return 1;
                    }
                    
                    usleep(10*1000);                    
                    pSerialSlot->state = DEV_STATE_HOMING_SEND;
                    return -1;
                    
                } else {
                    pSerialSlot->state = DEV_STATE_HOMING_RECV;
                    pSerialSlot->streamErrorCount = 0;
                    return 1;
                }
                break;
            }
                
        
            default:
                break;
        }

        pSerialSlot->state = DEV_STATE_HOMING_RECV;
        return 1;
    }

    return 0;
}



int32_t handle_modbus_homing_recv(void *pvSerialSlot) {
    if (pvSerialSlot) {
        SerialSlot *pSerialSlot = (SerialSlot *) pvSerialSlot;
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pSerialSlot->pActuator;
        float newPosition = 0.0f;
        char str[256];
        int res_read = -1;
        int32_t avaiable_bytes = 0;
        
        
        switch (pActuator->homingSeq) {
            case 10:
            case 1010: {
                ///////////////////////////////////
                // go to home : recv read resp
                //
                avaiable_bytes = modbus_data_available((modbus_t*) pSerialSlot->modbus_ctx);
                if (avaiable_bytes >= pSerialSlot->waitingRespSize) {
                    if ((res_read = xrt_modbus_read_registers_recv((modbus_t*) pSerialSlot->modbus_ctx, &pSerialSlot->data[0])) < 0) {
                        pSerialSlot->streamErrorCount++;
                        if (pSerialSlot->streamErrorCount > MAX_MODBUS_STREAM_ERROR) {
                            // Generazione Allarme        
                            snprintf(str, sizeof (str), "[SER#%d] CMD FEEDBACK Send Error", pSerialSlot->stationId);
                            if (generate_alarm((char*) str, 6008, 0, (int32_t) ALARM_FATAL_ERROR, 0+1) < 0) {
                            }
                            // Homing failed
                            pActuator->homingSeq = 5000;
                            pSerialSlot->state = DEV_STATE_HOMING_DONE;
                            return -1;
                        }
                    } else {
                        pSerialSlot->streamErrorCount = 0;
                        // Prossima sequenza
                        pSerialSlot->state = DEV_STATE_HOMING_DONE;
                    }
                    
                } else {
                    
                    //////////////////////////
                    // Controllo Timeout
                    //
                    if (xTaskGetTickCount() - pActuator->homingStartTime > pActuator->homing_timeout_ms) {

                        if (App.DebugMode) {

                            snprintf(str, sizeof (str), "[SER#%d] HOMING FORCED By Timeout of %0.2fsec in DebugMode", pSerialSlot->stationId, (float)pActuator->homing_timeout_ms / 1000.0f);
                            if (generate_alarm((char*) str, 6009, 0, (int32_t) ALARM_WARNING, 0+1) < 0) {                         
                            }

                            // Homing OK for test
                            pActuator->homingSeq = 3000;
                            pSerialSlot->state = DEV_STATE_HOMING_DONE;            
                            return 1;

                        } else {

                            /////////////////////////////
                            // Comunicazione in timeout 
                            //
                            snprintf(str, sizeof (str), "[SER#%d] HOMING Timeout : %0.2fsec", pSerialSlot->stationId, (float)pActuator->homing_timeout_ms / 1000.0f);
                            if (generate_alarm((char*) str, 6009, 0, (int32_t) ALARM_ERROR, 0+1) < 0) {                         
                            }

                            // Homing failed
                            pActuator->homingSeq = 5000;            
                            pSerialSlot->state = DEV_STATE_HOMING_DONE;            
                            return -1;
                        }
                    }                    
                }
                break;
            }
        }


        
        
        switch (pActuator->homingSeq) {
            
            case 0:
                ////////////////////
                // go to home
                //
                break;

            case 10: {
                ///////////////////////////////////
                // go to home recv read resp
                //
                if (res_read >= 0) {
                    pSerialSlot->driverTurns = (int32_t) ((int16_t)pSerialSlot->data[4 + 1]);
                    pSerialSlot->driverPulses = (int32_t) ((uint16_t)pSerialSlot->data[4 + 0]);
                    pSerialSlot->driverPosition = (int32_t) pSerialSlot->driverPulses + (int32_t) pSerialSlot->driverTurns * (int32_t)pActuator->pulsesOverflow;
                    pSerialSlot->driverSpeed = (int32_t) ((int16_t) pSerialSlot->data[0]);
                    pSerialSlot->driverAcc = (int32_t) MAKEDWORD(0, 0);

                    pSerialSlot->readCount++;

                    // Passa il dato alla logica
                    if (pActuator) {

                        pActuator->driverTurns = pSerialSlot->driverTurns;
                        pActuator->driverPulses = pSerialSlot->driverPulses;
                        pSerialSlot->driverPosition = pSerialSlot->driverPosition;

                        actuator_encoder_to_position((void *) pActuator, (int32_t) pSerialSlot->driverTurns, (int32_t) pSerialSlot->driverPulses, &newPosition);                        
                        actuator_handle_read_position((void *) pActuator, newPosition, false);
                        
                        pActuator->readCounter++;
                        pActuator->readPositionCounter++; // conteggio letture posizione nell'ambito del movimento

                        // posizione virtuale (0...1000)
                        update_actuator_virtual_pos(pActuator);
                    }
                }
                break;
            }
                
            case 100:
                // go to lost signal
                break;
                
            case 200:
                // go to zero encoder
                break;
                
            case 1000:
                // Home by torque mode
                break;
                
            case 1010:
                // Home by torque mode : monitoring
                
                if (res_read >= 0) {
                    int32_t driverPosition = (int32_t) ((uint16_t)pSerialSlot->data[4 + 0]) + (int32_t) ((int16_t)pSerialSlot->data[4 + 1]) * (int32_t)pActuator->pulsesOverflow;
                    int32_t driverSpeed = (int32_t) ((uint16_t)pSerialSlot->data[0 + 0]);
                    
                    if (driverSpeed == 0) {
                        pActuator->homingZeroCounter++;
                        if (pActuator->homingZeroCounter >= NUM_ZERO_SPEED_TO_HOMING) {
                            snprintf(App.Msg, App.MsgSize, "[%sZero speed reached on Torque Mode%s]     \r", (char*) ANSI_COLOR_YELLOW, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);                           
                            pActuator->homingSeq = 3000;
                            pSerialSlot->state = DEV_STATE_HOMING_DONE;
                            return 1;
                        }
                    } else {
                        pActuator->homingZeroCounter = 0;
                    }
                }
                break;

                
            default:
                break;
        }
        
        return 1;
    }

    return 0;
}




int32_t handle_modbus_homing_done(void *pvSerialSlot) {
    if (pvSerialSlot) {
        SerialSlot *pSerialSlot = (SerialSlot *) pvSerialSlot;
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pSerialSlot->pActuator;
        int res_read = -1;
        int32_t avaiable_bytes = 0;
        char str[256];
        
        switch (pActuator->homingSeq) {
            
            case 0:
                if (check_homing_trigger((void *)pvSerialSlot) == 1) {
                    pActuator->homingSeq = 100;
                } else {
                    // go to home done : go to read stream
                    pActuator->homingSeq = 10;
                }
                pSerialSlot->state = DEV_STATE_HOMING_SEND;
                break;
                
            case 10:
                ///////////////////////////////////
                // // Verifica ingresso digitale
                // 
                if (pActuator->homingBoardId && pActuator->homingDI) {
                    for (int32_t i = 0; i < machine.numIOBoardSlots; i++) {
                        if (machine.ioBoardSlots[i].id == pActuator->homingBoardId) {
                            if ((int) pActuator->homingDI > 0 && pActuator->homingDI <= machine.ioBoardSlots[i].numDigitalOUT) {
                                if (machine.ioBoardSlots[i].digitalOUT[pActuator->homingDI - 1] == pActuator->homingDIvalue) {
                                    // Segnale raggiunto : prossima sequenza
                                    pActuator->homingSeq = 100;
                                }
                            }
                        }
                    }
                }
                pSerialSlot->state = DEV_STATE_HOMING_SEND;
                break;
                
            case 100:
                // go to lost signal
                if (check_homing_trigger((void *)pvSerialSlot) == 0) {
                    pActuator->homingSeq = 200;
                }
                pSerialSlot->state = DEV_STATE_HOMING_SEND;
                break;
                
            case 200:
                // go to zero encoder : non ha senso perchè lo zero encoder varia sempre
                pActuator->homingSeq = 3000;
                pSerialSlot->state = DEV_STATE_HOMING_SEND;
                break;

            case 1000:
                pActuator->homingSeq = 1001;
                pSerialSlot->state = DEV_STATE_HOMING_SEND;
                break;
                
            case 1001:
                pActuator->homingSeq = 1002;
                pSerialSlot->state = DEV_STATE_HOMING_SEND;
                break;
                
            case 1002:
                pActuator->homingSeq = 1010;
                pSerialSlot->state = DEV_STATE_HOMING_SEND;
                break;
                
            case 1010:
                pSerialSlot->state = DEV_STATE_HOMING_SEND;
                break;

                
            case 3000: {
                // Home done
                vDisplayMessage("[Homing done]");
                
                reenter_position_mode((void *)pvSerialSlot);
        
                int32_t rc = modbus_purge_comm((modbus_t*) pSerialSlot->modbus_ctx);
                if (rc) {
                    fprintf(stdout, "[purge comm serial board#%d - %d bytes]\n", pSerialSlot->boardId, rc);
                    fflush(stdout);
                }
                
                pActuator->step = STEP_READY;
                pActuator->error = 0;
               
                // snprintf(App.Msg, App.MsgSize, "[Waiting before enter setup...Homing state : %d ]\n", pSerialSlot->state); vDisplayMessage(App.Msg);
                                               
                pActuator->homingStartTime = xTaskGetTickCount();
                pSerialSlot->state = DEV_STATE_HOMING_DONE;
                pActuator->homingSeq = 4000;
                break;
            }
            

            case 4000: {
                // Home post action : re-setup
                if (xTaskGetTickCount() - pActuator->homingStartTime >= 500) {

                    // vDisplayMessage("[Reading actual position...]");

                    char hHigh = 0, bLow = 0;
                    int32_t addr = MAKEWORD(bLow, hHigh);
                    int32_t rc, nb = 6;
                    int32_t offsetTurnsPPT = 0, offsetPulsesPPT = 0;
                    

                    if ((rc = modbus_read_registers((modbus_t*) pSerialSlot->modbus_ctx, addr, 1, (uint16_t *) & pSerialSlot->data)) < 0) {
                        snprintf(App.Msg, App.MsgSize, "[%s modbus Pr%d.%d read error : %s %s]\n", (char*) ANSI_COLOR_RED, hHigh, bLow, (char*) modbus_strerror(errno), (char*) ANSI_COLOR_RESET);
                        vDisplayMessage(App.Msg);
                    }

                    // Offset in Pulse units
                    pActuator->homingTurnsPPT = 0;
                    pActuator->homingPulsesPPT = 0;
                    
                    actuator_position_to_encoder( (void *)pActuator, pActuator->homing_offset_mm, &offsetTurnsPPT, &offsetPulsesPPT);
                                       

                    pSerialSlot->driverTurns = (int32_t) ((int16_t)pSerialSlot->data[4 + 1]);
                    pSerialSlot->driverPulses = (int32_t) ((uint16_t)pSerialSlot->data[4 + 0]);                

                    actuator_ppo_to_ppt((void *)pActuator, pSerialSlot->driverTurns, pSerialSlot->driverPulses, &pActuator->homingPulsesPPT, &pActuator->homingTurnsPPT);
                            
                    // Aggiunta alle pulsazioni correnti l'offset
                    actuator_add_pulse_ppt( (void *)pActuator, offsetTurnsPPT, offsetPulsesPPT, &pActuator->homingTurnsPPT, &pActuator->homingPulsesPPT, 0+0);

                    


                    float newPosition = 0.0f;                    
                    actuator_encoder_to_position((void *) pActuator, (int32_t) pSerialSlot->driverTurns, (int32_t) pSerialSlot->driverPulses, &newPosition);
                    actuator_handle_read_position((void *) pActuator, newPosition, false);
                    

                    // posizione virtuale (0...1000)
                    update_actuator_virtual_pos(pActuator);

                            
                    snprintf(App.Msg, App.MsgSize, "[%sHoming Torque Mode Done : Turns:%d+%d=%d - Pulses:%d+%d=%d%s]\n"
                            , (char*) ANSI_COLOR_GREEN
                            , pActuator->driverTurns, offsetTurnsPPT, pActuator->homingTurnsPPT
                            , pActuator->driverPulses, offsetPulsesPPT, pActuator->homingPulsesPPT
                           , (char*) ANSI_COLOR_RESET);
                    vDisplayMessage(App.Msg);
                                   
                    

                    
                    // Posizione homing offset corrente
                    if (handle_actuator_prepare_for_run((void *) pActuator, AC_SERVO_HOMING_POSITION_IN_TABLE) < 0) {
                        if (generate_alarm((char*) "MODBUS : handle_actuator_prepare_for_run() Error", 6110, 0, (int32_t) ALARM_WARNING, 0+1) < 0) {
                        }
                        pSerialSlot->state = DEV_STATE_HOMING_DONE;
                        pActuator->homingSeq = 5000;
                        return -1;
                    }
                      
                    /*
                    addr = MAKEWORD((uint8_t) 01, (uint8_t) HEX_TO_INT(2, 6));
                    int bit_pos = 1 + (1 * 16 + 1 * 32 + 0 * 64 + 0 * 128) + 256;
                    // Lunghezza risposta attesa
                    pSerialSlot->waitingRespSize = 7;
                    if (xrt_modbus_write_register_send((modbus_t*) pSerialSlot->modbus_ctx, addr, bit_pos) < 0) {                        
                    }
                    */
                    
                    pActuator->homingStartTime = xTaskGetTickCount();
                    pActuator->homingSeq = 4010;
                    pSerialSlot->state = DEV_STATE_HOMING_DONE;

                } else {
                    // snprintf(App.Msg, App.MsgSize, "[%dms left]", 500 - (xTaskGetTickCount() - pActuator->homingStartTime)); vDisplayMessage(App.Msg);                    
                }                
                break;
            }
            
            case 4010: {
                // Home post action : jump out
                if (xTaskGetTickCount() - pActuator->homingStartTime >= 500) {

                    // vDisplayMessage("[Going to offseted position...]");

                    pActuator->homingStartTime = xTaskGetTickCount();
                    pActuator->homingSeq = 4020;
                    pSerialSlot->state = DEV_STATE_HOMING_DONE;
                    
                } else {
                    /*
                    avaiable_bytes = modbus_data_available((modbus_t*) pSerialSlot->modbus_ctx);
                    if (avaiable_bytes >= pSerialSlot->waitingRespSize) {
                        if ((res_read = xrt_modbus_read_registers_recv((modbus_t*) pSerialSlot->modbus_ctx, &pSerialSlot->data[0])) < 0) {
                        } else {
                            pActuator->homingStartTime = xTaskGetTickCount();
                            pActuator->homingSeq = 4020;
                            pSerialSlot->state = DEV_STATE_HOMING_DONE;
                            
                        }
                    }
                    */
                }
                break;
            }

            
            case 4020: {
                // Home post action : start movement
                if (actuator_set_aux_io((void *) pActuator, (int32_t) SSR_STATE_OFF_AND_ON, (int32_t) 0) < 0) {
                    if (generate_alarm((char*) "MODBUS : actuator_set_aux_io() Error", 6111, 0, (int32_t) /*ALARM_FATAL_ERROR*/ALARM_WARNING, 0+1) < 0) {
                    }                        
                    pSerialSlot->state = DEV_STATE_HOMING_DONE;
                    pActuator->homingSeq = 5000;
                    return -1;
                } else {
                    pActuator->homingStartTime = xTaskGetTickCount();
                    pActuator->homingSeq = 4030;
                    pSerialSlot->state = DEV_STATE_HOMING_DONE;                        
                }
                break;
            }
            
            
            case 4030: {
                // Home post action : jump out
                if (xTaskGetTickCount() - pActuator->homingStartTime >= 1500) {
                    
                    // Posizione homing offset corrente
                    if (handle_actuator_prepare_for_run((void *) pActuator, AC_SERVO_NEUTRAL_POSITION_IN_TABLE) < 0) {
                        if (generate_alarm((char*) "MODBUS : handle_actuator_prepare_for_run() Error", 6110, 0, (int32_t) /*ALARM_FATAL_ERROR*/ALARM_WARNING, 0+1) < 0) {
                        }
                        pSerialSlot->state = DEV_STATE_HOMING_DONE;
                        pActuator->homingSeq = 5000;
                        return -1;
                    }
                                      
                    pActuator->homingStartTime = xTaskGetTickCount();
                    pActuator->homingSeq = 4040;

                    
                } else {
                    // snprintf(App.Msg, App.MsgSize, "[%s waiting:%d.%d %s]\n", (char*) ANSI_COLOR_BLUE, pSerialSlot->state, pActuator->homingSeq, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);
                }
                break;
            }

            
            case 4040: {
                // Verify Homed OK
                float newPosition = 0.0f;                    
                char hHigh = 0, bLow = 0;
                int32_t addr = MAKEWORD(bLow, hHigh);
                int32_t nb = 6;

                if (modbus_read_registers((modbus_t*) pSerialSlot->modbus_ctx, addr, nb, (uint16_t *) & pSerialSlot->data) < 0) {
                    snprintf(App.Msg, App.MsgSize, "[%s modbus Pr%d.%d read error : %s %s]\n", (char*) ANSI_COLOR_RED, hHigh, bLow, (char*) modbus_strerror(errno), (char*) ANSI_COLOR_RESET);
                    vDisplayMessage(App.Msg);
                } else {
                    pSerialSlot->driverTurns = (int32_t) ((int16_t)pSerialSlot->data[4 + 1]);
                    pSerialSlot->driverPulses = (int32_t) ((uint16_t)pSerialSlot->data[4 + 0]);                
                    actuator_encoder_to_position((void *) pActuator, (int32_t) pSerialSlot->driverTurns, (int32_t) pSerialSlot->driverPulses, &newPosition);
                    actuator_handle_read_position((void *) pActuator, newPosition, false);
                    // posizione virtuale (0...1000)
                    update_actuator_virtual_pos(pActuator);
                }

                if (xTaskGetTickCount() - pActuator->homingStartTime >= 3000) {
                    if (fabs(newPosition) >= MAX_HOMING_POSITION_ERROR ) {
                        // Errore
                        snprintf(str, sizeof(str), "Homing Failed Error : %0.3f / %0.3f" , fabs(newPosition), MAX_HOMING_POSITION_ERROR );
                        if (generate_alarm((char*) str, 6101, 0, (int32_t) ALARM_ERROR, 0+1) < 0) {
                        }
                        pActuator->homingDone = false;
                        
                    } else {
                        // OK
                        snprintf(App.Msg, App.MsgSize, "[%sHoming done : pos:%d.%d = %0.3f%s]\n"
                                , (char*) ANSI_COLOR_BLUE
                                , pActuator->driverTurns, pActuator->driverPulses, newPosition
                               , (char*) ANSI_COLOR_RESET);
                        vDisplayMessage(App.Msg);

                        pActuator->homingDone = true;
                    }
                    
                    pActuator->homingSeq = 0;
                    pActuator->homingRequest = 0;
                    pSerialSlot->homingRequest = 0;
                    pSerialSlot->state = DEV_STATE_INIT_STREAM;
                    return 1;
                }

                break;
            }
            
            case 5000:
                // Home failed
                vDisplayMessage("[Homing failed!]");
                pActuator->homingRequest = 0;
                pActuator->homingDone = false;
                pActuator->homingPulsesPPT = 0;
                pActuator->homingTurnsPPT = 0;
                
                pActuator->step = STEP_ERROR;
                pActuator->error = 5000;

                reenter_position_mode((void *)pvSerialSlot);

                pSerialSlot->homingRequest = 0;
                pSerialSlot->state = DEV_STATE_INIT_STREAM;
                break;

            default:
                pSerialSlot->state = DEV_STATE_INIT_STREAM;
                break;
                
        }
        return 1;
    }

    return 0;
}







int32_t handle_modbus_service_reset(void *pvSerialSlot) {
    if (pvSerialSlot) {
        SerialSlot *pSerialSlot = (SerialSlot *) pvSerialSlot;
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pSerialSlot->pActuator;

        float newPosition = 0.0;
        char hHigh = 0, bLow = 0;
        int32_t addr = 0;
        int32_t nb = 0;

        pSerialSlot->resetRequest = 0;

        if (pActuator) {
            
            if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN) {
            
                if (generate_alarm((char*) "Resetting AC Servo Params...", 4001, 0, (int32_t) ALARM_WARNING, 0+1) < 0) {
                }
                
                // 6.01 - Servo Off
                if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 6), 01, 0, true, 2, App.DebugModbusParam) < 0) {
                }
                
                // Perdita della continuita' con l'encoder
                snprintf(App.Msg, App.MsgSize, "[%s Servo OFF : Lost encoder reference%s]\n", (char*) ANSI_COLOR_YELLOW, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);

                pActuator->homingRequest = 0;
                pActuator->homingDone = 0;               
                pActuator->homingTurnsPPT = 0;
                pActuator->homingPulsesPPT = 0;
                
                usleep (500*1000);
                
                // 1.61 - Reset
                if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 1), 61, 65535, true, 2, App.DebugModbusParam) < 0) {
                }
                
                // 7.00 - Station ID
                if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 7), 0, 1, true, 2, App.DebugModbusParam) < 0) {
                }

                // 7.01 - Baud
                if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 7), 1, 4, true, 2, App.DebugModbusParam) < 0) {
                }

                // 7.02 - Serail param : 8+N+1
                if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 7), 2, 0, true, 2, App.DebugModbusParam) < 0) {
                }

                // 8.00 - Servo off at start
                if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 8), 0, 0, true, 2, App.DebugModbusParam) < 0) {
                }

                // 8.01 - fault
                if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 8), 1, 0, true, 2, App.DebugModbusParam) < 0) {
                }

                // 8.05 - reboot
                if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 8), 5, 1, true, 2, App.DebugModbusParam) < 0) {
                }
                
                
            } else if (pActuator->protocol == MODBUS_AC_SERVO_DELTA) {
            }
        }
            
        
        pSerialSlot->state = DEV_STATE_INIT_STREAM;

        return 1;
    }

    return 0;
}





int32_t handle_modbus_service_setup(void *pvSerialSlot) {
    if (pvSerialSlot) {
        SerialSlot *pSerialSlot = (SerialSlot *) pvSerialSlot;
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pSerialSlot->pActuator;

        int32_t targetTurnsPPT = 0, targetPulsesPPT = 0, targetTurnsPPT2 = 0, targetPulsesPPT2 = 0;
        float rpos = 0.0f;
        float newPosition = 0.0;
        char hHigh = 0, bLow = 0;
        int32_t addr = 0;
        int32_t nb = 0;


        // snprintf(App.Msg, App.MsgSize, "[%s handle_modbus_service_setup() %s]", (char*) ANSI_COLOR_GREEN, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);

        pSerialSlot->setupRequest = 0;

        if (pActuator) {
            int32_t speed_rpm = pActuator->speed_auto1;
            int32_t acc_rpms2_1 = pActuator->acc_auto1, dec_rpms2_1 = acc_rpms2_1;
            int32_t speed_rpm2 = pActuator->speed_auto2;
            int32_t acc_rpms2_2 = pActuator->acc_auto2, dec_rpms2_2 = acc_rpms2_2;


            // Posizione finale in unità encoder
            actuator_position_to_encoder(pActuator, pActuator->end_rpos, &targetTurnsPPT, &targetPulsesPPT);

            // Posizione di ritorno
            targetTurnsPPT2 = -targetTurnsPPT;
            targetPulsesPPT2 = -targetPulsesPPT;

            
            if (handle_actuator_servo_off((void *) pActuator) < 0) {
                // return -1;
                if (generate_alarm((char*) "MODBUS : handle_actuator_start() Error", 6101, 0, (int32_t) ALARM_FATAL_ERROR, 0+1) < 0) {
                }
            }
        }
            
        pSerialSlot->start_time = xTaskGetTickCount();
        
        pSerialSlot->state = DEV_STATE_SERVICE_SETUP_I;

        return 1;
    }

    return 0;
}

int32_t handle_modbus_service_setup_I(void *pvSerialSlot) {
    if (pvSerialSlot) {
        SerialSlot *pSerialSlot = (SerialSlot *) pvSerialSlot;
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pSerialSlot->pActuator;


        if (xTaskGetTickCount() - pSerialSlot->start_time >= 500) {
            pSerialSlot->start_time = 0;
            pSerialSlot->state = DEV_STATE_SERVICE_SETUP_II;
        }

        return 1;
    }

    return 0;
}

int32_t handle_modbus_service_setup_II(void *pvSerialSlot) {
    if (pvSerialSlot) {
        SerialSlot *pSerialSlot = (SerialSlot *) pvSerialSlot;
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pSerialSlot->pActuator;

        int32_t targetTurnsPPT = 0, targetPulsesPPT = 0, targetTurnsPPT2 = 0, targetPulsesPPT2 = 0, targetTurnsForwPPT = 0, targetTurnsBackwPPT = 0, targetPulsesForwPPT = 0, targetPulsesBackwPPT = 0;
        float rpos = 0.0f;
        float newPosition = 0.0;
        char hHigh = 0, bLow = 0;
        int32_t addr = 0;
        int32_t nb = 0;


        // snprintf(App.Msg, App.MsgSize, "[%s handle_modbus_service_setup() %s]", (char*) ANSI_COLOR_GREEN, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);

        pSerialSlot->setupRequest = 0;

        if (pActuator) {
            int32_t speed_rpm = pActuator->speed_auto1;
            int32_t acc_rpms2_1 = pActuator->acc_auto1, dec_rpms2_1 = acc_rpms2_1;
            int32_t speed_rpm2 = pActuator->speed_auto2;
            int32_t acc_rpms2_2 = pActuator->acc_auto2, dec_rpms2_2 = acc_rpms2_2;
            int32_t homingTurnsPPT = 0;
            int32_t homingPulsesPPT = 0;

            
            
            
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //      N.B.: Preapara l'ulscita Alta; per lo start è necessario spegnere a riaccendere l'uscta
            //
            if (actuator_set_aux_io((void *) pActuator, (int32_t) 1, (int32_t) 0) < 0) {
                if (generate_alarm((char*) "AUX DO no activated", 6019, 0, (int32_t) ALARM_FATAL_ERROR, 0+1) < 0) {
                }
            }            

            
            
            // Unità encoder inizio e fine
            actuator_position_to_encoder( pActuator, (pActuator->start_rpos), &targetTurnsPPT, &targetPulsesPPT);
            actuator_position_to_encoder( pActuator, (pActuator->end_rpos), &targetTurnsPPT2, &targetPulsesPPT2);
            
            // Corsa in avanti in unità encoder
            actuator_delta_pulse(pActuator, targetTurnsPPT2, targetTurnsPPT, targetPulsesPPT2, targetPulsesPPT, &targetTurnsForwPPT, &targetPulsesForwPPT, 0+0);
            
            
            // Posizione di ritorno
            targetTurnsBackwPPT = -targetTurnsForwPPT;
            targetPulsesBackwPPT = -targetPulsesForwPPT;

            


            
            if (pActuator->homingTorqueMode) {
                // Corsa verso la posizione zero : l'offset è già stato assegnato
                actuator_position_to_encoder( pActuator, (0.0f), &targetTurnsPPT, &targetPulsesPPT);
                actuator_position_to_encoder( pActuator, (pActuator->homing_offset_mm), &targetTurnsPPT2, &targetPulsesPPT2);
                actuator_delta_pulse(pActuator, targetTurnsPPT2, targetTurnsPPT, targetPulsesPPT2, targetPulsesPPT, &homingTurnsPPT, &homingPulsesPPT, 0+0);
            } else {
                // Direzione Homing
                if (pActuator->homing_position == OFF) {
                    // Da ON a OFF
                    homingTurnsPPT = -targetTurnsForwPPT;
                    homingPulsesPPT = -targetPulsesForwPPT;
                } else {
                    // Da OFF a ON
                    homingTurnsPPT = targetTurnsForwPPT;
                    homingPulsesPPT = targetPulsesForwPPT;
                }
            }


            /*
            snprintf(App.Msg, App.MsgSize, "[%s %s Homing:%d.%d - toPos:%d.%d %s]", (char*) ANSI_COLOR_BLUE
                    ,pActuator->name
                    ,homingTurnsPPT, homingPulsesPPT
                    ,targetTurnsForwPPT, targetPulsesForwPPT
                    ,(char*) ANSI_COLOR_RESET);
            vDisplayMessage(App.Msg);
            */
                    
            if (handle_actuator_position_mode((void *) pActuator, 
                    targetTurnsForwPPT, targetPulsesForwPPT, speed_rpm, acc_rpms2_1, dec_rpms2_1, 
                    targetTurnsBackwPPT, targetPulsesBackwPPT, speed_rpm2, acc_rpms2_2, dec_rpms2_2, 
                    homingTurnsPPT, homingPulsesPPT, 
                    pActuator->homing_speed_rpm) < 0) {
                // return -1;
                if (generate_alarm((char*) "MODBUS : handle_actuator_position_mode() Error", 6100, 0, (int32_t) /*ALARM_FATAL_ERROR*/ALARM_WARNING, 0+1) < 0) {
                }
            }


            // Posizione neutrale corrente
            if (handle_actuator_prepare_for_run((void *) pActuator, AC_SERVO_NEUTRAL_POSITION_IN_TABLE) < 0) {
                if (generate_alarm((char*) "MODBUS : handle_actuator_prepare_for_run() Error", 6110, 0, (int32_t) /*ALARM_FATAL_ERROR*/ALARM_WARNING, 0+1) < 0) {
                }
            }
            
            
            // snprintf(App.Msg, App.MsgSize, "[%s %s handle_modbus_service_setup() %s]", (char*) ANSI_COLOR_GREEN, pActuator->name, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);

            //////////////////////////////
            // Lettura posizione
            //
            if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN) {
                hHigh = 0, bLow = 0;
                addr = MAKEWORD(bLow, hHigh);
                nb = 6;
            } else if (pActuator->protocol == MODBUS_AC_SERVO_DELTA) {
                hHigh = 5, bLow = 16;
                addr = MAKEWORD(bLow, hHigh);
                nb = 2;
            }

            if (modbus_read_registers((modbus_t*) pSerialSlot->modbus_ctx, addr, nb, (uint16_t*) pSerialSlot->data) < 0) {
                if (generate_alarm((char*) "modbus_read_registers() Error", 6102, 0, (int32_t) ALARM_FATAL_ERROR, 0+1) < 0) {
                }
            } else {
                // Passa il dato alla logica
                if (pActuator) {
                    if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN) {
                        // posizione
                        actuator_encoder_to_position((void *) pActuator, (int32_t) pSerialSlot->data[4+1], (int32_t) pSerialSlot->data[4+0], &newPosition);                        
                        actuator_handle_read_position((void *) pActuator, newPosition, true);
                        
                        // velocita                        
                        pActuator->speed = (float) pSerialSlot->data[0];

                    } else if (pActuator->protocol == MODBUS_AC_SERVO_DELTA) {
                        // posizione
                        int32_t pulses = 0, turns = 0;
                        memcpy(&pulses, &pSerialSlot->data[0], 4);
                        turns = pulses / (int32_t)pActuator->pulsesPerTurn;
                        pulses = pulses % (int32_t)pActuator->pulsesPerTurn;
                        actuator_encoder_to_position((void *) pActuator, (int32_t) turns, pulses, &newPosition);
                        actuator_handle_read_position((void *) pActuator, newPosition, true);
                        // velocita
                        pActuator->speed = (float) pSerialSlot->data[0];
                    }

                    // Asse pronto
                    if (pActuator->step == STEP_UNINITIALIZED || pActuator->step == STEP_ERROR || pActuator->step == STEP_STOPPED) {
                        pActuator->step = STEP_READY;
                    }

                    // posizione virtuale (0...1000)
                    update_actuator_virtual_pos(pActuator);
                    
                    pSerialSlot->setupDone = 1;

                    snprintf(App.Msg, App.MsgSize, "[%s station#%d Setup Done%s]", (char*) ANSI_COLOR_GREEN, pSerialSlot->stationId, (char*) ANSI_COLOR_RESET);
                    vDisplayMessage(App.Msg);

                } else {
                    if (generate_alarm((char*) "SER: Actuator not linked", 6103, 0, (int32_t) ALARM_FATAL_ERROR, 0+1) < 0) {
                    }
                }
            }
        }            
            
            
        pSerialSlot->start_time = xTaskGetTickCount();
        pSerialSlot->state = DEV_STATE_SERVICE_SETUP_III;
        

        return 1;
    }

    return 0;
}


int32_t handle_modbus_service_setup_III(void *pvSerialSlot) {
    if (pvSerialSlot) {
        SerialSlot *pSerialSlot = (SerialSlot *) pvSerialSlot;
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pSerialSlot->pActuator;

        if (xTaskGetTickCount() - pSerialSlot->start_time >= 500) {
            pSerialSlot->start_time = 0;
            pSerialSlot->state = DEV_STATE_SERVICE_SETUP_IV;
        }

        return 1;
    }

    return 0;
}


int32_t handle_modbus_service_setup_IV(void *pvSerialSlot) {
    if (pvSerialSlot) {
        SerialSlot *pSerialSlot = (SerialSlot *) pvSerialSlot;
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pSerialSlot->pActuator;


        pSerialSlot->setupRequest = 0;

        if (pActuator) {
           
            
            ////////////////////////////////////////
            // Avvio driver (comando Servo ON)
            //
            if (!pActuator->disabled) {
                if (handle_actuator_servo_on((void *) pActuator) < 0) {
                    // return -1;
                    if (generate_alarm((char*) "MODBUS : handle_actuator_servo_on() Error", 6101, 0, (int32_t) ALARM_FATAL_ERROR, 0+1) < 0) {
                    }
                }
            } else {
                if (handle_actuator_servo_off((void *) pActuator) < 0) {
                    // return -1;
                    if (generate_alarm((char*) "MODBUS : handle_actuator_servo_off() Error", 6101, 0, (int32_t) ALARM_FATAL_ERROR, 0+1) < 0) {
                    }
                }
                snprintf(App.Msg, App.MsgSize, "[%s AC Servo %s was disabled by app%s]", (char*) ANSI_COLOR_YELLOW, pActuator->name, (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);                
            }
            // Debug mode                        
            // modbus_set_debug((modbus_t*)((SerialSlot*)pActuator->pSerialSlot)->modbus_ctx, false);
        }

        
        ////////////////////////////////////////////////////////////////////////
        // N.B.: L'azzeramento dell'asse è fatto manualmente o dal recovery
        //
        if (!pActuator->homingDone) {
            // Esegue l'homing degli assi
            // pSerialSlot->state = DEV_STATE_HOMING_INIT;
        }
            
        // inizializa il flusso
        pSerialSlot->state = DEV_STATE_INIT_STREAM;
        

        return 1;
    }

    return 0;
}






int32_t handle_modbus_service_stop(void *pvSerialSlot) {
    if (pvSerialSlot) {
        SerialSlot *pSerialSlot = (SerialSlot *) pvSerialSlot;
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pSerialSlot->pActuator;

        pSerialSlot->stopRequest = 0;

        if (pActuator) {
            
            handle_actuator_servo_off((void *) pActuator);

            pActuator->step = STEP_STOPPED;
        
            pSerialSlot->state = DEV_STATE_SERVICE_OUT;
        }
        
        return 1;
    }

    return 0;
}




///////////////////////////////////////////////////
// Inizializza il comando di posizionamento
//

int32_t handle_modbus_cmd_init(void *pvSerialSlot) {
    if (pvSerialSlot) {
        SerialSlot *pSerialSlot = (SerialSlot *) pvSerialSlot;
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pSerialSlot->pActuator;


        if (!pSerialSlot->setupDone) {
            if (generate_alarm((char*) "handle_modbus_cmd_init:setup NOT completed!", 6014, 0, (int32_t) ALARM_FATAL_ERROR, 0+1) < 0) {
            }
            // reinizializa il flusso
            pSerialSlot->state = DEV_STATE_INIT;
            return -1;
        }

        if (pActuator) {
            float acc = 0.0f, dec = 0.0f, speed = 0.0f;

            if (pSerialSlot->pendingCommand == 1) {
                acc = pActuator->acc_auto1;
                dec = pActuator->dec_auto1;
                speed = pActuator->speed_auto1;
            } else if (pSerialSlot->pendingCommand == -1) {
                acc = pActuator->acc_auto2;
                dec = pActuator->dec_auto2;
                speed = pActuator->speed_auto2;
            } else {
            }

            // N.B.: L'aggiornamento della velocità avviene dall'e funzioni di I/O con l'intefaccia utente

            pActuator->readCounter = 0;
            pActuator->readPositionCounter = 0;


            pSerialSlot->doneCommand = 0;

            pSerialSlot->preTimeout = 0;
            pSerialSlot->start_time = xTaskGetTickCount();
            pSerialSlot->state = DEV_STATE_CMD_INIT_SEND;

            pSerialSlot->readCount = 0;
            pSerialSlot->streamErrorCount = 0;
            
            

        } else {
            if (generate_alarm((char*) "DEV_STATE_CMD_INIT:no actuator linked", 6014, 0, (int32_t) ALARM_FATAL_ERROR, 0+1) < 0) {
            }
            // reinizializa il flusso
            pSerialSlot->state = DEV_STATE_INIT;
            return -1;
        }


        return 1;
    }

    return 0;
}



int32_t handle_modbus_cmd_init_send(void *pvSerialSlot) {
    if (pvSerialSlot) {
        SerialSlot *pSerialSlot = (SerialSlot *) pvSerialSlot;
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pSerialSlot->pActuator;

        char str[256];
        char hHigh = 0, bLow = 0;
        int32_t addr = 0;
        int32_t nb = 0;
        int32_t res_send = -1;


        if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN) {


            // Non utilizzato poichè lento (NON Asincrono)
            // handle_actuator_run ();

            //////////////////////////////////////////////////////////////////////////////
            // PR 6.01 : DI logic level : assegnamento della posizione in tabella 
            //
            addr = MAKEWORD((uint8_t) 01, (uint8_t) HEX_TO_INT(2, 6));
            int32_t bit_pos = 0;

            // N.B: La tabella stabilisce quanti giri compiere, non la posizione di arrivo
            if (pSerialSlot->pendingCommand == 1) {
                bit_pos = modbus_get_bits_by_pable_pos( (void *)pActuator, AC_SERVO_FORWARD_POSITION_IN_TABLE );
            } else if (pSerialSlot->pendingCommand == -1) {
                bit_pos = modbus_get_bits_by_pable_pos( (void *)pActuator, AC_SERVO_BACKWARD_POSITION_IN_TABLE );
            } else {
                ////////////////////////////
                // Generazione Allarme                
                //
                snprintf(str, sizeof (str), "[SER#%d] CMD UNkNOWN", pSerialSlot->stationId);
                if (generate_alarm((char*) str, 6015, 0, (int32_t) ALARM_FATAL_ERROR, 0+1) < 0) {
                }
                // reinizializa il flusso
                pSerialSlot->state = DEV_STATE_INIT;
            }


            // Lunghezza risposta attesa
            pSerialSlot->waitingRespSize = 7;

            res_send = xrt_modbus_write_register_send((modbus_t*) pSerialSlot->modbus_ctx, addr, bit_pos);



        } else if (pActuator->protocol == MODBUS_AC_SERVO_DELTA) {

            // Non utilizzato poichè lento (NON Asincrono)
            // handle_actuator_run ();

            //////////////////////////////////////////////////////////////
            // PR 5.07 : Trigger Position Command (PR mode only)
            //
            addr = MAKEWORD((uint8_t) 07, (uint8_t) HEX_TO_INT(0, 5));
            int32_t bit_pos = 1;

            // N.B: La tabella stabilisce quanti giri compiere, non la posizione di arrivo
            if (pSerialSlot->pendingCommand == 1) {
                bit_pos = 1;
            } else if (pSerialSlot->pendingCommand == -1) {
                bit_pos = 2;
            } else {
                ////////////////////////////
                // Generazione Allarme                
                //
                snprintf(str, sizeof (str), "[SER#%d] CMD UNkNOWN", pSerialSlot->stationId);
                if (generate_alarm((char*) str, 6015, 0, (int32_t) ALARM_FATAL_ERROR, 0+1) < 0) {
                }
                // reinizializa il flusso
                pSerialSlot->state = DEV_STATE_INIT;
            }


            // Lunghezza risposta attesa
            pSerialSlot->waitingRespSize = 7;

            res_send = xrt_modbus_write_register_send((modbus_t*) pSerialSlot->modbus_ctx, addr, bit_pos);

        } else if (pActuator->protocol == MODBUS_AC_SERVO_DELTA) {

            // utilizzo della funzione sincrona handle_actuator_run ... (più lenta)
            handle_actuator_prepare_for_run(pActuator, 1);
        }
        
        


        if (res_send < 0) {
            pSerialSlot->streamErrorCount++;

            if (pSerialSlot->streamErrorCount > MAX_MODBUS_STREAM_ERROR) {
                ////////////////////////////
                // Generazione Allarme                
                //
                snprintf(str, sizeof (str), "[SER#%d] CMD INIT Error", pSerialSlot->stationId);
                if (generate_alarm((char*) str, 6005, 0, (int32_t) ALARM_FATAL_ERROR, 0+1) < 0) {
                }

            // reinizializa il flusso
            pSerialSlot->state = DEV_STATE_INIT;            
            return -1;
            }
                
        } else {
            pSerialSlot->streamErrorCount = 0;
            pSerialSlot->preTimeout = 0;
            pSerialSlot->start_time = xTaskGetTickCount();
            pSerialSlot->state = DEV_STATE_CMD_INIT_RECV;
            pSerialSlot->streamErrorCount = 0;
        }


        return 1;
    }

    return 0;
}



int32_t handle_modbus_cmd_init_recv(void *pvSerialSlot) {
    if (pvSerialSlot) {
        SerialSlot *pSerialSlot = (SerialSlot *) pvSerialSlot;
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pSerialSlot->pActuator;
        char str[256];

        int32_t avaiable_bytes = modbus_data_available((modbus_t*) pSerialSlot->modbus_ctx);
        if (avaiable_bytes >= pSerialSlot->waitingRespSize) {
            
            // snprintf(App.Msg, App.MsgSize, "[handle_modbus_cmd_init_recv: %d available]", avaiable_bytes); vDisplayMessage(App.Msg);
            
            // Dato disponibile sulla Seriale ...Lettura risposta ...
            if (xrt_modbus_write_register_recv((modbus_t*) pSerialSlot->modbus_ctx, pSerialSlot->data) < 0) {

                // handle_serial_borad_error(i, (char*) "recv", pSerialSlot->state);
                pSerialSlot->streamErrorCount++;

                if (pSerialSlot->streamErrorCount > MAX_MODBUS_STREAM_ERROR) {
                    //
                    // Errore lettura dato : Emergenza Macchina
                    /////////////////////////
                    // Generazione Allarme
                    //
                    snprintf(str, sizeof (str), "[SER#%d] CMD INIT Recv Error", pSerialSlot->stationId);
                    if (generate_alarm((char*) str, 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0+1) < 0) {
                    }
                    // reinizializa il flusso
                    pSerialSlot->state = DEV_STATE_INIT;
                }

            } else {

                // snprintf(App.Msg, App.MsgSize, "[%s board:%d - recived %dbytes :[%2x] - starting feedback read %s]\n", (char*) ANSI_COLOR_MAGENTA, pSerialSlot->boardId, (int32_t) avaiable_bytes, (int32_t) pSerialSlot->data[0], (char*) ANSI_COLOR_RESET);
                // vDisplayMessage(App.Msg);


                /////////////////////////////////////////////////////////////////////////////////////////////////////////////
                //
                //      Avvio Motore :
                //
                //      Attivazione Uscite ausiliarie con trigger discesa automatica : Partenza sul fronte di discesa
                //
                //      N.B.: Evita che l'eventuale calo di tensione o discontinuita' causi la partenza del motore
                //              su commutazione accidentali
                //
                if (actuator_set_aux_io((void *) pActuator, (int32_t) SSR_STATE_OFF_AND_ON, (int32_t) 0) < 0) {
                    if (generate_alarm((char*) "AUX DO no activated", 6019, 0, (int32_t) ALARM_FATAL_ERROR, 0+1) < 0) {
                    }
                } else {
                    // snprintf(App.Msg, App.MsgSize, "[%s board:%d - AUX DO activated %s]\n", (char*) ANSI_COLOR_MAGENTA, pSerialSlot->boardId, (char*) ANSI_COLOR_RESET);
                    // vDisplayMessage(App.Msg);
                    
                    // Stato di Comando in esecuzione
                    pSerialSlot->runningCommand = pSerialSlot->pendingCommand;
                    pSerialSlot->pendingCommand = 0;

                    pSerialSlot->streamErrorCount = 0;

                    // Ciclo lettura posizione
                    pSerialSlot->state = DEV_STATE_CMD_FEEDBACK_SEND;
                    
                }
            }

        } else {
            

            ///////////////////////////////////////////////////////////////////
            // Controllo del PreTimeput, rispedento in caso il messggio
            // if (modbus_check_pretimeout(pvSerialSlot) < 0) return -1;
            
            if (xTaskGetTickCount() - pSerialSlot->start_time > MODBUS_STREAM_TIMEOUT_MS) {
                /////////////////////////////
                // Comunicazione in timeout 
                //
                // Generazione Allarme
                //                    

                App.SERMeasurePostTimeout = xTaskGetTickCount();

                snprintf(str, sizeof (str), "[SER#%d] CMD Timeout", pSerialSlot->stationId);
                if (generate_alarm((char*) str, 6006, 0, (int32_t) ALARM_FATAL_ERROR, 0+1) < 0) {
                }
                // handle_serial_borad_error(i, (char*) "timeout!", -1);

                // reinizializa il flusso
                pSerialSlot->state = DEV_STATE_INIT;
            }
        }

        return 1;
    }

    return 0;
}

int32_t handle_modbus_cmd_feedback_send(void *pvSerialSlot) {
    if (pvSerialSlot) {
        SerialSlot *pSerialSlot = (SerialSlot *) pvSerialSlot;
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pSerialSlot->pActuator;

        char str[256];
        char hHigh = 0, bLow = 0;
        int32_t addr = 0;
        int32_t nb = 0;

        
        if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN) {
            hHigh = 0, bLow = 0;
            addr = MAKEWORD(bLow, hHigh);
            nb = 6;
        } else if (pActuator->protocol == MODBUS_AC_SERVO_DELTA) {
            hHigh = 5, bLow = 16;
            addr = MAKEWORD(bLow, hHigh);
            nb = 2;
        }


        // Lunghezza risposta attesa
        pSerialSlot->waitingRespSize = 3 + 2 * nb + 2;

        if (xrt_modbus_read_registers_send((modbus_t*) pSerialSlot->modbus_ctx, addr, nb) < 0) {

            pSerialSlot->streamErrorCount++;

            if (pSerialSlot->streamErrorCount > MAX_MODBUS_STREAM_ERROR) {
                //////////////////////////////
                // Generazione Allarme        
                //
                snprintf(str, sizeof (str), "[SER#%d] CMD FEEDBACK Send Error", pSerialSlot->stationId);
                if (generate_alarm((char*) str, 6007, 0, (int32_t) ALARM_FATAL_ERROR, 0+1) < 0) {
                }
            }
            
            // handle_serial_borad_error(i, (char*) "send", pSerialSlot->state);
            // pSerialSlot->streamErrorCount++;

        } else {

            // snprintf(App.Msg, App.MsgSize, "[handle_modbus_cmd_feedback_send: ok]"); vDisplayMessage(App.Msg);
            
            pSerialSlot->preTimeout = 0;
            pSerialSlot->streamErrorCount = 0;
            pSerialSlot->start_time = xTaskGetTickCount();
            pSerialSlot->state = DEV_STATE_CMD_FEEDBACK_RECV;
        }


        return 1;
    }

    return 0;
}



int32_t handle_modbus_cmd_feedback_recv(void *pvSerialSlot) {
    if (pvSerialSlot) {
        SerialSlot *pSerialSlot = (SerialSlot *) pvSerialSlot;
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pSerialSlot->pActuator;

        char str[256];
        float newPosition = 0.0;

        char hHigh = 0, bLow = 0;
        int32_t addr = 0;
        int32_t nb = 0;
        int32_t res = 0;


        int32_t avaiable_bytes = modbus_data_available((modbus_t*) pSerialSlot->modbus_ctx);
        if (avaiable_bytes >= pSerialSlot->waitingRespSize) {

            // snprintf(App.Msg, App.MsgSize, "[handle_modbus_cmd_feedback_recv:%d data available]", avaiable_bytes); vDisplayMessage(App.Msg);
            
            /////////////////////////////////////////////////////////////////
            // Dato disponibile sulla Seriale ...Lettura risposta ...
            //
            if ((res = xrt_modbus_read_registers_recv((modbus_t*) pSerialSlot->modbus_ctx, &pSerialSlot->data[0])) < 0) {
                
                pSerialSlot->streamErrorCount++;

                if (pSerialSlot->streamErrorCount > MAX_MODBUS_STREAM_ERROR) {
                    //////////////////////////////
                    // Generazione Allarme        
                    //
                    snprintf(str, sizeof (str), "[SER#%d] CMD FEEDBACK Send Error", pSerialSlot->stationId);
                    if (generate_alarm((char*) str, 6008, 0, (int32_t) ALARM_FATAL_ERROR, 0+1) < 0) {
                    }
                    // handle_serial_borad_error(i, (char*) "recv", pSerialSlot->state);
                    // pSerialSlot->streamErrorCount++;

                    // reinizializa il flusso
                    pSerialSlot->state = DEV_STATE_INIT;
                
                } else {
                    avaiable_bytes = modbus_data_available((modbus_t*) pSerialSlot->modbus_ctx);
                    if (avaiable_bytes <= 0) {
                        snprintf(App.Msg, App.MsgSize, "[xrt_modbus_read_registers_recv failed and no more data! error:%d]", res);
                        vDisplayMessage(App.Msg);
                    } else {                        
                        snprintf(App.Msg, App.MsgSize, "[xrt_modbus_read_registers_recv failed...repeatingerror:%d]", res);
                        vDisplayMessage(App.Msg);
                    }
                }
                
            } else {


               
                handle_force_process_read_data:

                ////////////////////////////////
                // Dati eccedenti ?
                //
                if (avaiable_bytes > pSerialSlot->waitingRespSize) {
                    snprintf((char*) str, sizeof (str), (char*) "%s>%d%s", (char*) ANSI_COLOR_CYAN, (int32_t) avaiable_bytes > pSerialSlot->waitingRespSize, (char*) ANSI_COLOR_RESET); vDisplayMessage(str);
                } else {
                }
            
                int8_t *lastRes = xrt_modbus_get_last_res((modbus_t*) pSerialSlot->modbus_ctx);
                
                if (lastRes[0] == pActuator->stationId && lastRes[1] == 0x3 && lastRes[2] == 0xC) {
                               
                    ///////////////////////////////////////////////////////////////////////
                    //
                    // N.B.:    Nel driver la posizione va da 0..65535 pulses e 0..65535 turns
                    //          Nella logica da da 0..65535 pulses e -32765..+32765 turns 
                    //
                    int32_t driverPosition = (int32_t) ((uint16_t)pSerialSlot->data[4 + 0]) + (int32_t) ((int16_t)pSerialSlot->data[4 + 1]) * (int32_t)pActuator->pulsesOverflow;
                    int32_t deltaPos = abs ( driverPosition - pSerialSlot->driverPosition  );


                    // snprintf((char*) str, sizeof (str), (char*) "%s(%d)%s" , (char*) ANSI_COLOR_MAGENTA , (int32_t) (pSerialSlot->data[4 + 0]), (char*) ANSI_COLOR_RESET ); vDisplayMessage(str);


                    /*
                    if (pSerialSlot->data[4 + 0] == 0xFFFF && pSerialSlot->data[4 + 1] == 0xFFFF) {
                        // snprintf((char*) str, sizeof (str), (char*) "%s[SER#%d ]%s" , (char*) ANSI_COLOR_RED , pSerialSlot->stationId , (char*) ANSI_COLOR_RESET ); vDisplayMessage(str);
                        snprintf((char*) str, sizeof (str), (char*) "%s[?]%s" , (char*) ANSI_COLOR_RED , (char*) ANSI_COLOR_RESET ); vDisplayMessage(str);
                        pSerialSlot->state = DEV_STATE_CMD_FEEDBACK_SEND;
                        pSerialSlot->streamErrorCount = 0;
                    } else */
                    
                    // Controllo variazione eccessiava (problema comunicazione (?)
                    if ( deltaPos > MAX_MODVAR_PULSES_VAR ) {

                        snprintf((char*) str, sizeof (str), (char*) "%s[SER#%d Discharged data - Delta:%d/%dpulses -  Dirver pos:%d/%d (%d.%d)]%s"
                                , (char*) ANSI_COLOR_RED
                                , (int32_t)pSerialSlot->stationId
                                , (int32_t)deltaPos, (int32_t)MAX_MODVAR_PULSES_VAR
                                , (int32_t)pSerialSlot->driverPosition, (int32_t)driverPosition, (int32_t)((int16_t)pSerialSlot->data[4 + 1]), (int32_t)((uint16_t)pSerialSlot->data[4 + 0])
                                , (char*) ANSI_COLOR_RESET
                                );
                        vDisplayMessage(str);

                        pSerialSlot->state = DEV_STATE_CMD_FEEDBACK_SEND;
                        pSerialSlot->streamErrorCount = 0;

                    } else {

                        ///////////////////////////////////////////////////////////////////////
                        //
                        // N.B.:    Nel driver la posizione va da 0..65535 pulses e 0..65535 turns
                        //          Nella logica da da 0..65535 pulses e -32765..+32765 turns 
                        //
                        pSerialSlot->driverTurns = (int32_t) ((int16_t)pSerialSlot->data[4 + 1]);
                        pSerialSlot->driverPulses = (int32_t) ((uint16_t)pSerialSlot->data[4 + 0]);
                        pSerialSlot->driverPosition = (int32_t) pSerialSlot->driverPulses + (int32_t) pSerialSlot->driverTurns * (int32_t)pActuator->pulsesOverflow;
                        pSerialSlot->driverSpeed = (int32_t) ((int16_t) pSerialSlot->data[0]);
                        pSerialSlot->driverAcc = (int32_t) MAKEDWORD(0, 0);

                        pSerialSlot->readCount++;


                        // snprintf(App.Msg, App.MsgSize, "[%s board:%d - feedback read pos:%d %s]\n", (char*) ANSI_COLOR_MAGENTA, pSerialSlot->boardId, (int32_t)pSerialSlot->driverPosition, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);




                        // Passa il dato alla logica
                        if (pActuator) {
                            bool positionReached = false;

                            pActuator->driverTurns = pSerialSlot->driverTurns;
                            pActuator->driverPulses = pSerialSlot->driverPulses;
                            pSerialSlot->driverPosition = pSerialSlot->driverPosition;

                            actuator_encoder_to_position((void *) pActuator, (int32_t) pSerialSlot->driverTurns, (int32_t) pSerialSlot->driverPulses, &newPosition);

                            if (fabs(pActuator->cur_rpos - newPosition) > 0.01) {
                                if (!pActuator->start_time1)
                                    pActuator->start_time1 = xTaskGetTickCount();
                            }

                            // velocita
                            pActuator->speed = (float) pSerialSlot->driverSpeed;


                            if (pActuator->speed > pActuator->max_speed)
                                pActuator->max_speed = pActuator->speed;
                            if (pActuator->speed < pActuator->min_speed)
                                pActuator->min_speed = pActuator->speed;

                            // posizione reale
                            pActuator->cur_rpos = newPosition;
                            pActuator->readCounter++;
                            pActuator->readPositionCounter++; // conteggio letture posizione nell'ambito del movimento


                            // posizione virtuale (0...1000)
                            update_actuator_virtual_pos(pActuator);


                            ///////////////////////////////
                            // Posizione raggiunta ?
                            //
                            if (pActuator->positionReached) {
                                //////////////////////////
                                // Termina la sequenza
                                // Il gestore degli attuattori analizza la poszione e processa il nuovo stato dell'attuatore
                                //

                                // Arrotonda la velocità letta
                                pActuator->speed = 0.0f;

                                if (pSerialSlot->runningCommand) {
                                    pSerialSlot->doneCommand = pSerialSlot->runningCommand;
                                    pSerialSlot->runningCommand = 0;
                                } else {
                                }

                                // posizione raggiunta fine sequenza
                                pSerialSlot->state = DEV_STATE_CMD_DONE;

                            } else {
                                if (pSerialSlot->stopRequest) {
                                    pSerialSlot->state = DEV_STATE_SERVICE_STOP;
                                } else {
                                    // Ripete la lettura
                                    pSerialSlot->state = DEV_STATE_CMD_FEEDBACK_SEND;
                                    pSerialSlot->streamErrorCount = 0;

                                }
                            }

                            /*
                            // Ancora dati nel buffer ?
                            int32_t available_data;
                            if ((available_data = modbus_data_available((modbus_t*) pSerialSlot->modbus_ctx)) > 0) {
                                snprintf(App.Msg, App.MsgSize, "[%d bytes pending]", (int32_t) available_data);
                                vDisplayMessage(App.Msg);
                            }
                            */

                        } else {
                            // nessun attuattore collegato : fine lettura
                            snprintf(App.Msg, App.MsgSize, "[%s No actuator linked...exit seq %s]\n", (char*) ANSI_COLOR_YELLOW, (char*) ANSI_COLOR_RESET);
                            vDisplayMessage(App.Msg);
                            pSerialSlot->state = DEV_STATE_STREAMING_DONE;
                        }
                    }
                } else {
                    snprintf(App.Msg, App.MsgSize, "[%sUnaspected response : %1x.%1x.%1x%s]\n", (char*) ANSI_COLOR_RED, lastRes[0], lastRes[1], lastRes[2], (char*) ANSI_COLOR_RESET);
                    vDisplayMessage(App.Msg);                    
                }
            }
            
        } else {
            
            // Controllo il PreTimeput, rispedendo in caso il messggio
            // if (modbus_check_pretimeout(pvSerialSlot) < 0) return -1;

            if (xTaskGetTickCount() - pSerialSlot->start_time > MODBUS_STREAM_TIMEOUT_MS) {
                /////////////////////////////
                // Comunicazione in timeout 
                //
                // Generazione Allarme
                //

                App.SERMeasurePostTimeout = xTaskGetTickCount();

                snprintf((char*) str, sizeof (str), (char*) "[SER#%d] CMD FEEDBACK Recv timeout (%d) - Available data:%d/%d"
                        , pSerialSlot->stationId
                        , (xTaskGetTickCount() - pSerialSlot->start_time)
                        , modbus_data_available((modbus_t*) pSerialSlot->modbus_ctx) , pSerialSlot->waitingRespSize);

                if (generate_alarm((char*) str, 6009, 0, (int32_t) ALARM_FATAL_ERROR, 0+1) < 0) {
                }
                // handle_serial_borad_error(i, (char*) "timeout!", -1);
                
                // reinizializa il flusso
                // pSerialSlot->state = DEV_STATE_INIT;
            }
        }





        //////////////////////////////////////////
        // Controllo del timeout del movimento
        //
        if (pSerialSlot->runningCommand) {
            if (pActuator) {
                uint timeout_ms = 0;
                float target_pos = 0.0f;
                char str[256];

                // Controllo stato dell'attuatore
                if (pActuator->step == STEP_DONE || pActuator->step == STEP_ERROR || pActuator->step == STEP_STOPPED) {
                } else {

                    if (pSerialSlot->runningCommand == 1) {
                        timeout_ms = pActuator->timeout1_ms;
                        target_pos = pActuator->end_rpos;
                    } else if (pSerialSlot->runningCommand == -1) {
                        timeout_ms = pActuator->timeout2_ms;
                        target_pos = pActuator->start_rpos;
                    }

                    if (timeout_ms > 0) {

                        if (xTaskGetTickCount() - pActuator->start_time > timeout_ms) {

                            if (!(pActuator->rtOptions & 2)) {
                                pActuator->rtOptions += 2;
                            }
                            snprintf(str, sizeof (str), "Actuator %s timeout (time:%0.3fsec - step:%s) - side:%d - Position:%0.3f/%0.3f - Err:%0.3f - DriverPos.:%d-%d.%d]"
                                    , pActuator->name
                                    , (float) (xTaskGetTickCount() - pActuator->start_time) / 1000.0f
                                    , get_actuator_step((void *)pActuator)
                                    , pSerialSlot->runningCommand
                                    , pActuator->cur_rpos, target_pos, (target_pos - pActuator->cur_rpos)
                                    , pSerialSlot->driverPosition, pSerialSlot->driverTurns, pSerialSlot->driverPulses
                                    );
                            if (generate_alarm((char*) str, 6011, 0, (int32_t) ALARM_ERROR, 0+1) < 0) {
                            }


                            //////////////////////////////
                            // Lettura posizione
                            //
                            if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN) {
                                hHigh = 0, bLow = 0;
                                addr = MAKEWORD(bLow, hHigh);
                                nb = 6;
                            } else if (pActuator->protocol == MODBUS_AC_SERVO_DELTA) {
                                hHigh = 5, bLow = 16;
                                addr = MAKEWORD(bLow, hHigh);
                                nb = 2;
                            }

                            if (modbus_read_registers((modbus_t*) pSerialSlot->modbus_ctx, addr, nb, (uint16_t*) pSerialSlot->data) < 0) {
                                if (generate_alarm((char*) "PostTimeout : modbus_read_registers() Error", 6102, 0, (int32_t) ALARM_WARNING, 0+1) < 0) {
                                }

                            } else {

                                ///////////////////////////////////////////////////////////////////////
                                //
                                // N.B.:    Nel driver la posizione va da 0..65535 pulses e 0..65535 turns
                                //          Nella logica da da 0..65535 pulses e -32765..+32765 turns 
                                //
                                int32_t driverPosition = (int32_t) ((uint16_t)pSerialSlot->data[4 + 0]) + ((int32_t) ((int16_t)pSerialSlot->data[4 + 1])) * (int32_t)pActuator->pulsesOverflow;
                                snprintf(App.Msg, App.MsgSize, "PostTimeout info : modbus_read_registers() driverPosition:%d-%d.%d", driverPosition, pSerialSlot->data[4 + 1], pSerialSlot->data[4 + 0]);
                                if (generate_alarm((char*) App.Msg, 6102, 0, (int32_t) ALARM_WARNING, 0+1) < 0) {
                                }

                            }


                            // reinizializa il flusso
                            pSerialSlot->state = DEV_STATE_INIT_STREAM;
                        }
                    }
                }
            }
        }



        return 1;
    }

    return 0;
}


int32_t handle_modbus_cmd_done(void *pvSerialSlot) {
    if (pvSerialSlot) {
        SerialSlot *pSerialSlot = (SerialSlot *) pvSerialSlot;
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pSerialSlot->pActuator;

        if (pActuator) {
            uint timewarn_ms = 0;
            char str[256];

            
            // Reset dati sequenza IDLE
            pActuator->IDLECounter = 0;
            pActuator->IDLESeq = 0;
            
            ///////////////////////////////
            // Verifica tempo esecuzione
            //
            if (pSerialSlot->runningCommand == 1) {
                timewarn_ms = pActuator->timewarn1_ms;
            } else if (pSerialSlot->runningCommand == -1) {
                timewarn_ms = pActuator->timewarn2_ms;
            }
            if (timewarn_ms > 0) {
                if (xTaskGetTickCount() - pActuator->start_time > timewarn_ms) {
                    if (!(pActuator->rtOptions & 1)) {
                        pActuator->rtOptions += 1;
                        snprintf(str, sizeof (str), "Actuator %s time warning (side:%d time:%0.3fsec)", pActuator->name, pSerialSlot->runningCommand, (float) timewarn_ms / 1000.0f);
                        if (generate_alarm((char*) str, 6010, 0, (int32_t) ALARM_WARNING, 0+1) < 0) {
                        }
                    }
                }
            }


            //////////////////////////////////////////////////////////////////////////////
            // Assegna la posizione neuta in tabella per sicurezza :
            // In caso di caduta di tensione il relè invia il comando al driver
            //
            //
            char hHigh = 0, bLow = 0;
            int32_t addr = 0;
            int32_t nb = 0;
            int32_t res_send = -1;
            int32_t bit_pos = bit_pos = 1 + (0 * 16 + 1 * 32 + 0 * 64 + 0 * 128) + 256;

            // PR 6.01 : DI logic level : assegnamento della posizione in tabella 
            addr = MAKEWORD((uint8_t) 01, (uint8_t) HEX_TO_INT(2, 6));

            
            // Lunghezza risposta attesa
            pSerialSlot->waitingRespSize = 7;

            
            if (xrt_modbus_write_register_send((modbus_t*) pSerialSlot->modbus_ctx, addr, bit_pos) < 0) {
                snprintf(str, sizeof (str), "Actuator %s time warning (side:%d time:%0.3fsec)", pActuator->name, pSerialSlot->runningCommand, (float) timewarn_ms / 1000.0f);
                if (generate_alarm((char*) str, 6010, 0, (int32_t) ALARM_WARNING, 0+1) < 0) {
                }            
            }


            ///////////////////////////////////////////////////////
            // Prepara l'uscita ausiliaria al fronte di discesa
            //
            // actuator_set_aux_io((void *) pActuator, (int32_t) SSR_STATE_ON, (int32_t) 0);

        }
        
        
        //////////////////////////////////////////////////
        // Legge alcune volte la posizione sul ciclo IDLE
        //
        pSerialSlot->ReadSinglePos = 200;
                
                

        // reinizializza : il purge non dovrebbe trovare dati
        pSerialSlot->state = DEV_STATE_CMD_DONE_FEEDBACK;
        // pSerialSlot->state = DEV_STATE_INIT_STREAM;

        return 1;
    }

    return 0;
}






int32_t handle_modbus_cmd_done_feedback (void *pvSerialSlot) {
    if (pvSerialSlot) {
        SerialSlot *pSerialSlot = (SerialSlot *) pvSerialSlot;
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pSerialSlot->pActuator;
        int32_t res = 0;

        if (pActuator) {
        }        
        
        // N.B.: il purge non dovrebbe trovare dati
        int32_t avaiable_bytes = modbus_data_available((modbus_t*) pSerialSlot->modbus_ctx);
        if (avaiable_bytes > 0) {
            while (modbus_data_available((modbus_t*) pSerialSlot->modbus_ctx) > 0) {
                /////////////////////////////////////////////////////////////////
                // Dato disponibile sulla Seriale ...Lettura risposta ...
                //
                if ((res = xrt_modbus_read_registers_recv((modbus_t*) pSerialSlot->modbus_ctx, &pSerialSlot->data[0])) < 0) {
                    // reinizializa il flusso
                    // pSerialSlot->state = DEV_STATE_INIT;
                    // return -1;                
                    break;
                } else {
                }
            }
            snprintf(App.Msg, App.MsgSize, "[%s Purged %d bytes from serial #%d %s]\n", (char*) ANSI_COLOR_RED, avaiable_bytes, pSerialSlot->boardId, (char*) ANSI_COLOR_RESET);
            vDisplayMessage(App.Msg);            
        }

        // reinizializza il flusso
        pSerialSlot->state = DEV_STATE_INIT_STREAM;

        return 1;
    }
    
    return 0;
}




int32_t handle_modbus_service_setup_speed_acc(void *pvSerialSlot) {
    if (pvSerialSlot) {
        SerialSlot *pSerialSlot = (SerialSlot *) pvSerialSlot;
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pSerialSlot->pActuator;

        int32_t Error = 0;
        int32_t speed_rpm = (int32_t) pActuator->speed_auto1 > 0 ? (int32_t) pActuator->speed_auto1 : 1;
        int32_t acc_rpms2_1 = pActuator->acc_auto1 >= 0.0f ? (int32_t) pActuator->acc_auto1 : 0;
        int32_t dec_rpms2_1 = pActuator->dec_auto1 >= 0.0f ? (int32_t) pActuator->dec_auto1 : 0;
        int32_t speed_rpm2 = (int32_t) pActuator->speed_auto2 > 0 ? (int32_t) pActuator->speed_auto2 : 1;
        int32_t acc_rpms2_2 = pActuator->acc_auto2 >= 0.0f ? (int32_t) pActuator->acc_auto2 : 0;
        int32_t dec_rpms2_2 = pActuator->dec_auto2 >= 0.0f ? (int32_t) pActuator->dec_auto2 : 0;


        pSerialSlot->preTimeout = 0;
        pSerialSlot->start_time = xTaskGetTickCount();

        if (pSerialSlot->paramToUpdate[SPEED_1]) {

            if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN) {
                if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 2), 02, speed_rpm, true, 2, App.DebugModbusParam) < 0) {
                    Error++;
                }
            } else if (pActuator->protocol == MODBUS_AC_SERVO_DELTA) {
                // Target speed: P5.60 ~ P5-75 (Moving Speed Setting of Position 0 ~ 15), total 16 groups
                if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, 5, 60, speed_rpm, true, 2, App.DebugModbusParam) < 0) {
                    Error++;
                }
            }

            pSerialSlot->paramToUpdate[SPEED_1] = 0;
            snprintf(App.Msg, App.MsgSize, "[%s update %s PR(x.y) { speed1:%d } [%dmsec] Err:%d %s]\n", (char*) ANSI_COLOR_GREEN, pActuator->name, speed_rpm, (xTaskGetTickCount() - pSerialSlot->start_time), Error, (char*) ANSI_COLOR_RESET);
            vDisplayMessage(App.Msg);
        }


        if (pSerialSlot->paramToUpdate[ACC_1] || pSerialSlot->paramToUpdate[ACC_2]) {
            if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN) {
                int32_t acc_ms = (int32_t) ((3000.0f - 0.0f) / 9.55f / acc_rpms2_1 * 1000.0f);
                if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 1), 39, acc_ms, true, 2, App.DebugModbusParam) < 0) {
                    Error++;
                }
            } else if (pActuator->protocol == MODBUS_AC_SERVO_DELTA) {
                if (pSerialSlot->paramToUpdate[ACC_1]) {
                    // Accel / Decel time: P5.20 ~ P5-35 (Accel / Decel Time 0 ~ 15), total 16 parameters
                    int32_t acc_ms = (int32_t) ((3000.0f - 0.0f) / 9.55f / acc_rpms2_1 * 1000.0f);
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, 5, 20, acc_ms, true, 2, App.DebugModbusParam) < 0) {
                        Error++;
                    }
                }
                if (pSerialSlot->paramToUpdate[ACC_2]) {
                    // Accel / Decel time: P5.20 ~ P5-35 (Accel / Decel Time 0 ~ 15), total 16 parameters
                    int32_t dec_ms = (int32_t) ((3000.0f - 0.0f) / 9.55f / dec_rpms2_1 * 1000.0f);
                    if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, 5, 21, dec_ms, true, 2, App.DebugModbusParam) < 0) {
                        Error++;
                    }
                }
            }

            pSerialSlot->paramToUpdate[ACC_1] = 0;
            pSerialSlot->paramToUpdate[ACC_2] = 0;
            snprintf(App.Msg, App.MsgSize, "[%s update %s PR(x.y) { acc:%d } [%dmsec] Err:%d %s]\n", (char*) ANSI_COLOR_GREEN, pActuator->name, acc_rpms2_1, (xTaskGetTickCount() - pSerialSlot->start_time), Error, (char*) ANSI_COLOR_RESET);
            vDisplayMessage(App.Msg);
        }


        if (pSerialSlot->paramToUpdate[SPEED_2]) {
            if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN) {
                if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 2), 06, speed_rpm2, true, 2, App.DebugModbusParam) < 0) {
                    Error++;
                }
            } else if (pActuator->protocol == MODBUS_AC_SERVO_DELTA) {
                // Target speed: P5.60 ~ P5-75 (Moving Speed Setting of Position 0 ~ 15), total 16 groups
                if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, 5, 61, speed_rpm2, true, 2, App.DebugModbusParam) < 0) {
                    Error++;
                }
            }
            pSerialSlot->paramToUpdate[SPEED_2] = 0;
            snprintf(App.Msg, App.MsgSize, "[%s update %s PR(x.y) { speed2:%d } [%dmsec] Err:%d %s]\n", (char*) ANSI_COLOR_GREEN, pActuator->name, speed_rpm2, (xTaskGetTickCount() - pSerialSlot->start_time), Error, (char*) ANSI_COLOR_RESET);
            vDisplayMessage(App.Msg);
        }


        if (pSerialSlot->paramToUpdate[FERR_1] || pSerialSlot->paramToUpdate[FERR_2]) {
            // Pr1.16 
            int32_t followErr = 0;
            
            if (pSerialSlot->paramToUpdate[FERR_1]) {
                followErr = (int32_t)(pActuator->follow_error1 * (float)pActuator->pulsesPerTurn);
                pActuator->follow_error2 = pActuator->follow_error1;
            } else if (pSerialSlot->paramToUpdate[FERR_2]) {
                followErr = (int32_t)(pActuator->follow_error2 * (float)pActuator->pulsesPerTurn);
                pActuator->follow_error1 = pActuator->follow_error2;
            }
            if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN) {
                if (modbus_param_update((modbus_t*) pSerialSlot->modbus_ctx, HEX_TO_INT(2, 1), 16, followErr, true, 2, App.DebugModbusParam) < 0) {
                    Error++;
                }
            } else if (pActuator->protocol == MODBUS_AC_SERVO_DELTA) {
            }
            pSerialSlot->paramToUpdate[FERR_1] = 0;
            pSerialSlot->paramToUpdate[FERR_2] = 0;
            snprintf(App.Msg, App.MsgSize, "[%s update %s PR(x.y) { speed2:%d } [%dmsec] Err:%d %s]\n", (char*) ANSI_COLOR_GREEN, pActuator->name, speed_rpm2, (xTaskGetTickCount() - pSerialSlot->start_time), Error, (char*) ANSI_COLOR_RESET);
            vDisplayMessage(App.Msg);
        }
        
        
        // N.B.: Per modificare il parametro acc di ritorno è necessario cambiarla la volo



        // fine lettura
        pSerialSlot->state = DEV_STATE_STREAMING_DONE;

        return 1;
    }

    return 0;
}

int32_t handle_modbus_streaming_done(void *pvSerialSlot) {
    if (pvSerialSlot) {
        SerialSlot *pSerialSlot = (SerialSlot *) pvSerialSlot;
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pSerialSlot->pActuator;
        uint32_t t1 = xTaskGetTickCount();

        if (t1 - pSerialSlot->tStat >= 5000) {
            ////////////////////////////////////
            // Debug : stampa ad ogni Intervallo
            //
            if (pSerialSlot->state >= 0) {
                if (pSerialSlot->state >= 10 && pSerialSlot->state <= 900) {
                    GLSERDataPerSec = (uint32_t) ((float) pSerialSlot->readCount * 1000.0 / (float) (t1 - pSerialSlot->tStat));

                    // Statistiche IO/sec
                    if (GLSERDataPerSec > GLSERMaxDataPerSec)
                        GLSERMaxDataPerSec = GLSERDataPerSec;
                    if (GLSERDataPerSec < GLSERMinDataPerSec && GLSERDataPerSec > 0)
                        GLSERMinDataPerSec = GLSERDataPerSec;

                    GLSERLastDataPerSec = GLSERDataPerSec;

                    if (machine.status != AUTOMATIC) {
                        int8_t *lastReq = xrt_modbus_get_last_req((modbus_t*) pSerialSlot->modbus_ctx);
                        int8_t *lastRes = xrt_modbus_get_last_res((modbus_t*) pSerialSlot->modbus_ctx);

                        fprintf(stdout, "[%s SER#%d- %d.%dpulses - %drpm {%1x.%1x.%1x.%1x.%1x.%1x.%1x.%1x|%1x.%1x.%1x.%1x.%1x.%1x.%1x.%1x}{%d.%d.%d.%d.%d.%d.%d.%d} [Rate:%0.f/s][Err:%d]%s]\n", (char*) ANSI_COLOR_YELLOW,
                                pSerialSlot->stationId,
                                (int32_t) pSerialSlot->driverTurns,
                                (int32_t) pSerialSlot->driverPulses,
                                (int32_t) pSerialSlot->driverSpeed,
                                (int32_t) lastRes[0], (int32_t) lastRes[1], (int32_t) lastRes[2], (int32_t) lastRes[3], (int32_t) lastRes[4], (int32_t) lastRes[5], (int32_t) lastRes[6], (int32_t) lastRes[7],
                                (int32_t) lastRes[8], (int32_t) lastRes[9], (int32_t) lastRes[10], (int32_t) lastRes[11], (int32_t) lastRes[12], (int32_t) lastRes[13], (int32_t) lastRes[14], (int32_t) lastRes[15],
                                (int32_t) pSerialSlot->data[0], (int32_t) pSerialSlot->data[1], (int32_t) pSerialSlot->data[2], (int32_t) pSerialSlot->data[3], (int32_t) pSerialSlot->data[4], (int32_t) pSerialSlot->data[5], (int32_t) pSerialSlot->data[6], (int32_t) pSerialSlot->data[7],
                                // (int32_t) pSerialSlot->data[8], (int32_t) pSerialSlot->data[9], (int32_t) pSerialSlot->data[10], (int32_t) pSerialSlot->data[11], (int32_t) pSerialSlot->data[12], (int32_t) pSerialSlot->data[13], (int32_t) pSerialSlot->data[14], (int32_t) pSerialSlot->data[15],
                                // (int32_t) lastReq[0], (int32_t) lastReq[1], (int32_t) lastReq[2], (int32_t) lastReq[3], (int32_t) lastReq[4], (int32_t) lastReq[5], (int32_t) lastReq[6], (int32_t) lastReq[7], //                             
                                (float) pSerialSlot->readCount / 5.0f, pSerialSlot->streamErrorCount,
                                (char*) ANSI_COLOR_RESET);
                        fflush(stdout);
                    }

                    //////////////////////////////////////
                    // DEBUG Lettura sincrona posizione
                    //
                    /*
                    char hHigh = 0, bLow = 0;
                    int32_t addr = 0;
                    int32_t nb = 0;
                    int32_t res = 0;

                    
                    if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN) {
                        hHigh = 0, bLow = 0;
                        addr = MAKEWORD(bLow, hHigh);
                        nb = 6;
                    } else if (pActuator->protocol == MODBUS_AC_SERVO_DELTA) {
                        hHigh = 5, bLow = 16;
                        addr = MAKEWORD(bLow, hHigh);
                        nb = 2;
                    }

                    if (modbus_read_registers((modbus_t*) pSerialSlot->modbus_ctx, addr, nb, (uint16_t*) pSerialSlot->data) < 0) {
                    } else {
                        lastReq = xrt_modbus_get_last_req((modbus_t*) pSerialSlot->modbus_ctx);
                        lastRes = xrt_modbus_get_last_res((modbus_t*) pSerialSlot->modbus_ctx);
                        int32_t driverPosition = (int32_t) pSerialSlot->data[4 + 0] + (int32_t) pSerialSlot->data[4 + 1] * (int32_t)pActuator->pulsesOverflow;
                        snprintf(App.Msg, App.MsgSize, "[DEBUG :driverPosition:%d-%d.%d - {%1x.%1x.%1x.%1x.%1x.%1x.%1x.%1x|%1x.%1x.%1x.%1x.%1x.%1x.%1x.%1x}]"
                            ,driverPosition, pSerialSlot->data[4 + 1], pSerialSlot->data[4 + 0]
                            ,(int32_t) lastRes[0], (int32_t) lastRes[1], (int32_t) lastRes[2], (int32_t) lastRes[3], (int32_t) lastRes[4], (int32_t) lastRes[5], (int32_t) lastRes[6], (int32_t) lastRes[7]
                            ,(int32_t) lastRes[8], (int32_t) lastRes[9], (int32_t) lastRes[10], (int32_t) lastRes[11], (int32_t) lastRes[12], (int32_t) lastRes[13], (int32_t) lastRes[14], (int32_t) lastRes[15]
                            );
                        fprintf(stdout, "%s", App.Msg);
                    }
                    */
                }
            }

            pSerialSlot->readCount = 0;
            pSerialSlot->streamErrorCount = 0;
            pSerialSlot->tStat = xTaskGetTickCount();            
            
            if (pActuator) {
                pActuator->readCounter = 0;
                pActuator->readPositionCounter = 0;
            }
        }



        //////////////////////////////////////////
        // Esecuzione Comando di test / debug      
        //
        if (GLTestSerial == 1) {

            GLTestSerial = 0;
            
            if (machine.status != AUTOMATIC) {
                handle_modbus_service_reset( (void*)pSerialSlot);
            }
            
            dump_ac_servo_params(pSerialSlot);

        } else if (GLTestSerial == 2) {

            GLTestSerial = 0;

            pSerialSlot->testCommand++;
            
            snprintf(App.Msg, App.MsgSize, "[POSITION_IN_TABLE:%d]", pSerialSlot->testCommand);
            vDisplayMessage(App.Msg);

            
            if (handle_actuator_prepare_for_run((void *) pActuator, pSerialSlot->testCommand) < 0) {
            }
            
            usleep(15*1000);
            
            if (actuator_set_aux_io((void *) pActuator, (int32_t) SSR_STATE_OFF_AND_ON, (int32_t) 0) < 0) {
            }           

            if (pSerialSlot->testCommand>=8) 
                pSerialSlot->testCommand = 0;
            
        } else if (GLTestSerial == 3) { // reset
            GLTestSerial = 0;
        } else if (GLTestSerial == 4) { // stop motor
            GLTestSerial = 0;
        } else if (GLTestSerial == 5) { // reset position table
            GLTestSerial = 0;
        } else if (GLTestSerial == 6) { // standby motor
            GLTestSerial = 0;
        
        } else if (GLTestSerial == 7) {
            GLTestSerial = 0;
            
            
        } else if (GLTestSerial == 8) {
            GLTestSerial = 0;
            
            // Test esecuzione comando
            if (pSerialSlot->testCommand == 0) pSerialSlot->testCommand = 1;
            else pSerialSlot->testCommand = pSerialSlot->testCommand * -1;
            
            handle_actuator_prepare_for_run (pvSerialSlot, pSerialSlot->testCommand==1?1:2);
            
            if (actuator_set_aux_io((void *) pActuator, (int32_t) SSR_STATE_OFF_AND_ON, (int32_t) 0) > 0) {
            }            
        }


        ///////////////////////////////////////////////
        // Richieste in pendenza
        //
        if (pSerialSlot->stopRequest) {
            pSerialSlot->state = DEV_STATE_SERVICE_STOP;
            // snprintf(App.Msg, App.MsgSize, "[SER Stop request]");
            // vDisplayMessage(App.Msg);
            
        } else if (pSerialSlot->setupRequest == 1) {
            pSerialSlot->state = DEV_STATE_SERVICE_SETUP;
            // snprintf(App.Msg, App.MsgSize, "[SER Setup request]");
            // vDisplayMessage(App.Msg);
            
        } else if (pSerialSlot->setupRequest == 1000) {
            pSerialSlot->state = DEV_STATE_SERVICE_FIRST_SETUP;
            // snprintf(App.Msg, App.MsgSize, "[SER First Setup request]");
            // vDisplayMessage(App.Msg);
            
        } else if (pSerialSlot->homingRequest) {
            pSerialSlot->state = DEV_STATE_HOMING_INIT;
            // snprintf(App.Msg, App.MsgSize, "[SER Homing request]");
            // vDisplayMessage(App.Msg);

        } else if (pSerialSlot->resetRequest) {
            pSerialSlot->state = DEV_STATE_SERVICE_RESET;
            // snprintf(App.Msg, App.MsgSize, "[SER Reset request]");
            // vDisplayMessage(App.Msg);
            
        } else {
            pSerialSlot->state = DEV_STATE_STREAMING_POST_ACT;
        }


        return 1;
    }

    return 0;
}

int32_t handle_modbus_streaming_recv(void *pvSerialSlot) {
    if (pvSerialSlot) {
        SerialSlot *pSerialSlot = (SerialSlot *) pvSerialSlot;
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pSerialSlot->pActuator;

        char str[256];
        char hHigh = 0, bLow = 0;
        int32_t addr = 0;
        int32_t nb = 0;
        int32_t res = 0;
        float newPosition = 0.0;

        int32_t avaiable_bytes = modbus_data_available((modbus_t*) pSerialSlot->modbus_ctx);
        if (avaiable_bytes >= pSerialSlot->waitingRespSize) {
            /////////////////////////////////////////////////////////////////
            // Dato disponibile sulla Seriale ...Lettura risposta ...
            //
            if ((res = xrt_modbus_read_registers_recv((modbus_t*) pSerialSlot->modbus_ctx, (uint16_t*)&pSerialSlot->data[0])) < 0) {

                pSerialSlot->streamErrorCount++;

                if (pSerialSlot->streamErrorCount > MAX_MODBUS_STREAM_ERROR) {
                    /////////////////////////////////////////////////
                    // Errore lettura dato : Emergenza Macchina
                    // Generazione Allarme
                    //
                    snprintf(str, sizeof (str), "[SER#%d] Recv Error:%d (%d/%d)", pSerialSlot->stationId, res, avaiable_bytes, pSerialSlot->waitingRespSize);
                    if (generate_alarm((char*) str, 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0+1) < 0) {
                    }
                    // reinizializa il flusso
                    pSerialSlot->state = DEV_STATE_INIT;
                }

            } else {

                switch (pActuator->IDLESeq) {

                    // Lettura posizione
                    case 0:
                        if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN) {

                            ///////////////////////////////////////////////////////////////////////
                            //
                            // N.B.:    Nel driver la posizione va da 0..65535 pulses e 0..65535 turns
                            //          Nella logica da da 0..65535 pulses e -32765..+32765 turns 
                            //
                            pSerialSlot->driverTurns = (int32_t) ((int16_t)pSerialSlot->data[4 + 1]);
                            pSerialSlot->driverPulses = (int32_t) ((uint16_t)pSerialSlot->data[4 + 0]);
                            pSerialSlot->driverPosition = (int32_t) pSerialSlot->driverPulses + ((int32_t) pSerialSlot->driverTurns) * (int32_t)pActuator->pulsesOverflow;
                            pSerialSlot->driverSpeed = (int32_t) ((int16_t) pSerialSlot->data[0]);
                            pSerialSlot->driverAcc = (uint32_t) MAKEDWORD(0, 0);


                        } else if (pActuator->protocol == MODBUS_AC_SERVO_DELTA) {
                            int32_t pulses = 0;
                            memcpy(&pulses, pSerialSlot->data, 4);

                            pSerialSlot->driverTurns = (int32_t) pulses / (int32_t)pActuator->pulsesPerTurn;
                            pSerialSlot->driverPulses = (int32_t) pulses % (int32_t)pActuator->pulsesPerTurn;
                            pSerialSlot->driverPosition = (int32_t) pSerialSlot->driverPulses + (int32_t) pSerialSlot->driverTurns * (int32_t)pActuator->pulsesOverflow;
                            pSerialSlot->driverSpeed = (int32_t) ((int16_t) 0);
                            pSerialSlot->driverAcc = (uint32_t) MAKEDWORD(0, 0);
                        }

                        pSerialSlot->readCount++;


                        ////////////////////////////////
                        // Dati eccedenti ?
                        //
                        if (avaiable_bytes > pSerialSlot->waitingRespSize) {
                            snprintf((char*) str, sizeof (str), (char*) "%s>%d%s", (char*) ANSI_COLOR_CYAN, (int32_t) avaiable_bytes > pSerialSlot->waitingRespSize, (char*) ANSI_COLOR_RESET); vDisplayMessage(str);                    
                        } else {
                        }

                        // Passa il dato alla logica
                        if (pActuator) {

                            pActuator->driverTurns = pSerialSlot->driverTurns;
                            pActuator->driverPulses = pSerialSlot->driverPulses;
                            pSerialSlot->driverPosition = pSerialSlot->driverPosition;

                            // posizione
                            actuator_encoder_to_position((void *) pActuator, (int32_t) pSerialSlot->driverTurns, (int32_t) pSerialSlot->driverPulses, &newPosition);

                            // posizione reale
                            actuator_handle_read_position((void *) pActuator, newPosition, false);

                            // velocita
                            pActuator->speed = (float) pSerialSlot->driverSpeed;


                            if (pActuator->speed > pActuator->max_speed)
                                pActuator->max_speed = pActuator->speed;
                            if (pActuator->speed < pActuator->min_speed)
                                pActuator->min_speed = pActuator->speed;

                            // posizione virtuale (0...1000)
                            update_actuator_virtual_pos(pActuator);
                        }

                        break;
                    

                        ///////////////////////////////////////////////////
                        // Lettura Coppia nominale (Rated torque)
                        //
                        // Pr0.14 Load rate (%) ?? sempre a zero
                        // Pr0.15 Rated torque ???
                        // Pr0.09 Rated torque ?
                        // Pr0.10 Current (A) ?
                        //
                    case 1:
                        if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN) {
                            pActuator->rated_torque = (float) ((int16_t)pSerialSlot->data[0 + 0]) / 100.0f / 100.0f;
                        } else if (pActuator->protocol == MODBUS_AC_SERVO_DELTA) {
                        }
                        break;

                        ////////////////////////////////////////
                        // Lettura Temperatura Driver
                        // Pr0.08 (?) Drive module temperature ℃
                        //
                    case 2:
                        if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN) {
                            int8_t temp = 0;
                            memcpy(&temp, &pSerialSlot->data[0 + 0], 1);
                            pActuator->driverTemp = (int32_t) ((int16_t)pSerialSlot->data[0 + 0]);
                            pActuator->driverTemp = (int32_t)temp;
                            pActuator->temp = (float)pActuator->driverTemp;
                        } else if (pActuator->protocol == MODBUS_AC_SERVO_DELTA) {
                        }
                        break;
                        
                        //////////////////////////////////////////////
                        // Position following error warning value
                        // Pr1.14 
                        //
                    case 3:
                        if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN) {
                            float fErr = ((int32_t)pSerialSlot->data[0 + 0] != (int32_t)65535) ? ((float)pSerialSlot->data[0 + 0] / (float)pActuator->pulsesPerTurn * 100.0f) : (0.0f);
                            
                            if (pSerialSlot->runningCommand == 1) {
                                pActuator->follow_error1 = (float) fErr;
                            } else if (pSerialSlot->runningCommand == -1) {
                                pActuator->follow_error2 = (float) fErr;
                            }
                            
                        } else if (pActuator->protocol == MODBUS_AC_SERVO_DELTA) {
                        }
                        break;
                        
                    default:
                        snprintf(str, sizeof (str), "[SER#%d] SEQ out of range:%d", pSerialSlot->stationId, pActuator->IDLESeq);
                        if (generate_alarm((char*) str, 6031, 0, (int32_t) ALARM_FATAL_ERROR, 0+1) < 0) {
                        }
                        break;
                    
                }
                


                // fine lettura
                pSerialSlot->state = DEV_STATE_STREAMING_DONE;
            }

            
            
        } else {

            ////////////////////////////////////////////////////////////////////
            //
            // Controllo il PreTimeput, rispedento in caso il messggio
            // if (modbus_check_pretimeout(pvSerialSlot) < 0) return -1;
            //
            
            if (xTaskGetTickCount() - pSerialSlot->start_time > MODBUS_STREAM_TIMEOUT_MS )  {
                /////////////////////////////
                // Comunicazione in timeout 
                //
                // Generazione Allarme
                //

                App.SERMeasurePostTimeout = xTaskGetTickCount();

                snprintf(str, sizeof (str), "[SER#%d] SEQ:%d Streaming timeout (%d/%d)", pSerialSlot->stationId, pActuator->IDLESeq, avaiable_bytes, pSerialSlot->waitingRespSize);
                if (generate_alarm((char*) str, 6003, 0, (int32_t) ALARM_FATAL_ERROR, 0+1) < 0) {
                }

                // handle_serial_borad_error(i, (char*) "timeout!", pSerialSlot->state);

                // reinizializza il flusso
                pSerialSlot->state = DEV_STATE_INIT;
            }
        }

        return 1;
    }

    return 0;
}

///////////////////////////////////////////////
// N.B, Test del 1/12/2017
//      Errore nello stream se < 7msec
#define TIME_BEFORE_START_STREAM_MS 10


int32_t handle_modbus_streaming_send(void *pvSerialSlot) {
    if (pvSerialSlot) {
        SerialSlot *pSerialSlot = (SerialSlot *) pvSerialSlot;
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pSerialSlot->pActuator;

        char str[256];
        char hHigh = 0, bLow = 0;
        int32_t addr = 0;
        int32_t nb = 0;
        float newPosition = 0.0;


        if (xTaskGetTickCount() - pSerialSlot->start_time >= TIME_BEFORE_START_STREAM_MS) {

            ////////////////////////////////////////////////////       
            // Verifica variazione paramtri degli assi
            //
            //
            if (pActuator) {
                LP_ACTUATOR pActuatorMirror = (LP_ACTUATOR) pActuator->actuator_mirror;
                BOOL reinitActuator = false;

                if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN) {
                    // N.B.: Le decellerazione sono uguali alla acc x imposizione del driver
                    pActuator->dec_auto1 = pActuator->acc_auto1;
                    pActuator->dec_auto2 = pActuator->acc_auto2;
                } else if (pActuator->protocol == MODBUS_AC_SERVO_DELTA) {
                }

                if (pActuatorMirror) {
                    if (fabs(pActuatorMirror->speed_auto1 - pActuator->speed_auto1) > 0.001) {
                        // snprintf(App.Msg, App.MsgSize, "[%s %s change speed1 %s]", (char*) ANSI_COLOR_GREEN, pActuator->name, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);
                        pActuatorMirror->speed_auto1 = pActuator->speed_auto1;
                        pSerialSlot->paramToUpdate[SPEED_1] = TRUE;
                        reinitActuator = true;
                    }
                    if (fabs(pActuatorMirror->speed_auto2 - pActuator->speed_auto2) > 0.001) {
                        // snprintf(App.Msg, App.MsgSize, "[%s %s change speed2 %s]", (char*) ANSI_COLOR_GREEN, pActuator->name, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);
                        pActuatorMirror->speed_auto2 = pActuator->speed_auto2;
                        pSerialSlot->paramToUpdate[SPEED_2] = TRUE;
                        reinitActuator = true;
                    }
                    if (fabs(pActuatorMirror->acc_auto1 - pActuator->acc_auto1) > 0.001) {
                        // snprintf(App.Msg, App.MsgSize, "[%s %s change acc_auto1 %s]", (char*) ANSI_COLOR_GREEN, pActuator->name, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);
                        pActuatorMirror->acc_auto1 = pActuator->acc_auto1;
                        pSerialSlot->paramToUpdate[ACC_1] = TRUE;
                        reinitActuator = true;
                    }
                    if (fabs(pActuatorMirror->acc_auto2 - pActuator->acc_auto2) > 0.001) {
                        // snprintf(App.Msg, App.MsgSize, "[%s %s change acc_auto2 %s]", (char*) ANSI_COLOR_GREEN, pActuator->name, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);
                        pActuatorMirror->acc_auto2 = pActuator->acc_auto2;
                        pSerialSlot->paramToUpdate[ACC_2] = TRUE;
                        reinitActuator = true;
                    }
                    if (fabs(pActuatorMirror->dec_auto1 - pActuator->dec_auto1) > 0.001) {
                        // snprintf(App.Msg, App.MsgSize, "[%s %s change dec_auto1 %s]", (char*) ANSI_COLOR_GREEN, pActuator->name, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);
                        pActuatorMirror->dec_auto1 = pActuator->dec_auto1;
                        pSerialSlot->paramToUpdate[ACC_1] = TRUE;
                        reinitActuator = true;
                    }
                    if (fabs(pActuatorMirror->dec_auto2 - pActuator->dec_auto2) > 0.001) {
                        // snprintf(App.Msg, App.MsgSize, "[%s %s change dec_auto2 %s]", (char*) ANSI_COLOR_GREEN, pActuator->name, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);
                        pActuatorMirror->dec_auto2 = pActuator->dec_auto2;
                        pSerialSlot->paramToUpdate[ACC_2] = TRUE;
                        reinitActuator = true;
                    }
                    if (fabs(pActuatorMirror->follow_error1 - pActuator->follow_error1) > 0.001) {
                        pActuatorMirror->follow_error1 = pActuator->follow_error1;
                        pSerialSlot->paramToUpdate[FERR_1] = TRUE;
                        reinitActuator = true;
                    }
                    if (fabs(pActuatorMirror->follow_error2 - pActuator->follow_error2) > 0.001) {
                        pActuatorMirror->follow_error2 = pActuator->follow_error2;
                        pSerialSlot->paramToUpdate[FERR_2] = TRUE;
                        reinitActuator = true;
                    }


                    if (reinitActuator) {
                        pSerialSlot->state = DEV_STATE_SERVICE_SETUP_SPEED_ACC;
                    }
                }
            }


            if (pSerialSlot->state != DEV_STATE_SERVICE_SETUP_SPEED_ACC) {
                // Priorita alla variazione dei parametri

                if (pSerialSlot->runningCommand || (App.SERReadPosIDLE && machine.status != AUTOMATIC)) {
                    pSerialSlot->ReadSinglePos = -1;
                }

                if (pSerialSlot->ReadSinglePos) {

                    pSerialSlot->ReadSinglePos--;


                    pActuator->IDLECounter++;

                    if (pActuator->IDLECounter == 1) {
                        pActuator->IDLESeq = 1;
                    } else if (pActuator->IDLECounter == 2) {
                        pActuator->IDLESeq = 2;
                    } else if (pActuator->IDLECounter == 3) {
                        pActuator->IDLESeq = 3;
                    } else if (pActuator->IDLECounter > 100 * 5) {
                        pActuator->IDLECounter = 0;
                        pActuator->IDLESeq = 0;
                    } else {
                        pActuator->IDLESeq = 0;
                    }

                    switch (pActuator->IDLESeq) {

                        ///////////////////////////
                        // Lettura posizione
                        //
                        case 0:
                            if (pActuator) {
                                if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN) {
                                    hHigh = 0, bLow = 0;
                                    addr = MAKEWORD(bLow, hHigh);
                                    nb = 6;
                                } else if (pActuator->protocol == MODBUS_AC_SERVO_DELTA) {
                                    hHigh = 5, bLow = 16;
                                    addr = MAKEWORD(bLow, hHigh);
                                    nb = 2;
                                }

                                // Lunghezza risposta attesa
                                pSerialSlot->waitingRespSize = 3 + 2 * nb + 2;

                                if (xrt_modbus_read_registers_send((modbus_t*) pSerialSlot->modbus_ctx, addr, nb) < 0) {
                                    /////////////////////////
                                    // Generazione Allarme
                                    //
                                    snprintf(str, sizeof (str), "[SER#%d] Send Error", pSerialSlot->stationId);
                                    if (generate_alarm((char*) str, 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0+1) < 0) {
                                    }

                                    // handle_serial_borad_error(i, (char*) "send", pSerialSlot->state);
                                    pSerialSlot->streamErrorCount++;

                                    // reinizializza il flusso
                                    pSerialSlot->state = DEV_STATE_INIT;

                                } else {
                                    pSerialSlot->streamErrorCount = 0;
                                    pSerialSlot->state = DEV_STATE_STREAMING_RECV;
                                }
                            }
                            break;


                            ///////////////////////////////////////////////////
                            // Lettura Coppia nominale (Rated torque)
                            // Pr0.14 Load rate (%) ?? sempre a zero
                            // Pr0.15 Rated torque ???
                            // Pr0.09 Rated torque ?
                            // Pr0.10 Current (A) ?
                            //
                        case 1:
                            if (pActuator) {
                                if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN) {
                                    hHigh = 0, bLow = 9;
                                    addr = MAKEWORD(bLow, hHigh);
                                    nb = 2;
                                } else if (pActuator->protocol == MODBUS_AC_SERVO_DELTA) {
                                    // ??????
                                    hHigh = 0, bLow = 0;
                                    addr = MAKEWORD(bLow, hHigh);
                                    nb = 1;
                                }

                                // Lunghezza risposta attesa
                                pSerialSlot->waitingRespSize = 3 + 2 * nb + 2;

                                if (xrt_modbus_read_registers_send((modbus_t*) pSerialSlot->modbus_ctx, addr, nb) < 0) {
                                    /////////////////////////
                                    // Generazione Allarme
                                    //
                                    snprintf(str, sizeof (str), "[SER#%d] Send Error", pSerialSlot->stationId);
                                    if (generate_alarm((char*) str, 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0+1) < 0) {
                                    }

                                    // handle_serial_borad_error(i, (char*) "send", pSerialSlot->state);
                                    pSerialSlot->streamErrorCount++;

                                    // reinizializza il flusso
                                    pSerialSlot->state = DEV_STATE_INIT;

                                } else {
                                    pSerialSlot->streamErrorCount = 0;
                                    pSerialSlot->state = DEV_STATE_STREAMING_RECV;
                                }
                            }
                            break;


                            ////////////////////////////////////////
                            // Lettura Temperatura Driver
                            // Pr0.30 (?) Drive module temperature  ℃
                            //
                        case 2:
                            if (pActuator) {
                                if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN) {
                                    hHigh = 0, bLow = 30;
                                    addr = MAKEWORD(bLow, hHigh);
                                    nb = 1;
                                } else if (pActuator->protocol == MODBUS_AC_SERVO_DELTA) {
                                    // ??????
                                    nb = 0;
                                }

                                // Lunghezza risposta attesa
                                pSerialSlot->waitingRespSize = 3 + 2 * nb + 2;

                                if (xrt_modbus_read_registers_send((modbus_t*) pSerialSlot->modbus_ctx, addr, nb) < 0) {
                                    /////////////////////////
                                    // Generazione Allarme
                                    //
                                    snprintf(str, sizeof (str), "[SER#%d] Send Error", pSerialSlot->stationId);
                                    if (generate_alarm((char*) str, 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0+1) < 0) {
                                    }

                                    // handle_serial_borad_error(i, (char*) "send", pSerialSlot->state);
                                    pSerialSlot->streamErrorCount++;

                                    // reinizializza il flusso
                                    pSerialSlot->state = DEV_STATE_INIT;

                                } else {
                                    pSerialSlot->streamErrorCount = 0;
                                    pSerialSlot->state = DEV_STATE_STREAMING_RECV;
                                }
                            }
                            break;

                            //////////////////////////////////////////////
                            // Position following error fault value
                            // Pr1.16 
                            //
                        case 3:
                            if (pActuator) {
                                if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN) {
                                    hHigh = 1, bLow = 16;
                                    addr = MAKEWORD(bLow, hHigh);
                                    nb = 2;
                                } else if (pActuator->protocol == MODBUS_AC_SERVO_DELTA) {
                                    // ??????
                                }

                                // Lunghezza risposta attesa
                                pSerialSlot->waitingRespSize = 3 + 2 * nb + 2;

                                if (xrt_modbus_read_registers_send((modbus_t*) pSerialSlot->modbus_ctx, addr, nb) < 0) {
                                    /////////////////////////
                                    // Generazione Allarme
                                    //
                                    snprintf(str, sizeof (str), "[SER#%d] Send Error", pSerialSlot->stationId);
                                    if (generate_alarm((char*) str, 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0+1) < 0) {
                                    }

                                    // handle_serial_borad_error(i, (char*) "send", pSerialSlot->state);
                                    pSerialSlot->streamErrorCount++;

                                    // reinizializza il flusso
                                    pSerialSlot->state = DEV_STATE_INIT;

                                } else {
                                    pSerialSlot->streamErrorCount = 0;
                                    pSerialSlot->state = DEV_STATE_STREAMING_RECV;
                                }
                            }
                            break;

                        default:
                            snprintf(str, sizeof (str), "[SER#%d] SEQ out of range:%d", pSerialSlot->stationId, pActuator->IDLESeq);
                            if (generate_alarm((char*) str, 6030, 0, (int32_t) ALARM_FATAL_ERROR, 0+1) < 0) {
                            }
                            pSerialSlot->streamErrorCount = 0;
                            pSerialSlot->state = DEV_STATE_INIT_STREAM;
                            break;
                    }

                
                    pSerialSlot->start_time = xTaskGetTickCount();



                } else {
                    /////////////////////////////////////////
                    // IDLE : nessun comando in esecuzione
                    //                
                    // fine lettura
                    pSerialSlot->state = DEV_STATE_STREAMING_DONE;
                }
            }


            return 1;
        }
    }
    
    return 0;
}

int32_t handle_modbus_streaming_init(void *pvSerialSlot) {
    if (pvSerialSlot) {
        SerialSlot *pSerialSlot = (SerialSlot *) pvSerialSlot;
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pSerialSlot->pActuator;


        // Abilitazione Seriale
        App.SEROK = INIT_AND_RUN;

        
        int32_t rc = modbus_purge_comm((modbus_t*) pSerialSlot->modbus_ctx);
        if (rc) {
            // fprintf(stdout, "[purge comm serial board#%d - %d bytes]\n", pSerialSlot->boardId, rc); fflush(stdout);
            /////////////////////////////////////////////////////////////
            // Misura il tempo della risposta (x regolare il timeout
            //
            if (App.SERMeasurePostTimeout) {
                snprintf(App.Msg, App.MsgSize, "[%s Post timeout response : %dmsec %s]\n", (char*) ANSI_COLOR_RED, (xTaskGetTickCount() - App.SERMeasurePostTimeout), (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);
                App.SERMeasurePostTimeout = 0;
            }
        }

        pSerialSlot->streamErrorCount = 0;
        pSerialSlot->preTimeout = 0;
        pSerialSlot->start_time = pSerialSlot->tStat = xTaskGetTickCount();



        ///////////////////////////////////////////////////////
        // Prepara l'uscita ausiliaria al fronte di discesa
        //
        // actuator_set_aux_io((void *) pActuator, (int32_t) SSR_STATE_ON, (int32_t) 0);




        ///////////////////////////////////////////////
        // Richieste in pendenza
        //
        if (pSerialSlot->stopRequest) {
            pSerialSlot->state = DEV_STATE_SERVICE_STOP;
            snprintf(App.Msg, App.MsgSize, "[SER Stop request]");
            vDisplayMessage(App.Msg);
            
        } else if (pSerialSlot->setupRequest == 1) {
            pSerialSlot->state = DEV_STATE_SERVICE_SETUP;
            snprintf(App.Msg, App.MsgSize, "[SER Setup request]");
            vDisplayMessage(App.Msg);

        } else if (pSerialSlot->setupRequest == 1000) {
            pSerialSlot->state = DEV_STATE_SERVICE_FIRST_SETUP;
            snprintf(App.Msg, App.MsgSize, "[SER First Setup request]");
            
        } else if (pSerialSlot->homingRequest) {
            pSerialSlot->state = DEV_STATE_HOMING_INIT;
            snprintf(App.Msg, App.MsgSize, "[SER Homing request]");
            vDisplayMessage(App.Msg);

        } else if (pSerialSlot->resetRequest) {
            pSerialSlot->state = DEV_STATE_SERVICE_RESET;
            snprintf(App.Msg, App.MsgSize, "[SER Reset request]");
            vDisplayMessage(App.Msg);
            
        } else {
            pSerialSlot->state = DEV_STATE_STREAMING_SEND;
        }



        return 1;
    }

    return 0;
}


//////////////////////////////////////////////
// Funzioni Eseguita all'avvio dell'asse
// Lettura parametri funzionali ...
//

int32_t handle_modbus_startup(void *pvSerialSlot) {
    if (pvSerialSlot) {
        SerialSlot *pSerialSlot = (SerialSlot *) pvSerialSlot;
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pSerialSlot->pActuator;

        char hHigh = 0, bLow = 0;
        int32_t rc;
        int32_t addr = 0;
        int32_t nb = 0;
        uint16_t data[256] = {0};


        modbus_set_debug((modbus_t*) pSerialSlot->modbus_ctx, false);

        if (pActuator) {

            if (pActuator->protocol == MODBUS_AC_SERVO_LICHUAN) {

                ///////////////////////////////////////////////
                // Lettura : Pr7.00 Station number setting
                //
                hHigh = 7, bLow = 0;
                addr = MAKEWORD(bLow, hHigh);
                nb = 1;


                // 01 03 00 00 00 01 84 0A

                if ((rc = modbus_read_registers((modbus_t*) pSerialSlot->modbus_ctx, addr, nb, &data[0])) < 0) {
                    snprintf(App.Msg, App.MsgSize, "[%s error on serial board error:%d/%d - %s %s]\n", (char*) ANSI_COLOR_RED, pSerialSlot->boardId, machine.numSerialSlots, (char*) modbus_strerror(errno), (char*) ANSI_COLOR_RESET);
                    vDisplayMessage(App.Msg);
                    return -1;

                } else {

                    int16_t stationId = 0;
                    memcpy(&stationId, data, 2);

                    // snprintf(App.Msg, App.MsgSize, "[%s serial board %d.%d(%d) - OK %s]\n", (char*) ANSI_COLOR_GREEN, pSerialSlot->boardId, pSerialSlot->stationId, stationId, (char*) ANSI_COLOR_RESET);
                    // vDisplayMessage(App.Msg);

                    bool dumpParams = false;

                    if (dumpParams) {
                        ///////////////////////////////////////////////
                        // Lettura : Pr0.20 Busbar voltage
                        //
                        hHigh = 0, bLow = 20;
                        addr = MAKEWORD(bLow, hHigh);
                        nb = 1;

                        if ((rc = modbus_read_registers((modbus_t*) pSerialSlot->modbus_ctx, addr, nb, (uint16_t*) & data[0])) < 0) {
                        } else {
                            snprintf(App.Msg, App.MsgSize, "[%s board:%d/%d - Device voltage:%dV%s ]\n", (char*) ANSI_COLOR_YELLOW, pSerialSlot->boardId, machine.numSerialSlots, int32_t(data[0]), (char*) ANSI_COLOR_RESET);
                            vDisplayMessage(App.Msg);
                        }
                        ///////////////////////////////////////////////
                        // Lettura : Pr0.30 Temperature
                        //
                        hHigh = 0, bLow = 30;
                        addr = MAKEWORD(bLow, hHigh);
                        nb = 1;

                        if ((rc = modbus_read_registers((modbus_t*) pSerialSlot->modbus_ctx, addr, nb, (uint16_t*) & data[0])) < 0) {
                        } else {
                            snprintf(App.Msg, App.MsgSize, "[%s board:%d/%d - Device temperature:%d°C%s ]\n", (char*) ANSI_COLOR_YELLOW, pSerialSlot->boardId, machine.numSerialSlots, int32_t(data[0]), (char*) ANSI_COLOR_RESET);
                            vDisplayMessage(App.Msg);
                        }

                        ///////////////////////////////////////////////
                        // Lettura : Pr0.0 - 07
                        //
                        hHigh = 0, bLow = 0;
                        addr = MAKEWORD(bLow, hHigh);
                        nb = 8;

                        if ((rc = modbus_read_registers((modbus_t*) pSerialSlot->modbus_ctx, addr, nb, (uint16_t*) & data[0])) < 0) {
                            snprintf(App.Msg, App.MsgSize, "[%s serial board:%d/%d - %s %s]\n", (char*) ANSI_COLOR_RED, pSerialSlot->boardId, machine.numSerialSlots, (char*) modbus_strerror(errno), (char*) ANSI_COLOR_RESET);
                            vDisplayMessage(App.Msg);
                        } else {
                            for (int32_t j = 0; j < nb; j++) {
                                snprintf(App.Msg, App.MsgSize, "[%s board:%d/%d - PR0.%d : %d %s ]\n", (char*) ANSI_COLOR_YELLOW, pSerialSlot->boardId, machine.numSerialSlots, j + bLow, (int32_t) int32_t(data[j]), (char*) ANSI_COLOR_RESET);
                                vDisplayMessage(App.Msg);
                            }
                        }

                        ///////////////////////////////////////////////
                        // Lettura : Pr0.08 - 15
                        //
                        hHigh = 0, bLow = 8;
                        addr = MAKEWORD(bLow, hHigh);
                        nb = 8;

                        if ((rc = modbus_read_registers((modbus_t*) pSerialSlot->modbus_ctx, addr, nb, (uint16_t*) & data[0])) < 0) {
                            snprintf(App.Msg, App.MsgSize, "[%s serial board:%d/%d - %s %s]\n", (char*) ANSI_COLOR_RED, pSerialSlot->boardId, machine.numSerialSlots, (char*) modbus_strerror(errno), (char*) ANSI_COLOR_RESET);
                            vDisplayMessage(App.Msg);
                        } else {
                            for (int32_t j = 0; j < nb; j++) {
                                snprintf(App.Msg, App.MsgSize, "[%s board:%d/%d - PR0.%d : %d %s ]\n", (char*) ANSI_COLOR_YELLOW, pSerialSlot->boardId, machine.numSerialSlots, j + bLow, (int32_t) int32_t(data[j]), (char*) ANSI_COLOR_RESET);
                                vDisplayMessage(App.Msg);
                            }
                        }

                        ///////////////////////////////////////////////
                        // Lettura : Pr0.16 - 23
                        //
                        hHigh = 0, bLow = 16;
                        addr = MAKEWORD(bLow, hHigh);
                        nb = 8;

                        if ((rc = modbus_read_registers((modbus_t*) pSerialSlot->modbus_ctx, addr, nb, (uint16_t*) & data[0])) < 0) {
                            snprintf(App.Msg, App.MsgSize, "[%s serial board:%d/%d - %s %s]\n", (char*) ANSI_COLOR_RED, pSerialSlot->boardId, machine.numSerialSlots, (char*) modbus_strerror(errno), (char*) ANSI_COLOR_RESET);
                            vDisplayMessage(App.Msg);
                        } else {
                            for (int32_t j = 0; j < nb; j++) {
                                snprintf(App.Msg, App.MsgSize, "[%s board:%d/%d - PR0.%d : %d %s ]\n", (char*) ANSI_COLOR_YELLOW, pSerialSlot->boardId, machine.numSerialSlots, j + bLow, (int32_t) int32_t(data[j]), (char*) ANSI_COLOR_RESET);
                                vDisplayMessage(App.Msg);
                            }
                        }
                        ///////////////////////////////////////////////
                        // Lettura : Pr0.24 - 31
                        //
                        hHigh = 0, bLow = 24;
                        addr = MAKEWORD(bLow, hHigh);
                        nb = 7;

                        if ((rc = modbus_read_registers((modbus_t*) pSerialSlot->modbus_ctx, addr, nb, (uint16_t*) & data[0])) < 0) {
                            snprintf(App.Msg, App.MsgSize, "[%s serial board:%d/%d - %s %s]\n", (char*) ANSI_COLOR_RED, pSerialSlot->boardId, machine.numSerialSlots, (char*) modbus_strerror(errno), (char*) ANSI_COLOR_RESET);
                            vDisplayMessage(App.Msg);
                        } else {
                            for (int32_t j = 0; j < nb; j++) {
                                snprintf(App.Msg, App.MsgSize, "[%s board:%d/%d - PR0.%d : %d %s ]\n", (char*) ANSI_COLOR_YELLOW, pSerialSlot->boardId, machine.numSerialSlots, j + bLow, (int32_t) int32_t(data[j]), (char*) ANSI_COLOR_RESET);
                                vDisplayMessage(App.Msg);
                            }
                        }

                        ///////////////////////////////////////////////
                        // Lettura : Pr1.22
                        //
                        hHigh = 1, bLow = 22;
                        addr = MAKEWORD(bLow, hHigh);
                        nb = 1;

                        if ((rc = modbus_read_registers((modbus_t*) pSerialSlot->modbus_ctx, addr, nb, (uint16_t*) & data[0])) < 0) {
                            snprintf(App.Msg, App.MsgSize, "[%s serial board:%d/%d - %s %s]\n", (char*) ANSI_COLOR_RED, pSerialSlot->boardId, machine.numSerialSlots, (char*) modbus_strerror(errno), (char*) ANSI_COLOR_RESET);
                            vDisplayMessage(App.Msg);
                        } else {
                            for (int32_t j = 0; j < nb; j++) {
                                snprintf(App.Msg, App.MsgSize, "[%s board:%d/%d - PR1.%d : %d %s ]\n", (char*) ANSI_COLOR_YELLOW, pSerialSlot->boardId, machine.numSerialSlots, j + bLow, (int32_t) int32_t(data[j]), (char*) ANSI_COLOR_RESET);
                                vDisplayMessage(App.Msg);
                            }
                        }


                        ///////////////////////////////////////////////
                        // Lettura : Pr2.00
                        //
                        hHigh = 2, bLow = 00;
                        addr = MAKEWORD(bLow, hHigh);
                        nb = 4;

                        if ((rc = modbus_read_registers((modbus_t*) pSerialSlot->modbus_ctx, addr, nb, (uint16_t*) & data[0])) < 0) {
                            snprintf(App.Msg, App.MsgSize, "[%s serial board:%d/%d - %s %s]\n", (char*) ANSI_COLOR_RED, pSerialSlot->boardId, machine.numSerialSlots, (char*) modbus_strerror(errno), (char*) ANSI_COLOR_RESET);
                            vDisplayMessage(App.Msg);
                        } else {
                            for (int32_t j = 0; j < nb; j++) {
                                snprintf(App.Msg, App.MsgSize, "[%s board:%d/%d - PR2.%d : %d %s ]\n", (char*) ANSI_COLOR_YELLOW, pSerialSlot->boardId, machine.numSerialSlots, j + bLow, (int32_t) int32_t(data[j]), (char*) ANSI_COLOR_RESET);
                                vDisplayMessage(App.Msg);
                            }
                        }
                    }

                }

            } else if (pActuator->protocol == MODBUS_AC_SERVO_DELTA) {
                ///////////////////////////////////////////////
                // Lettura : Pr3.00 Station number setting
                //
                hHigh = 3, bLow = 0;
                addr = MAKEWORD(bLow, hHigh);
                nb = 1;


                // es.: 02 03 00 00 00 01 84 0A

                if ((rc = modbus_read_registers((modbus_t*) pSerialSlot->modbus_ctx, addr, nb, &data[0])) < 0) {
                    snprintf(App.Msg, App.MsgSize, "[%s error on serial board %d.%d - %s %s]\n", (char*) ANSI_COLOR_RED, pSerialSlot->boardId, pSerialSlot->stationId, (char*) modbus_strerror(errno), (char*) ANSI_COLOR_RESET);
                    vDisplayMessage(App.Msg);
                    return -1;
                } else {
                    int16_t stationId = 0;
                    memcpy(&stationId, data, 2);

                    // snprintf(App.Msg, App.MsgSize, "[%s serial board %d.%d(%d) - OK %s]\n", (char*) ANSI_COLOR_GREEN, pSerialSlot->boardId, pSerialSlot->stationId, stationId, (char*) ANSI_COLOR_RESET);
                    // vDisplayMessage(App.Msg);

                    bool dumpParams = false;

                    if (dumpParams) {
                        ///////////////////////////////////////////////
                        // Lettura : Pr2.10
                        //
                        hHigh = 2, bLow = 10;
                        addr = MAKEWORD(bLow, hHigh);
                        nb = 1;

                        if ((rc = modbus_read_registers((modbus_t*) pSerialSlot->modbus_ctx, addr, nb, (uint16_t*) & data[0])) < 0) {
                        } else {
                            snprintf(App.Msg, App.MsgSize, "[%s board:%d/%d - PR2.10 SON:%d %s ]\n", (char*) ANSI_COLOR_YELLOW, pSerialSlot->boardId, machine.numSerialSlots, int32_t(data[0]), (char*) ANSI_COLOR_RESET);
                            vDisplayMessage(App.Msg);
                        }
                    }
                }
            } else {
                snprintf(App.Msg, App.MsgSize, "[%s error on serial board %d.%d - Unknown protocol %s]\n", (char*) ANSI_COLOR_RED, pSerialSlot->boardId, pSerialSlot->stationId, (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);
                return -1;
            }
        } else {
            snprintf(App.Msg, App.MsgSize, "[%s handle_modbus_startup() : no actuator %s]\n", (char*) ANSI_COLOR_RED, (char*) ANSI_COLOR_RESET);
            vDisplayMessage(App.Msg);
            return -1;
        }

        return 1;
    }

    return 0;
}

int32_t dump_ac_servo_params(void *pvSerialSlot) {
    if (pvSerialSlot) {
        SerialSlot *pSerialSlot = (SerialSlot *) pvSerialSlot;
        LP_ACTUATOR pActuator = (LP_ACTUATOR) pSerialSlot->pActuator;

        char hHigh = 0, bLow = 0;
        int32_t rc;
        int32_t addr = 0;
        int32_t nb = 0;
        uint16_t data[256] = {0};

        ///////////////////////////////////////////////
        // Lettura : Pr2.10
        //
        snprintf(App.Msg, App.MsgSize, "[Board:%d params]\n", pSerialSlot->boardId);
        vDisplayMessage(App.Msg);

        for (int32_t i = 0; i < 32; i++) {
            
            hHigh = 0, bLow = 1 + i;
            addr = MAKEWORD(bLow, hHigh);
            nb = 1;
            
            if ((rc = modbus_read_registers((modbus_t*) pSerialSlot->modbus_ctx, addr, nb, (uint16_t*) & data[0])) < 0) {
                snprintf(App.Msg, App.MsgSize, "[%sPR%d.%d=ERROR %s]", (char*) ANSI_COLOR_YELLOW, hHigh, bLow, (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);
            } else {
                snprintf(App.Msg, App.MsgSize, "[%sPR%d.%d=%d %s]", (char*) ANSI_COLOR_YELLOW, hHigh, bLow, int32_t(data[0]), (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);
            }
            usleep(20 * 1000);
        }

        vDisplayMessage("\n");
    }

    return 1;
}