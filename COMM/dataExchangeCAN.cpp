//============================================================================
// Name        : dataExchangeCAN
// Author      : Cristian Andreon
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


#include <math.h>
#include <linux/can.h>
#include <linux/can/raw.h>



// #define PRINT_PDO_PULSES



// 3000rpm / 60sec * 1280000pulse/round / 250read/sec
#define MAX_ENCODER_VAR     (6500 * 1000)

// Variazione negativa (correzioni di quota nel poszionamento)
#define MIN_ENCODER_VAR     (-550 *1000)

int GLControlPositionData = false;


#define DEFAULT_CANBUS_PORT 7372

int dataExchangeDumpCAN(char *msg, size_t msg_size) {
    for (int i = 0; i < machine.numCANSlots; i++) {
        snprintf(msg, msg_size, "[CANBUS#%d - ID:%d - cycles:%d - tout:%d - state:%d - {pend:%d,run:%d,done:%d,stop:%d,setup:%d} ]\n"
                , (int) machine.CANSlots[i].boardId
                , (int) machine.CANSlots[i].stationId
                , (int) machine.CANSlots[i].readCount
                , (int) machine.CANSlots[i].timeoutCount
                , (int) machine.CANSlots[i].state
                , (int) machine.CANSlots[i].pendingCommand
                , (int) machine.CANSlots[i].runningCommand
                , (int) machine.CANSlots[i].doneCommand
                , (int) machine.CANSlots[i].stopRequest
                , (int) machine.CANSlots[i].setupRequest
                );
        vDisplayMessage(msg);
    }
    return 1;
}

int can_purge_comm(void * pvCANSlot) {
    if (pvCANSlot) {
        return 1;
    }
    return 0;
}

int can_free(void * pvCANSlot) {
    if (pvCANSlot) {
        return 1;
    }
    return 0;
}

int dataExchangeResetCAN() {
    int i;

    for (i = 0; i < machine.numCANSlots; i++) {
        machine.CANSlots[i].state = -1;
    }

    usleep(1 * 1000);

    for (i = 0; i < machine.numCANSlots; i++) {
        if (machine.CANSlots[i].can_ctx) {
            // can_close((void *) machine.CANSlots[i].can_ctx);
        }
    }

    for (i = 0; i < machine.numCANSlots; i++) {
        if (machine.CANSlots[i].can_ctx) {
            machine.CANSlots[i].timeoutCount = 0;
            // can_connect((void *) machine.CANSlots[i].can_ctx);
        }
    }

    return 0;
}

int dataEchangeStopCAN() {
    App.CANRunLoop = 0;
}


// Ritorna 0 se lo stato passa a bloccato (-1)

int handle_CAN_borad_init_error(int i) {
    int retVal = 0;
    machine.CANSlots[i].timeoutCount++;
    if (machine.CANSlots[i].timeoutCount > MAX_CANOPEN_ATTEMPS) {
        snprintf(App.Msg, App.MsgSize, "[%s board:%d - stopping communication (%s) %s]\n", (char*) ANSI_COLOR_MAGENTA, i + 1, (char*) "unk", (char*) ANSI_COLOR_RESET);
        vDisplayMessage(App.Msg);
        machine.CANSlots[i].state = -1;
        retVal = 0;
    } else {
        // Ripete il ciclo
        machine.CANSlots[i].state = 0;
        retVal = 1;
    }
    return retVal;
}

int handle_CAN_borad_error(int i, char *msg, int state) {
    int retVal = 0;

    machine.CANSlots[i].timeoutCount++;

    if (machine.CANSlots[i].timeoutCount > MAX_CANOPEN_LOOP_ERROR) {

        if (state >= 0) {
            snprintf(App.Msg, App.MsgSize, "[%s board:%d - stream error (%s) (%s) state:%d %s]\n", (char*) ANSI_COLOR_MAGENTA, i + 1, (char*) msg, (char*) "unk", state, (char*) ANSI_COLOR_RESET);
        } else {
            snprintf(App.Msg, App.MsgSize, "[%s board:%d - %s %s]\n", (char*) ANSI_COLOR_MAGENTA, i + 1, (char*) msg, (char*) ANSI_COLOR_RESET);
        }
        vDisplayMessage(App.Msg);

        dataExchangeResetCAN();


        if (can_purge_comm((void*) machine.CANSlots[i].can_ctx)) {
        }


        machine.CANSlots[i].state = DEV_STATE_STREAMING_SEND;
        retVal = 0;

        /////////////////////////
        // Generazione Allarme
        //
        if (generate_alarm((char*) "CAN Communication Timeout", 6001, 0, (int) ALARM_FATAL_ERROR, 0 + 1) < 0) {
        }


    } else {
        // Ripete il ciclo

        if (state >= 0) {
            snprintf(App.Msg, App.MsgSize, "[%s board:%d - (%s)(%s) state:%d %s]\n", (char*) ANSI_COLOR_RED, i + 1, (char*) msg, (char*) "unk", state, (char*) ANSI_COLOR_RESET);
        } else {
            snprintf(App.Msg, App.MsgSize, "[%s board:%d - %s %s]\n", (char*) ANSI_COLOR_RED, i + 1, (char*) msg, (char*) ANSI_COLOR_RESET);
        }
        vDisplayMessage(App.Msg);

        machine.CANSlots[i].state = DEV_STATE_STREAMING_SEND;
        retVal = 1;
    }
    return retVal;
}

int link_actuators_to_canbus(LP_CANSlot pCANSlot) {
    bool actLinked = false;
    int i_act;

    if (pCANSlot) {
        pCANSlot->stationId = -1; // data[0];
        for (i_act = 0; i_act < machine.num_actuator; i_act++) {
            if (machine.actuator[i_act].boardId) {
                // N.B.: Link fra ID Scheda (CANBUS) e ID Stazione CAN (Programmatata nel driver)
                if (machine.actuator[i_act].protocol == CANOPEN_AC_SERVO_DELTA || 
                        machine.actuator[i_act].protocol == VIRTUAL_AC_SERVO ||
                        machine.actuator[i_act].protocol == PROTOCOL_NONE
                        ) {
                    if (machine.actuator[i_act].boardId == pCANSlot->boardId /* && machine.actuator[i_act].stationId == pCANSlot->stationId */) {
                        if (pCANSlot->nActuators < MAX_CANBUS_ACTUATORS) {
                            pCANSlot->pActuators[pCANSlot->nActuators++] = (void*) &machine.actuator[i_act];
                            machine.actuator[i_act].pCANSlot = (void*) pCANSlot;
                            // snprintf(App.Msg, App.MsgSize, "[%s Linked at %s %s]\n", (char*) ANSI_COLOR_GREEN, machine.actuator[i_act].name, (char*) ANSI_COLOR_RESET);
                            // vDisplayMessage(App.Msg);
                            actLinked = true;
                            break;
                        } else {
                            snprintf(App.Msg, App.MsgSize, "[%s board:%d too many actuators linked! %s ]\n", (char*) ANSI_COLOR_RED, pCANSlot->boardId, (char*) ANSI_COLOR_RESET);
                            vDisplayMessage(App.Msg);
                        }
                    }
                }
            }
        }

        if (!actLinked) {
            snprintf(App.Msg, App.MsgSize, "[%s board:%d/%d - CANBUS active (Board:%d) - %sNOT Linked! %s ]\n", (char*) ANSI_COLOR_YELLOW, pCANSlot->boardId, machine.numSerialSlots, (int) pCANSlot->stationId, (char*) ANSI_COLOR_RED, (char*) ANSI_COLOR_RESET);
            vDisplayMessage(App.Msg);

        } else {
            snprintf(App.Msg, App.MsgSize, "[%s board:%d/%d - CANBUS active (Board:%d) - Linked to %s%s ]\n", (char*) ANSI_COLOR_YELLOW, pCANSlot->boardId, machine.numSerialSlots, (int) pCANSlot->stationId, (char*) machine.actuator[i_act].name, (char*) ANSI_COLOR_RESET);
            vDisplayMessage(App.Msg);
        }
    }

    return (int) actLinked;
}

int read_canbus_cfg() {
    int retVal = 0;
    uint8_t MacAddr[6];
    FILE *f = fopen(CAN_BOARD_CFG_FILE, "r");

    machine.numCANSlots = 0;

    if (f) {
        char str[256];
        char *pstr = NULL, *pMac = NULL;

        while (fgets(str, sizeof (str), f)) {
            pMac = strstr(str, "=");
            if (pMac) {
                pMac[0] = 0;
                pMac++;

                if (str[0] == '#') {
                    int boardId = atoi((char*) &str[1]);
                    if (boardId >= 1 && boardId <= machine.numCANSlotsAllocated) {
                        if (pMac) {
                            LP_CANSlot CANSlot = (LP_CANSlot) & machine.CANSlots[boardId - 1];

                            CANSlot->boardId = boardId;

                            // Mac address
                            for (int i = 0; i < 6; i++) {
                                if (pMac) {
                                    pstr = strstr(pMac, ".");
                                    int ipVal = 0;

                                    if (pstr) pstr[0] = 0;

                                    sscanf(pMac, "%x", &ipVal);
                                    // ipVal = atoi(pMac);

                                    if (ipVal >= 0 && ipVal < 255) {
                                        MacAddr[i] = ipVal;
                                        // next portion
                                        pMac = pstr ? pstr + 1 : NULL;
                                    } else {
                                        MacAddr[i] = 0;
                                        snprintf(App.Msg, App.MsgSize, "[CFG !CANBUS.mac value]\n");
                                        vDisplayMessage(App.Msg);
                                    }
                                }
                            }

                            memcpy(&CANSlot->mac, &MacAddr, 6);

                            CANSlot->state = -1;

                            // Aggiorna il contatore N. scehde
                            if ((boardId - 1) >= machine.numCANSlots)
                                machine.numCANSlots = (boardId - 1) + 1;
                        }




                    } else {
                        // Numero scheda non valido
                        snprintf(App.Msg, App.MsgSize, "[CFG !CANBUS addr]\n");
                        vDisplayMessage(App.Msg);
                    }

                } else {
                    // Indirizzo board non valido
                    snprintf(App.Msg, App.MsgSize, "[CFG !CANBUS #]\n");
                    vDisplayMessage(App.Msg);
                }
            }

            retVal = 1;
        }


        fclose(f);
        f = NULL;
    }

    return retVal;
}

int update_canbus_cfg() {
    int retVal = 0;

    if (machine.numCANSlots) {
        char str[256];
        FILE *f = fopen(CAN_BOARD_CFG_FILE, "w");
        if (f) {
            for (int i = 0; i < machine.numCANSlots; i++) {
                CANSlot *CANSlot = &machine.CANSlots[i];

                snprintf(str, sizeof (str), "#%d=%02x.%02x.%02x.%02x.%02x.%02x\n", CANSlot->boardId, CANSlot->mac[0], CANSlot->mac[1], CANSlot->mac[2], CANSlot->mac[3], CANSlot->mac[4], CANSlot->mac[5]);
                if (fputs(str, f) != 0) {
                    if (retVal == 0)
                        retVal++;
                } else {
                    retVal = -1;
                    snprintf(App.Msg, App.MsgSize, "[CANBUS CFG update error]\n");
                    vDisplayMessage(App.Msg);
                }
            }

            fclose(f);
            f = NULL;

        } else {
            snprintf(App.Msg, App.MsgSize, "[CANBUS CFG file not open]\n");
            vDisplayMessage(App.Msg);
            retVal = -1;
        }

    } else {
        snprintf(App.Msg, App.MsgSize, "[update_CAN_cfg : No CANBUS boards]\n");
        vDisplayMessage(App.Msg);
        retVal = 0;
    }

    return retVal;
}

char *get_canbus_status(void *pvCANSlot) {

    if (pvCANSlot) {
        LP_CANSlot CANSlot = (LP_CANSlot) pvCANSlot;

        switch (CANSlot->state) {
            
            case DEV_STATE_INIT:
                return (char*) "INIT";
                break;
            case DEV_STATE_WAITING:
                return (char*) "WAITING";
                break;
            case DEV_STATE_CONNECTING:
                return (char*) "CONNECTING";
                break;
                
            case DEV_STATE_HOMING_INIT:
                return (char*) "DEV_STATE_HOMING_INIT";
                break;
            case DEV_STATE_HOMING_SEND:
                return (char*) "DEV_STATE_HOMING_SEND";
                break;
            case DEV_STATE_HOMING_RECV:
                return (char*) "DEV_STATE_HOMING_RECV";
                break;
            case DEV_STATE_HOMING_DONE:
                return (char*) "DEV_STATE_HOMING_DONE";
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
            default:
                return (char*) "???";
                break;
        }
    }
    return (char*) "";
}

int dataExchangeInitCAN(int Mode) {
    int retVal = 1, shouldUpdateIoCfg = 0, curCANSlot = 0;
    EthAddr_t eth_dest = {0};
    int i, j, i_act, res, rc;
    char str[256];
    int res_cfg = 0;

    machine.numCANSlotsAllocated = 16;
    machine.CANSlots = (CANSlot*) calloc(sizeof (CANSlot) * machine.numCANSlotsAllocated + 1, 1);
    machine.numCANSlots = 0;

    if (!machine.CANSlots) {
        machine.numCANSlotsAllocated = 0;
        return -999;
    }

    App.numNewCanbus = 0;
    App.numUnresolvedCanbus = 0;
    App.numDuplicateCanbus = 0;


    if (App.CANDetectError <= 3) {
        // Cerca le schede definite
        res_cfg = read_canbus_cfg();
    } else {
        // Esegue la scansione
        snprintf(str, sizeof (str), "[ERROR : Unable to detect CANBUS Device(s)...Entering search mode...");
        if (generate_alarm((char*) str, 5110, 0, (int) ALARM_ERROR, 0 + 1) < 0) {
        }
        res_cfg = -1;
        shouldUpdateIoCfg = true;
    }
    

    
    if (res_cfg > 0) {
        snprintf(App.Msg, App.MsgSize, "[Reading %d CANBUS Device(s) ", machine.numCANSlots);
        vDisplayMessage(App.Msg);


        App.numCanbusOK = 0;

        for (int j = START_CAN_ADDR; j < END_CAN_ADDR; j++) {
            IpAddr_t target_ip = {192, 168, 1, (uint8_t) j};
            int n_attemp = (j - START_CAN_ADDR) < machine.numCANSlots ? 2 : 1;

            vDisplayMessage(".");

            // Richiesta mac addr da ip
            // rc = get_mac_addr_from_ip( target_ip, &eth_dest, 0 );

            for (int attemp = 0; attemp < n_attemp; attemp++) {

                rc = get_MAC_addr(target_ip, (uint16_t) DEFAULT_CANBUS_PORT, &eth_dest, 0);

                if (rc > 0) {

                    res = 0;

                    if (eth_dest[0] == 0x73 && eth_dest[1] == 0x72) {
                        ///////////////////////
                        // Scheda CANBUS
                        //

                        snprintf(App.Msg, App.MsgSize, "[IP:%s%d.%d.%d.%d%s"
                                , (char*) ANSI_COLOR_BLUE
                                , (int) target_ip[0], (int) target_ip[1], (int) target_ip[2], (int) target_ip[3]
                                , (char*) ANSI_COLOR_RESET
                                );
                        vDisplayMessage(App.Msg);

                        for (int i = 0; i < machine.numCANSlots; i++) {
                            CANSlot *pCANSlot = &machine.CANSlots[i];

                            if (pCANSlot->mac[0] == eth_dest[0] && pCANSlot->mac[1] == eth_dest[1] && pCANSlot->mac[2] == eth_dest[2] &&
                                    pCANSlot->mac[3] == eth_dest[3] && pCANSlot->mac[4] == eth_dest[4] && pCANSlot->mac[5] == eth_dest[5]) {


                                // L'id � assegnato dalla lettura del file di IO
                                // pCANSlot->boardId = 

                                memcpy(&pCANSlot->ip, &target_ip, 4);

                                pCANSlot->port = DEFAULT_CANBUS_PORT;
                                pCANSlot->Kbps = 1000;


                                pCANSlot->state = DEV_STATE_INIT;

                                App.numCanbusOK++;

                                res++;
                            }
                        }

                        if (res == 0) {
                            //////////////////////////////////////
                            /// Sostituzione o aggiunta scheda
                            //
                            snprintf(App.Msg, App.MsgSize, ":New CANBUS Board...Please update %s]", IO_BOARD_CFG_FILE);
                            vDisplayMessage(App.Msg);

                            if (machine.numCANSlots < machine.numCANSlotsAllocated) {
                                CANSlot *pCANSlot = &machine.CANSlots[machine.numCANSlots];

                                machine.numCANSlots++;

                                // Assegnamento ID
                                pCANSlot->boardId = machine.numCANSlots;

                                memcpy(&pCANSlot->mac, &eth_dest, 6);
                                memcpy(&pCANSlot->ip, &target_ip, 4);

                                pCANSlot->port = DEFAULT_CANBUS_PORT;
                                pCANSlot->Kbps = 1000;


                                pCANSlot->state = DEV_STATE_INIT;


                                shouldUpdateIoCfg++;

                            }


                            ///////////////////////////////////////////////////////////////////////////
                            // N.B.: Impossibile avviare la logica : Campbio Delle schede di IO
                            //
                            App.numNewCanbus++;

                        } else if (res == 1) {
                            snprintf(App.Msg, App.MsgSize, ":%sOK%s].", (char*) ANSI_COLOR_GREEN, (char*) ANSI_COLOR_RESET);
                            vDisplayMessage(App.Msg);
                        } else {
                            snprintf(App.Msg, App.MsgSize, ":%sduplicated%s].", (char*) ANSI_COLOR_RED, (char*) ANSI_COLOR_RESET);
                            vDisplayMessage(App.Msg);
                        }

                        usleep(50 * 1000);


                    } else if (eth_dest[0] == 0x73 && eth_dest[1] == 0x73) {
                        /////////////////////////////////
                        // Scheda SCR : porta errata
                        //
                        snprintf(App.Msg, App.MsgSize, "[FILTERED OUT SCR IP:%d.%d.%d.%d - MAC:%02x.%02x.%02x.%02x.%02x.%02x]\n"
                                , (int) target_ip[0], (int) target_ip[1], (int) target_ip[2], (int) target_ip[3]
                                , (int) eth_dest[0], (int) eth_dest[1], (int) eth_dest[2], (int) eth_dest[3], (int) eth_dest[4], (int) eth_dest[5]
                                );
                        vDisplayMessage(App.Msg);

                    } else if (eth_dest[0] == 0x73 && eth_dest[1] == 0x62) {
                        /////////////////////////////////
                        // Scheda IO : porta errata
                        //
                        snprintf(App.Msg, App.MsgSize, "[FILTERED OUT IO IP:%d.%d.%d.%d - MAC:%02x.%02x.%02x.%02x.%02x.%02x]\n"
                                , (int) target_ip[0], (int) target_ip[1], (int) target_ip[2], (int) target_ip[3]
                                , (int) eth_dest[0], (int) eth_dest[1], (int) eth_dest[2], (int) eth_dest[3], (int) eth_dest[4], (int) eth_dest[5]
                                );
                        vDisplayMessage(App.Msg);


                    } else {
                        if ((int) eth_dest[0] || (int) eth_dest[1] || (int) eth_dest[2] || (int) eth_dest[3] || (int) eth_dest[4] || (int) eth_dest[5]) {
                            snprintf(App.Msg, App.MsgSize, "[FILTERED OUT IP:%d.%d.%d.%d - MAC:%02x.%02x.%02x.%02x.%02x.%02x]\n"
                                    , (int) target_ip[0], (int) target_ip[1], (int) target_ip[2], (int) target_ip[3]
                                    , (int) eth_dest[0], (int) eth_dest[1], (int) eth_dest[2], (int) eth_dest[3], (int) eth_dest[4], (int) eth_dest[5]
                                    );
                            vDisplayMessage(App.Msg);
                        }
                    }

                    break;

                } else {
                    // errore risoluzione scheda
                }
            }
        }

        vDisplayMessage("]\n");


end_read_io:

        if (shouldUpdateIoCfg) {
            if (update_canbus_cfg() < 0) {
                snprintf(App.Msg, App.MsgSize, "[%sCANBUS file %s updated%s]\n", (char*) ANSI_COLOR_GREEN, CAN_BOARD_CFG_FILE, (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);
            }
        }


        if (App.numCanbusOK < machine.numCANSlots) {
            //////////////////////////////////////
            // Generazione Warning Macchina 
            //
            if (generate_alarm((char*) "No CANBUS Board detected", 5001, 0, (int) ALARM_WARNING, 0 + 1) < 0) {
            }
        } else {
        }


    } else {

        ////////////////////////////////////////////////////////////
        // File non trovato : inizializzazione legengo gli ip
        //

        snprintf(App.Msg, App.MsgSize, "[Searching for CANBUS on %s...\n", App.ETHInterfaceName);
        vDisplayMessage(App.Msg);


        curCANSlot = 0;

        for (int i = START_CAN_ADDR; i < END_CAN_ADDR; i++) {

            // Richiesta mac addr
            IpAddr_t target_ip = {192, 168, 1, (uint8_t) i};
            EthAddr_t eth_dest = {0};

            // rc = get_mac_addr_from_ip( target_ip, &eth_dest, 1 );

            rc = get_MAC_addr(target_ip, (uint16_t) DEFAULT_CANBUS_PORT, &eth_dest, 0);

            if (rc > 0) {

                if (eth_dest[0] == 0x73 && eth_dest[1] == 0x72) {
                    ////////////////////
                    // Scheda CANBUS
                    //

                    if (curCANSlot < machine.numCANSlotsAllocated) {
                        CANSlot *pCANSlot = &machine.CANSlots[curCANSlot];
                        curCANSlot++;

                        // Assegnamento ID
                        pCANSlot->boardId = curCANSlot;

                        memcpy(&pCANSlot->mac, &eth_dest, 6);
                        memcpy(&pCANSlot->ip, &target_ip, 4);

                        pCANSlot->port = DEFAULT_CANBUS_PORT;
                        pCANSlot->Kbps = 1000;

                        // Stato :
                        pCANSlot->state = DEV_STATE_INIT;

                        snprintf(App.Msg, App.MsgSize, "[CANBUS #%d > IP:%s%d.%d.%d.%d%s - MAC:%02x.%02x.%02x.%02x.%02x.%02x]\n"
                                , (int) pCANSlot->boardId
                                , (char*) ANSI_COLOR_BLUE
                                , (int) pCANSlot->ip[0], (int) pCANSlot->ip[1], (int) pCANSlot->ip[2], (int) pCANSlot->ip[3]
                                , (char*) ANSI_COLOR_RESET
                                , (int) pCANSlot->mac[0], (int) pCANSlot->mac[1], (int) pCANSlot->mac[2], (int) pCANSlot->mac[3], (int) pCANSlot->mac[4], (int) pCANSlot->mac[5]
                                );
                        vDisplayMessage(App.Msg);

                    } else {
                        snprintf(App.Msg, App.MsgSize, "[CANBUS : too many IO boards]\n");
                        vDisplayMessage(App.Msg);
                    }

                    usleep(50 * 1000);


                } else if (eth_dest[0] == 0x73 && eth_dest[1] == 0x62) {
                    ///////////////////////
                    // Scheda IO
                    //
                } else if (eth_dest[0] == 0x73 && eth_dest[1] == 0x71) {
                    ///////////////////////
                    // Scheda SCR
                    //


                } else {
                    snprintf(App.Msg, App.MsgSize, "[FILTERED OUT IP:%d.%d.%d.%d - MAC:%s%02x.%02x.%02x.%02x.%02x.%02x%s]\n"
                            , (int) target_ip[0], (int) target_ip[1], (int) target_ip[2], (int) target_ip[3]
                            , (char*) ANSI_COLOR_MAGENTA
                            , (int) eth_dest[0], (int) eth_dest[1], (int) eth_dest[2], (int) eth_dest[3], (int) eth_dest[4], (int) eth_dest[5]
                            , (char*) ANSI_COLOR_RESET
                            );
                    vDisplayMessage(App.Msg);
                }


            } else if (res < 0) {
                snprintf(App.Msg, App.MsgSize, "[%sCANBUS #%d > IP:%d.%d.%d.%d - mac error!%s]\n"
                        , (char*) ANSI_COLOR_RED
                        , (int) (i+1)
                        , (int) target_ip[0], (int) target_ip[1], (int) target_ip[2], (int) target_ip[3]
                        , (char*) ANSI_COLOR_RESET
                        );
                vDisplayMessage(App.Msg);
            }
        }


        machine.numCANSlots = curCANSlot;

        if (machine.numCANSlots) {
            if (update_canbus_cfg() > 0) {
                snprintf(App.Msg, App.MsgSize, "[%sCANBUS file %s updated%s]\n", (char*) ANSI_COLOR_GREEN, IO_BOARD_CFG_FILE, (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);
            }
        }

        //////////////////////////////////////////////////////////////////////////////////
        // Nessuna scheda dichiarata : Inizializzazione OK, a cura dell primo avvio
        //
        App.CANOK = true;

    }






    ////////////////////////////
    // Verifiche finali
    //
    curCANSlot = 0;

    for (int i = 0; i < machine.numCANSlots; i++) {
        if (machine.CANSlots[i].boardId > 0) {
            if (machine.CANSlots[i].ip[0] > 0 && machine.CANSlots[i].ip[1] > 0 && machine.CANSlots[i].ip[2] > 0 && machine.CANSlots[i].ip[3] > 0) {
                for (int j = 0; j < machine.numCANSlots; j++) {
                    CANSlot *pCANSlot2 = &machine.CANSlots[j];
                    if (i != j) {
                        if (machine.CANSlots[i].ip[0] == pCANSlot2->ip[0] && machine.CANSlots[i].ip[1] == pCANSlot2->ip[1] && machine.CANSlots[i].ip[2] == pCANSlot2->ip[2] && machine.CANSlots[i].ip[3] == pCANSlot2->ip[3]) {
                            pCANSlot2->boardId = 0;
                            memset(&pCANSlot2->ip, 0, 4);
                            App.numDuplicateCanbus++;
                        }
                    }
                }

                curCANSlot = i + 1;
            } else {
                App.numUnresolvedCanbus++;
            }
        }
    }


    // Numero di Shede di IO
    if (machine.numCANSlots != curCANSlot) {

    }



    // Collega tutti gli attuatori che si riferiscono all canbus
    for (int i = 0; i < machine.numCANSlots; i++) {
        CANSlot *pCANSlot = &machine.CANSlots[i];
        if (machine.CANSlots[i].boardId > 0) {
            if (machine.CANSlots[i].state > 0) {
                if (link_actuators_to_canbus(pCANSlot) < 0) {
                }
            }
        }
    }



    if (machine.numCANSlots > 0) {
        char *pANSI_COLOR = (char *) ((machine.numCANSlots > 0 && App.numNewCanbus == 0 && App.numUnresolvedCanbus == 0 && App.numDuplicateCanbus == 0) ? (ANSI_COLOR_GREEN) : (ANSI_COLOR_RED));
        snprintf(App.Msg, App.MsgSize, "[%s%d CANBUS Board defined, %d new, %d unresolved, %d duplicated%s]\n", (char*) pANSI_COLOR, (int) machine.numCANSlots, (int) App.numNewCanbus, (int) App.numUnresolvedCanbus, (int) App.numDuplicateCanbus, (char*) ANSI_COLOR_RESET);
        vDisplayMessage(App.Msg);

        //////////////////////////////////////
        // Generazione Emergenza Macchina
        //
        if (App.numUnresolvedCanbus > 0 || App.numDuplicateCanbus > 0) {
            snprintf(App.Msg, App.MsgSize, "CANBUS Board error : %d unresolved, %d duplicated", App.numUnresolvedCanbus, App.numDuplicateCanbus);
            vDisplayMessage(App.Msg);
            retVal = -1;
            /*
            if (generate_alarm((char*) "CANBUS Board NOT detected", 5002, 0, (int) ALARM_FATAL_ERROR) < 0) {
            }
             */
        }

    } else {
        if (res_cfg > 0) {
            snprintf(App.Msg, App.MsgSize, "[%sNO CANBUS Board detected%s]\n", (char*) ANSI_COLOR_RED, (char*) ANSI_COLOR_RESET);
            vDisplayMessage(App.Msg);
            retVal = -1;
            /*
            if (generate_alarm((char*) App.Msg, 5023, 0, (int) ALARM_FATAL_ERROR) < 0) {
            }
             */
        }
    }




    ///////////////////////////////////////////
    // Verifica collegamento degli attuatori
    //

    if (machine.numCANSlots) {
        snprintf(str, sizeof (str), "[%s Checking %d actuators...%s]", (char*) ANSI_COLOR_YELLOW, machine.num_actuator, (char*) ANSI_COLOR_RESET);
        vDisplayMessage(str);

        for (i_act = 0; i_act < machine.num_actuator; i_act++) {

            if (machine.actuator[i_act].boardId > 0) {

                if (machine.actuator[i_act].protocol == CANOPEN_AC_SERVO_DELTA) {

                    snprintf(str, sizeof (str), "[%s %s ...%s", (char*) ANSI_COLOR_YELLOW, machine.actuator[i_act].name, (char*) ANSI_COLOR_RESET);
                    vDisplayMessage(str);

                    if (!machine.actuator[i_act].pCANSlot) {
                        //////////////////////////////////////
                        // Generazione Emergenza Macchina
                        //
                        snprintf(str, sizeof (str), "Actuator %s not linked!", machine.actuator[i_act].name);
                        vDisplayMessage(str);
                        retVal = -2;
                        /*
                        if (generate_alarm((char*) str, 5103, 0, (int) ALARM_FATAL_ERROR) < 0) {
                        }
                         */
                    } else {
                        CANSlot *pCANSlot = (CANSlot *) machine.actuator[i_act].pCANSlot;

                        if (pCANSlot->boardId <= 0) {
                            //////////////////////////////////////
                            // Generazione Emergenza Macchina
                            //
                            snprintf(str, sizeof (str), "Actuator %s driver offline", machine.actuator[i_act].name);
                            vDisplayMessage(str);
                            /*
                            if (generate_alarm((char*) str, 5104, 0, (int) ALARM_FATAL_ERROR) < 0) {
                            }
                             */
                            retVal = -3;

                        } else {
                            if (pCANSlot->state < 0) {
                                //////////////////////////////////////
                                // Generazione Emergenza Macchina
                                //
                                snprintf(str, sizeof (str), "Actuator %s invalid serial state (%d)", machine.actuator[i_act].name, machine.CANSlots[i].state);
                                vDisplayMessage(str);
                                retVal = -3;
                                /*
                                if (generate_alarm((char*) str, 5105, 0, (int) ALARM_FATAL_ERROR) < 0) {                                 
                                } else {
                                }
                                 */
                            } else {
                                snprintf(str, sizeof (str), "%sOK%s]", (char*) ANSI_COLOR_GREEN, (char*) ANSI_COLOR_RESET);
                                vDisplayMessage(str);
                            }
                        }
                    }
                }
            }
        }
        vDisplayMessage("\n");
    }



    if (App.numNewCanbus > 0 || App.numUnresolvedCanbus > 0 || App.numDuplicateCanbus > 0) {
        if (App.numNewCanbus == 1 && App.numUnresolvedCanbus == 1 && App.numDuplicateCanbus == 0) {
            // sostituzione scheda CANBUS
        } else {
            // Impossibile determinare lo slot delle schede : necessario riconfigurazione manuale
            App.CANDetectError++;
            return -1;
        }
    }


    // Registra la callback
    if (retVal > 0) {
        if (machine.numCANSlots) {
            if (xrt_callback_udp((uint16_t) GLCANIpAddrPort, &udpCANHandler) <= 0) {
                //////////////////////////////////////
                // Generazione Emergenza Macchina
                //
                if (generate_alarm((char*) "No CANBUS UDP callback", 5010, 0, (int) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                }
                retVal = -999;
            } else {
                // snprintf(App.Msg, App.MsgSize, "[CANBUS Callback Installed]\n"); vDisplayMessage(App.Msg);            
            }
        }
    }


    if (retVal > 0) {
        ////////////////////////////////////////////////////////
        // Tutte le schede identificate : Inizializzazione OK
        //
        App.CANOK = true;
    } else {
        App.CANDetectError++;
    }

    return retVal;
}






/*
 * 
 * DELTA 
 * 
 * 
 
P1-01 = 0B  (CANopen)

P3-00   ADR     Address Setting         0x7C N/A O O O O 9.2
P3-01   BRT     Transmission Speed      0x0203 (2 = 500 Kbit/s CAMopen, 3 = 38400 Modbus)
P3-02   PTL     Communication Protocol  6 N/A O O O O 9.2
P3-03   FLT     Communication Error Disposal 0 N/A O O O O 9.2
P3-04   CWD     Communication Timeout 0 sec O O O O 9.2
P3-05   CMM     Communication Mechanism 0 N/A O O O O 9.2
P3-06   SDI     Control Switch of Digital Input (DI) 0 N/A O O O O 9.2
P3-07   CDT     Communication Response Delay Time 0 1ms O O O O 9.2
P3-08   MNS     Monitor Mode 0000 N/A O O O O 9.2
P3-09   SYC     CANopen Synchronize Setting 0x57A1 N/A O O O O 9.2 

 * 
 * 
 */


///////////////////////////////////
// Legge dai dispositivi CAN
//

int dataExchangeLoopCAN(void) {
    int rc, retVal = 0;
    int i, i_act;
    uint8_t canId = 0;
    uint16_t ObjIndex = 0;
    uint8_t ObjSubIndex = 0;
    char str[256];

    char hHigh = 0, bLow = 0;
    int avaiable_bytes = 0;
    float newPosition = 0.0;



    App.CANRunning = machine.numCANSlots;

    for (i = 0; i < machine.numCANSlots; i++) {
        LP_ACTUATOR pActuator = NULL;

        machine.CANSlots[i].t1 = xTaskGetTickCount();
        
        if (!machine.CANSlots[i].disabled) {

            if (machine.CANSlots[i].state == DEV_STATE_INIT) {

            machine.CANSlots[i].streamErrorCount = 0;

            machine.CANSlots[i].StreamingMode = false;

            machine.CANSlots[i].messageState = 0;
            machine.CANSlots[i].timeoutCount = 0;

            machine.CANSlots[i].stopRequest = 0;
            machine.CANSlots[i].setupRequest = 0;
            machine.CANSlots[i].pendingCommand = 0;
            machine.CANSlots[i].runningCommand = 0;
            machine.CANSlots[i].doneCommand = 0;

            // ID driver : tutti i driver sono accessibili dalla scheda attraverso il campo CAN_ID
            machine.CANSlots[i].stationId = -1;


            machine.CANSlots[i].data_size = MAX_CAN_MESSAGE_SIZE;
            machine.CANSlots[i].data = (char *) calloc(machine.CANSlots[i].data_size, 1);
            if (!machine.CANSlots[i].data) {
                machine.CANSlots[i].data_size = 0;
                retVal = -1;
            }


            /////////////////////////
            // Stop motore
            //
            if (handle_actuator_servo_off( (void*)pActuator ) < 0) {
            }
            
            if (handle_actuator_prepare_for_run( (void*)pActuator, CONTROL_WORD_QUICK_STOP ) < 0) {
            }
            
            // Abilitazione CanBus
            App.CANOK = INIT_AND_RUN;

            
            machine.CANSlots[i].state = DEV_STATE_WAITING;

        } else if (machine.CANSlots[i].state == DEV_STATE_WAITING) {
            machine.CANSlots[i].state = DEV_STATE_CONNECTING;

        } else if (machine.CANSlots[i].state == DEV_STATE_CONNECTING) {

            // Reset dati sequenza IDLE
            for (int iact = 0; iact < machine.CANSlots[i].nActuators; iact++) {
                LP_ACTUATOR pActuator = (LP_ACTUATOR) machine.CANSlots[i].pActuators[iact];
                pActuator->IDLECounter = 0;
                pActuator->IDLESeq = 0;
            }

            machine.CANSlots[i].state = DEV_STATE_INIT_STREAM;





        } else if (machine.CANSlots[i].state == DEV_STATE_HOMING_INIT) {
            // Homing Init...
            handle_canbus_homing_init(&machine.CANSlots[i]);

        } else if (machine.CANSlots[i].state == DEV_STATE_HOMING_SEND) {
            // Homing Send...
            handle_canbus_homing_send(&machine.CANSlots[i]);

        } else if (machine.CANSlots[i].state == DEV_STATE_HOMING_RECV) {
            // Homing Recv...
            handle_canbus_homing_recv(&machine.CANSlots[i]);


        } else if (machine.CANSlots[i].state == DEV_STATE_HOMING_DONE) {
            // Homing Init...
            handle_canbus_homing_done (&machine.CANSlots[i]);
            

            
            
            
        } else if (machine.CANSlots[i].state == DEV_STATE_INIT_STREAM) {

            if (handle_canbus_streaming_init((void *) &machine.CANSlots[i]) < 0) {
            }


        } else if (machine.CANSlots[i].state == DEV_STATE_STREAMING_SEND) {

            if (handle_canbus_streaming_send((void *) &machine.CANSlots[i]) < 0) {
            }


        } else if (machine.CANSlots[i].state == DEV_STATE_STREAMING_RECV) {

            if (handle_canbus_streaming_recv((void *) &machine.CANSlots[i]) < 0) {
            }



        } else if (machine.CANSlots[i].state == DEV_STATE_STREAMING_DONE) {

            if (handle_canbus_streaming_done((void *) &machine.CANSlots[i]) < 0) {
            }



        } else if (machine.CANSlots[i].state == DEV_STATE_STREAMING_POST_ACT) {

            if (handle_canbus_streaming_post_act((void *) &machine.CANSlots[i]) < 0) {
            }




        } else if (machine.CANSlots[i].state == DEV_STATE_STREAMING_POST_WAIT) {

            if (handle_canbus_streaming_post_wait((void *) &machine.CANSlots[i]) < 0) {
            }






            ////////////////////////////////////////
            // Comando di servizio
            //

        } else if (machine.CANSlots[i].state == DEV_STATE_SERVICE) {

            if (handle_canbus_service((void *) &machine.CANSlots[i]) < 0) {
            }





        } else if (machine.CANSlots[i].state == DEV_STATE_SERVICE_SETUP_SPEED_ACC) {

            if (handle_canbus_service_setup_speed_acc((void*) &machine.CANSlots[i]) < 0) {
            }

        } else if (machine.CANSlots[i].state == DEV_STATE_SERVICE_SETUP) {

            if (handle_canbus_service_setup((void*) &machine.CANSlots[i]) < 0) {
            }

        } else if (machine.CANSlots[i].state == DEV_STATE_SERVICE_FIRST_SETUP) {

            if (handle_canbus_service_first_setup((void*) &machine.CANSlots[i]) < 0) {
            }



        } else if (machine.CANSlots[i].state == DEV_STATE_SERVICE_STOP) {

            if (handle_canbus_service_stop((void*) &machine.CANSlots[i]) < 0) {
            }


        } else if (machine.CANSlots[i].state == DEV_STATE_SERVICE_OUT) {

            if (handle_canbus_service_out((void*) &machine.CANSlots[i]) < 0) {
            }

            
        } else if (machine.CANSlots[i].state == DEV_STATE_SERVICE_RESET) {

            if (handle_canbus_service_reset(&machine.serialSlots[i]) < 0) {                
            }




        } else if (machine.CANSlots[i].state == DEV_STATE_CMD_INIT) {

            if (handle_canbus_cmd_init((void*) &machine.CANSlots[i]) < 0) {
            }

        } else if (machine.CANSlots[i].state == DEV_STATE_CMD_INIT_SEND) {

            if (handle_canbus_cmd_init_send((void*) &machine.CANSlots[i]) < 0) {
            }

        } else if (machine.CANSlots[i].state == DEV_STATE_CMD_INIT_RECV) {

            if (handle_canbus_cmd_init_recv((void*) &machine.CANSlots[i]) < 0) {
            }

        } else if (machine.CANSlots[i].state == DEV_STATE_CMD_FEEDBACK_SEND) {

            if (handle_canbus_cmd_feedback_send((void*) &machine.CANSlots[i]) < 0) {
            }

        } else if (machine.CANSlots[i].state == DEV_STATE_CMD_FEEDBACK_RECV) {

            if (handle_canbus_cmd_feedback_recv((void*) &machine.CANSlots[i]) < 0) {
            }

        } else if (machine.CANSlots[i].state == DEV_STATE_CMD_DONE) {

            if (handle_canbus_cmd_done((void*) &machine.CANSlots[i]) < 0) {
            }





        } else if (machine.CANSlots[i].state == DEV_STATE_CMD_ERROR) {

            // reinizializza il flusso
            // machine.CANSlots[i].state = DEV_STATE_INIT_STREAM;

            // Fuori servizio
            machine.CANSlots[i].state = DEV_STATE_SERVICE_OUT;





        } else if (machine.CANSlots[i].state == DEV_STATE_CLOSING) {
            machine.CANSlots[i].state = DEV_STATE_SERVICE_STOP;
            snprintf(App.Msg, App.MsgSize, "[RTC] CANBUS #%d Stopped\n", (i + 1));
            vDisplayMessage(App.Msg);
        }
        }
    }

    return retVal;
}

int dataExchangeIsRunningCAN() {
    for (uint32_t i = 0; i < machine.numCANSlots; i++) {
        if (machine.CANSlots[i].state != DEV_STATE_SERVICE_OUT && machine.CANSlots[i].state != 0 && machine.CANSlots[i].state != -1) {
            return 1;
        }
    }
    return 0;
}








int handle_canbus_pulses_change ( void* pAvctuator, int32_t driverData, uint32_t CANSeqID ) {
    LP_ACTUATOR pActuator = (LP_ACTUATOR) pAvctuator;
    float newPosition = 0.0f;

    if (pActuator->driverPosition != driverData) {
        int32_t curEncoderVar = 0;

        if (pActuator->step == STEP_MOVING) {
            if (pActuator->target_position == ON) {
                // Attesa crescente
                curEncoderVar = (driverData - pActuator->driverPosition);
            } else {
                // Attesa calante
                curEncoderVar = (pActuator->driverPosition - driverData);
            }
        }



        if (!GLControlPositionData || 
                ((int32_t) curEncoderVar > (int32_t) MIN_ENCODER_VAR 
                && (int32_t) curEncoderVar < (int32_t) MAX_ENCODER_VAR)) {

            // Passa il dato alla logica
            uint32_t cTime = xTaskGetTickCount();

            // Calcolo velocià come derivata
            if (pActuator->driverLastTime != 0) {
                if (pActuator->driverPosCount >= 2) {
                    if (pActuator->driverLastTime < cTime) {
                        float derivatePulsesPerMsec = (float) (driverData - pActuator->driverPosition) / (float) (cTime - pActuator->driverLastTime);
                        float derivateSpeedRpm = derivatePulsesPerMsec * 1000.0f * 60.0f / (float) pActuator->pulsesPerTurn;
                        pActuator->driverSpeed = (int32_t) derivatePulsesPerMsec;

                        if (pActuator->target_position == ON) {
                            if (derivateSpeedRpm >= 0.0f && derivateSpeedRpm <= pActuator->speed_auto1 * 1.001f) {
                                pActuator->speed = derivateSpeedRpm;
                            } else {
                                // Resetta la statistica; usando il metodo delle derivate qualche volta sbaglia il conteggio
                                // pActuator->max_speed = 0.0f;
                            }
                        } else if (pActuator->target_position == OFF) {
                            if (derivateSpeedRpm <= 0.0f && derivateSpeedRpm >= pActuator->speed_auto2 * -1.001f) {
                                pActuator->speed = derivateSpeedRpm;
                            } else {
                                // Resetta la statistica; usando il metodo delle derivate qualche volta sbaglia il conteggio
                                // pActuator->min_speed = 0.0f;
                            }
                        }

                        if (pActuator->speed > pActuator->max_speed)
                            pActuator->max_speed = pActuator->speed;
                        if (pActuator->speed < pActuator->min_speed)
                            pActuator->min_speed = pActuator->speed;
                    }
                }
            }

            pActuator->driverPosition = driverData;
            pActuator->driverTurns = driverData / (int32_t)pActuator->pulsesOverflow;
            pActuator->driverPulses = driverData % (int32_t)pActuator->pulsesOverflow;

            pActuator->driverLastTime = cTime;

            actuator_encoder_to_position((void *) pActuator, (int32_t) 0, (int32_t) driverData, &newPosition);


            if (!pActuator->start_time1)
                pActuator->start_time1 = cTime;


            // posizione reale
            pActuator->cur_rpos = (float) newPosition;

            if (pActuator->cur_rpos < 0.01f) {
                int lb = 1;
            }


            // posizione virtuale (0...1000)
            update_actuator_virtual_pos(pActuator);


            pActuator->readPositionCounter++;


        } else {
            if ((int32_t) curEncoderVar < (int32_t) MIN_ENCODER_VAR) {
                snprintf(App.Msg, App.MsgSize, "%s[CAN:L.range:%d/%d-%d]%s", (char*) ANSI_COLOR_RED, (int32_t) curEncoderVar, (int32_t) MIN_ENCODER_VAR, (int32_t) CANSeqID, (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);
            } else if ((int32_t) curEncoderVar > (int32_t) MAX_ENCODER_VAR) {
                snprintf(App.Msg, App.MsgSize, "%s[CAN:U.range:%d/%d-%d]%s", (char*) ANSI_COLOR_RED, (int32_t) curEncoderVar, (int32_t) MAX_ENCODER_VAR, (int32_t) CANSeqID, (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);
            }
        }
    } else {
        // nessuna variazione ... asse fermo ?
    }
    
    return 1;
}




/////////////////////////////////////
// Errori interpolazione Multiasse
//
char GLCANBUSInterolationErrorDesc[256] = {0};

char *get_interpolation_error_desc( int32_t errorCode, int32_t targetPosX, int32_t targetPosY, int32_t targetPosZ) {
    switch (errorCode) {
        case 0:
            snprintf(GLCANBUSInterolationErrorDesc, sizeof(GLCANBUSInterolationErrorDesc), "No error");
            break;
        case 1:
            snprintf(GLCANBUSInterolationErrorDesc, sizeof(GLCANBUSInterolationErrorDesc), "Reset control word failed");
            break;
        case 2:
            snprintf(GLCANBUSInterolationErrorDesc, sizeof(GLCANBUSInterolationErrorDesc), "Current position unknown");
            break;
        case 3:
            snprintf(GLCANBUSInterolationErrorDesc, sizeof(GLCANBUSInterolationErrorDesc), "Invalid no. steps : %d", targetPosX);
            break;
        case 4:
            snprintf(GLCANBUSInterolationErrorDesc, sizeof(GLCANBUSInterolationErrorDesc), "Invalid Direction : %d", targetPosX);
            break;
        case 5:
            snprintf(GLCANBUSInterolationErrorDesc, sizeof(GLCANBUSInterolationErrorDesc), "Invalid target position / cos() error : %d pulses", targetPosX);
            break;
        case 6:
            snprintf(GLCANBUSInterolationErrorDesc, sizeof(GLCANBUSInterolationErrorDesc), "Invalid target position / sin() error : %d pulses", targetPosX);
            break;
        case 7:
            snprintf(GLCANBUSInterolationErrorDesc, sizeof(GLCANBUSInterolationErrorDesc), "Preparing axis failed");
            break;
        case 8:
            snprintf(GLCANBUSInterolationErrorDesc, sizeof(GLCANBUSInterolationErrorDesc), "PDO mapping failed");
            break;
        case 9:
            snprintf(GLCANBUSInterolationErrorDesc, sizeof(GLCANBUSInterolationErrorDesc), "PDO send failed");
            break;
    }
    
    return (char*)GLCANBUSInterolationErrorDesc;
}
        
        
        
        
        
uint32_t GLPDOCounter = 0;


/////////////////////////////////////////////
// callback pacchetto UDP (schede CANBUS)
//

void udpCANHandler(IpAddr_t ip, uint16_t sport, uint16_t dport, int8_t *buf, uint16_t buf_size) {

    if (buf) {

        if (buf_size) {
            uint16_t udpLen = xrt_ntohs((uint16_t) buf_size);

            // IpHeader *ip = (IpHeader *)(packet + sizeof(EthHeader));
            // UdpHeader *udp2 = (UdpHeader *)(ip->payloadPtr());
            // uint8_t *pData = (uint8_t *)(udp + sizeof(UdpHeader));

            uint8_t *pData = (uint8_t *) (buf);
            uint8_t IOBoardFound = 0;
            unsigned char opCode = 0;

            char str[256];



            for (int i = 0; i < machine.numCANSlots; i++) {

                if (machine.CANSlots[i].boardId) {

                    if (machine.CANSlots[i].ip[0] == ip[0] && machine.CANSlots[i].ip[1] == ip[1] &&
                            machine.CANSlots[i].ip[2] == ip[2] && machine.CANSlots[i].ip[3] == ip[3]) {

                        IOBoardFound = true;

                        // Reset watch dog
                        // machine.CANSlots[i].tickTocIOWatchDog = 0;
                        machine.CANSlots[i].t1 = machine.CANSlots[i].tickTocIOWatchDog = (uint32_t) xTaskGetTickCount();


                        machine.CANSlots[i].data_available = buf_size;


                        machine.CANSlots[i].readCount++;
                        machine.CANSlots[i].statReadCount++;


                        opCode = pData[0];


                        if (opCode == '=') {
                            unsigned char canId = 0;
                            unsigned char retCmd = 0;
                            unsigned short objectID = 0;
                            unsigned char subObjectID = 0;
                            int32_t driverData = 0;
                            int16_t CANSendError = 0;
                            int16_t driverData16H = 0, driverData16L = 0;
                            uint16_t CANSeqID = 0;


                            memcpy(&canId, &pData[1], 1);
                            memcpy(&retCmd, &pData[2], 1);
                            memcpy(&objectID, &pData[3], 2);
                            memcpy(&subObjectID, &pData[5], 1);
                            memcpy(&driverData, &pData[6], 4);
                            memcpy(&driverData16H, &pData[6], 2);
                            memcpy(&driverData16L, &pData[8], 2);
                            memcpy(&CANSeqID, &pData[10], 2);

                            if (buf_size >= 14) {
                                memcpy(&CANSendError, &pData[12], 2);
                            }

                            machine.CANSlots[i].streamErrorCount = 0;

                            for (int iact = 0; iact < machine.CANSlots[i].nActuators; iact++) {
                                LP_ACTUATOR pActuator = (LP_ACTUATOR) machine.CANSlots[i].pActuators[iact];

                                if (pActuator->boardId == machine.CANSlots[i].boardId && pActuator->stationId == canId) {

                                    pActuator->readCounter++;

                                    if ((uint32_t) CANSeqID > pActuator->CANSeqID || CANSeqID == 0) {

                                        pActuator->CANSeqID = (uint32_t) CANSeqID;


                                        switch (objectID) {

                                                ///////////////////////////
                                                // Posizione
                                                //                                           
                                            case 0x6064:
                                                if ((retCmd & 64) && (retCmd & 32)) {
                                                    // Write result :
                                                } else if ((retCmd & 64) && !(retCmd & 32)) {
                                                    // Read result
                                                    handle_canbus_pulses_change ( (void*)pActuator, driverData, CANSeqID );
                                                    pActuator->driverPosCount++;
                                                } else {
                                                    snprintf(App.Msg, App.MsgSize, "%s[CAN:cPos Error [%d][%d] - [%d][%d] - [%d][%d][%d] = %d]%s"
                                                            , (char*) ANSI_COLOR_RED
                                                            , (retCmd & 1), (retCmd & 2)
                                                            , (retCmd & 4), (retCmd & 8)
                                                            , (retCmd & 128), (retCmd & 64), (retCmd & 32)
                                                            , (int32_t) driverData
                                                            , (char*) ANSI_COLOR_RESET);
                                                    vDisplayMessage(App.Msg);
                                                }
                                                break;



                                                ///////////////////////////
                                                // Mode
                                                //
                                            case 0x6060:
                                                if ((retCmd & 64) && (retCmd & 32)) {
                                                    // Write result :
                                                } else if ((retCmd & 64) && !(retCmd & 32)) {
                                                    // Read result
                                                } else {
                                                    snprintf(App.Msg, App.MsgSize, "%s[CAN:Mode Error:%d]%s", (char*) ANSI_COLOR_RED, (int32_t) driverData, (char*) ANSI_COLOR_RESET);
                                                    vDisplayMessage(App.Msg);
                                                }
                                                break;


                                                ///////////////////////
                                                // Control Word
                                                //
                                            case 0x6040:
                                                if ((retCmd & 64) && (retCmd & 32)) {
                                                    // Write result :
                                                    // Comando avvio ricevuto (Control Word)
                                                    pActuator->driverStatus |= BIT1;
                                                } else if ((retCmd & 64) && !(retCmd & 32)) {
                                                    // Read result
                                                    // Comando avvio letto dal driver (Control Word)
                                                    pActuator->driverStatus |= BIT11;
                                                } else {
                                                    snprintf(App.Msg, App.MsgSize, "%s[CAN:CW Error:%d]%s", (char*) ANSI_COLOR_RED, (int32_t) driverData, (char*) ANSI_COLOR_RESET);
                                                    vDisplayMessage(App.Msg);
                                                }
                                                break;


                                                ///////////////////////
                                                // Status Word
                                                //
                                            case 0x6041:
                                                if ((retCmd & 64) && (retCmd & 32)) {
                                                    // Write result :
                                                } else if ((retCmd & 64) && !(retCmd & 32)) {
                                                    // Read result
                                                    // 15~11    Operation / Manufacturer Specific

                                                    // 10   Target reached
                                                    // 9    Remote
                                                    // 8~6  N/A
                                                    // 5    Quick stop
                                                    // 4    N/A
                                                    // 3    Fault
                                                    // 2    Operation enabled
                                                    // 1    Switched on
                                                    // 0    Ready to switch on

                                                } else {
                                                    snprintf(App.Msg, App.MsgSize, "%s[CAN:SW Error:%d]%s", (char*) ANSI_COLOR_RED, (int32_t) driverData, (char*) ANSI_COLOR_RESET);
                                                    vDisplayMessage(App.Msg);
                                                }
                                                break;



                                                //////////////////////////////////////////////
                                                // Following error window (pulses) : 6065h
                                                //





                                                ///////////////////////////////////////////////////        
                                                // Following error actual value (pulses) : 60F4h
                                                //
                                            case 0x60F4:
                                                if ((retCmd & 64) && (retCmd & 32)) {
                                                    // Write result :
                                                } else if ((retCmd & 64) && !(retCmd & 32)) {                                                    
                                                    float folowingErrMM = (float) driverData / (float)pActuator->pulsesPerTurn * pActuator->cam_ratio;

                                                    if (machine.CANSlots[i].runningCommand == 1) {
                                                        pActuator->cur_follow_error1 = folowingErrMM;
                                                    } else if (machine.CANSlots[i].runningCommand == -1) {
                                                        pActuator->cur_follow_error2 = folowingErrMM;
                                                    } else {
                                                        if (machine.CANSlots[i].doneCommand == 1) {
                                                            pActuator->cur_follow_error1 = folowingErrMM;
                                                        } else if (machine.CANSlots[i].doneCommand == -1) {
                                                            pActuator->cur_follow_error2 = folowingErrMM;
                                                        }
                                                    }
                                                } else {
                                                    snprintf(App.Msg, App.MsgSize, "%s[CAN: F.Err.0x60F4 Error:%d]%s", (char*) ANSI_COLOR_RED, (int32_t) driverData, (char*) ANSI_COLOR_RESET);
                                                    vDisplayMessage(App.Msg);
                                                }
                                                break;



                                                ///////////////////////
                                                // Posizione Arrivo
                                                //
                                            case 0x607A:
                                                if ((retCmd & 64) && (retCmd & 32)) {
                                                    // Write result :
                                                    // BIT2 ->  Comando posizione arrivo ricevuto
                                                    pActuator->driverStatus |= BIT2;
                                                } else if ((retCmd & 64) && !(retCmd & 32)) {
                                                    // Read result
                                                    // Comando posizione arrivo letto dal driver
                                                    pActuator->driverStatus |= BIT12;
                                                } else {
                                                    snprintf(App.Msg, App.MsgSize, "%s[CAN:TPos Error:%d]%s", (char*) ANSI_COLOR_RED, (int32_t) driverData, (char*) ANSI_COLOR_RESET);
                                                    vDisplayMessage(App.Msg);
                                                }
                                                break;


                                                ///////////////////////////
                                                // Velocità corrente
                                                //
                                            case 0x606C:
                                                if ((retCmd & 64) && (retCmd & 32)) {
                                                    // Write result :
                                                } else if ((retCmd & 64) && !(retCmd & 32)) {
                                                    // Read result                                               
                                                    int32_t driverSpeed = (int32_t) 0;                            
                                                    memcpy (&driverSpeed, &driverData, sizeof(int32_t));

                                                    pActuator->driverSpeed = (int32_t)((float)driverSpeed / 10.0f);
                                                    pActuator->driverSpeedCount++;
                                                    
                                                    pActuator->speed = (float) driverSpeed / 10.0f;
                                                    if (pActuator->speed > pActuator->max_speed)
                                                        pActuator->max_speed = pActuator->speed;
                                                    if (pActuator->speed < pActuator->min_speed)
                                                        pActuator->min_speed = pActuator->speed;
                                                } else {
                                                    snprintf(App.Msg, App.MsgSize, "%s[CAN:S Error:%d]%s", (char*) ANSI_COLOR_RED, (int32_t) driverData, (char*) ANSI_COLOR_RESET);
                                                    vDisplayMessage(App.Msg);
                                                }
                                                break;

                                                ///////////////////////////
                                                // Velocità
                                                //
                                            case 0x6081:
                                                if ((retCmd & 64) && (retCmd & 32)) {
                                                    // Write result :
                                                    // Comando avvio ricevuto (Status Word)
                                                    pActuator->driverStatus |= BIT3;
                                                } else if ((retCmd & 64) && !(retCmd & 32)) {
                                                    // Read result
                                                    // Comando velocita letto dal driver
                                                    pActuator->driverStatus |= BIT13;

                                                    pActuator->speed = (float) driverData;
                                                    if (pActuator->speed > pActuator->max_speed)
                                                        pActuator->max_speed = pActuator->speed;
                                                    if (pActuator->speed < pActuator->min_speed)
                                                        pActuator->min_speed = pActuator->speed;
                                                } else {
                                                    snprintf(App.Msg, App.MsgSize, "%s[CAN:S Error:%d]%s", (char*) ANSI_COLOR_RED, (int32_t) driverData, (char*) ANSI_COLOR_RESET);
                                                    vDisplayMessage(App.Msg);
                                                }
                                                break;

                                                
                                                
                                                ///////////////////////////
                                                // Accelerazione
                                                //
                                            case 0x6083:
                                                if ((retCmd & 64) && (retCmd & 32)) {
                                                    // Write result :
                                                    // Comando Accelerazione ricevuto
                                                    pActuator->driverStatus |= BIT4;
                                                } else if ((retCmd & 64) && !(retCmd & 32)) {
                                                    // Read result
                                                    // Comando Accelerazione letto dal driver
                                                    pActuator->driverStatus |= BIT14;
                                                    pActuator->driverAcc = (int32_t) driverData;
                                                } else {
                                                    snprintf(App.Msg, App.MsgSize, "%s[CAN:A Error:%d]%s", (char*) ANSI_COLOR_RED, (int32_t) driverData, (char*) ANSI_COLOR_RESET);
                                                    vDisplayMessage(App.Msg);
                                                }
                                                break;

                                                ///////////////////////////
                                                // Decelerazione
                                                //
                                            case 0x6084:
                                                if ((retCmd & 64) && (retCmd & 32)) {
                                                    // Write result :
                                                    // Comando Decelerazione ricevuto
                                                    pActuator->driverStatus |= BIT5;
                                                } else if ((retCmd & 64) && !(retCmd & 32)) {
                                                    // Read result
                                                    // Comando Decelerazione letto dal driver
                                                    pActuator->driverStatus |= BIT15;
                                                    pActuator->driverAcc = (int32_t) driverData;
                                                } else {
                                                    snprintf(App.Msg, App.MsgSize, "%s[CAN:D Error:%d]%s", (char*) ANSI_COLOR_RED, (int32_t) driverData, (char*) ANSI_COLOR_RESET);
                                                    vDisplayMessage(App.Msg);
                                                }
                                                break;


                                                ////////////////////////////////////////
                                                // Rated Torque  (x1000)
                                                // Unit: per thousand of rated torque
                                                //
                                            case 0x6076:
                                                if ((retCmd & 64) && (retCmd & 32)) {
                                                } else if ((retCmd & 64) && !(retCmd & 32)) {
                                                    // Read result
                                                    pActuator->rated_torque = (float) driverData16H / 1000.0f; // N.B.: Dato in % (0 - 1) (* 100 dalla ui-io.cpp)
                                                    driverData16L = driverData16L;
                                                } else {
                                                    snprintf(App.Msg, App.MsgSize, "%s[CAN:D Error:%d]%s", (char*) ANSI_COLOR_RED, (int32_t) driverData, (char*) ANSI_COLOR_RESET);
                                                    vDisplayMessage(App.Msg);
                                                }
                                                break;

                                                
                                                
                                            default:
                                                break;
                                        }

                                    } else {
                                        pActuator->CANSeqID = (uint32_t) CANSeqID;
                                        snprintf(App.Msg, App.MsgSize, "[%s!CAN.SEQ:%d/%d%s]", (char*) ANSI_COLOR_RED, (uint32_t) pActuator->CANSeqID, (uint32_t) CANSeqID, (char*) ANSI_COLOR_RESET);
                                        vDisplayMessage(App.Msg);
                                    }
                                                                        
                                    break;                                    
                                }
                            }

                            // Comando in esecuzione ?
                            if (machine.CANSlots[i].runningCommand) {
                            }


                        } else if (opCode == 'E') {
                            // Emergency
                            unsigned char canId = 0;
                            int8_t errorCode = 0;
                            int16_t errorRegister = 0;
                            int32_t errorData = 0;
                            uint16_t  CANSendError = 0;

                            memcpy(&canId, &pData[1], 1);
                            memcpy(&errorCode, &pData[2], 2);
                            memcpy(&errorRegister, &pData[4], 1);
                            memcpy(&errorData, &pData[5], 4);
                            memcpy(&CANSendError, &pData[9], 2);

                            
                            snprintf(str, sizeof(str), "[CANBUS Emergency - errorCode:%d - errorRegister:%x - %s ]", errorCode, errorRegister, get_emc_error(errorData) );
                            if (generate_alarm((char*) str, 6400, 0, (int) ALARM_ERROR, 0 + 1) < 0) {
                            }
                            
                            if (errorCode == 0 && errorRegister && errorData == 0) {
                                for (int iact = 0; iact < machine.CANSlots[i].nActuators; iact++) {
                                    LP_ACTUATOR pActuator = (LP_ACTUATOR) machine.CANSlots[i].pActuators[iact];
                                    if (pActuator->Id == canId) {
                                        pActuator->homingDone = false;
                                        snprintf(str, sizeof(str), "Actuaror %s need homing procedure", pActuator->name );
                                        if (generate_alarm((char*) str, 6030, 0, (int) ALARM_WARNING, 0 + 1) < 0) {
                                        }
                                    }
                                }
                            }

                        } else if (opCode == 'P') {
                            ////////////////////////
                            // PDO 0x6064 Pulse
                            //
                            unsigned char canId = 0;
                            int32_t driverData  = 0;
                            uint32_t CANSeqID = 0xFFFFFFFF;
        
                            memcpy(&canId, &pData[1], 1);
                            memcpy(&driverData, &pData[2], 4);

#ifdef PRINT_PDO_PULSES
                            if (App.DebugMode) {
                                if (GLPDOCounter % 1000 == 0) {
                                    snprintf(str, sizeof(str), "%s[PDO recived...", (char*) ANSI_COLOR_BLUE);
                                    vDisplayMessage(str);
                                }
                            }
#endif
                            
                            machine.CANSlots[i].readPDOCount++;
                            
                            for (int iact = 0; iact < machine.CANSlots[i].nActuators; iact++) {
                                LP_ACTUATOR pActuator = (LP_ACTUATOR) machine.CANSlots[i].pActuators[iact];

                                if (pActuator->boardId == machine.CANSlots[i].boardId && pActuator->stationId == canId) {
                                    
                                    
                                    pActuator->readCounter++;
                                    pActuator->driverPosCount++;
                                    

                                    // pActuator->CANSeqID = (uint32_t) CANSeqID;
                                    
                                    handle_canbus_pulses_change ( (void*)pActuator, driverData, CANSeqID );

#ifdef PRINT_PDO_PULSES
                                    if (App.DebugMode) {                                    
                                        if (GLPDOCounter % 1000 == 0) {
                                            snprintf(str, sizeof(str), "Set #%d=%d", (int32_t) canId, (uint32_t)driverData );
                                            vDisplayMessage(str);
                                        }
                                    }
#endif
                                    
                                    break;
                                }
                            }

#ifdef PRINT_PDO_PULSES
                            if (App.DebugMode) {
                                if (GLPDOCounter % 1000 == 0) {
                                    snprintf(str, sizeof(str), "]%s", (char*) ANSI_COLOR_RESET);
                                    vDisplayMessage(str);
                                }
                            }
#endif
                            
                            GLPDOCounter++;

                        } else if (opCode == '#') {
                            ////////////////////////////
                            // Command/Setup feedback
                            //
                            char cmdResult[256];
                            
                            memcpy(cmdResult, &pData[1], MIN(buf_size, sizeof(cmdResult)));
                            
                            if (cmdResult[0]=='S' && cmdResult[1]=='I' && cmdResult[2]=='=') {
                                if (machine.CANSlots[i].si_ms != cmdResult[3]) {
                                    snprintf(str, sizeof(str), "%s[CAN: SI=%d]%s", (char*) ANSI_COLOR_RED, (int32_t) cmdResult[4], (char*) ANSI_COLOR_RESET);
                                    vDisplayMessage(str);
                                }
                            } else if (cmdResult[0]=='R' && cmdResult[1]=='I' && cmdResult[2]=='=') {
                                if (machine.CANSlots[i].ri_ms != cmdResult[3]) {
                                    snprintf(str, sizeof(str), "%s[CAN: RI=%d]%s", (char*) ANSI_COLOR_RED, (int32_t) cmdResult[4], (char*) ANSI_COLOR_RESET);
                                    vDisplayMessage(str);
                                }
                                
                            } else if (cmdResult[0]=='+') {
                                uint8_t errors = (uint8_t)cmdResult[1];
                                uint8_t stationId = (uint8_t)cmdResult[2];
                                uint32_t targetPos = 0;
                                
                                memcpy(&targetPos, &cmdResult[3], 4);
                                        
                                if (errors) {
                                    //////////////////////////////////////
                                    // Generazione Emergenza Macchina
                                    //
                                    if (generate_alarm((char*) "CAN : Full Start CMD error : can queue error", 5005, 0, (int) ALARM_ERROR, 0 + 1) < 0) {
                                    }
                                    
                                    if ((uint8_t)cmdResult[2] == 0) {
                                        //////////////////////////////////////
                                        // Generazione Emergenza Macchina
                                        //
                                        if (generate_alarm((char*) "CAN : Full Start CMD error : no station Id", 5006, 0, (int) ALARM_ERROR, 0 + 1) < 0) {
                                        }
                                    } else {
                                        if (targetPos != machine.CANSlots[i].lastTargetPos) {
                                            //////////////////////////////////////
                                            // Generazione Emergenza Macchina
                                            //
                                            if (generate_alarm((char*) "CAN : Full Start CMD error : wrong canbus target", 5007, 0, (int) ALARM_ERROR, 0 + 1) < 0) {
                                            }
                                        } else {
                                            for (int iact = 0; iact < machine.CANSlots[i].nActuators; iact++) {
                                                LP_ACTUATOR pActuator = (LP_ACTUATOR) machine.CANSlots[i].pActuators[iact];
                                                if (pActuator->boardId == machine.CANSlots[i].boardId && pActuator->stationId == stationId) {                                    
                                                    if (targetPos != pActuator->lastTargetPos) {
                                                        //////////////////////////////////////
                                                        // Generazione Emergenza Macchina
                                                        //
                                                        if (generate_alarm((char*) "CAN : Full Start CMD error : wrong actuator target", 5008, 0, (int) ALARM_ERROR, 0 + 1) < 0) {
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    // Azzeramento del timeput fullCMD format
                                    machine.CANSlots[i].fullCMDFeedbackStartTime = 0;

                                } else {
                                    // Azzeramento del timeput fullCMD format
                                    machine.CANSlots[i].fullCMDFeedbackStartTime = 0;
                                }
                            
                                
                                /////////////////////////////////
                                // Interpolazione multiasse
                                //
                            } else if ((uint8_t)cmdResult[0] == (uint8_t)0xA7) { // §
                                uint8_t errorCode = (uint8_t)cmdResult[1];
                                uint8_t numAxis = (uint8_t)cmdResult[2];
                                int32_t targetPosX = 0;
                                int32_t targetPosY = 0;
                                int32_t targetPosZ = 0;
                                
                                memcpy(&targetPosX, &cmdResult[3], 4);
                                memcpy(&targetPosY, &cmdResult[7], 4);
                                memcpy(&targetPosZ, &cmdResult[11], 4);
                                        
                                if (errorCode == 255) {
                                    ///////////////////////////////////////////
                                    // Fine Interpolazione con successo
                                    //
                                    for (int iact = 0; iact < machine.CANSlots[i].nActuators; iact++) {
                                        LP_ACTUATOR pActuator = (LP_ACTUATOR) machine.CANSlots[i].pActuators[iact];
                                        if (pActuator->step == STEP_MOVING) {
                                            if (pActuator->target_position == INTERPOLATE_POSITION) {
                                                float posErr = fabs(pActuator->cur_rpos - pActuator->target_rpos);
                                                float posErr2 = fabs((float)targetPosX / pActuator->pulsesPerTurn * pActuator->cam_ratio - pActuator->target_rpos);
                                                float target_rpos_toll = MAX(pActuator->start_rpos_toll, pActuator->end_rpos_toll);

                                                if (target_rpos_toll <= 0.0f)
                                                    target_rpos_toll = machine.App.Epsilon;
                                                
                                                if (posErr > target_rpos_toll) {
                                                    snprintf(str, sizeof(str), "[INTERPL] Actuator %s target position cErr:%0.3f / pErr:%0.3f", pActuator->name, posErr, posErr2);
                                                    if (generate_alarm((char*) str, 5555, 0, (int) ALARM_WARNING, 0 + 1) < 0) {
                                                    }
                                                }

                                                /////////////////////////////////////
                                                // Assegnamento movimento eseguito
                                                //
                                                handle_actuator_position_reached ( (void*) pActuator );
                                            }
                                        }
                                    }

                                } else if (errorCode == 254) {
                                    ///////////////////////////////////////
                                    // Fine Interpolazione con errore
                                    //
                                    snprintf(str, sizeof(str), "[INTERPL] Error : %d", numAxis);
                                    if (generate_alarm((char*) str, 5555, 0, (int) ALARM_ERROR, 0 + 1) < 0) {
                                    }
                                    
                                } else if (errorCode) {
                                    //////////////////////////////////////
                                    // Errore avvio interpolazione
                                    //
                                    // Generazione Emergenza Macchina
                                    //
                                    snprintf(str, sizeof(str), "[INTERPL] MultiAxis Full Start CMD error : error:%d (%s)", errorCode, get_interpolation_error_desc(errorCode, targetPosX, targetPosY, targetPosZ) );
                                    if (generate_alarm((char*) str, 5005, 0, (int) ALARM_ERROR, 0 + 1) < 0) {
                                    }
                                    
                                } else {
                                    if (numAxis == 0) {
                                        //////////////////////////////////////
                                        // Generazione Emergenza Macchina
                                        //
                                        snprintf(str, sizeof(str), "CAN : MultiAxis Full Start CMD error : no axis set");
                                        if (generate_alarm((char*) str, 5006, 0, (int) ALARM_ERROR, 0 + 1) < 0) {
                                        }
                                    } else {
                                        // Azzeramento del timeput fullCMD format
                                        machine.CANSlots[i].fullCMDFeedbackStartTime = 0;
                                    }
                                }

                                
                                
                            } else if (cmdResult[0] == '?') {
                                // Errore inatteso
                                uint8_t errorCode = (uint8_t)cmdResult[1];
                                uint8_t stationId = (uint8_t)cmdResult[2];
                                
                                if (errorCode == 3) {
                                    //////////////////////////////////////
                                    // Generazione Emergenza Macchina
                                    //
                                    snprintf(str, sizeof(str), "CAN: ETH packet size error:%d - max size:%d", errorCode, stationId);
                                    if (generate_alarm((char*) str, 5005, 0, (int) ALARM_ERROR, 0 + 1) < 0) {
                                    }
                                } else {
                                    //////////////////////////////////////
                                    // Generazione Emergenza Macchina
                                    //
                                    snprintf(str, sizeof(str), "CAN: Unexpected ETH error:%d - stationId:%d ", errorCode, stationId);
                                    if (generate_alarm((char*) str, 5005, 0, (int) ALARM_ERROR, 0 + 1) < 0) {
                                    }
                                }
                                
                            } else {
                                snprintf(str, sizeof(str), "%s[CAN: Unknown CMD Feedback:%s]%s", (char*) ANSI_COLOR_RED, (char *) cmdResult, (char*) ANSI_COLOR_RESET);
                                vDisplayMessage(str);
                            }


                            
                        } else {
                            snprintf(str, sizeof(str), "[CANBUS Error - opCode:%d]", (int) opCode);
                            if (generate_alarm((char*) str, 6401, 0, (int) ALARM_ERROR, 0 + 1) < 0) {
                            }
                        }
                    }
                }
            }

            if (!IOBoardFound) {
                snprintf(str, sizeof(str), "[!CANBUSxIP:%d.%d.%d.%d]\n", (int) ip[0], (int) ip[1], (int) ip[2], (int) ip[3]);
                vDisplayMessage(str);

                for (int i = 0; i < machine.numCANSlots; i++) {
                    snprintf(str, sizeof(str), "[CANBUS ID:%d - IP:%d.%d.%d.%d]\n", (int) machine.CANSlots[i].boardId, machine.CANSlots[i].ip[0], machine.CANSlots[i].ip[1], machine.CANSlots[i].ip[2], machine.CANSlots[i].ip[3]);
                    vDisplayMessage(str);
                }

                //////////////////////////////////////
                // Generazione Emergenza Macchina
                //
                if (generate_alarm((char*) "IO Communication board not found", 5004, 0, (int) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                }
            }
        }

    } else {
#ifdef DEBUG_PRINTF
        snprintf(App.Msg, App.MsgSize, "[!packet]\n");
        vDisplayMessage(App.Msg);
#endif
    }
}
