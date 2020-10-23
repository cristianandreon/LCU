//============================================================================
// Name        : canbusCommands.cpp
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



// USO TEMPORANEO : Rele statico normalmente chiuso
#define SSR_STATE_OFF   0
#define SSR_STATE_ON   1


#define MAX_HOMING_POSITION_ERROR   0.06f



int32_t xrt_can_message_send(void *pvCANSlots, uint8_t CANID, int16_t Index, int16_t SubIndex, int8_t *Data, int8_t DataSize, bool writeMode) {
    if (pvCANSlots) {
        if (DataSize > 0 && DataSize <= 4) {
            CANSlot *pCANSlot = (CANSlot *) pvCANSlots;
            uint8_t str[64];
            uint16_t str_size = 1 + 1 + 2 + 1 + 1 + DataSize;

            if (DataSize > sizeof (str) - 6) {
                snprintf(App.Msg, App.MsgSize, "[%s xrt_can_message_send data too large:%d %s]\n", (char*) ANSI_COLOR_RED, (int32_t) DataSize, (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);
                DataSize = sizeof (str) - 6;
            }

            if (writeMode)
                str[0] = '>';
            else
                str[0] = '!';

            memcpy(&str[1], &CANID, 1);
            memcpy(&str[2], &Index, 2);
            memcpy(&str[4], &SubIndex, 1);
            memcpy(&str[5], &DataSize, 1);
            if (Data)
                memcpy(&str[6], &Data[0], DataSize);
            else
                memset(&str[6], 0, DataSize);

            return xrt_send_udp(pCANSlot->ip, (uint16_t) pCANSlot->port, (uint16_t) pCANSlot->port, (uint8_t *) str, (uint16_t) str_size, (uint8_t) 0);
        }
    }

    return -1;
}

int32_t xrt_can_message_recv(void *pvCANSlot, char *Data, int32_t DataSize) {
    if (pvCANSlot) {
        CANSlot *pCANSlot = (CANSlot *) pvCANSlot;

        int32_t retVal = -1, index1B = xrt_connect_udp(pCANSlot->ip, pCANSlot->port, pCANSlot->port);
        if (index1B > 0) {
            if (GLUdpHost[index1B - 1].sock > 0) {
                uint32_t bytes_available = 0;
                ioctl(GLUdpHost[index1B - 1].sock, FIONREAD, &bytes_available);
                if (bytes_available > 0) {
                    if (Data) {
                        retVal = xrt_recv(GLUdpHost[index1B - 1].sock, Data, DataSize);
                        if (retVal > 0) {
                        } else {
                            // vDisplayMessage("No udp callbacks");
                        }
                    }
                } else if (bytes_available == 0) {
                    retVal = 0;
                }
            }
        }

        return retVal;
    }


    return 0;
}

int32_t canbus_data_available(void* pvCANSlot) {
    if (pvCANSlot) {
        CANSlot *pCANSlot = (CANSlot *) pvCANSlot;
        int32_t retVal = -1, index1B = xrt_connect_udp(pCANSlot->ip, pCANSlot->port, pCANSlot->port);
        if (index1B > 0) {
            if (GLUdpHost[index1B - 1].sock > 0) {
                uint32_t bytes_available = 0;
                int32_t rc = ioctl(GLUdpHost[index1B - 1].sock, FIONREAD, &bytes_available);
                if (rc < 0) {
                    perror("canbus_data_available():ioctl:");
                    return 0;
                } else {
                    return bytes_available;
                }
            }
        }
    }
    return 0;
}

char *get_CAN_status(void *pvCANSlots) {

    if (pvCANSlots) {
        CANSlot *CANSlots = (CANSlot *) pvCANSlots;

        switch (CANSlots->state) {
            case DEV_STATE_INIT:
                return (char*) "INIT";
                break;
            case DEV_STATE_WAITING:
                return (char*) "WAITING";
                break;
            case DEV_STATE_CONNECTING:
                return (char*) "CONNECTING";
                break;
            case DEV_STATE_INIT_STREAM:
                return (char*) "INIT_STREAM";
                break;
            case DEV_STATE_STREAMING_SEND:
                return (char*) "STREAMING_SEND";
                break;
            case DEV_STATE_STREAMING_RECV:
                return (char*) "STREAMING_RECV";
                break;
            case DEV_STATE_STREAMING_DONE:
                return (char*) "STREAMING_DONE";
                break;
            case DEV_STATE_STREAMING_POST_ACT:
                return (char*) "STREAMING_POST_ACT";
                break;
            case DEV_STATE_STREAMING_POST_WAIT:
                return (char*) "STREAMING_POST_WAIT";
                break;

            case DEV_STATE_SERVICE:
                return (char*) "SERVICE";
                break;
            case DEV_STATE_SERVICE_SETUP:
                return (char*) "SERVICE_SETUP";
                break;
            case DEV_STATE_SERVICE_SETUP_SPEED_ACC:
                return (char*) "SERVICE_SETUP_SPEED_ACC";
                break;
            case DEV_STATE_SERVICE_STOP:
                return (char*) "SERVICE_STOP";
                break;
            case DEV_STATE_SERVICE_OUT:
                return (char*) "SERVICE_OUT";
                break;
            case DEV_STATE_CMD_INIT:
                return (char*) "CMD_INIT";
                break;
            case DEV_STATE_CMD_INIT_SEND:
                return (char*) "CMD_INIT_SEND";
                break;
            case DEV_STATE_CMD_INIT_RECV:
                return (char*) "CMD_INIT_RECV";
                break;
            case DEV_STATE_CMD_FEEDBACK_SEND:
                return (char*) "CMD_FEEDBACK_SEND";
                break;
            case DEV_STATE_CMD_FEEDBACK_RECV:
                return (char*) "CMD_FEEDBACK_RECV";
                break;
            case DEV_STATE_CMD_DONE:
                return (char*) "CMD_DONE";
                break;
            case DEV_STATE_CMD_DONE_FEEDBACK:
                return (char*) "CMD_DONE_FEEDBACK";
                break;
            case DEV_STATE_CMD_ERROR:
                return (char*) "CMD_ERROR";
                break;
            case DEV_STATE_CLOSING:
                return (char*) "CLOSING";
                break;
            case DEV_STATE_MAX:
                return (char*) "MAX";
                break;
        }
    }
    return (char*) "";
}


/////////////////////////////////////////////////////
// Debug & Test
//

int32_t GLTestPulses = 500;

int32_t test_canbus_position(void *pvCANSlots) {
    if (pvCANSlots) {
        CANSlot *pCANSlot = (CANSlot *) pvCANSlots;
        for (int32_t i_atc = 0; i_atc < pCANSlot->nActuators; i_atc++) {
            LP_ACTUATOR pActuator = (LP_ACTUATOR) pCANSlot->pActuators[i_atc];
            if (pActuator) {
            }
        }
    }
    return 0;
}

/////////////////////////////////////////////////////








//////////////////////////////////////////////
// Funzioni Homing
//

#define CANBUS_HOMING_TIMEOUT_MS        (15 * 1000)

int32_t handle_canbus_homing_init(void *pvCANSlot) {
    if (pvCANSlot) {
        CANSlot *pCANSlot = (CANSlot *) pvCANSlot;

        for (int32_t iact = 0; iact < pCANSlot->nActuators; iact++) {
            LP_ACTUATOR pActuator = (LP_ACTUATOR) pCANSlot->pActuators[iact];
            if (pActuator) {
                if (pActuator->homingRequest) {
                    pActuator->homingDone = 0;
                    pActuator->homingZeroCounter = 0;
                    pActuator->homingPulsesPPT = 0;
                    pActuator->homingTurnsPPT = 0;
                    pActuator->homingSeq = 0;
                    pActuator->homingRequest = 2; // in corso
                }
            }
        }

        pCANSlot->homingRequest = 1;
        pCANSlot->state = DEV_STATE_HOMING_SEND;

        return 1;
    }

    return 0;
}

int32_t handle_canbus_homing_send(void *pvCANSlot) {
    if (pvCANSlot) {
        CANSlot *pCANSlot = (CANSlot *) pvCANSlot;

        for (int32_t iact = 0; iact < pCANSlot->nActuators; iact++) {
            LP_ACTUATOR pActuator = (LP_ACTUATOR) pCANSlot->pActuators[iact];

            if (pActuator) {
                switch (pActuator->homingSeq) {

                    case 0:
                        if (pActuator->homingTorqueMode) {

                            snprintf(App.Msg, App.MsgSize, "[%sStart Homing : entering Torque Mode%s]\n", (char*) ANSI_COLOR_YELLOW, (char*) ANSI_COLOR_RESET);
                            vDisplayMessage(App.Msg);

                            /////////////////////////
                            // Stop motore
                            //
                            // CONTROL_WORD_QUICK_STOP
                            // CONTROL_WORD_RESET

                            // OK
                            if (handle_actuator_prepare_for_run((void*) pActuator, CONTROL_WORD_RESET) < 0) {
                                // Generazione Allarme
                                if (generate_alarm((char*) "CAN Stop AC Servo Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                                }
                                // Homing failed
                                pActuator->homingSeq = 5000;
                                pCANSlot->state = DEV_STATE_HOMING_DONE;
                                return -1;
                            }

                            /**/
                            if (handle_actuator_servo_off((void *) pActuator) < 0) {
                                // return -1;
                                if (generate_alarm((char*) "CANBUS: handle_actuator_start() Error", 6101, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                                }
                                // Homing failed
                                pActuator->homingSeq = 5000;
                                pCANSlot->state = DEV_STATE_HOMING_DONE;
                                return -1;
                            }
                            

                            pActuator->homingStartTime = xTaskGetTickCount();
                            pActuator->homingSeq = 1000;
                            pCANSlot->state = DEV_STATE_HOMING_SEND;
                            return 1;
                        } else {
                            // Homing failed
                            pActuator->homingSeq = 5000;
                            pCANSlot->state = DEV_STATE_HOMING_DONE;
                            return -1;                            
                        }
                        break;

                        
                    case 1000:
                        if (xTaskGetTickCount() - pActuator->homingStartTime > 500) {
                            pActuator->homingSeq = 1010;
                            pCANSlot->state = DEV_STATE_HOMING_SEND;
                            return 1;
                        } else {
                            pCANSlot->state = DEV_STATE_HOMING_SEND;
                            return 0;
                        }
                        break;
                        
                        
                    case 1010: {
                        int16_t objIndex = 0;
                        
                        // Set【Mode of operations:6060h】 to profile torque mode(4).
                        //  ②   Set【Controlword:6040h】 to servo on drive and make motor work.
                        //          (After drive switches to servo-on, internal torque command will be reset and OD-6071h will be
                        //          cleared. It means the drive is servo-on, then starts receiving torque command.)
                        //  ③   Set【Torque slope:6087h】to plan torque slope time. (unit: millisecond from 0 to 100﹪rated torque)
                        //  ④   Set【Target torque:6071h】to target torque. The unit is given per thousand of rated torque.
                        //          (OD-6071h will be cleared to zero if OD-6060h[Mode] changed, Servo-Off or Quick-Stop is activated.)        
                        


                        ///////////////////////////////////////////////////////////////////////////////////////
                        // Enable Speed & Torque Limit P1-02 = 0x11     (P1.02=11 Speed and Torque enabled)
                        // N.B.: Nel canopen 0x2xyz = value  ->   Px.yz = value
                        //
                        // Dev'essere fatto a Servo-OFF
                        //
                        int16_t limitSpeedTorque = 0x0011;
                        objIndex = 0x2000 + 0x100 + 2;
                        
                        if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) objIndex, (int16_t) 0, (int8_t *) & limitSpeedTorque, (int8_t)sizeof (limitSpeedTorque), true) < 0) {
                            // Generazione Allarme
                            if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                            }
                            pCANSlot->state = DEV_STATE_HOMING_DONE;
                            pActuator->homingSeq = 5000;
                            return -1;
                        }


                        ///////////////////////////////////////////////////////////
                        // Set DI2 to Torque Limit DI2 = TCMLT normally open
                        //
                        // P2-11	DI2 = TCMLT [default=116 = {normally open:1}{addr:16h} ]	
                        //                      0 = contatto normalmente chiuso : speed limit sempre abilitato
                        //                      10 = SPDLM (l'ingresso DI2 o SoftDI2 comanda il SPDLM= speed limit)	
                        //                      09 = TRQLM (l'ingresso DI2 o SoftDI2 comanda il TRQLM= torque limit)
                        
                        int16_t di2_mode = 0x0109;
                        objIndex = 0x2000 + 0x200 + 11;
                        
                        if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) objIndex, (int16_t) 0, (int8_t *) & di2_mode, (int8_t)sizeof (di2_mode), true) < 0) {
                            // Generazione Allarme
                            if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                            }
                            pCANSlot->state = DEV_STATE_HOMING_DONE;
                            pActuator->homingSeq = 5000;
                            return -1;
                        }

                        
                        ///////////////////////////////////////////////////////////
                        // Set DI3 to Torque Command DI3=TCM0 normally open
                        //
                        // P2-12	DI3 = TCM0
                        //
                        int16_t di3_mode = 0x0116;
                        objIndex = 0x2000 + 0x200 + 12;
                        
                        if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) objIndex, (int16_t) 0, (int8_t *) & di3_mode, (int8_t)sizeof (di3_mode), true) < 0) {
                            // Generazione Allarme
                            if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                            }
                            pCANSlot->state = DEV_STATE_HOMING_DONE;
                            pActuator->homingSeq = 5000;
                            return -1;
                        }

                        
                        ///////////////////////////////////////////////////////////
                        // P4-07 : Set SDI1, SDI2, SDI3 = 1
                        //
                        int16_t sdi_value = 1*1 + 1*2 + 1*4 + 0*8 + 0*16 + 0*32 + 0*64 + 0*128;
                        // int16_t sdi_value = 0x3FFF;
                        objIndex = 0x2000 + 0x400 + 7;
                        
                        if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) objIndex, (int16_t) 0, (int8_t *) & sdi_value, (int8_t)sizeof (sdi_value), true) < 0) {
                            // Generazione Allarme
                            if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                            }
                            pCANSlot->state = DEV_STATE_HOMING_DONE;
                            pActuator->homingSeq = 5000;
                            return -1;
                        }

                        
                        
                        
                        
                        //////////////////////////////////////////////////////////////////////////////
                        // Set Torque Limit (%)  P1-12 = Torque limit (-300 .. +300)
                        //
                        int16_t torque_limit = (int16_t) (pActuator->homing_rated_torque * 1.0f) * (pActuator->homing_position == OFF ? +1 : -1);
                        objIndex = 0x2000 + 0x100 + 12;
                        
                        if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) objIndex, (int16_t) 0, (int8_t *) & torque_limit, (int8_t)sizeof (torque_limit), true) < 0) {
                            // Generazione Allarme
                            if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                            }
                            pCANSlot->state = DEV_STATE_HOMING_DONE;
                            pActuator->homingSeq = 5000;
                            return -1;
                        }                        

                        /*
                        objIndex = 0x2000 + 0x100 + 13;                        
                        if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) objIndex, (int16_t) 0, (int8_t *) & torque_limit, (int8_t)sizeof (torque_limit), true) < 0) {
                            // Generazione Allarme
                            if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                            }
                            pCANSlot->state = DEV_STATE_HOMING_DONE;
                            pActuator->homingSeq = 5000;
                            return -1;
                        }                        

                        objIndex = 0x2000 + 0x100 + 14;                        
                        if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) objIndex, (int16_t) 0, (int8_t *) & torque_limit, (int8_t)sizeof (torque_limit), true) < 0) {
                            // Generazione Allarme
                            if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                            }
                            pCANSlot->state = DEV_STATE_HOMING_DONE;
                            pActuator->homingSeq = 5000;
                            return -1;
                        }                        
                        */
                        
                        

                        
                        ///////////////////////////////////////////////////////////
                        // P4-07 : Set SDI1, SDI2, SDI3 = 1
                        //
                        // P2-11	DI2 = 1 (TCMLT) [default=116 = {normally open:1}{addr:16h}]
                        //
                        sdi_value = 1*1 + 1*2 + 1*4 + 0*8 + 0*16 + 0*32 + 0*64 + 0*128; // OK
                        objIndex = 0x2000 + 0x400 + 7;
                        
                        if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) objIndex, (int16_t) 0, (int8_t *) & sdi_value, (int8_t)sizeof (sdi_value), true) < 0) {
                            // Generazione Allarme
                            if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                            }
                            pCANSlot->state = DEV_STATE_HOMING_DONE;
                            pActuator->homingSeq = 5000;
                            return -1;
                        }
                        
                        
                        /////////////////////////////
                        // Torque Mode : non ruota
                        // Speed Mode : OK
                        //
                        int8_t mode = 0x03;
                        // int8_t mode = 0x0A;
                        // int8_t mode = 0x013;
                        
                        if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x6060, (int16_t) 0, (int8_t *) & mode, (int8_t)sizeof (mode), true) < 0) {
                            // Generazione Allarme
                            if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                            }
                            pCANSlot->state = DEV_STATE_HOMING_DONE;
                            pActuator->homingSeq = 5000;
                            return -1;
                        }


                        /////////////////////////
                        // Avvio motore
                        //
                        if (handle_actuator_servo_on((void *) pActuator) < 0) {
                            // return -1;
                            if (generate_alarm((char*) "CANBUS: handle_actuator_start() Error", 6101, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                            }
                            // Homing failed
                            pActuator->homingSeq = 5000;
                            pCANSlot->state = DEV_STATE_HOMING_DONE;
                            return -1;
                        }
                        
                        
                        /**/
                        if (handle_actuator_prepare_for_run((void*) pActuator, 999) < 0) {
                            // Generazione Allarme
                            if (generate_alarm((char*) "CAN Start AC Servo Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                            }
                            // Homing failed
                            pActuator->homingSeq = 5000;
                            pCANSlot->state = DEV_STATE_HOMING_DONE;
                            return -1;
                        }
                       

                        

                        // Set【Profile velocity:6081h】to profile velocity. (unit: PUU per second)
                        uint32_t cspeed = (uint32_t) ((float) pActuator->homing_speed_rpm / 60.0f * (float) pActuator->pulsesPerTurn);
                        if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x6081, (int16_t) 0, (int8_t *) & cspeed, (int8_t) sizeof (cspeed), true) < 0) {
                            // Generazione Allarme
                            if (generate_alarm((char*) "CAN Unable to set 0x6081", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                            }
                            pActuator->homingSeq = 5000;
                            pCANSlot->state = DEV_STATE_HOMING_DONE;
                            return -1;
                        }

                        // 6080h Max motor speed Unit:rpm
                        cspeed = (uint32_t) ((float) pActuator->homing_speed_rpm);
                        if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x6080, (int16_t) 0, (int8_t *) & cspeed, (int8_t) sizeof (cspeed), true) < 0) {
                            // Generazione Allarme
                            if (generate_alarm((char*) "CAN Unable to set 0x6080", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                            }
                            pActuator->homingSeq = 5000;
                            pCANSlot->state = DEV_STATE_HOMING_DONE;
                            return -1;
                        }

                        // Set【Target velocity:60FFh】. The unit of Target velocity is 0.1rpm. (INTEGER32)
                        int32_t cspeedi32 = (int32_t) ((float) pActuator->homing_speed_rpm * 10.0f);
                        if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x60FF, (int16_t) 0, (int8_t *) & cspeedi32, (int8_t) sizeof (cspeedi32), true) < 0) {
                            // Generazione Allarme
                            if (generate_alarm((char*) "CAN Unable to set 0x6080", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                            }
                            pActuator->homingSeq = 5000;
                            pCANSlot->state = DEV_STATE_HOMING_DONE;
                            return -1;
                        }

                        
                        // 0x607F Max profile velocity Unit:0.1rpm
                        cspeed = (uint32_t) ((float) pActuator->homing_speed_rpm * 10.0f);
                        if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x607F, (int16_t) 0, (int8_t *) & cspeed, (int8_t) sizeof (cspeed), true) < 0) {
                            // Generazione Allarme
                            if (generate_alarm((char*) "CAN Unable to set 0x607F", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                            }
                            pActuator->homingSeq = 5000;
                            pCANSlot->state = DEV_STATE_HOMING_DONE;
                            return -1;
                        }


                        uint32_t homing_rated_acc_ms = (uint32_t) (2000);
                        if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x6087, (int16_t) 0, (int8_t *) & homing_rated_acc_ms, (int8_t)sizeof (homing_rated_acc_ms), false) < 0) {
                            // Generazione Allarme
                            if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                            }
                            pActuator->homingSeq = 5000;
                            pCANSlot->state = DEV_STATE_HOMING_DONE;
                            return -1;
                        }

                        
                        int16_t homing_rated_torque = (int16_t) (pActuator->homing_rated_torque * 10.0f) * (pActuator->homing_position == OFF ? +1 : -1);
                        if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x6071, (int16_t) 0, (int8_t *) & homing_rated_torque, (int8_t)sizeof (homing_rated_torque), false) < 0) {
                            // Generazione Allarme
                            if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                            }
                            pActuator->homingSeq = 5000;
                            pCANSlot->state = DEV_STATE_HOMING_DONE;
                            return -1;
                        }

                        
                        
                        pActuator->homingStartTime = xTaskGetTickCount();
                        pActuator->homingSeq = 1020;
                        pCANSlot->state = DEV_STATE_HOMING_SEND;
                        return 1;
                        break;
                    }

                    case 1020:
                        if (xTaskGetTickCount() - pActuator->homingStartTime > 250) {
                            pActuator->homingStartTime = xTaskGetTickCount();
                            pActuator->driverSpeedCount = 0;
                            pActuator->homingSeq = 2000;
                            pCANSlot->state = DEV_STATE_HOMING_SEND;
                            return 1;
                        } else {
                            pCANSlot->state = DEV_STATE_HOMING_SEND;
                            return 0;
                        }
                        break;

                    case 2000:
                        // Lettura velocita'
                        pActuator->driverSpeedCount = 0;
                        pActuator->readCounter = 0;
                        pActuator->homingLastSendReqTime = xTaskGetTickCount();
                        if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x606C, (int16_t) 0, (int8_t *) NULL, (int8_t) 1, false) < 0) {
                        } else {
                        }
                        break;
                }
            }
        }

        pCANSlot->state = DEV_STATE_HOMING_RECV;
        return 1;
    }

    return 0;
}

int32_t handle_canbus_homing_recv(void *pvCANSlot) {
    if (pvCANSlot) {
        CANSlot *pCANSlot = (CANSlot *) pvCANSlot;
        char str[256];
        int32_t doneSEQ = 0;

        
        for (int32_t iact = 0; iact < pCANSlot->nActuators; iact++) {
            LP_ACTUATOR pActuator = (LP_ACTUATOR) pCANSlot->pActuators[iact];

            if (pActuator) {

                switch (pActuator->homingSeq) {

                    case 0:
                        // go to home
                        doneSEQ++;
                        pActuator->homingSeq = 1000;
                        break;

                    case 2000:
                        if (xrt_recive_udp(pCANSlot->ip, pCANSlot->port, pCANSlot->port, (uint8_t*) pCANSlot->data, pCANSlot->data_size, 0) > 0) {
                            // velocita
                            if (pActuator->driverSpeedCount) {
                                if (pActuator->driverSpeed <= 1) {
                                    pActuator->homingZeroCounter++;
                                    if (pActuator->homingZeroCounter >= NUM_ZERO_SPEED_TO_HOMING) {
                                        snprintf(App.Msg, App.MsgSize, "[%sZero speed reached on Torque Mode%s]     \r", (char*) ANSI_COLOR_YELLOW, (char*) ANSI_COLOR_RESET);
                                        vDisplayMessage(App.Msg);
                                        pActuator->homingSeq = 3000;
                                        pCANSlot->state = DEV_STATE_HOMING_SEND;
                                        return 1;
                                    }
                                } else {
                                    if (pActuator->homingZeroCounter) {
                                        snprintf(App.Msg, App.MsgSize, "[%sZero speed counter reset by speed:%d %s]     \r", (char*) ANSI_COLOR_YELLOW, pActuator->driverSpeed, (char*) ANSI_COLOR_RESET);
                                        vDisplayMessage(App.Msg);
                                    } else {
                                        // snprintf(App.Msg, App.MsgSize, "[speed:%d]", pActuator->driverSpeed); vDisplayMessage(App.Msg);                                        
                                    }
                                    pActuator->homingZeroCounter = 0;
                                }
                                
                                pCANSlot->state = DEV_STATE_HOMING_SEND;
                                return 1;
                            }
                        }


                        //////////////////////////
                        // Controllo Timeout
                        //
                        if (xTaskGetTickCount() - pActuator->homingStartTime > (pActuator->homing_timeout_ms > 0 ? pActuator->homing_timeout_ms : CANBUS_HOMING_TIMEOUT_MS)) {

                            if (App.DebugMode) {
                                snprintf(str, sizeof (str), "[CAN#%d] HOMING FORCED By Timeout of %0.2fsec - ZeroSpeed Reads:%d", pCANSlot->boardId, (float) CANBUS_HOMING_TIMEOUT_MS / 1000.0f, pActuator->homingZeroCounter);
                                if (generate_alarm((char*) str, 6009, 0, (int32_t) ALARM_WARNING, 0 + 1) < 0) {
                                }
                                // Homing OK for test
                                pActuator->homingSeq = 3000;
                                pCANSlot->state = DEV_STATE_HOMING_DONE;
                                return 1;

                            } else {
                                // Comunicazione in timeout 
                                snprintf(str, sizeof (str), "[CAN#%d] HOMING FEEDBACK Timeout : %0.2fsec - data available:%d/%d", pCANSlot->boardId, (float) CANBUS_HOMING_TIMEOUT_MS / 1000.0f, pCANSlot->data_size, pCANSlot->waitingRespSize);
                                if (generate_alarm((char*) str, 6009, 0, (int32_t) ALARM_ERROR, 0 + 1) < 0) {
                                }

                                // Homing failed
                                pActuator->homingSeq = 5000;
                                pCANSlot->state = DEV_STATE_HOMING_DONE;
                                return -1;
                            }
                        }

                        
                        //////////////////////////
                        // Reinvio richiesta
                        //
                        if (xTaskGetTickCount() - pActuator->homingLastSendReqTime > 250) {
                            pCANSlot->state = DEV_STATE_HOMING_SEND;
                            return 1;
                        }
                        
                        pCANSlot->data_available = 0;

                        // Attende il dato sulla velocità : rimane sulla sequenza
                        pCANSlot->state = DEV_STATE_HOMING_RECV;
                        return 0;
                        break;


                    default:
                        pCANSlot->state = DEV_STATE_HOMING_DONE;
                        break;
                }
            }
        }

        return 1;
    }

    return 0;
}

int32_t handle_canbus_homing_done(void *pvCANSlot) {
    if (pvCANSlot) {
        CANSlot *pCANSlot = (CANSlot *) pvCANSlot;
        int32_t doneSEQ = 0;
        int8_t mode = 0;

        for (int32_t iact = 0; iact < pCANSlot->nActuators; iact++) {
            LP_ACTUATOR pActuator = (LP_ACTUATOR) pCANSlot->pActuators[iact];
            char str[256];

            if (pActuator) {

                switch (pActuator->homingSeq) {

                    case 0:
                        pActuator->homingSeq = 1000;
                        break;

                    case 1:
                        break;

                    case 2:
                        break;

                    case 3000: {
                        // Home done
                        doneSEQ++;
                        pActuator->homingRequest = 0;
                        pActuator->homingDone = true;
                        pActuator->step = STEP_READY;
                        pActuator->error = 0;

                        /////////////////////////
                        // Stop motore
                        //                        
                        if (handle_actuator_prepare_for_run((void*) pActuator, CONTROL_WORD_QUICK_STOP) < 0) {
                            /////////////////////////
                            // Generazione Allarme
                            //
                            if (generate_alarm((char*) "CAN Stop AC Servo Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                            }
                            // Homing failed
                            pActuator->homingSeq = 5000;
                            pCANSlot->state = DEV_STATE_HOMING_DONE;
                            return -1;
                        }


                        pCANSlot->homingRequest = 0;

                        pActuator->homingStartTime = xTaskGetTickCount();
                        pCANSlot->state = DEV_STATE_HOMING_DONE;
                        pActuator->homingSeq = 3010;
                        return 1;
                        break;
                    }


                    case 3010:
                        // Home post action : re-setup
                        if (xTaskGetTickCount() - pActuator->homingStartTime >= 100) {
                            int16_t objIndex = 0x0;


                            ///////////////////////////////////////////////////////////
                            // P4-07 : Set SDI2 + SDI3, SDI4 = 1
                            // DI4 = SPD1 Speed command
                            // Evita che l'albero si sposti
                            //
                            int16_t sdi_value = 1*1 + 1*2 + 1*4 + 1*8 + 0*16 + 0*32 + 0*64 + 0*128;                           
                            // sdi_value = 0xFF;
                            objIndex = 0x2000 + 0x400 + 7;

                            if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) objIndex, (int16_t) 0, (int8_t *) & sdi_value, (int8_t)sizeof (sdi_value), true) < 0) {
                                // Generazione Allarme
                                if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                                }
                                pCANSlot->state = DEV_STATE_HOMING_DONE;
                                pActuator->homingSeq = 5000;
                                return -1;
                            }

                            

                            
                            pCANSlot->state = DEV_STATE_HOMING_DONE;
                            pActuator->homingSeq = 3020;
                            return 1;
                        } else {
                            return 1;
                        }
                        break;

                    case 3020: {
                        //////////////////////////////////
                        // Reenter position Mode
                        //                        
                        mode = 0;
                        if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x6060, (int16_t) 0, (int8_t *) & mode, (int8_t)sizeof (mode), true) < 0) {
                            /////////////////////////
                            // Generazione Allarme
                            //
                            if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                            }
                            pCANSlot->state = DEV_STATE_HOMING_DONE;
                            pActuator->homingSeq = 5000;
                            return -1;
                        } else {
                            pActuator->homingStartTime = xTaskGetTickCount();
                            pCANSlot->state = DEV_STATE_HOMING_DONE;
                            pActuator->homingSeq = 4000;
                            return 1;
                        }
                    }


                    case 4000:
                        // Home post action : re-setup
                        if (xTaskGetTickCount() - pActuator->homingStartTime >= 500) {
                            pCANSlot->state = DEV_STATE_HOMING_DONE;
                            pActuator->homingSeq = 4010;
                            return 1;
                        } else {
                            return 1;
                        }
                        break;

                        
                    case 4010:
                        ///////////////////////////////////
                        // Lettura posizione
                        //
                        pActuator->readCounter = 0;
                        pActuator->driverPosCount = 0;
                        
                        if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x6064, (int16_t) 0, (int8_t *) NULL, (int8_t) 1, false) < 0) {
                            // Generazione Allarme
                            if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                            }
                            pCANSlot->state = DEV_STATE_HOMING_DONE;
                            pActuator->homingSeq = 5000;
                            return -1;
                        } else {
                            pActuator->homingStartTime = xTaskGetTickCount();
                            pCANSlot->state = DEV_STATE_HOMING_DONE;
                            pActuator->homingSeq = 4020;
                            return 1;
                        }
                        break;


                    case 4020:
                        if (xTaskGetTickCount() - pActuator->homingStartTime >= 500) {
                            pActuator->homingStartTime = xTaskGetTickCount();
                            pCANSlot->state = DEV_STATE_HOMING_DONE;
                            pActuator->homingSeq = 4030;
                            return 1;
                        } else {
                            return 1;
                        }
                        break;


                    case 4030:
                        if (xrt_recive_udp(pCANSlot->ip, pCANSlot->port, pCANSlot->port, (uint8_t*) pCANSlot->data, pCANSlot->data_size, 0) > 0) {
                            if (pActuator->driverPosCount) {
                                float newPosition = 0.0f;
                                int32_t offsetTurnsPPT = 0, offsetPulsesPPT = 0;

                                // Offset in Pulse units
                                pActuator->homingTurnsPPT = 0;
                                pActuator->homingPulsesPPT = 0;

                                actuator_position_to_encoder((void *) pActuator, pActuator->homing_offset_mm, &offsetTurnsPPT, &offsetPulsesPPT);

                                actuator_ppo_to_ppt((void *) pActuator, pActuator->driverTurns, pActuator->driverPulses, &pActuator->homingPulsesPPT, &pActuator->homingTurnsPPT);

                                // Aggiunta alle pulsazioni correnti l'offset
                                actuator_add_pulse_ppt((void *) pActuator, offsetTurnsPPT, offsetPulsesPPT, &pActuator->homingTurnsPPT, &pActuator->homingPulsesPPT, 0 + 0);


                                // Imposta l'offdset in pulses / turns
                                pActuator->homingPulsesPPT = pActuator->driverPulses;
                                pActuator->homingTurnsPPT = pActuator->driverTurns;

                                
                                actuator_encoder_to_position((void *) pActuator, (int32_t) pActuator->driverTurns, (int32_t) pActuator->driverPulses, &newPosition);
                                actuator_handle_read_position((void *) pActuator, newPosition, false);


                                // posizione virtuale (0...1000)
                                update_actuator_virtual_pos(pActuator);


                                snprintf(App.Msg, App.MsgSize, "[%sHoming Torque Mode Done : Pos:%d.%d%s]\n"
                                        , (char*) ANSI_COLOR_GREEN
                                        , pActuator->homingTurnsPPT
                                        , pActuator->homingPulsesPPT
                                        , (char*) ANSI_COLOR_RESET);
                                vDisplayMessage(App.Msg);


                                handle_canbus_service_setup((void *) pvCANSlot);

                                pActuator->homingSeq = 4040;
                                pCANSlot->state = DEV_STATE_HOMING_DONE;
                                return 1;
                                
                            } else {
                                if (xTaskGetTickCount() - pActuator->homingStartTime >= 10 * 1000) {
                                    // Generazione Allarme
                                    if (generate_alarm((char*) "CAN Unable to read position", 6102, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                                    }
                                    pCANSlot->state = DEV_STATE_HOMING_DONE;
                                    pActuator->homingSeq = 5000;
                                    return -1;
                                }
                                return -1;
                            }
                        } else {
                            if (xTaskGetTickCount() - pActuator->homingStartTime >= 10 * 1000) {
                                // Generazione Allarme
                                if (generate_alarm((char*) "CAN Unable to read position", 6103, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                                }
                                pCANSlot->state = DEV_STATE_HOMING_DONE;
                                pActuator->homingSeq = 5000;
                                return -1;
                            }
                            return -1;
                        }
                        break;






                    case 4040: {
                        
                        ///////////////////////////////////////////////////////////////////////////////////////
                        // Disable Speed & Torque Limit P1-02 = 0x11     (P1.02=11 Speed and Torque enabled)
                        // N.B.: Nel canopen 0x2xyz = value  ->   Px.yz = value
                        //
                        // Dev'essere fatto a Servo-OFF
                        //
                        int16_t limitSpeedTorque = 0x0000;
                        
                        if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x2102, (int16_t) 0, (int8_t *) & limitSpeedTorque, (int8_t)sizeof (limitSpeedTorque), true) < 0) {
                            // Generazione Allarme
                            if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                            }
                            pCANSlot->state = DEV_STATE_HOMING_DONE;
                            pActuator->homingSeq = 5000;
                            return -1;
                        }

                        if (handle_actuator_prepare_for_run((void*) pActuator, CONTROL_WORD_RESET) < 0) {
                            /////////////////////////
                            // Generazione Allarme
                            //
                            if (generate_alarm((char*) "CAN Stop AC Servo Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                            }
                            // Homing failed
                            pActuator->homingSeq = 5000;
                            pCANSlot->state = DEV_STATE_HOMING_DONE;
                            return -1;
                        }
                        
                        pActuator->homingSeq = 4050;
                        return 1;                        
                        break;
                    }
                        
                        
                    case 4050:
                        //////////////////////////////////////////
                        // Posizionamento sulla quota di zero
                        //
                        pActuator->readCounter = 0;
                        pActuator->driverPosCount = 0;
                        pActuator->homingStartTime = xTaskGetTickCount();
                        pActuator->homingSeq = 4060;
                        return 1;
                        break;
                        
                        
                        
                    case 4060:
                        ///////////////////////////////////
                        // Comando Lettura posizione
                        //                        
                        if (xTaskGetTickCount() - pActuator->homingStartTime >= 50) {
                            if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x6064, (int16_t) 0, (int8_t *) NULL, (int8_t) 1, false) < 0) {
                                // Generazione Allarme
                                if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                                }
                                pCANSlot->state = DEV_STATE_HOMING_DONE;
                                pActuator->homingSeq = 5000;
                                return -1;
                            } else {
                                pActuator->homingStartTime = xTaskGetTickCount();
                                pCANSlot->state = DEV_STATE_HOMING_DONE;
                                pActuator->homingSeq = 4070;
                                return 1;
                            }
                        }
                        break;
                        
                        
                        
                    case 4070:
                        ///////////////////////////////////
                        // Lettura posizione
                        //
                        if (xrt_recive_udp(pCANSlot->ip, pCANSlot->port, pCANSlot->port, (uint8_t*) pCANSlot->data, pCANSlot->data_size, 0) > 0) {
                            if (pActuator->driverPosCount > 250) {
                                float newPosition = 0.0f;
                                int32_t offsetTurnsPPT = 0, offsetPulsesPPT = 0;

                                actuator_encoder_to_position((void *) pActuator, (int32_t) pActuator->driverTurns, (int32_t) pActuator->driverPulses, &newPosition);
                                actuator_handle_read_position((void *) pActuator, newPosition, false);


                                // posizione virtuale (0...1000)
                                update_actuator_virtual_pos(pActuator);

                                if (fabs(newPosition) <= MAX_HOMING_POSITION_ERROR) {
                                    pActuator->step = STEP_READY;
                                    pActuator->position = pActuator->homing_position;
                                    pActuator->homingSeq = 4100;
                                    pCANSlot->state = DEV_STATE_HOMING_DONE;
                                    return 1;
                                } else {
                                    // Posizionamento sullo zero
                                    pActuator->homingStartTime = xTaskGetTickCount();
                                    // pActuator->homingSeq = 4060;
                                    // pCANSlot->state = DEV_STATE_HOMING_DONE;
                                    
                                    snprintf(App.Msg, App.MsgSize, "[%s Moving to Zero from altered cur pos:%0.3f %s]     \r", (char*) ANSI_COLOR_BLUE, newPosition, (char*) ANSI_COLOR_RESET);
                                    vDisplayMessage(App.Msg);
                                    
                                    pActuator->step = STEP_MOVING;
                                    if (pActuator->homing_position == OFF) {
                                        pCANSlot->pendingCommand = -1;
                                    } else if (pActuator->homing_position == ON) {
                                        pCANSlot->pendingCommand = 1;
                                    }
                                    
                                    pCANSlot->state = DEV_STATE_STREAMING_DONE; // DEV_STATE_CMD_INIT;
                                    return 1;                                    
                                }
                            }
                        } else {
                            if (xTaskGetTickCount() - pActuator->homingStartTime >= App.CanBusStreamTimeoutMS) {
                                // Timeout
                                // Generazione Allarme
                                if (generate_alarm((char*) "CAN Communication Timeout Reading POS", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                                }
                                pCANSlot->state = DEV_STATE_HOMING_DONE;
                                pActuator->homingSeq = 5000;
                                return -1;
                            }
                        }
                        break;

                        
                    case 4100:
                        pActuator->homingSeq = 0;
                        pCANSlot->state = DEV_STATE_INIT_STREAM;
                        return 1;
                        break;
                        
                        


                        
                        
                        
                    case 5000:
                    {
                        // Home failed
                        pActuator->homingRequest = 0;
                        pActuator->homingDone = false;
                        pActuator->homingPulsesPPT = 0;
                        pActuator->homingTurnsPPT = 0;
                        pActuator->step = STEP_ERROR;
                        pActuator->error = 5000;

                        snprintf(str, sizeof (str), "HOMING of %s Failed", pActuator->name);
                        if (generate_alarm((char*) str, 6009, 0, (int32_t) ALARM_ERROR, 0 + 1) < 0) {
                        }
                        pCANSlot->homingRequest = 0;
                        pCANSlot->state = DEV_STATE_INIT_STREAM;
                        return -1;
                        break;
                    }
                }
            }
        }


        return 1;
    }

    return 0;
}




///////////////////////////////////
// Operazioni di primo setup
//
int32_t handle_canbus_service_first_setup(void *pvCANSlot) {
    if (pvCANSlot) {
        CANSlot *pCANSlot = (CANSlot *) pvCANSlot;

        for (int32_t iact = 0; iact < pCANSlot->nActuators; iact++) {
            LP_ACTUATOR pActuator = (LP_ACTUATOR) pCANSlot->pActuators[iact];
            int8_t str[32];
            if (pActuator) {

                ///////////////////////////////////
                // Reset nodes
                //
                str[0] = '<'; str[1] = '<'; str[2] = '<'; str[3] = 0;
                if (xrt_send_udp(pCANSlot->ip, (uint16_t) pCANSlot->port, (uint16_t) pCANSlot->port, (uint8_t *) str, (uint16_t) 4, (uint8_t) 0) < 0) {
                    if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0+1) < 0) {
                    }
                } else {                
                    // snprintf(App.Msg, App.MsgSize, "[RI=%d]", (int32_t)ri_ms); vDisplayMessage(App.Msg);
                }



                usleep(5*1000);
            }
            
        }
        
        return 1;        
    }
            
            
    
    return -1;   
}




int32_t handle_canbus_service_setup(void *pvCANSlot) {
    if (pvCANSlot) {
        CANSlot *pCANSlot = (CANSlot *) pvCANSlot;

        int32_t targetTurnsPPT = 0, targetPulsesPPT = 0, targetTurnsPPT2 = 0, targetPulsesPPT2 = 0;
        float rpos = 0.0f;


        pCANSlot->setupRequest = 0;

        for (int32_t iact = 0; iact < pCANSlot->nActuators; iact++) {
            LP_ACTUATOR pActuator = (LP_ACTUATOR) pCANSlot->pActuators[iact];
            if (pActuator) {
                int32_t speed_rpm = pActuator->speed_auto1;
                int32_t acc_ms = pActuator->acc_auto1, dec_ms = acc_ms;
                int32_t speed_rpm2 = pActuator->speed_auto2;
                int32_t acc_ms2 = pActuator->acc_auto2, dec_ms2 = acc_ms2;
                int32_t homingTurns = 0;
                int32_t homingPulses = 0;
                int16_t objIndex = 0;
                int8_t str[32];
                
               
                
                //////////////////////////////////////////////////////////////////////////////////////////
                //
                // P2-11	109	DI2 = SPDLM o TRQLM [se speed o torque mode]) [default=104 CCLR] (Fatto dal SW.setup)
                //                      1 = contatto normalmente aperto
                //                      0 = contatto normalmente chiuso : speed limit sempre abilitato
                //                      10 = SPDLM (l'ingresso DI2 o SoftDI2 comanda il SPDLM= speed limit)	
                //                      09 = TRQLM (l'ingresso DI2 o SoftDI2 comanda il TRQLM= torque limit)


                // N.B.: In modalita speed control il DI2 viene usato per comandare il TRQLM
                int16_t di211_value = 0x0109;
                objIndex = 0x2000 + 0x200 + 11;

                if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) objIndex, (int16_t) 0, (int8_t *) & di211_value, (int8_t)sizeof (di211_value), true) < 0) {
                    // Generazione Allarme
                    if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                    }
                    // reinizializa il flusso
                    pCANSlot->state = DEV_STATE_INIT;
                    return -1;
                }
                
                
                //////////////////////////////////////////////////////////////////////////////////////////
                //
                // P2-12	016	DI3 = TCM0 [default=116] (Fatto dal SW.setup) 
                //                      0 = contatto normalmente chiuso : torque command sempre abilitato
                //                      1 = contatto normalmente aperto
                //                      16 = TCM0 (l'ingresso DI3 o SoftDI3 comanda il TCM0 = Torque command)
                
                int16_t di212_value = 0x0116;
                objIndex = 0x2000 + 0x200 + 12;
                if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) objIndex, (int16_t) 0, (int8_t *) & di212_value, (int8_t)sizeof (di212_value), true) < 0) {
                    // Generazione Allarme
                    if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                    }
                    // reinizializa il flusso
                    pCANSlot->state = DEV_STATE_INIT;
                    return -1;
                }

                usleep(5*1000);

                


                
                ///////////////////////////////////////////////////////////
                // Reset AL013 - Emergency stop - Data Size: 16-bit
                // P2-15 = 0
                // P2-16 = 0
                // P2-17 = 0

                int16_t em_stop_mode = 0;
                objIndex = 0x2000 + 0x200 + 15;

                if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) objIndex, (int16_t) 0, (int8_t *) & em_stop_mode, (int8_t)sizeof (em_stop_mode), true) < 0) {
                    // Generazione Allarme
                    if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                    }
                    // reinizializa il flusso
                    pCANSlot->state = DEV_STATE_INIT;
                    return -1;
                }
                objIndex = 0x2000 + 0x200 + 16;
                if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) objIndex, (int16_t) 0, (int8_t *) & em_stop_mode, (int8_t)sizeof (em_stop_mode), true) < 0) {
                    // Generazione Allarme
                    if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                    }
                    // reinizializa il flusso
                    pCANSlot->state = DEV_STATE_INIT;
                    return -1;
                }
                objIndex = 0x2000 + 0x200 + 17;
                if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) objIndex, (int16_t) 0, (int8_t *) & em_stop_mode, (int8_t)sizeof (em_stop_mode), true) < 0) {
                    // Generazione Allarme
                    if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                    }
                    // reinizializa il flusso
                    pCANSlot->state = DEV_STATE_INIT;
                    return -1;
                }

                usleep(5*1000);

                
                
                
                //////////////////////////////////////////////////
                // Enable DI setted By software (SDI)
                // P3.06 = 3FFF = ALL DI controlled by software
                // P3.06 = 0*1 + 1*2 + 1*4   = DI2 e DI3 controlled by software
                //
                // int16_t sdi_mode = 0*1 + 1*2 + 1*4 + 0*8 + 0*16 + 0*32 + 0*64 + 0*128; // 0x3FFF;
                int16_t sdi_mode = 0x3FFF;
                objIndex = 0x2000 + 0x300 + 6;

                if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) objIndex, (int16_t) 0, (int8_t *) & sdi_mode, (int8_t)sizeof (sdi_mode), true) < 0) {
                    // Generazione Allarme
                    if (generate_alarm((char*) "CANBUS: Unbale to set SDI mode", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                    }
                }

                usleep(5*1000);


                ///////////////////////////////////////////////////////////
                // P4-07 : Set SDI2 + SDI3 = 0
                //
                //
                // int16_t sdi_value = 1*1 + 0*2 + 0*4 + 0*8 + 0*16 + 0*32 + 0*64 + 0*128; // NO
                // int16_t sdi_value = 1*1 + 1*2 + 1*4 + 0*8 + 0*16 + 0*32 + 0*64 + 0*128; // Limite torcente ancora attivo
                int16_t sdi_value = 1*1 + 0*2 + 1*4 + 1*8 + 1*16 + 1*32 + 1*64 + 1*128; // OK
                objIndex = 0x2000 + 0x400 + 7;

                if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) objIndex, (int16_t) 0, (int8_t *) & sdi_value, (int8_t)sizeof (sdi_value), true) < 0) {
                    // Generazione Allarme
                    if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                    }
                    // reinizializa il flusso
                    pCANSlot->state = DEV_STATE_INIT;
                    return -1;
                }




                
                
                ///////////////////////////////////////////////////////
                // P3-09	9FFFF	Sincronismo x Interpolazione
                //
                int16_t syncSetting = 0x9FFF;
                objIndex = 0x2000 + 0x300 + 9;
                if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) objIndex, (int16_t) 0, (int8_t *) & syncSetting, (int8_t)sizeof (syncSetting), true) < 0) {
                    // Generazione Allarme
                    if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                    }
                    // reinizializa il flusso
                    pCANSlot->state = DEV_STATE_INIT;
                    return -1;
                }

                usleep(5*1000);




                

                ////////////////////////////////////////////////////////////////
                // Startup nodes : Go online : Enter operational and sync
                //
                str[0] = '>';
                str[1] = '>';
                str[2] = '>';
                str[3] = (int8_t)pActuator->Id;
                str[4] = 0;
                if (xrt_send_udp(pCANSlot->ip, (uint16_t) pCANSlot->port, (uint16_t) pCANSlot->port, (uint8_t *) str, (uint16_t) 4, (uint8_t) 0) < 0) {
                    if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                    }
                } else {
                    // snprintf(App.Msg, App.MsgSize, "[RI=%d]", (int32_t)ri_ms); vDisplayMessage(App.Msg);
                }



                /*                
                ///////////////////////////////////
                // Reset nodes
                //
                str[0] = '<'; str[1] = '<'; str[2] = '<'; str[3] = 0;
                if (xrt_send_udp(pCANSlot->ip, (uint16_t) pCANSlot->port, (uint16_t) pCANSlot->port, (uint8_t *) str, (uint16_t) 4, (uint8_t) 0) < 0) {
                    if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0+1) < 0) {
                    }
                } else {                
                    // snprintf(App.Msg, App.MsgSize, "[RI=%d]", (int32_t)ri_ms); vDisplayMessage(App.Msg);
                }
                 * */
                
                
                usleep(5*1000);



                
                
                ///////////////////////////////////////////
                //
                // Posizioni Tabella in Pulsazioni
                // N.B.: Non necessaria con il canopen
                //

                actuator_position_to_encoder(pActuator, pActuator->end_rpos, &targetTurnsPPT, &targetPulsesPPT);
                actuator_position_to_encoder(pActuator, pActuator->start_rpos, &targetTurnsPPT2, &targetPulsesPPT2);


                // Direzione Homing
                if (pActuator->homing_position == OFF) {
                    // Da ON a OFF
                    homingTurns = targetTurnsPPT2 - targetTurnsPPT;
                    homingPulses = targetPulsesPPT2 - targetPulsesPPT;
                } else {
                    // Da OFF a ON
                    homingTurns = targetTurnsPPT - targetTurnsPPT2;
                    homingPulses = targetPulsesPPT - targetPulsesPPT2;
                }




                /////////////////////////////
                // Reset Control Word
                //
                if (handle_actuator_prepare_for_run(pActuator, CONTROL_WORD_RESET) < 0) {
                }



                // if (pActuator->step == STEP_UNINITIALIZED) {
                if (handle_actuator_position_mode((void *) pActuator, targetTurnsPPT, targetPulsesPPT, speed_rpm, acc_ms, dec_ms, targetTurnsPPT2, targetPulsesPPT2, speed_rpm2, acc_ms2, dec_ms2, homingTurns, homingPulses, pActuator->homing_speed_rpm) < 0) {
                    // return -1;
                    if (generate_alarm((char*) "CANBUS: handle_actuator_position_mode() Error", 6100, 0, (int32_t) /*ALARM_FATAL_ERROR*/ALARM_WARNING, 0 + 1) < 0) {
                    }
                }



                usleep(5*1000);
                
                
                // 6080h Max motor speed Unit:rpm
                uint32_t cspeed = (uint32_t) ((float) MAX(pActuator->speed_auto1, pActuator->speed_auto2));
                if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x6080, (int16_t) 0, (int8_t *) & cspeed, (int8_t) sizeof (cspeed), true) < 0) {
                }

                usleep(5*1000);
                
                // 0x607F Max profile velocity Unit:0.1rpm
                cspeed = (uint32_t) ((float) cspeed * 10.0f);
                if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x607F, (int16_t) 0, (int8_t *) & cspeed, (int8_t) sizeof (cspeed), true) < 0) {
                }

                
                usleep(5*1000);

                //////////////////////////////////////
                // Avvio driver (comando Servo ON)
                //
                if (!pActuator->disabled) {
                    if (handle_actuator_servo_on((void *) pActuator) < 0) {
                        // return -1;
                        if (generate_alarm((char*) "CANBUS: handle_actuator_start() Error", 6101, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                        }
                    }
                }

                usleep(5*1000);
                
                ///////////////////////////////////////////
                // Tempo minimo Invio messagggio CAN
                //
                pCANSlot->si_ms = 5;   // OK testato con 9h di produzione a 1200 cicli/h      *** SeedDuino CANBUS + Mega256 + Ethernet I (wz5100)
                // pCANSlot->si_ms = 4; // OK testato con 16h di produzione a 1200 cicli/h  *** SeedDuino CANBUS + Mega256 + Ethernet I (wz5100)
                // uint8_t si_ms = 3;   // Dopo 1 e 10min blocco
                // uint8_t si_ms = 2;   // Dopo 10min blocco
                // uint8_t si_ms = 1;   // Letture fasulle

                // DUE-CANBUS : non risolve
                //
                // pCANSlot->si_ms = 20;
                // pCANSlot->si_ms = 2;
                //
                // DUE-CANBUS : Invio in ridondanza del frame a 2ms non risolve
                // Inserita l'attesa della recezione prima del sucessivo invio : non risolve
                //  IL PROBELMA E' ETHERNED2 CHE PERDE IL PACCHETTO
                pCANSlot->si_ms = 1;

                str[0] = 'S';
                str[1] = 'I';
                str[2] = '=';
                memcpy(&str[3], &pCANSlot->si_ms, 1);
                if (xrt_send_udp(pCANSlot->ip, (uint16_t) pCANSlot->port, (uint16_t) pCANSlot->port, (uint8_t *) str, (uint16_t) 4, (uint8_t) 0) < 0) {
                    if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                    }
                } else {
                    // snprintf(App.Msg, App.MsgSize, "[SI=%d]", (int32_t)si_ms); vDisplayMessage(App.Msg);
                }

                usleep(5*1000);
                
                ///////////////////////////////////////////
                // Tempo minimo Recezione messagggio CAN
                //
                // uint8_t ri_ms = 5;   // OK testato con 9h di produzione a 1200 cicli/h   *** SeedDuino CANBUS + Mega256 + Ethernet I (wz5100)
                pCANSlot->ri_ms = 4; // OK testato con 16h di produzione a 1200 cicli/h     *** SeedDuino CANBUS + Mega256 + Ethernet I (wz5100)
                // uint8_t ri_ms = 3;   // Dopo 1 e 10min blocco
                // uint8_t ri_ms = 2;   // Dopo 10min blocco
                // uint8_t ri_ms = 1;   // Letture fasulle


                // DUE-CANBUS : test
                pCANSlot->ri_ms = 1;
        
                str[0] = 'R';
                str[1] = 'I';
                str[2] = '=';
                memcpy(&str[3], &pCANSlot->ri_ms, 1);
                if (xrt_send_udp(pCANSlot->ip, (uint16_t) pCANSlot->port, (uint16_t) pCANSlot->port, (uint8_t *) str, (uint16_t) 4, (uint8_t) 0) < 0) {
                    if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                    }
                } else {
                    // snprintf(App.Msg, App.MsgSize, "[RI=%d]", (int32_t)ri_ms); vDisplayMessage(App.Msg);
                }


                usleep(5*1000);


                ///////////////////////////////////
                // Lettura posizione
                //
                if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x6064, (int16_t) 0, (int8_t *) NULL, (int8_t) 1, false) < 0) {
                    /////////////////////////
                    // Generazione Allarme
                    //
                    if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                    }

                } else {

                    usleep(3 * 1000);

                    if (xrt_recive_udp(pCANSlot->ip, pCANSlot->port, pCANSlot->port, (uint8_t*) pCANSlot->data, pCANSlot->data_size, 0) > 0) {

                        if (pActuator->readCounter) {
                            float newPosition = 0.0f;

                            actuator_encoder_to_position((void *) pActuator, (int32_t) 0, (int32_t) pActuator->driverPosition, (float*) &newPosition);

                            actuator_handle_read_position((void *) pActuator, newPosition, true);

                            // velocita
                            // pActuator->speed = (float) pCANSlot->data[0];


                            // Asse pronto
                            if (pActuator->step == STEP_UNINITIALIZED || pActuator->step == STEP_ERROR || pActuator->step == STEP_STOPPED) {
                                pActuator->step = STEP_READY;
                            }

                            pCANSlot->setupDone = 1;

                        } else {
                            if (generate_alarm((char*) "CAN: Actuator's position not read", 6103, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                            }
                        }

                        pCANSlot->data_available = 0;

                    } else {
                        // Dato non acora disponibile : rimane sulla sequenza
                        pCANSlot->streamErrorCount++;
                        if (pCANSlot->streamErrorCount > 3) {
                            // reinizializa il flusso
                            pCANSlot->state = DEV_STATE_INIT;
                            if (generate_alarm((char*) "CAN: Unable to read Actuator's position", 6103, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                            }
                        }
                        return -1;
                    }
                }
            }
        }

        int shouldHoming = 0;
        for (int32_t iact = 0; iact < pCANSlot->nActuators; iact++) {
            LP_ACTUATOR pActuator = (LP_ACTUATOR) pCANSlot->pActuators[iact];
            if (pActuator) {
                
                if (pActuator->step == STEP_ERROR || pActuator->step == STEP_STOPPED) {
                    pActuator->step = STEP_READY;
                }
                
                if (!pActuator->homingDone) {
                    shouldHoming++;
                }
            }
        }

        ////////////////////////////////////////////////////////////////////////
        // N.B.: L'azzeramento dell'asse è fatto manualmente o dal recovery
        //
        if (shouldHoming) {
            // Esegue l'homing degli assi
            // pCANSlot->state = DEV_STATE_HOMING_INIT;            
        }

        // inizializa il flusso
        pCANSlot->state = DEV_STATE_INIT_STREAM;



        if (!App.CANRunLoop) {
            pCANSlot->state = DEV_STATE_CLOSING;
        }


        return 1;
    }

    return 0;
}

int32_t handle_canbus_service_stop(void *pvCANSlot) {
    if (pvCANSlot) {
        CANSlot *pCANSlot = (CANSlot *) pvCANSlot;

        pCANSlot->stopRequest = 0;




        /*
        603Fh VAR Error Code UNSIGNED16 RO Y
        6040h VAR Controlword UNSIGNED16 RW Y
        6041h VAR Statusword UNSIGNED16 RO Y
        605Bh VAR Shutdown option code INTEGER16 RW N
        605Eh VAR Fault reaction option code INTEGER16 RW N
        6060h VAR Modes of operation INTEGER8 RW Y
        6061h VAR Modes of operation display INTEGER8 RO Y
        6062h VAR Position demand value [PUU] INTEGER32 RO Y
        6063h VAR Position actual value [increment] INTEGER32 RO Y
        6064h VAR Position actual value INTEGER32 RO Y
        6065h VAR Following error window UNSIGNED32 RW Y
        6067h VAR Position windows UNSIGNED32 RW Y
        6068h VAR Position window time UNSIGNED16 RW Y
         */



        ///////////////////////////////////////////////
        // (Stop motor)
        //
        for (int32_t iact = 0; iact < pCANSlot->nActuators; iact++) {
            LP_ACTUATOR pActuator = (LP_ACTUATOR) pCANSlot->pActuators[iact];

            handle_actuator_servo_stop((void *) pActuator);
        }

        pCANSlot->state = DEV_STATE_SERVICE_OUT;


        return 1;
    }

    return 0;
}

int32_t handle_canbus_service_out(void *pvCANSlot) {
    if (pvCANSlot) {
        CANSlot *pCANSlot = (CANSlot *) pvCANSlot;

        App.CANRunning--;

        if (App.CANRunLoop) {
            if (pCANSlot->setupRequest == 1) {
                pCANSlot->state = DEV_STATE_SERVICE_SETUP;
                snprintf(App.Msg, App.MsgSize, "[Restart CANBUS service by request]");
                vDisplayMessage(App.Msg);

                
            } else if (pCANSlot->setupRequest == 1000) {
                pCANSlot->state = DEV_STATE_SERVICE_FIRST_SETUP;
                snprintf(App.Msg, App.MsgSize, "[Setup CANBUS service by request]");
                vDisplayMessage(App.Msg);
                    
                        
            } else if (pCANSlot->stopRequest) {
                pCANSlot->stopRequest = 0;

                // reinizializza : il purge non dovrebbe trovare dati
                pCANSlot->state = DEV_STATE_INIT_STREAM;
                
            } else if (pCANSlot->pendingCommand) {
                // Ripresa del servizio su comando in pendenza
                pCANSlot->setupRequest = 1;
                snprintf(App.Msg, App.MsgSize, "[Restart CANBUS service by pending command]");
                vDisplayMessage(App.Msg);

                // reinizializza : il purge non dovrebbe trovare dati
                pCANSlot->state = DEV_STATE_INIT_STREAM;

            } else if (pCANSlot->homingRequest) {
                // Ripresa del servizio su comando in pendenza
                pCANSlot->setupRequest = 1;
                snprintf(App.Msg, App.MsgSize, "[Restart CANBUS service by pending homing]");
                vDisplayMessage(App.Msg);

                // reinizializza : il purge non dovrebbe trovare dati
                pCANSlot->state = DEV_STATE_INIT_STREAM;
            }
        }

        for (int32_t iact = 0; iact < pCANSlot->nActuators; iact++) {
            LP_ACTUATOR pActuator = (LP_ACTUATOR) pCANSlot->pActuators[iact];
        }

        return 1;
    }

    return 0;
}

int32_t handle_canbus_service_reset(void *pvCANSlot) {
    if (pvCANSlot) {
        CANSlot *pCANSlot = (CANSlot *) pvCANSlot;
        char str[256];


        pCANSlot->resetRequest = 0;

        for (int32_t iact = 0; iact < pCANSlot->nActuators; iact++) {
            LP_ACTUATOR pActuator = (LP_ACTUATOR) pCANSlot->pActuators[iact];
            if (pActuator) {



                /////////////////////////////
                // Reset Control Word
                //
                if (handle_actuator_prepare_for_run(pActuator, CONTROL_WORD_RESET) < 0) {
                }




                // Avvio driver (comando Servo ON)
                if (!pActuator->disabled) {
                    if (handle_actuator_servo_on((void *) pActuator) < 0) {
                        // return -1;
                        if (generate_alarm((char*) "CANBUS: handle_actuator_start() Error", 6101, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                        }
                    }
                } else {
                    if (handle_actuator_servo_off((void *) pActuator) < 0) {
                        // return -1;
                        if (generate_alarm((char*) "CANBUS: handle_actuator_start() Error", 6101, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                        }
                    }
                }

                ///////////////////////////////////
                // Reset nodes
                //
                str[0] = '<';
                str[1] = '<';
                str[2] = '<';
                str[3] = 0;
                if (xrt_send_udp(pCANSlot->ip, (uint16_t) pCANSlot->port, (uint16_t) pCANSlot->port, (uint8_t *) str, (uint16_t) 4, (uint8_t) 0) < 0) {
                    if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                    }
                } else {
                    // snprintf(App.Msg, App.MsgSize, "[RI=%d]", (int32_t)ri_ms); vDisplayMessage(App.Msg);
                }
            }
        }

        // inizializa il flusso
        pCANSlot->state = DEV_STATE_INIT_STREAM;


        if (!App.CANRunLoop) {
            pCANSlot->state = DEV_STATE_CLOSING;
        }


        return 1;
    }

    return 0;
}





///////////////////////////////////////////////////
// Inizializza il comando di posizionamento
//

int32_t handle_canbus_send_sinlge_command(void *pvCANSlot, LP_ACTUATOR pActuator, int32_t tposition, int32_t cspeed, int32_t acc_ms, int32_t dec_ms, int32_t folowingErrPulses) {
    if (pvCANSlot) {
        CANSlot *pCANSlot = (CANSlot *) pvCANSlot;
        
        if (App.CanBusUseFullCommand) {

            /////////////////////////////
            // Unico comando completo
            //

            uint8_t fullStartCMD[64] = {0};
            uint32_t nfullStartCMD = 0;


            fullStartCMD[nfullStartCMD] = '+';
            nfullStartCMD += 1;

            fullStartCMD[nfullStartCMD] = (uint8_t) pActuator->stationId;
            nfullStartCMD += 1;

            fullStartCMD[nfullStartCMD] = 'T';
            nfullStartCMD += 1;
            memcpy(&fullStartCMD[nfullStartCMD], &tposition, 4);
            nfullStartCMD += 4;

            fullStartCMD[nfullStartCMD] = 'S';
            nfullStartCMD += 1;
            memcpy(&fullStartCMD[nfullStartCMD], &cspeed, 4);
            nfullStartCMD += 4;

            fullStartCMD[nfullStartCMD] = 'A';
            nfullStartCMD += 1;
            memcpy(&fullStartCMD[nfullStartCMD], &acc_ms, 4);
            nfullStartCMD += 4;

            fullStartCMD[nfullStartCMD] = 'D';
            nfullStartCMD += 1;
            memcpy(&fullStartCMD[nfullStartCMD], &dec_ms, 4);
            nfullStartCMD += 4;

            fullStartCMD[nfullStartCMD] = 'E';
            nfullStartCMD += 1;
            memcpy(&fullStartCMD[nfullStartCMD], &folowingErrPulses, 4);
            nfullStartCMD += 4;

            // Azzera il backup : sarà verificato dal feedback con il canbus
            pCANSlot->nfullStartCMD = 0;

            //////////////////////////////////////////////////
            // Test : a volte l'ETH2 perde il pacchetto
            /////////////////
            // Keep Alive
            //
            if (xrt_send_udp(pCANSlot->ip, (uint16_t) pCANSlot->port, (uint16_t) pCANSlot->port, (uint8_t *) ".", (uint16_t) 1, (uint8_t) 0) < 0) {
            }

            if (xrt_send_udp(pCANSlot->ip, (uint16_t) pCANSlot->port, (uint16_t) pCANSlot->port, (uint8_t *) fullStartCMD, (uint16_t) nfullStartCMD, (uint8_t) 0) < 0) {
                /////////////////////////
                // Generazione Allarme
                //
                if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                }
                // reinizializa il flusso
                pCANSlot->state = DEV_STATE_INIT;
                return -1;
            } else {
                ////////////////////////////////////////////
                // Assegnamento del timeout fullCMD format
                //
                pCANSlot->fullCMDFeedbackStartTime = xTaskGetTickCount();
                pCANSlot->fullCMDFeedbackError = 0;
                if (nfullStartCMD < FULL_START_CMD_SIZE) {
                    memcpy(pCANSlot->fullStartCMD, &fullStartCMD, nfullStartCMD);
                    pCANSlot->nfullStartCMD = nfullStartCMD;
                } else {
                    /////////////////////////
                    // Generazione Allarme
                    //
                    if (generate_alarm((char*) "Not enough room for backup Full Start CMD", 6999, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                    }
                    return -1;
                }
            }

            // snprintf(App.Msg, App.MsgSize, "[%s%s Target pos :%d %s]", (char*) ANSI_COLOR_BLUE, pActuator->name, (tposition), (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);


        } else {

            if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x607A, (int16_t) 0, (int8_t *) & tposition, (int8_t) sizeof (tposition), true) < 0) {
                /////////////////////////
                // Generazione Allarme
                //
                if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                }
                return -1;
            } else {
                if (machine.status == MANUAL) {
                    // snprintf(App.Msg, App.MsgSize, "[ %s%s Target pos :%d pulses (%d - %d)%s ]", (char*) ANSI_COLOR_YELLOW, pActuator->name, (targetPulsesPPT - targetPulsesPPT2), targetPulsesPPT, targetPulsesPPT2, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);
                } else if (machine.status == AUTOMATIC) {
                    // snprintf(App.Msg, App.MsgSize, "[ %s%s Target pos :%d pulses (%d - %d)%s ]", (char*) ANSI_COLOR_BLUE, pActuator->name, (targetPulsesPPT - targetPulsesPPT2), targetPulsesPPT, targetPulsesPPT2, (char*) ANSI_COLOR_RESET ); vDisplayMessage(App.Msg);
                }
            }
            if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x6081, (int16_t) 0, (int8_t *) & cspeed, (int8_t) sizeof (cspeed), true) < 0) {

                /////////////////////////
                // Generazione Allarme
                //
                if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                }
                return -1;
            }
            if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x6083, (int16_t) 0, (int8_t *) & acc_ms, (int8_t) sizeof (acc_ms), true) < 0) {
                /////////////////////////
                // Generazione Allarme
                //
                if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                }
                return -1;
            }
            if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x6084, (int16_t) 0, (int8_t *) & dec_ms, (int8_t) sizeof (dec_ms), true) < 0) {
                /////////////////////////
                // Generazione Allarme
                //
                if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                }
                return -1;
            }
            if (folowingErrPulses > 0) {
                if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x6065, (int16_t) 0, (int8_t *) & folowingErrPulses, (int8_t) sizeof (folowingErrPulses), true) < 0) {
                    /////////////////////////
                    // Generazione Allarme
                    //
                    if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                    }
                    return -1;
                }
            } else {
            }

            // Reset Control Word
            if (handle_actuator_prepare_for_run(pActuator, CONTROL_WORD_RESET) < 0) {
            }
        }

        return 1;
        
    }
    
    return 0;
}

int32_t handle_canbus_start_multiple_command(void *pvCANSlot) {
    
    if (pvCANSlot) {
        CANSlot *pCANSlot = (CANSlot *) pvCANSlot;
        if (App.CanBusUseFullCommand) {
            
            pCANSlot->fullStartCMD[0] = 0;
            pCANSlot->nfullStartCMD = 0;

            pCANSlot->fullStartCMD[pCANSlot->nfullStartCMD] = (uint8_t)0xA7; // '§'; // double S or template : Intepolation multiaxis
            pCANSlot->nfullStartCMD += 1;
            
            return 1;
        } else {
            return -1;
        }
    }
    return -1;
 }
            

int32_t handle_canbus_add_multiple_command(void *pvCANSlot, LP_ACTUATOR pActuator,
        int32_t tposition,                                  // Posizione finale (valore informativo)
        float start_angle, float end_angle, float radius,   // Angolo iniziale finale e raggio
        int32_t axisSinCosLin,                              // 0 = uso del coseno, 1 = uso del seno, 2 = lineare
        int32_t direction,                                  // 0 = uso CW orario, 1 = CCW antiorario
        int32_t precision_nsteps,                           // Precisione in numero di passi
        int32_t period_msec,                                // Periodo msec
        float feed_pulses_min,                              // Velocità media in pulsazioni / min
        int32_t cspeed,                                     // velocità massima
        int32_t acc_ms, int32_t dec_ms,                     // Accelerazione dec. in msec
        int32_t folowingErrPulses) {

    if (pvCANSlot) {
        CANSlot *pCANSlot = (CANSlot *) pvCANSlot;
        if (App.CanBusUseFullCommand) {
            /////////////////////////////
            // Unico comando completo
            //

            pCANSlot->fullStartCMD[pCANSlot->nfullStartCMD] = 'I';
            pCANSlot->nfullStartCMD += 1;
            pCANSlot->fullStartCMD[pCANSlot->nfullStartCMD] = (uint8_t) pActuator->stationId;
            pCANSlot->nfullStartCMD += 1;

            pCANSlot->fullStartCMD[pCANSlot->nfullStartCMD] = 'T';
            pCANSlot->nfullStartCMD += 1;
            memcpy(&pCANSlot->fullStartCMD[pCANSlot->nfullStartCMD], &tposition, 4);
            pCANSlot->nfullStartCMD += 4;

            pCANSlot->fullStartCMD[pCANSlot->nfullStartCMD] = 'S';
            pCANSlot->nfullStartCMD += 1;
            memcpy(&pCANSlot->fullStartCMD[pCANSlot->nfullStartCMD], &cspeed, 4);
            pCANSlot->nfullStartCMD += 4;

            pCANSlot->fullStartCMD[pCANSlot->nfullStartCMD] = 'A';
            pCANSlot->nfullStartCMD += 1;
            memcpy(&pCANSlot->fullStartCMD[pCANSlot->nfullStartCMD], &acc_ms, 4);
            pCANSlot->nfullStartCMD += 4;

            pCANSlot->fullStartCMD[pCANSlot->nfullStartCMD] = 'D';
            pCANSlot->nfullStartCMD += 1;
            memcpy(&pCANSlot->fullStartCMD[pCANSlot->nfullStartCMD], &dec_ms, 4);
            pCANSlot->nfullStartCMD += 4;

            pCANSlot->fullStartCMD[pCANSlot->nfullStartCMD] = 'E';
            pCANSlot->nfullStartCMD += 1;
            memcpy(&pCANSlot->fullStartCMD[pCANSlot->nfullStartCMD], &folowingErrPulses, 4);
            pCANSlot->nfullStartCMD += 4;


            
            
            // start_angle
            pCANSlot->fullStartCMD[pCANSlot->nfullStartCMD] = 's';
            pCANSlot->nfullStartCMD += 1;
            memcpy(&pCANSlot->fullStartCMD[pCANSlot->nfullStartCMD], &start_angle, 4);
            pCANSlot->nfullStartCMD += 4;

            // end_angle
            pCANSlot->fullStartCMD[pCANSlot->nfullStartCMD] = 'e';
            pCANSlot->nfullStartCMD += 1;
            memcpy(&pCANSlot->fullStartCMD[pCANSlot->nfullStartCMD], &end_angle, 4);
            pCANSlot->nfullStartCMD += 4;

            // radius : conversione in pulses per agevolare il calcolo del canbus
            int32_t radiusPulses = 0;
            int32_t turnsPPT = 0, pulsesPPT = 0;
            
            actuator_position_to_encoder((void *) pActuator, radiusPulses, &turnsPPT, &pulsesPPT);
            
                
            
            radiusPulses = ( radius / (pActuator->cam_ratio > 0.0f ? pActuator->cam_ratio : 1.0f) ) * pActuator->pulsesPerTurn;
            
            pCANSlot->fullStartCMD[pCANSlot->nfullStartCMD] = 'r';
            pCANSlot->nfullStartCMD += 1;
            memcpy(&pCANSlot->fullStartCMD[pCANSlot->nfullStartCMD], &radiusPulses, 4);
            pCANSlot->nfullStartCMD += 4;

            // precision nsteps
            pCANSlot->fullStartCMD[pCANSlot->nfullStartCMD] = 'n';
            pCANSlot->nfullStartCMD += 1;
            memcpy(&pCANSlot->fullStartCMD[pCANSlot->nfullStartCMD], &precision_nsteps, 4);
            pCANSlot->nfullStartCMD += 4;

            // periood usec
            uint32_t period_usec = period_msec * 1000;
            pCANSlot->fullStartCMD[pCANSlot->nfullStartCMD] = 'p';
            pCANSlot->nfullStartCMD += 1;
            memcpy(&pCANSlot->fullStartCMD[pCANSlot->nfullStartCMD], &period_usec, 4);
            pCANSlot->nfullStartCMD += 4;

            // direction
            pCANSlot->fullStartCMD[pCANSlot->nfullStartCMD] = 'd';
            pCANSlot->nfullStartCMD += 1;
            pCANSlot->fullStartCMD[pCANSlot->nfullStartCMD] = (uint8_t)direction;
            pCANSlot->nfullStartCMD += 1;

            // feed
            pCANSlot->fullStartCMD[pCANSlot->nfullStartCMD] = 'f';
            pCANSlot->nfullStartCMD += 1;
            memcpy(&pCANSlot->fullStartCMD[pCANSlot->nfullStartCMD], &feed_pulses_min, 4);
            pCANSlot->nfullStartCMD += 4;

            
            /////////////////////////////////////////////////////
            // Ultimo comando
            // aXis : assegna l'asse e incrementa il contantore
            //
            pCANSlot->fullStartCMD[pCANSlot->nfullStartCMD] = 'x';
            pCANSlot->nfullStartCMD += 1;
            pCANSlot->fullStartCMD[pCANSlot->nfullStartCMD] = (uint8_t)axisSinCosLin;
            pCANSlot->nfullStartCMD += 1;

            
        } else {
            return -1;
        }
    }

    return 0;
}




int32_t handle_canbus_send_multiple_command(void *pvCANSlot) {
    if (pvCANSlot) {
        CANSlot *pCANSlot = (CANSlot *) pvCANSlot;
        if (App.CanBusUseFullCommand) {
    
            //////////////////////////////////////////////////
            // Test : a volte l'ETH2 perde il pacchetto
            /////////////////
            // Keep Alive
            //
            if (xrt_send_udp(pCANSlot->ip, (uint16_t) pCANSlot->port, (uint16_t) pCANSlot->port, (uint8_t *) ".", (uint16_t) 1, (uint8_t) 0) < 0) {
            }

            if (xrt_send_udp(pCANSlot->ip, (uint16_t) pCANSlot->port, (uint16_t) pCANSlot->port, (uint8_t *) pCANSlot->fullStartCMD, (uint16_t) pCANSlot->nfullStartCMD, (uint8_t) 0) < 0) {
                /////////////////////////
                // Generazione Allarme
                //
                if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                }
                // reinizializa il flusso
                pCANSlot->state = DEV_STATE_INIT;
                return -1;
            } else {
                ////////////////////////////////////////////
                // Assegnamento del timeout fullCMD format
                //
                pCANSlot->fullCMDFeedbackStartTime = xTaskGetTickCount();
                pCANSlot->fullCMDFeedbackError = 0;
            }
    
        } else {
            return -1;
        }
    }
    
    return 0;
}





///////////////////////////////////////////////////
// Inizializza il comando di posizionamento
//

int32_t handle_canbus_cmd_init(void *pvCANSlot) {
    if (pvCANSlot) {
        CANSlot *pCANSlot = (CANSlot *) pvCANSlot;


        if (!pCANSlot->setupDone) {
            if (generate_alarm((char*) "handle_canbus_cmd_init:setup NOT completed!", 6014, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
            }
            // reinizializa il flusso
            pCANSlot->state = DEV_STATE_INIT;

            return -1;
        }
        
        

        ////////////////////////////////////////////////////////
        // Preparazione comando Interpolazione multiasse
        //
        if (pCANSlot->pendingCommand == 11) {
            if (handle_canbus_start_multiple_command( (void *)pvCANSlot) < 0) {
            }            
        }
        

        for (int32_t iact = 0; iact < pCANSlot->nActuators; iact++) {
            LP_ACTUATOR pActuator = (LP_ACTUATOR) pCANSlot->pActuators[iact];

            if (pActuator) {
                float acc_rpms2 = 0.0f, dec_rpms2 = 0.0f, speed_rpm = 0.0f;
                char str[256];
                int32_t targetTurnsPPT = 0, targetPulsesPPT = 0, targetTurnsPPT2 = 0, targetPulsesPPT2 = 0;

                //////////////////////////////////////////////////////////////
                // Lettura : Current position:6064h (unit: PUU) : UINTEGER32
                //
                if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x6064, (int16_t) 0, (int8_t *) NULL, (int8_t) 1, false) < 0) {
                    /////////////////////////
                    // Generazione Allarme
                    //
                    if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                    }
                }

                // N.B: La tabella stabilisce quanti giri compiere, non la posizione di arrivo
                if (pCANSlot->pendingCommand == 1) {

                    pActuator->max_speed = 0.0f;

                    actuator_position_to_encoder(pActuator, pActuator->end_rpos, &targetTurnsPPT, &targetPulsesPPT);

                    // N.B.: NON viene usato il posizionamento assoluto con la control word (???)
                    actuator_position_to_encoder(pActuator, pActuator->cur_rpos, &targetTurnsPPT2, &targetPulsesPPT2);

                    if (pActuator->position == ON) {
                    } else if (pActuator->position == OFF) {
                        if (fabs(pActuator->cur_rpos - pActuator->start_rpos) > pActuator->start_rpos_toll) {
                            pCANSlot->streamErrorCount++;
                            if (pCANSlot->streamErrorCount > 10) {
                                if (machine.status == AUTOMATIC) {
                                    actuator_position_to_encoder(pActuator, pActuator->start_rpos, &targetTurnsPPT2, &targetPulsesPPT2);
                                    snprintf(App.Msg, App.MsgSize, "%s ignoring unexpected end pos :%0.3f (Err.:%0.3f)", pActuator->name, pActuator->cur_rpos, (pActuator->cur_rpos - pActuator->end_rpos));
                                    if (generate_alarm((char*) App.Msg, 6302, 0, (int32_t) ALARM_WARNING, 0 + 1) < 0) {
                                    }
                                }
                            } else if (pCANSlot->streamErrorCount > 1 && pCANSlot->streamErrorCount < 2) {
                                snprintf(App.Msg, App.MsgSize, "%s unexpected end pos :%0.3f (Err.:%0.3f)", pActuator->name, pActuator->cur_rpos, (pActuator->cur_rpos - pActuator->end_rpos));
                                if (generate_alarm((char*) App.Msg, 6303, 0, (int32_t) ALARM_ERROR, 0 + 1) < 0) {
                                }
                                return -1;
                            } else {
                                // Esce rimanedo sulla sequenza corrente
                                if (!pActuator->error) {
                                    pActuator->error = 6304;
                                    snprintf(App.Msg, App.MsgSize, "%s unexpected start pos :%0.3f (Err.:%0.3f)", pActuator->name, pActuator->cur_rpos, (pActuator->cur_rpos - pActuator->start_rpos));
                                    if (generate_alarm((char*) App.Msg, 6304, 0, (int32_t) ALARM_ERROR, 0 + 1) < 0) {
                                    }
                                }
                                return -1;
                            }
                        }
                    } else {
                    }



                } else if (pCANSlot->pendingCommand == -1) {
                    /////////////////////////////////
                    // N.B.: In Coordinate relative
                    //

                    pActuator->min_speed = 0.0f;

                    actuator_position_to_encoder(pActuator, pActuator->start_rpos, &targetTurnsPPT, &targetPulsesPPT);

                    // N.B.: NON viene usato il posizionamento assoluto con la control word (???)
                    actuator_position_to_encoder(pActuator, pActuator->cur_rpos, &targetTurnsPPT2, &targetPulsesPPT2);

                    if (pActuator->position == ON) {
                        if (fabs(pActuator->cur_rpos - pActuator->end_rpos) > pActuator->end_rpos_toll) {
                            pCANSlot->streamErrorCount++;
                            if (pCANSlot->streamErrorCount > 10) {
                                if (machine.status == AUTOMATIC) {
                                    snprintf(App.Msg, App.MsgSize, "%s ignoring unexpected end pos :%0.3f (Err.:%0.3f)", pActuator->name, pActuator->cur_rpos, (pActuator->cur_rpos - pActuator->end_rpos));
                                    actuator_position_to_encoder(pActuator, pActuator->end_rpos, &targetTurnsPPT2, &targetPulsesPPT2);
                                    if (generate_alarm((char*) App.Msg, 6304, 0, (int32_t) ALARM_WARNING, 0 + 1) < 0) {
                                    }
                                }
                            } else if (pCANSlot->streamErrorCount > 1 && pCANSlot->streamErrorCount < 2) {
                                snprintf(App.Msg, App.MsgSize, "%s unexpected end pos :%0.3f (Err.:%0.3f)", pActuator->name, pActuator->cur_rpos, (pActuator->cur_rpos - pActuator->end_rpos));
                                if (generate_alarm((char*) App.Msg, 6305, 0, (int32_t) ALARM_ERROR, 0 + 1) < 0) {
                                }
                                return -1;
                            } else {
                                // Esce rimanedo sulla sequenza corrente
                                if (!pActuator->error) {
                                    pActuator->error = 6306;
                                    snprintf(App.Msg, App.MsgSize, "%s unexpected end pos :%0.3f (Err.:%0.3f)", pActuator->name, pActuator->cur_rpos, (pActuator->cur_rpos - pActuator->end_rpos));
                                    if (generate_alarm((char*) App.Msg, 6306, 0, (int32_t) ALARM_ERROR, 0 + 1) < 0) {
                                    }
                                }
                                return -1;
                            }
                        }
                    } else if (pActuator->position == OFF) {
                    } else if (pActuator->position == USER_POSITION) {
                    } else if (pActuator->position == INTERPOLATE_POSITION) {
                    } else {
                    }

                    
                } else if (pCANSlot->pendingCommand == 1) {
                    /////////////////////////////////
                    // N.B.: In Coordinate relative
                    //
                    pActuator->max_speed = 0.0f;

                    actuator_position_to_encoder(pActuator, pActuator->end_rpos, &targetTurnsPPT, &targetPulsesPPT);

                    // N.B.: NON viene usato il posizionamento assoluto con la control word (???)
                    actuator_position_to_encoder(pActuator, pActuator->cur_rpos, &targetTurnsPPT2, &targetPulsesPPT2);

                    if (pActuator->position == ON) {
                    } else if (pActuator->position == OFF) {
                        if (fabs(pActuator->cur_rpos - pActuator->start_rpos) > pActuator->start_rpos_toll) {
                            pCANSlot->streamErrorCount++;
                            if (pCANSlot->streamErrorCount > 10) {
                                if (machine.status == AUTOMATIC) {
                                    actuator_position_to_encoder(pActuator, pActuator->start_rpos, &targetTurnsPPT2, &targetPulsesPPT2);
                                    snprintf(App.Msg, App.MsgSize, "%s ignoring unexpected end pos :%0.3f (Err.:%0.3f)", pActuator->name, pActuator->cur_rpos, (pActuator->cur_rpos - pActuator->end_rpos));
                                    if (generate_alarm((char*) App.Msg, 6302, 0, (int32_t) ALARM_WARNING, 0 + 1) < 0) {
                                    }
                                }
                            } else if (pCANSlot->streamErrorCount > 1 && pCANSlot->streamErrorCount < 2) {
                                snprintf(App.Msg, App.MsgSize, "%s unexpected end pos :%0.3f (Err.:%0.3f)", pActuator->name, pActuator->cur_rpos, (pActuator->cur_rpos - pActuator->end_rpos));
                                if (generate_alarm((char*) App.Msg, 6303, 0, (int32_t) ALARM_ERROR, 0 + 1) < 0) {
                                }
                                return -1;
                            } else {
                                // Esce rimanedo sulla sequenza corrente
                                if (!pActuator->error) {
                                    pActuator->error = 6304;
                                    snprintf(App.Msg, App.MsgSize, "%s unexpected start pos :%0.3f (Err.:%0.3f)", pActuator->name, pActuator->cur_rpos, (pActuator->cur_rpos - pActuator->start_rpos));
                                    if (generate_alarm((char*) App.Msg, 6304, 0, (int32_t) ALARM_ERROR, 0 + 1) < 0) {
                                    }
                                }
                                return -1;
                            }
                        }
                    } else {
                    }



                } else if (pCANSlot->pendingCommand == 2) {
                    /////////////////////////////////
                    // N.B.: In Coordinate relative
                    //
                    pActuator->min_speed = 0.0f;

                    actuator_position_to_encoder(pActuator, pActuator->target_rpos, &targetTurnsPPT, &targetPulsesPPT);

                    // N.B.: NON viene usato il posizionamento assoluto con la control word (???)
                    actuator_position_to_encoder(pActuator, pActuator->cur_rpos, &targetTurnsPPT2, &targetPulsesPPT2);

                    if (pActuator->position == ON) {
                    } else if (pActuator->position == OFF) {
                    } else {
                    }

                    
                } else if (pCANSlot->pendingCommand == 11) {
                    /////////////////////////////////
                    // N.B.: In Coordinate assolute
                    //

                    actuator_position_to_encoder(pActuator, pActuator->target_rpos, &targetTurnsPPT, &targetPulsesPPT);

                    targetTurnsPPT2 = 0;
                    targetPulsesPPT2 = 0;
                    // actuator_position_to_encoder(pActuator, pActuator->cur_rpos, &targetTurnsPPT2, &targetPulsesPPT2);

                    


                    
                } else {
                    ////////////////////////////
                    // Generazione Allarme                
                    //
                    snprintf(str, sizeof (str), "[CAN#%d] CMD UNkNOWN", pCANSlot->stationId);
                    if (generate_alarm((char*) str, 6015, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                    }
                    // reinizializa il flusso
                    pCANSlot->state = DEV_STATE_INIT;
                    return -1;
                }

                
                
                
                

                //////////////////////////////////////////////////////////////
                // Target position:607Ah (unit: PUU) : INTEGER32
                //
                int32_t tposition = (targetTurnsPPT - targetTurnsPPT2) * (int32_t) pActuator->pulsesPerTurn + (targetPulsesPPT - targetPulsesPPT2);



                if (pCANSlot->pendingCommand == 1) {
                    acc_rpms2 = pActuator->acc_auto1;
                    dec_rpms2 = pActuator->dec_auto1;
                    speed_rpm = pActuator->speed_auto1;
                } else if (pCANSlot->pendingCommand == -1) {
                    acc_rpms2 = pActuator->acc_auto2;
                    dec_rpms2 = pActuator->dec_auto2;
                    speed_rpm = pActuator->speed_auto2;
                } else if (pCANSlot->pendingCommand == 2) {
                    if (pActuator->target_rpos > pActuator->cur_rpos) {
                        acc_rpms2 = pActuator->acc_auto1;
                        dec_rpms2 = pActuator->dec_auto1;
                        speed_rpm = pActuator->speed_auto1;
                    } else {
                        acc_rpms2 = pActuator->acc_auto2;
                        dec_rpms2 = pActuator->dec_auto2;
                        speed_rpm = pActuator->speed_auto2;
                    }
                } else if (pCANSlot->pendingCommand == 11) {
                    // Interpolazione Lineare/Circolar
                    acc_rpms2 = pActuator->acc_auto1;
                    dec_rpms2 = pActuator->dec_auto1;
                    speed_rpm = pActuator->speed_auto1;
                } else {
                    /////////////////////////
                    // Generazione Allarme
                    //
                    if (generate_alarm((char*) "CAN Communication Unk Command", 6003, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                    }
                }


                ////////////////////////////////////////////////////////////////////////////////////////////////////////////
                //
                // N.B.: L'aggiornamento della velocità NON avviene dalle funzioni di I/O con l'intefaccia utente
                //          dev'essere passata al driver di volta in volta

                
                ////////////////////////////////////////////////////////////////////////////////////////////////////////////
                // 【Profile velocity:6081h】to profile velocity. (unit: pulse per second) : UNSIGNED32
                //
                uint32_t cspeed = (uint32_t) ((float) (speed_rpm?speed_rpm:1.0f) / 60.0f * (float) pActuator->pulsesPerTurn);


                ///////////////////////////////////////////////////////////////////////////////////////
                // Profile acceleration:6083h】(millisecond from 0 to 3000) : UNSIGNED32
                //
                uint32_t acc_ms = (uint32_t) ((3000.0f - 0.0f) / 9.55f / (float) (acc_rpms2 > 0 ? acc_rpms2 : 1) * 1000.0f);

                /////////////////////////////////////////////////////////////////////////////////////
                // Profile deceleration:6084】(millisecond from 0 to 3000) : UNSIGNED32
                //
                uint32_t dec_ms = (uint32_t) ((3000.0f - 0.0f) / 9.55f / (float) (dec_rpms2 ? dec_rpms2 : 1) * 1000.0f);


                // Avamnzamento in pulses / min
                float feed_pulses_min = pCANSlot->FeedMMMin * (float) pActuator->pulsesPerTurn / pActuator->cam_ratio;


                //////////////////////////////////////////////
                // Following error window (pulses) : 6065h
                //
                float folowingErrMM = (float) (pCANSlot->pendingCommand == 1 ? pActuator->follow_error1 : pActuator->follow_error2);
                int32_t folowingErrPulses = folowingErrMM / (pActuator->cam_ratio > 0.0f ? pActuator->cam_ratio : 1.0f) * (int32_t) pActuator->pulsesPerTurn;

                if (folowingErrPulses <= 5) {
                    folowingErrPulses = 3840000;
                }


                // Posizione passata al canbus per controllo runtime
                pActuator->lastTargetPos = tposition;
                pCANSlot->lastTargetPos = tposition;

                
                
                
                
                //////////////////////////////////////////////////////
                // Invio del comando relativo all'asse al CANBUS
                //
                if (pCANSlot->pendingCommand == 1 || pCANSlot->pendingCommand == -1 || pCANSlot->pendingCommand == 2 ) {
                
                    if (handle_canbus_send_sinlge_command( (void *)pvCANSlot, pActuator, tposition, cspeed, acc_ms, dec_ms, folowingErrPulses) < 0) {
                        if (generate_alarm((char*) " Unable to send command to CANBUS", 6014, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                        }
                        // reinizializa il flusso
                        pCANSlot->state = DEV_STATE_INIT;
                        return -1;
                    }
                    

                    
                    ///////////////////////////////////////////////
                    // Accodamento del comando relativo all'asse 
                    //
                } else if ( pCANSlot->pendingCommand == 11) {

                    if (handle_canbus_add_multiple_command((void *)pvCANSlot, pActuator,
                            tposition,
                            pCANSlot->StartAngleRad, pCANSlot->EndAngleRad, pCANSlot->RadiusMM,         // Angolo iniziale finale e raggio
                            pActuator->AxisSinCosLin,                               // 0 = uso del coseno, 1 = uso del seno, 2 = lineare
                            pCANSlot->Direction,                                    // 0 = uso CW orario, 1 = CCW antiorario
                            pCANSlot->PrecisionNSteps,                              // Numero di passi
                            pCANSlot->PeriodMsec,
                            feed_pulses_min,                                        // Avanzamento
                            cspeed,                                                 // velocità massima
                            acc_ms, dec_ms,
                            folowingErrPulses) < 0) {
                        if (generate_alarm((char*) " Unable to send command to CANBUS", 6014, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                        }
                        // reinizializa il flusso
                        pCANSlot->state = DEV_STATE_INIT;
                        return -1;                    
                    }
                }

            } else {
                if (generate_alarm((char*) "DEV_STATE_CMD_INIT:no actuator linked", 6014, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                }
                // reinizializa il flusso
                pCANSlot->state = DEV_STATE_INIT;
                return -1;
            }
            
            // Ultima sequenza pacchetto
            pActuator->CANSeqID = 0;
            pActuator->start_time1 = 0;
            pActuator->start_time2 = 0;
            pActuator->start_time3 = 0;
            
        }

 
                
        /////////////////////////////////////////////////////////////
        // Invio del comando relativo a tutti gli assi al CANBUS
        //
        if (pCANSlot->pendingCommand == 1 || pCANSlot->pendingCommand == -1 || pCANSlot->pendingCommand == 2 ) {
        } else if ( pCANSlot->pendingCommand == 11) {
            if (handle_canbus_send_multiple_command( (void *)pvCANSlot) < 0) {
                if (generate_alarm((char*) " Unable to send command to CANBUS", 6014, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                }
                // reinizializa il flusso
                pCANSlot->state = DEV_STATE_INIT;
                return -1;                
            }
        }                
    
    


        pCANSlot->start_time = xTaskGetTickCount();

        pCANSlot->doneCommand = 0;
        pCANSlot->readCount = 0;
        pCANSlot->preTimeout = 0;
        pCANSlot->state = DEV_STATE_CMD_INIT_SEND;


        return 1;
    }

    return 0;
}

int32_t handle_canbus_cmd_init_send(void *pvCANSlot) {
    if (pvCANSlot) {
        CANSlot *pCANSlot = (CANSlot *) pvCANSlot;
        int32_t res_send = -1;


        for (int32_t iact = 0; iact < pCANSlot->nActuators; iact++) {
            LP_ACTUATOR pActuator = (LP_ACTUATOR) pCANSlot->pActuators[iact];

            if (pActuator->protocol == CANOPEN_AC_SERVO_DELTA) {


                if (!pCANSlot->StreamingMode) {

                    /////////////////
                    // Start Stream
                    //
                    if (xrt_send_udp(pCANSlot->ip, (uint16_t) pCANSlot->port, (uint16_t) pCANSlot->port, (uint8_t *) "...", (uint16_t) 3, (uint8_t) 0) < 0) {
                        if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                        }
                    }

                    pCANSlot->StreamingMode = true;
                }




                if (App.CanBusUseFullCommand) {
                } else {
                    /////////////////////////
                    // Avvio motore
                    //
                    if (handle_actuator_prepare_for_run((void*) pActuator, 999) < 0) {
                        /////////////////////////
                        // Generazione Allarme
                        //
                        if (generate_alarm((char*) "CAN Start AC Servo Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                        }
                    }
                }


                pCANSlot->streamErrorCount = 0;
                pCANSlot->preTimeout = 0;
                pCANSlot->state = DEV_STATE_CMD_INIT_RECV;

                // Lunghezza risposta attesa
                pCANSlot->waitingRespSize = 6;
            }
        }


        return 1;
    }

    return 0;
}

int32_t handle_canbus_cmd_init_recv(void *pvCANSlot) {
    if (pvCANSlot) {
        CANSlot *pCANSlot = (CANSlot *) pvCANSlot;
        char str[256];



        // Stato di Comando in esecuzione
        if(pCANSlot->pendingCommand) {
            pCANSlot->runningCommand = pCANSlot->pendingCommand;
            pCANSlot->pendingCommand = 0;
        }

        if (xrt_recive_udp(pCANSlot->ip, pCANSlot->port, pCANSlot->port, (uint8_t*) pCANSlot->data, pCANSlot->data_size, 0) > 0) {
        }

        for (int32_t iact = 0; iact < pCANSlot->nActuators; iact++) {
            LP_ACTUATOR pActuator = (LP_ACTUATOR) pCANSlot->pActuators[iact];
            if (pActuator->readPositionCounter > 0) {
            }
        }




        if (App.CanBusUseFullCommand) {
            if (pCANSlot->fullCMDFeedbackStartTime > 0) {
                /////////////////////////////////////
                // Conteggio timeout innescato 
                //
                if (xTaskGetTickCount() - pCANSlot->fullCMDFeedbackStartTime > App.CanBusFullCmdFeedbackTimeoutMS) {
                    pCANSlot->fullCMDFeedbackError++;
                    if (pCANSlot->fullCMDFeedbackError <= App.CanBusFullCmdMacAttemps) {
                        
                        // Warning dopo il 2° tentativo
                        if (pCANSlot->fullCMDFeedbackError >= 1) {
                            snprintf((char*) str, sizeof (str), (char*) "[CAN#%d] FULLCMD FEEDBACK not recived : resending...." , pCANSlot->stationId );
                            if (generate_alarm((char*) str, 6009, 0, (int32_t) ALARM_WARNING, 0 + 1) < 0) {
                            }
                        }
                        if (xrt_send_udp(pCANSlot->ip, (uint16_t) pCANSlot->port, (uint16_t) pCANSlot->port, (uint8_t *) pCANSlot->fullStartCMD, (uint16_t) pCANSlot->nfullStartCMD, (uint8_t) 0) < 0) {
                            // Generazione Allarme
                            if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                            }
                            // reinizializa il flusso
                            pCANSlot->state = DEV_STATE_INIT;
                            return -1;
                        } else {                        
                            // Ri-Assegnamento del timeout fullCMD format e nuovo computo timeout
                            pCANSlot->fullCMDFeedbackStartTime = xTaskGetTickCount();
                            // pCANSlot->fullCMDFeedbackError = 0;
                            return 0;
                        }
                        
                    } else if (pCANSlot->fullCMDFeedbackError > App.CanBusFullCmdMacAttemps) {
                        snprintf((char*) str, sizeof (str), (char*) "[CAN#%d] FULLCMD FEEDBACK timeout" , pCANSlot->stationId );
                        if (generate_alarm((char*) str, 6009, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                        }
                        pCANSlot->state = DEV_STATE_INIT;
                        return -1;
                    }
                    
                } else {
                    // Attesa
                }
                
            } else {
                pCANSlot->state = DEV_STATE_CMD_FEEDBACK_SEND;
            }
        } else {
            // Ciclo lettura posizione
            pCANSlot->state = DEV_STATE_CMD_FEEDBACK_SEND;
        }
        

        return 1;
    }

    return 0;
}

int32_t handle_canbus_cmd_feedback_send(void *pvCANSlot) {
    if (pvCANSlot) {
        CANSlot *pCANSlot = (CANSlot *) pvCANSlot;

        char str[256];



        if (pCANSlot->StreamingMode) {
            for (int32_t iact = 0; iact < pCANSlot->nActuators; iact++) {
                LP_ACTUATOR pActuator = (LP_ACTUATOR) pCANSlot->pActuators[iact];

                if (pActuator->protocol == CANOPEN_AC_SERVO_DELTA) {
                    // } else if (pActuator->protocol == ) {
                }
            }

            pCANSlot->readCount = 0;
            pCANSlot->tickTocIOWatchDog = pCANSlot->start_time = pCANSlot->t1;
            pCANSlot->preTimeout = 0;
            pCANSlot->state = DEV_STATE_CMD_FEEDBACK_RECV;

        } else {
            // N.B.: L'invio continuo di messaggi blocca la scheda           
            pCANSlot->state = DEV_STATE_CMD_ERROR;
        }


        return 1;
    }

    return 0;
}

int32_t handle_canbus_cmd_feedback_recv(void *pvCANSlot) {
    if (pvCANSlot) {
        CANSlot *pCANSlot = (CANSlot *) pvCANSlot;

        int32_t targetPos = 0;
        char str[256];
        float newPosition = 0.0;


        if (xrt_recive_udp(pCANSlot->ip, pCANSlot->port, pCANSlot->port, (uint8_t*) pCANSlot->data, pCANSlot->data_size, 0) > 0) {

            for (int32_t iact = 0; iact < pCANSlot->nActuators; iact++) {
                LP_ACTUATOR pActuator = (LP_ACTUATOR) pCANSlot->pActuators[iact];

                // snprintf(App.Msg, App.MsgSize, "[%s board:%d - feedback read pos:%d %s]\n", (char*) ANSI_COLOR_MAGENTA, pCANSlot->boardId, (int32_t)pCANSlot->driverPosition, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);

                if (pActuator->readCounter) {

                    // Posizione raggiunta ?
                    if (pActuator->positionReached) {
                        //////////////////////////
                        // Termina la sequenza
                        // Il gestore degli attuattori analizza la poszione e processa il nuovo stato dell'attuatore
                        //

                        // snprintf(App.Msg, App.MsgSize, "[%s %s Position reached:%0.3f - Unable to add alarm %s]\n", (char*) ANSI_COLOR_YELLOW, pActuator->name, pActuator->cur_rpos, (char*) ANSI_COLOR_RESET);
                        // vDisplayMessage(App.Msg);

                        // Arrotonda la velocità letta
                        pActuator->speed = 0.0f;

                        if (pCANSlot->runningCommand) {
                            pCANSlot->doneCommand = pCANSlot->runningCommand;
                            pCANSlot->runningCommand = 0;
                        } else {
                        }

                        // posizione raggiunta fine sequenza
                        pCANSlot->state = DEV_STATE_CMD_DONE;

                    } else {

                        // Asse in stato di Errore : esce dalla catena del comando
                        if (pActuator->step == STEP_STOPPED || pActuator->step == STEP_ERROR) {
                            pCANSlot->state = DEV_STATE_SERVICE_STOP;
                            return -1;
                        }
                                
                        // Riesegue la lettura posizione
                        if (pCANSlot->stopRequest) {
                            pCANSlot->state = DEV_STATE_SERVICE_STOP;
                        } else {
                            // Ripete la lettura
                            pCANSlot->state = DEV_STATE_CMD_FEEDBACK_SEND;
                        }

                    }
                }
            }
        }


        if (xTaskGetTickCount() - pCANSlot->tickTocIOWatchDog > App.CanBusStreamTimeoutMS) {
            bool issueAlarm = false;
            char str[256];
            
            for (int32_t iact = 0; iact < pCANSlot->nActuators; iact++) {
                LP_ACTUATOR pActuator = (LP_ACTUATOR) pCANSlot->pActuators[iact];
                if (pActuator) {
                    if (pActuator->CheckFeedbackTimeout) {
                        issueAlarm = true;
                    }
                }                
            }
            
            /////////////////////////////
            // Comunicazione in timeout 
            //
            // Generazione Allarme
            //
            if (issueAlarm) {

                App.CANMeasurePostTimeout = xTaskGetTickCount();

                snprintf((char*) str, sizeof (str), (char*) "[CAN#%d] CMD FEEDBACK Recv timeout (%d)"
                        , pCANSlot->stationId
                        , (xTaskGetTickCount() - pCANSlot->start_time)
                        );

                if (generate_alarm((char*) str, 6009, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                }
                // handle_serial_borad_error(i, (char*) "timeout!", -1);

                // reinizializa il flusso
                pCANSlot->state = DEV_STATE_INIT;
                return -1;
            } else {
                snprintf((char*) str, sizeof (str), (char*) "[CAN#%d] CMD FEEDBACK Recv timeout (%d)"
                        , pCANSlot->stationId
                        , (xTaskGetTickCount() - pCANSlot->start_time)
                        );

                if (generate_alarm((char*) str, 6009, 0, (int32_t) ALARM_WARNING, 0 + 1) < 0) {
                }                
                // Ripete la lettura
                pCANSlot->state = DEV_STATE_CMD_FEEDBACK_SEND;
            }
        }




        //////////////////////////////////////////
        // Controllo del timeout del movimento
        //
        for (int32_t iact = 0; iact < pCANSlot->nActuators; iact++) {
            LP_ACTUATOR pActuator = (LP_ACTUATOR) pCANSlot->pActuators[iact];
            if (pActuator) {
                uint timeout_ms = 0;
                float target_pos = 0.0f;
                char str[256];

                    // Controllo stato dell'attuatore
                    if (pActuator->step == STEP_DONE || pActuator->step == STEP_ERROR || pActuator->step == STEP_STOPPED) {
                        // reinizializa il flusso
                        pCANSlot->state = DEV_STATE_INIT_STREAM;
                    }

                    
                if (pActuator->step == STEP_MOVING) {

                    if (pCANSlot->runningCommand == 1) {
                        timeout_ms = pActuator->timeout1_ms;
                        target_pos = pActuator->end_rpos;
                    } else if (pCANSlot->runningCommand == -1) {
                        timeout_ms = pActuator->timeout2_ms;
                        target_pos = pActuator->start_rpos;                       
                    } else if (pCANSlot->runningCommand == 2) {
                        timeout_ms = pActuator->timeout3_ms;
                        target_pos = pActuator->target_rpos;                       
                    } else if (pCANSlot->runningCommand == 11) {
                        timeout_ms = pActuator->timeout4_ms;
                        target_pos = pActuator->target_rpos;                       
                    }
                    
                    if (timeout_ms > 0) {
                        
                        if (pCANSlot->t1 - pActuator->start_time > timeout_ms) {

                            if (!(pActuator->rtOptions & 2)) {
                                pActuator->rtOptions += 2;
                            }

                            pActuator->error = 6012;

                            snprintf(str, sizeof (str), "Actuator %s timeout (side:%d time:%0.3fsec - step:%s - Status:%s) - Position:%0.3f/%0.3f - DriverPos.:%d-%d.%d"
                                    , pActuator->name
                                    , pCANSlot->runningCommand
                                    , (float) (pCANSlot->t1 - pActuator->start_time) / 1000.0f
                                    , (char*) get_actuator_step(pActuator)
                                    , (char*) get_actuator_driver_status((void*) pActuator)
                                    , pActuator->cur_rpos
                                    , target_pos
                                    , pActuator->driverPosition, pActuator->driverTurns, pActuator->driverPulses
                                    );
                            if (generate_alarm((char*) str, 6012, 0, (int32_t) ALARM_ERROR, 0 + 1) < 0) {
                            }
                            // reinizializa il flusso
                            pCANSlot->state = DEV_STATE_INIT_STREAM;
                        }
                    }
                }
            }
        }

        return 1;
    }

    return 0;
}



int32_t handle_canbus_cmd_done(void *pvCANSlot) {
    if (pvCANSlot) {
        CANSlot *pCANSlot = (CANSlot *) pvCANSlot;
        for (int32_t iact = 0; iact < pCANSlot->nActuators; iact++) {
            LP_ACTUATOR pActuator = (LP_ACTUATOR) pCANSlot->pActuators[iact];

            if (pActuator) {
                uint timewarn_ms = 0;
                char str[256];


                // Reset dati sequenza IDLE
                pActuator->IDLECounter = 0;
                pActuator->IDLESeq = 0;


                ///////////////////////////////
                // Verifica tempo esecuzione
                //
                if (pCANSlot->runningCommand == 1) {
                    timewarn_ms = pActuator->timewarn1_ms;
                } else if (pCANSlot->runningCommand == -1) {
                    timewarn_ms = pActuator->timewarn2_ms;
                }
                if (timewarn_ms > 0) {
                    if (xTaskGetTickCount() - pActuator->start_time > timewarn_ms) {
                        if (!(pActuator->rtOptions & 1)) {
                            pActuator->rtOptions += 1;
                            snprintf(str, sizeof (str), "Actuator %s time warning (side:%d time:%0.3fsec)", pActuator->name, pCANSlot->runningCommand, (float) timewarn_ms / 1000.0f);
                            if (generate_alarm((char*) str, 6010, 0, (int32_t) ALARM_WARNING, 0 + 1) < 0) {
                            }
                        }
                    }
                }
            }
        }


        //////////////////////////////////////////////////
        // Legge alcune volte la posizione sul ciclo IDLE
        //
        // pCANSlot->ReadSinglePos = 500;

        pCANSlot->ReadSinglePos = -1;



        // reinizializza : il purge non dovrebbe trovare dati
        pCANSlot->state = DEV_STATE_INIT_STREAM;

        return 1;
    }

    return 0;
}

int32_t handle_canbus_service_setup_speed_acc(void *pvCANSlot) {
    if (pvCANSlot) {
        CANSlot *pCANSlot = (CANSlot *) pvCANSlot;

        for (int32_t iact = 0; iact < pCANSlot->nActuators; iact++) {
            LP_ACTUATOR pActuator = (LP_ACTUATOR) pCANSlot->pActuators[iact];

            int32_t Error = 0;
            int32_t speed_rpm = (int32_t) pActuator->speed_auto1 > 0 ? (int32_t) pActuator->speed_auto1 : 1;
            int32_t acc_rpms2_1 = pActuator->acc_auto1 >= 0.0f ? (int32_t) pActuator->acc_auto1 : 0;
            int32_t speed_rpm2 = (int32_t) pActuator->speed_auto2 > 0 ? (int32_t) pActuator->speed_auto2 : 1;
            int32_t acc_rpms2_2 = pActuator->acc_auto2 >= 0.0f ? (int32_t) pActuator->acc_auto2 : 0;


            pCANSlot->start_time = xTaskGetTickCount();

            if (pCANSlot->paramToUpdate[SPEED_1]) {
                ////////////////////////////////////////////////////////////////////////////////////////////////////////////
                // Scrittura : 【Profile velocity:6081h】to profile velocity. (unit: pulse per second) : UNSIGNED32
                //
                /*
                uint32_t cspeed = (uint32_t)((float)speed_rpm / 60.0f * (float)pActuator->pulsesPerTurn);
                if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x6081, (int16_t) 0, (int8_t *) & cspeed, (int8_t) sizeof(cspeed), true ) < 0) {
                    /////////////////////////
                    // Generazione Allarme
                    //
                    if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR) < 0) {
                    }
                }                
                 * */
                pCANSlot->paramToUpdate[SPEED_1] = 0;
                snprintf(App.Msg, App.MsgSize, "[%s update %s param 0x6081 { speed1:%d } [%dmsec] Err:%d %s]", (char*) ANSI_COLOR_GREEN, pActuator->name, speed_rpm, (xTaskGetTickCount() - pCANSlot->start_time), Error, (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);
            }

            if (pCANSlot->paramToUpdate[ACC_1]) {
                ///////////////////////////////////////////////////////////////////////////////////////
                // Scrittura : Profile acceleration:6083h】(millisecond from 0 to 3000) : UNSIGNED32
                //
                /*
                uint32_t acc_ms = (uint32_t)((3000.0f - 0.0f) / 9.55f / (float)(acc_rpms2_1>0?acc_rpms2_1:1) * 1000.0f);                 
                if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x6083, (int16_t) 0, (int8_t *) & acc_ms, (int8_t) sizeof(acc_ms), true ) < 0) {
                    /////////////////////////
                    // Generazione Allarme
                    //
                    if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR) < 0) {
                    }
                }
                 */
                pCANSlot->paramToUpdate[ACC_1] = 0;
                pCANSlot->paramToUpdate[ACC_2] = 0;
                snprintf(App.Msg, App.MsgSize, "[%s update %s param 0x6083 { acc1:%d } [%dmsec] Err:%d %s]", (char*) ANSI_COLOR_GREEN, pActuator->name, acc_rpms2_1, (xTaskGetTickCount() - pCANSlot->start_time), Error, (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);

            }
            if (pCANSlot->paramToUpdate[ACC_2]) {
                ///////////////////////////////////////////////////////////////////////////////////////
                // Scrittura : Profile acceleration:6083h】(millisecond from 0 to 3000) : UNSIGNED32
                //
                /*
                uint32_t acc_ms = (uint32_t)((3000.0f - 0.0f) / 9.55f / (float)(acc_rpms2_1>0?acc_rpms2_1:1) * 1000.0f);                 
                if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x6083, (int16_t) 0, (int8_t *) & acc_ms, (int8_t) sizeof(acc_ms), true ) < 0) {
                    /////////////////////////
                    // Generazione Allarme
                    //
                    if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR) < 0) {
                    }
                }
                 */
                pCANSlot->paramToUpdate[ACC_1] = 0;
                pCANSlot->paramToUpdate[ACC_2] = 0;
                snprintf(App.Msg, App.MsgSize, "[%s update %s param 0x6083 { acc2:%d } [%dmsec] Err:%d %s]", (char*) ANSI_COLOR_GREEN, pActuator->name, acc_rpms2_1, (xTaskGetTickCount() - pCANSlot->start_time), Error, (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);

            }

            if (pCANSlot->paramToUpdate[SPEED_2]) {
                ////////////////////////////////////////////////////////////////////////////////////////////////////////////
                // Scrittura : 【Profile velocity:6081h】to profile velocity. (unit: pulse per second) : UNSIGNED32
                //
                /*
                uint32_t cspeed = (uint32_t)((float)speed_rpm2 / 60.0f * (float)pActuator->pulsesPerTurn);
                if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x6040, (int16_t) 0, (int8_t *) & cspeed, (int8_t) sizeof(cspeed), true ) < 0) {
                    /////////////////////////
                    // Generazione Allarme
                    //
                    if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR) < 0) {
                    }
                }
                 * */
                pCANSlot->paramToUpdate[SPEED_2] = 0;
                snprintf(App.Msg, App.MsgSize, "[%s update %s param 0x6081 { speed2:%d } [%dmsec] Err:%d %s]", (char*) ANSI_COLOR_GREEN, pActuator->name, speed_rpm2, (xTaskGetTickCount() - pCANSlot->start_time), Error, (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);
            }
        }



        // fine lettura
        pCANSlot->state = DEV_STATE_STREAMING_DONE;

        return 1;
    }

    return 0;
}

int32_t handle_canbus_service(void *pvCANSlot) {
    if (pvCANSlot) {
        CANSlot *pCANSlot = (CANSlot *) pvCANSlot;

        // reinizializa il flusso
        pCANSlot->state = DEV_STATE_INIT_STREAM;


        if (!App.CANRunLoop) {
            pCANSlot->state = DEV_STATE_CLOSING;
        }

        return 1;
    }

    return 0;
}

int32_t handle_canbus_streaming_post_wait(void *pvCANSlot) {
    if (pvCANSlot) {
        CANSlot *pCANSlot = (CANSlot *) pvCANSlot;

        ///////////////////////////
        // Attesa prossimo loop
        //
        if (pCANSlot->t1 - pCANSlot->start_time >= CANBUS_LOOP_TIME_MS) {
            ///////////////////////////
            // Comando da eseguire ?
            //
            if (pCANSlot->pendingCommand) {
                pCANSlot->state = DEV_STATE_CMD_INIT;

            } else if (pCANSlot->stopRequest) {
                pCANSlot->state = DEV_STATE_SERVICE_STOP;

            } else if (pCANSlot->setupRequest == 1) {
                pCANSlot->state = DEV_STATE_SERVICE_SETUP;
                snprintf(App.Msg, App.MsgSize, "[Restart CANBUS service post wait]");
                vDisplayMessage(App.Msg);
                
            } else if (pCANSlot->setupRequest == 1000) {
                pCANSlot->state = DEV_STATE_SERVICE_FIRST_SETUP;
                snprintf(App.Msg, App.MsgSize, "[Setup CANBUS service by request]");
                vDisplayMessage(App.Msg);

            } else if (pCANSlot->homingRequest) {
                pCANSlot->state = DEV_STATE_HOMING_INIT;
                snprintf(App.Msg, App.MsgSize, "[HOMING CANBUS service post wait]");
                vDisplayMessage(App.Msg);

            } else if (pCANSlot->resetRequest) {
                pCANSlot->state = DEV_STATE_SERVICE_RESET;
                snprintf(App.Msg, App.MsgSize, "[RESET CANBUS service post wait]");
                vDisplayMessage(App.Msg);

            } else {
                // Ritorna alla sequenza di lettura
                pCANSlot->state = DEV_STATE_STREAMING_SEND;
            }
        }


        if (!App.CANRunLoop) {
            pCANSlot->state = DEV_STATE_CLOSING;
        }

        return 1;
    }

    return 0;
}

int32_t handle_canbus_streaming_post_act(void *pvCANSlot) {
    if (pvCANSlot) {
        CANSlot *pCANSlot = (CANSlot *) pvCANSlot;


        // Fine comunicazione, imposta la traccia temporale
        pCANSlot->start_time = pCANSlot->t1;
        pCANSlot->state = DEV_STATE_STREAMING_POST_WAIT;

        return 1;
    }

    return 0;
}

int32_t handle_canbus_streaming_done(void *pvCANSlot) {
    if (pvCANSlot) {
        CANSlot *pCANSlot = (CANSlot *) pvCANSlot;

        ////////////////////////////////////////////////////
        // Fine comunicazione : azione post lettura
        //                        


        if (pCANSlot->t1 - pCANSlot->tStat >= 5000) {
            ////////////////////////////////////
            // Debug : stampa ad intervallo
            //
            if (pCANSlot->state >= DEV_STATE_INIT_STREAM && pCANSlot->state <= 900) {


                GLCANDataPerSec = (uint32_t) ((float) pCANSlot->statReadCount * 1000.0 / (float) (pCANSlot->t1 - pCANSlot->tStat));

                // Statistiche IO/sec
                if (GLCANDataPerSec > GLCANMaxDataPerSec)
                    GLCANMaxDataPerSec = GLCANDataPerSec;
                if (GLCANDataPerSec < GLCANMinDataPerSec && GLCANDataPerSec > 0)
                    GLCANMinDataPerSec = GLCANDataPerSec;

                GLCANLastDataPerSec = GLCANDataPerSec;
            }



            if (machine.status != AUTOMATIC) {
                if (pCANSlot->nActuators && pCANSlot->pActuators) {
                    fprintf(stdout, "[%s CAN#%d- %dpulses - %drpm [Rate:%0.f/s - PDO:%0.f/s][Err:%d]%s]\n", (char*) ANSI_COLOR_YELLOW
                            , pCANSlot->stationId
                            , pCANSlot->pActuators ? ((LP_ACTUATOR) pCANSlot->pActuators[0])->driverPosition : 0, pCANSlot->pActuators ? ((LP_ACTUATOR) pCANSlot->pActuators[0])->driverSpeed : 0
                            , (float) pCANSlot->statReadCount / 5.0f
                            , (float) pCANSlot->readPDOCount / 5.0f
                            , pCANSlot->streamErrorCount
                            , (char*) ANSI_COLOR_RESET);
                    fflush(stdout);
                } else {
                    fprintf(stdout, "[%s CAN#%d- [Rate:%0.f/s - PDO:%0.f/s][Err:%d]%s]\n", (char*) ANSI_COLOR_YELLOW
                            , pCANSlot->stationId
                            , (float) pCANSlot->statReadCount / 5.0f
                            , (float) pCANSlot->readPDOCount / 5.0f
                            , pCANSlot->streamErrorCount
                            , (char*) ANSI_COLOR_RESET);
                    fflush(stdout);
                }
            }
            

            

            /////////////////
            // Keep Alive
            //
            if (xrt_send_udp(pCANSlot->ip, (uint16_t) pCANSlot->port, (uint16_t) pCANSlot->port, (uint8_t *) ".", (uint16_t) 1, (uint8_t) 0) < 0) {
                if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                }
            } else {
                // fprintf(stdout, "%s[KA]%s", (char*) ANSI_COLOR_YELLOW, (char*) ANSI_COLOR_RESET); fflush(stdout);                
            }


            pCANSlot->streamErrorCount = 0;
            pCANSlot->readPDOCount = 0;
            pCANSlot->readCount = 0;
            pCANSlot->statReadCount = 0;
            

            pCANSlot->tStat = pCANSlot->t1;
        }



        //////////////////////////////////////////
        // Esecuzione Comando di test / debug      
        //
        if (GLTestCAN == 1) { // pos test
            test_canbus_position((void*) pCANSlot);
            GLTestCAN = 0;

        } else if (GLTestCAN == 2) { // speed test
            GLTestCAN = 0;
        } else if (GLTestCAN == 3) { // reset
            GLTestCAN = 0;
        } else if (GLTestCAN == 4) { // stop motor
            GLTestCAN = 0;
        } else if (GLTestCAN == 5) { // reset position table
            GLTestCAN = 0;
        } else if (GLTestCAN == 6) { // standby motor
            GLTestCAN = 0;
        }


        ///////////////////////////////////////////////
        // Richiesta di chiusura o rilettura
        //
        if (pCANSlot->stopRequest == 1) {
            pCANSlot->state = DEV_STATE_SERVICE_STOP;
        } else if (pCANSlot->setupRequest == 1000) {
            pCANSlot->state = DEV_STATE_SERVICE_FIRST_SETUP;
        } else if (pCANSlot->setupRequest) {
            pCANSlot->state = DEV_STATE_SERVICE_SETUP;
            // snprintf(App.Msg, App.MsgSize, "[Restart CANBUS service on streaming done]"); vDisplayMessage(App.Msg);
        } else if (pCANSlot->homingRequest) {
            pCANSlot->state = DEV_STATE_HOMING_INIT;
            // snprintf(App.Msg, App.MsgSize, "[HOMING CANBUS service on streaming done]"); vDisplayMessage(App.Msg);
        } else {
            pCANSlot->state = DEV_STATE_STREAMING_POST_ACT;
        }


        if (!App.CANRunLoop) {
            pCANSlot->state = DEV_STATE_CLOSING;
        }

        return 1;
    }

    return 0;
}

int32_t handle_canbus_streaming_recv(void *pvCANSlot) {
    if (pvCANSlot) {
        CANSlot *pCANSlot = (CANSlot *) pvCANSlot;
        char str[256];


        // pCANSlot->data etc è gestito dalla callback UDP

        if (pCANSlot->ReadSinglePos > 0) {

            if (xrt_recive_udp(pCANSlot->ip, pCANSlot->port, pCANSlot->port, (uint8_t*) pCANSlot->data, pCANSlot->data_size, 0) > 0) {
                if (pCANSlot->data_available >= pCANSlot->waitingRespSize) {
                    pCANSlot->start_time = pCANSlot->t1;
                    pCANSlot->data_available = 0;
                    // fine lettura
                    pCANSlot->state = DEV_STATE_STREAMING_DONE;
                }
            } else {
            }


            if (pCANSlot->t1 - pCANSlot->tickTocIOWatchDog > App.CanBusStreamTimeoutMS) {
                /////////////////////////////
                // Comunicazione in timeout 
                //
                // Generazione Allarme
                //

                App.CANMeasurePostTimeout = pCANSlot->t1;

                snprintf(str, sizeof (str), "CAN Communication Streaming timeout (%d/%d)", canbus_data_available((void*) pCANSlot), pCANSlot->waitingRespSize);
                if (generate_alarm((char*) str, 6003, 0, (int32_t) ALARM_WARNING, 0 + 1) < 0) {
                }

                // handle_CAN_borad_error(i, (char*) "timeout!", pCANSlot->state);

                // reinizializza il flusso
                pCANSlot->state = DEV_STATE_CMD_ERROR; /* DEV_STATE_INIT_STREAM; */


                pCANSlot->state = DEV_STATE_INIT_STREAM;

                if (pCANSlot->StreamingMode) {
                    pCANSlot->StreamingMode = false;
                    /////////////////
                    // Stop Stream
                    //
                    if (xrt_send_udp(pCANSlot->ip, (uint16_t) pCANSlot->port, (uint16_t) pCANSlot->port, (uint8_t *) "|||", (uint16_t) 3, (uint8_t) 0) < 0) {
                        if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                        }
                    }
                }
            }

        } else {
            // Svuotamento coda ETH
            if (xrt_recive_udp(pCANSlot->ip, pCANSlot->port, pCANSlot->port, (uint8_t*) pCANSlot->data, pCANSlot->data_size, 0) < 0) {
            }
        }



        return 1;
    }

    return 0;
}

int32_t handle_canbus_streaming_send(void *pvCANSlot) {
    if (pvCANSlot) {
        CANSlot *pCANSlot = (CANSlot *) pvCANSlot;

        char str[256];
        float newPosition = 0.0;


        //////////////////////////////////////////////////////////////////////////
        // Invio comando lettura in Streaming (ASINCRONA) o Single Mode
        //
        // N.B.: La lettura continua della poszione non è necessaria se l'asse non è in movimento
        //

        ////////////////////////////////////////////////////       
        // Verifica variazione paramtri degli assi
        //
        //
        for (int32_t iact = 0; iact < pCANSlot->nActuators; iact++) {
            LP_ACTUATOR pActuator = (LP_ACTUATOR) pCANSlot->pActuators[iact];

            if (pActuator) {
                LP_ACTUATOR pActuatorMirror = (LP_ACTUATOR) pActuator->actuator_mirror;
                BOOL reinitActuator = false;

                if (pActuatorMirror) {
                    if (fabs(pActuatorMirror->speed_auto1 - pActuator->speed_auto1) > 0.001) {
                        // snprintf(App.Msg, App.MsgSize, "[%s %s change speed1 %s]", (char*) ANSI_COLOR_GREEN, pActuator->name, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);
                        pActuatorMirror->speed_auto1 = pActuator->speed_auto1;
                        pCANSlot->paramToUpdate[SPEED_1] = TRUE;
                        reinitActuator = true;
                    }
                    if (fabs(pActuatorMirror->speed_auto2 - pActuator->speed_auto2) > 0.001) {
                        // snprintf(App.Msg, App.MsgSize, "[%s %s change speed2 %s]", (char*) ANSI_COLOR_GREEN, pActuator->name, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);
                        pActuatorMirror->speed_auto2 = pActuator->speed_auto2;
                        pCANSlot->paramToUpdate[SPEED_2] = TRUE;
                        reinitActuator = true;
                    }
                    if (fabs(pActuatorMirror->acc_auto1 - pActuator->acc_auto1) > 0.001) {
                        // snprintf(App.Msg, App.MsgSize, "[%s %s change acc_auto1 %s]", (char*) ANSI_COLOR_GREEN, pActuator->name, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);
                        pActuatorMirror->acc_auto1 = pActuator->acc_auto1;
                        pCANSlot->paramToUpdate[ACC_1] = TRUE;
                        reinitActuator = true;
                    }
                    if (fabs(pActuatorMirror->acc_auto2 - pActuator->acc_auto2) > 0.001) {
                        // snprintf(App.Msg, App.MsgSize, "[%s %s change acc_auto2 %s]", (char*) ANSI_COLOR_GREEN, pActuator->name, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);
                        pActuatorMirror->acc_auto2 = pActuator->acc_auto2;
                        pCANSlot->paramToUpdate[ACC_2] = TRUE;
                        reinitActuator = true;
                    }
                    if (fabs(pActuatorMirror->dec_auto1 - pActuator->dec_auto1) > 0.001) {
                        // snprintf(App.Msg, App.MsgSize, "[%s %s change dec_auto1 %s]", (char*) ANSI_COLOR_GREEN, pActuator->name, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);
                        pActuatorMirror->dec_auto1 = pActuator->dec_auto1;
                        pCANSlot->paramToUpdate[ACC_1] = TRUE;
                        reinitActuator = true;
                    }
                    if (fabs(pActuatorMirror->dec_auto2 - pActuator->dec_auto2) > 0.001) {
                        // snprintf(App.Msg, App.MsgSize, "[%s %s change dec_auto2 %s]", (char*) ANSI_COLOR_GREEN, pActuator->name, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);
                        pActuatorMirror->dec_auto2 = pActuator->dec_auto2;
                        pCANSlot->paramToUpdate[ACC_2] = TRUE;
                        reinitActuator = true;
                    }

                    // Forse dev'essere asincrono
                    if (reinitActuator) {
                        pCANSlot->state = DEV_STATE_SERVICE_SETUP_SPEED_ACC;
                    }
                }
            }
        }


        if (App.CANReadPosIDLE && machine.status != AUTOMATIC) {
            pCANSlot->ReadSinglePos = -1;
        }

        if (pCANSlot->ReadSinglePos > 0) {

            if (pCANSlot->state == DEV_STATE_STREAMING_SEND) {
                int32_t nSent = 0;

                // Lunghezza risposta attesa
                pCANSlot->waitingRespSize = 6;

                pCANSlot->ReadSinglePos--;

                if (pCANSlot->ReadSinglePos > 0) {


                    if (pCANSlot->pActuators) {
                        for (int32_t iact = 0; iact < pCANSlot->nActuators; iact++) {
                            LP_ACTUATOR pActuator = (LP_ACTUATOR) pCANSlot->pActuators[iact];

                            if (pActuator->stationId) {
                                int8_t localData = 0;


                                pActuator->IDLECounter++;

                                if (pActuator->IDLECounter == 1) {
                                    pActuator->IDLESeq = 1;
                                } else if (pActuator->IDLECounter == 2) {
                                    pActuator->IDLESeq = 4;
                                } else if (pActuator->IDLECounter == 3) {
                                    pActuator->IDLESeq = 0;
                                } else if (pActuator->IDLECounter > 250 * 5) {
                                    pActuator->IDLECounter = 0;
                                    pActuator->IDLESeq = 0;
                                } else {
                                    pActuator->IDLESeq = 0;
                                }

                                switch (pActuator->IDLESeq) {

                                        // Lettura posizione
                                    case 0:
                                        // Comando Lettura posizione 
                                        if (!pCANSlot->StreamingMode) {
                                            if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x6064, (int16_t) 0, (int8_t *) & localData, (int8_t) 1, false) < 0) {
                                                /////////////////////////
                                                // Generazione Allarme
                                                //
                                                if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                                                }
                                            }
                                        } else {
                                            nSent++;
                                        }
                                        break;

                                        // Lettura Coppia nominale (Rated torque 6076h)
                                    case 1:
                                        if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x6076, (int16_t) 0, (int8_t *) NULL, (int8_t) 1, false) < 0) {
                                        }
                                        nSent++;
                                        break;

                                        // Lettura Temperatura Driver
                                    case 2:
                                        // Dato non disponibile
                                        break;

                                        // Lettura Temperatura Motore
                                    case 3:
                                        // Dato non disponibile
                                        break;


                                        // Lettura 60F4h: Following error actual value
                                    case 4:
                                        if (xrt_can_message_send((void*) pCANSlot, (uint8_t) pActuator->stationId, (int16_t) 0x60F4, (int16_t) 0, (int8_t *) NULL, (int8_t) 1, false) < 0) {
                                        }
                                        nSent++;
                                        break;
                                }
                            }
                        }

                        // 【Torque actual value:6077h】to get instantaneous torque in the drive motor.
                        // 【Current actual value:6078h】to get instantaneous current in the drive motor. (unit: per thousand of rated current)
                    }






                    if (nSent) {
                        pCANSlot->tickTocIOWatchDog = pCANSlot->start_time = pCANSlot->t1;
                        pCANSlot->state = DEV_STATE_STREAMING_RECV;
                    } else {
                        // fine lettura
                        pCANSlot->state = DEV_STATE_STREAMING_DONE;
                    }

                } else {
                    ////////////////////
                    // Ultima lettura
                    //
                }

            } else {
                // priorità al comando di aggiormanto parametri asse
            }

        } else {
            /////////////////////////////////////////
            // IDLE : nessun comando in esecuzione
            //                
            // fine lettura
            pCANSlot->state = DEV_STATE_STREAMING_DONE;
        }

        return 1;
    }

    return 0;
}

int32_t handle_canbus_streaming_init(void *pvCANSlot) {
    if (pvCANSlot) {
        CANSlot *pCANSlot = (CANSlot *) pvCANSlot;


        pCANSlot->t1 = pCANSlot->tickTocIOWatchDog = pCANSlot->start_time = pCANSlot->tStat = xTaskGetTickCount();


        if (App.CANReadPosIDLE && machine.status != AUTOMATIC) {
            pCANSlot->ReadSinglePos = -1;
        }


        if (pCANSlot->ReadSinglePos > 0) {

            pCANSlot->ReadSinglePos--;

            if (!pCANSlot->StreamingMode) {
                pCANSlot->StreamingMode = true;
                /////////////////
                // Start Stream
                //
                if (xrt_send_udp(pCANSlot->ip, (uint16_t) pCANSlot->port, (uint16_t) pCANSlot->port, (uint8_t *) "...", (uint16_t) 3, (uint8_t) 0) < 0) {
                    if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                    }
                }
            }

            /////////////////////////////////
            // Messaggio lettura posizione
            //
            for (uint32_t i_act = 0; i_act < pCANSlot->nActuators; i_act++) {
                LP_ACTUATOR pActuator = (LP_ACTUATOR) pCANSlot->pActuators[i_act];
                if (pActuator) {

                    if (xrt_can_message_send((void*) pCANSlot, (uint8_t) ((LP_ACTUATOR) pCANSlot->pActuators[i_act])->stationId, (int16_t) 0x6064, (int16_t) 0, (int8_t *) NULL, (int8_t) 1, false) < 0) {
                        /////////////////////////
                        // Generazione Allarme
                        //
                        if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                        }
                    }
                }
            }


            if (pCANSlot->ReadSinglePos == 0) {
                ////////////////////
                // Ultima lettura
                //
                if (pCANSlot->StreamingMode) {
                    pCANSlot->StreamingMode = false;
                    /////////////////
                    // Stop Stream
                    //
                    if (xrt_send_udp(pCANSlot->ip, (uint16_t) pCANSlot->port, (uint16_t) pCANSlot->port, (uint8_t *) "|||", (uint16_t) 3, (uint8_t) 0) < 0) {
                        if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                        }
                    }
                    snprintf(App.Msg, App.MsgSize, "[STOP CANBUS STREAM]");
                    vDisplayMessage(App.Msg);
                }
            }


        } else {

            if (pCANSlot->StreamingMode) {
                pCANSlot->StreamingMode = false;
                /////////////////
                // Stop Stream
                //
                if (xrt_send_udp(pCANSlot->ip, (uint16_t) pCANSlot->port, (uint16_t) pCANSlot->port, (uint8_t *) "|||", (uint16_t) 3, (uint8_t) 0) < 0) {
                    if (generate_alarm((char*) "CAN Communication Send Error", 6002, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                    }
                }
                snprintf(App.Msg, App.MsgSize, "[STOP CANBUS STREAM]");
                vDisplayMessage(App.Msg);
            }

            // Svuotamento coda ETH
            if (xrt_recive_udp(pCANSlot->ip, pCANSlot->port, pCANSlot->port, (uint8_t*) pCANSlot->data, pCANSlot->data_size, 0) < 0) {
            }
        }

        ///////////////////////////////////////////////
        // Avvio loop
        //
        if (pCANSlot->pendingCommand) {
            pCANSlot->state = DEV_STATE_CMD_INIT;
        
        } else if (pCANSlot->stopRequest) {
            pCANSlot->state = DEV_STATE_SERVICE_STOP;
        
        } else if (pCANSlot->setupRequest == 1) {
            pCANSlot->state = DEV_STATE_SERVICE_SETUP;
            // snprintf(App.Msg, App.MsgSize, "[Restart CANBUS service streaming init]"); vDisplayMessage(App.Msg);
        
        } else if (pCANSlot->setupRequest == 1000) {
            pCANSlot->state = DEV_STATE_SERVICE_FIRST_SETUP;
            
        } else if (pCANSlot->homingRequest) {
            pCANSlot->state = DEV_STATE_HOMING_INIT;
            // snprintf(App.Msg, App.MsgSize, "[Restart CANBUS service streaming init]"); vDisplayMessage(App.Msg);
        
        } else {
            pCANSlot->state = DEV_STATE_STREAMING_SEND;
        }

        if (!App.CANRunLoop) {
            pCANSlot->state = DEV_STATE_CLOSING;
        }






        return 1;
    }

    return 0;
}


//////////////////////////////////////////////
// Funzioni Eseguita all'avvio dell'asse
// Lettura parametri funzionali ...
//

int32_t handle_canbus_startup(void *pvCANSlot) {
    if (pvCANSlot) {
        CANSlot *pCANSlot = (CANSlot *) pvCANSlot;
        uint16_t data[256] = {0};



        for (int32_t iact = 0; iact < pCANSlot->nActuators; iact++) {
            LP_ACTUATOR pActuator = (LP_ACTUATOR) pCANSlot->pActuators[iact];
            if (pActuator) {

                if (pActuator->protocol == CANOPEN_AC_SERVO_DELTA) {
                    // } else if (pActuator->protocol == MODBUS_AC_SERVO_DELTA) {
                } else {
                    snprintf(App.Msg, App.MsgSize, "[%s error on serial board %d.%d - Unknown protocol %s]\n", (char*) ANSI_COLOR_RED, pCANSlot->boardId, pCANSlot->stationId, (char*) ANSI_COLOR_RESET);
                    vDisplayMessage(App.Msg);
                    return -1;
                }

            } else {
                snprintf(App.Msg, App.MsgSize, "[%s handle_canbus_startup() : no actuator %s]\n", (char*) ANSI_COLOR_RED, (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);
                return -1;
            }
        }

        return 1;
    }

    return 0;
}




char *get_emc_error( int errorData ) {

  switch (errorData) {
    
    case 0x0000:
      return (char*)"Reset Node";
    case 0x0009:
      return (char*)"Excessive deviation";
    break;
    case 0x0185:
      return (char*)"CANbus error (Warning)";
    break;    
    case 0x0121:
      return (char*)"Index error accessing PDO object";
    break;
    case 0x0301:
      return (char*)"CANopen SYNC failed";
    break;
    case 0x0302:
      return (char*)"CANopen SYNC signal error (too early)";
    break;
    case 0x0303:
      return (char*)"CANopen SYNC time out (SON)";
    break;
    case 0x0305:
      return (char*)"SYNC period error";
    break;    
    case 0x03E3:
      return (char*)"CANopen SYNC time out(SOFF)";
    break;
    case 0x0401:
      return (char*)"CANopen state error";
    break;

    
  }

return (char*)"unknown";
}