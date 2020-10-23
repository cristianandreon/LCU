//============================================================================
// Name        : dataExchangeSER
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

int dataExchangeDumpSerial(char *msg, size_t msg_size) {
    for (int i = 0; i < machine.numSerialSlots; i++) {
        snprintf(msg, msg_size, "[SER#%d - ID:%d - baud:%d - cycles:%d - tout:%d - state:%d.%d - Pos.:%d - Speed:%d - home:%d.%d]\n"
                , (int) machine.serialSlots[i].boardId
                , (int) machine.serialSlots[i].stationId
                , (int) machine.serialSlots[i].baud
                , (int) machine.serialSlots[i].readCount
                , (int) machine.serialSlots[i].timeoutCount
                , (int) machine.serialSlots[i].state, (machine.serialSlots[i].pActuator ? ((LP_ACTUATOR) machine.serialSlots[i].pActuator)->homingSeq : 0)
                , (int) machine.serialSlots[i].driverPosition
                , (int) machine.serialSlots[i].driverSpeed
                , (machine.serialSlots[i].pActuator ? ((LP_ACTUATOR) machine.serialSlots[i].pActuator)->homingTurnsPPT : 0)
                , (machine.serialSlots[i].pActuator ? ((LP_ACTUATOR) machine.serialSlots[i].pActuator)->homingPulsesPPT : 0)
                );
        vDisplayMessage(msg);
    }
    return 1;
}

char *get_serial_status(void *pvSerialSlots) {

    if (pvSerialSlots) {
        SerialSlot *serialSlots = (SerialSlot *) pvSerialSlots;

        switch (serialSlots->state) {
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

/*
 * 
 
Pr7.00 Station number setting 1 1~254 / ○ P S T
 
 
Pr7.01
    Communication transmission rate:
    0：4800；
    1：9600；
    2：19200；
    3：38400；
    4：57600；
    1 0~4 / ○ P S T

 Pr7.02
    Communication data format:
    0: no check 1+8+N+1；
    1: odd check 1+8+O+1；
    2: even check 1+8+E+1；
    3: no check 1+8+N+2；
    4: odd check 1+8+O+2；
    5: even check 1+8+E+2；
   
 */






int dataExchangeResetSerial() {
    int i;

    for (i = 0; i < machine.numSerialSlots; i++) {
        machine.serialSlots[i].state = -1;
    }

    usleep(1 * 1000);

    for (i = 0; i < machine.numSerialSlots; i++) {
        if (machine.serialSlots[i].modbus_ctx) {
            modbus_close((modbus_t *) machine.serialSlots[i].modbus_ctx);
            // modbus_free((modbus_t *)machine.serialSlots[i].modbus_ctx);
        }
    }

    for (i = 0; i < machine.numSerialSlots; i++) {
        if (machine.serialSlots[i].modbus_ctx) {
            machine.serialSlots[i].timeoutCount = 0;
            modbus_connect((modbus_t *) machine.serialSlots[i].modbus_ctx);
            // modbus_free((modbus_t *)machine.serialSlots[i].modbus_ctx);
        }
    }

    return 0;
}

int dataEchangeStopSerial() {
    App.SERRunLoop = 0;
}


// Ritorna 0 se lo stato passa a bloccato (-1)

int handle_serial_borad_init_error(int i) {
    int retVal = 0;
    machine.serialSlots[i].timeoutCount++;
    if (machine.serialSlots[i].timeoutCount > MAX_SERIAL_ATTEMPS) {
        snprintf(App.Msg, App.MsgSize, "[%s board:%d - stopping communication (%s) %s]\n", (char*) ANSI_COLOR_MAGENTA, i + 1, (char*) modbus_strerror(errno), (char*) ANSI_COLOR_RESET);
        vDisplayMessage(App.Msg);
        machine.serialSlots[i].state = -1;
        retVal = 0;
    } else {
        // Ripete il ciclo
        machine.serialSlots[i].state = DEV_STATE_INIT;
        retVal = 1;
    }
    return retVal;
}

int handle_serial_borad_error(int i, char *msg, int state) {
    int retVal = 0;

    machine.serialSlots[i].timeoutCount++;

    if (machine.serialSlots[i].timeoutCount > MAX_SERIAL_LOOP_ERROR) {

        if (state >= 0) {
            snprintf(App.Msg, App.MsgSize, "[%s board:%d - stream error (%s) (%s) state:%d %s]\n", (char*) ANSI_COLOR_MAGENTA, i + 1, (char*) msg, (char*) modbus_strerror(errno), state, (char*) ANSI_COLOR_RESET);
        } else {
            snprintf(App.Msg, App.MsgSize, "[%s board:%d - %s %s]\n", (char*) ANSI_COLOR_MAGENTA, i + 1, (char*) msg, (char*) ANSI_COLOR_RESET);
        }
        vDisplayMessage(App.Msg);

        dataExchangeResetSerial();


        if (modbus_purge_comm((modbus_t*) machine.serialSlots[i].modbus_ctx)) {
        }


        machine.serialSlots[i].state = DEV_STATE_STREAMING_SEND;
        retVal = 0;

        /////////////////////////
        // Generazione Allarme
        //
        if (generate_alarm((char*) "Serial Communication Timeout", 6001, 0, (int) ALARM_FATAL_ERROR, 0 + 1) < 0) {
        }


    } else {
        // Ripete il ciclo

        if (state >= 0) {
            snprintf(App.Msg, App.MsgSize, "[%s board:%d - (%s)(%s) state:%d %s]\n", (char*) ANSI_COLOR_RED, i + 1, (char*) msg, (char*) modbus_strerror(errno), state, (char*) ANSI_COLOR_RESET);
        } else {
            snprintf(App.Msg, App.MsgSize, "[%s board:%d - %s %s]\n", (char*) ANSI_COLOR_RED, i + 1, (char*) msg, (char*) ANSI_COLOR_RESET);
        }
        vDisplayMessage(App.Msg);

        machine.serialSlots[i].state = DEV_STATE_STREAMING_SEND;
        retVal = 1;
    }
    return retVal;
}

int read_serial_cfg() {
    int retVal = 0;
    FILE *f = fopen(SER_BOARD_CFG_FILE, "r");

    machine.numSerialSlots = 0;

    if (f) {
        char str[256];
        char *pstr = NULL, *pSer = NULL;


        while (fgets(str, sizeof (str), f)) {
            pSer = strstr(str, "=");
            if (pSer) {
                pSer[0] = 0;
                pSer++;

                if (str[0] == '#') {
                    int boardId = atoi((char*) &str[1]);
                    if (boardId >= 1 && boardId <= machine.numSerialSlotsAllocated) {
                        if (pSer) {
                            SerialSlot *serialSlot = (SerialSlot *) & machine.serialSlots[boardId - 1];

                            serialSlot->boardId = boardId;

                            // Device
                            pstr = strstr(pSer, ".");
                            if (pstr) {
                                pstr[0] = 0;
                                COPY_POINTER(serialSlot->device, pSer);
                                pSer = pstr + 1;
                            }

                            // Baud
                            pstr = strstr(pSer, ".");
                            if (pstr) {
                                pstr[0] = 0;
                                serialSlot->baud = atoi(pSer);
                                pSer = pstr + 1;
                            }

                            // data bit
                            pstr = strstr(pSer, ".");
                            if (pstr) {
                                pstr[0] = 0;
                                serialSlot->data_bit = atoi(pSer);
                                pSer = pstr + 1;
                            }

                            // parity bit
                            pstr = strstr(pSer, ".");
                            if (pstr) {
                                pstr[0] = 0;
                                serialSlot->parity = (char) pSer[0];
                                pSer = pstr + 1;
                            }

                            // stop bit
                            pstr = strstr(pSer, ".");
                            if (pstr) {
                                pstr[0] = 0;
                                pSer = pstr + 1;
                            }
                            serialSlot->stop_bit = atoi(pSer);

                            // Station ID
                            pstr = strstr(pSer, ".");
                            if (pstr) {
                                pstr[0] = 0;
                                pSer = pstr + 1;
                            }
                            serialSlot->stationId = atoi(pSer);


                            if (boardId > machine.numSerialSlots) {
                                machine.numSerialSlots = boardId;
                            }

                            /*
                            snprintf(App.Msg, App.MsgSize, "[CFG serial : ID:%d - %s @%d %d-%c-%d]\n", serialSlot->boardId, serialSlot->device, serialSlot->baud, serialSlot->data_bit, (char)serialSlot->parity, serialSlot->stop_bit );
                            vDisplayMessage(App.Msg);
                             */
                        }




                    } else {
                        // Numero scheda non valido
                        snprintf(App.Msg, App.MsgSize, "[CFG !serial addr]\n");
                        vDisplayMessage(App.Msg);
                    }

                } else {
                    // Indirizzo board non valido
                    snprintf(App.Msg, App.MsgSize, "[CFG !serial #]\n");
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

int update_serial_cfg() {
    int retVal = 0;

    if (machine.numSerialSlots) {
        char str[256];
        FILE *f = fopen(SER_BOARD_CFG_FILE, "w");
        if (f) {
            for (int i = 0; i < machine.numSerialSlots; i++) {
                SerialSlot *serialSlot = &machine.serialSlots[i];

                snprintf(str, sizeof (str), "#%d=%s.%d.%d.%c.%d.%d\n", serialSlot->boardId, serialSlot->device, serialSlot->baud, serialSlot->data_bit, serialSlot->parity, serialSlot->stop_bit, serialSlot->stationId);
                if (fputs(str, f) != 0) {
                    if (retVal == 0)
                        retVal++;
                } else {
                    retVal = -1;
                    snprintf(App.Msg, App.MsgSize, "[SER CFG update error]\n");
                    vDisplayMessage(App.Msg);
                }
            }

            fclose(f);
            f = NULL;

        } else {
            snprintf(App.Msg, App.MsgSize, "[SER CFG file not open]\n");
            vDisplayMessage(App.Msg);
            retVal = -1;
        }

    } else {
        snprintf(App.Msg, App.MsgSize, "[update_serial_cfg : No Serial boards]\n");
        vDisplayMessage(App.Msg);
        retVal = 0;
    }

    return retVal;
}

int handle_modbus_connect(SerialSlot *serialSlots, uint32_t slotIndex, char *device, int baud, char parity, int data_bit, int stop_bit) {

    int i, j, i_act, res;
    modbus_t *ctx = NULL;
    int retVal = 0;


    ctx = modbus_new_rtu(device, baud, parity, data_bit, stop_bit);

    if (ctx) {
        // Assegnamento della slot
        if (slotIndex < machine.numSerialSlotsAllocated) {


            res = modbus_connect(ctx);
            if (res >= 0) {


#ifdef MODBUS_DEBUG
                // Debug mode                        
                modbus_set_debug(ctx, 1);
#endif




                uint32_t tv_sec = MODBUS_TIMEOUT_SEC;
                uint32_t tv_usec = MODBUS_TIMEOUT_USEC;
                modbus_get_byte_timeout(ctx, &tv_sec, &tv_usec);


                tv_sec = MODBUS_TIMEOUT_SEC;
                tv_usec = MODBUS_TIMEOUT_USEC;
                modbus_set_response_timeout(ctx, tv_sec, tv_usec);

                // xrt_make_socket_non_blocking (ctx);


                int rts = modbus_rtu_get_rts(ctx);

                int res = modbus_rtu_set_rts(ctx, MODBUS_RTU_RTS_NONE);


                serialSlots[slotIndex].boardId = slotIndex + 1;
                if (!serialSlots[slotIndex].stationId)
                    serialSlots[slotIndex].stationId = slotIndex + 1;


                snprintf(App.Msg, App.MsgSize,
                        "[%s%s #%d.%d - @%d %d%c%d OK%s]",
                        (char*) ANSI_COLOR_GREEN,
                        (char*) device,
                        serialSlots[slotIndex].boardId, serialSlots[slotIndex].stationId,
                        (int32_t) baud, (int32_t) data_bit, (char) parity, (int32_t) stop_bit,
                        (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);

                // id impostato dalle lettura dello slave
                // machine.serialSlots[slotIndex].board_id = slotIndex;

                serialSlots[slotIndex].modbus_ctx = ctx;
                serialSlots[slotIndex].device = device;
                serialSlots[slotIndex].baud = baud;
                serialSlots[slotIndex].parity = parity;
                serialSlots[slotIndex].data_bit = data_bit;
                serialSlots[slotIndex].stop_bit = stop_bit;

                serialSlots[slotIndex].pActuator = NULL;
                serialSlots[slotIndex].state = DEV_STATE_INIT;
                serialSlots[slotIndex].preTimeout = 0;
                serialSlots[slotIndex].start_time = 0;
                serialSlots[slotIndex].messageState = 0;
                serialSlots[slotIndex].timeoutCount = 0;


                if (slotIndex + 1 > machine.numSerialSlots)
                    machine.numSerialSlots = slotIndex + 1;

                ctx = NULL;

                retVal = 1;

            } else {
                fprintf(stderr, "[%s%s - baud:%d FAILED:", (char*) ANSI_COLOR_RED, device, baud);
                fprintf(stderr, "%s]\n", (char*) ANSI_COLOR_RESET);
            }
        }

        if (ctx)
            modbus_free(ctx);

    } else {
        fprintf(stderr, "[%s%s - baud:%d FAILED:", (char*) ANSI_COLOR_RED, device, baud);
        fprintf(stderr, "%s]\n", (char*) ANSI_COLOR_RESET);
        retVal = -1;
    }

    return retVal;
}

int dataExchangeInitSerial(int Mode) {
    int retVal = 1;
    char *device_list[] = {
        (char *) "/dev/ttyS1", (char *) "/dev/ttyS2", (char *) "/dev/ttyS3", (char *) "/dev/ttyS4", (char *) "/dev/ttyS5", (char *) "/dev/ttyS6", (char *) "/dev/ttyS7", (char *) "/dev/ttyS8"
        , (char *) "/dev/ttyUSB0", (char *) "/dev/ttyUSB1", (char *) "/dev/ttyUSB2", (char *) "/dev/ttyUSB3", (char *) "/dev/ttyUSB4", (char *) "/dev/ttyUSB5", (char *) "/dev/ttyUSB6", (char *) "/dev/ttyUSB7", (char *) "/dev/ttyUSB8"
    };
    int baud_list[] = {9600}, n_baud_list = sizeof (baud_list) / sizeof (baud_list[0]);
    int n_device_list = sizeof (device_list) / sizeof (device_list[0]);
    char parity = 'N';
    // N for none
    // E for even
    // O for odd

    int data_bit = 8;
    int stop_bit = 1;

    // 9600 8 N 1

    int i, j, i_act, res;
    char str[256];



    machine.numSerialSlotsAllocated = 16;
    machine.serialSlots = (SerialSlot*) calloc(sizeof (SerialSlot) * machine.numSerialSlotsAllocated + 1, 1);
    machine.numSerialSlots = 0;

    if (!machine.serialSlots) {
        machine.numSerialSlotsAllocated = 0;
        return 999;
    }



    /*
    snprintf(App.Msg, App.MsgSize, "[Testing Serial Port");
    vDisplayMessage(App.Msg);
    test_serial();
     */


    res = read_serial_cfg();


    if (res > 0) {
        snprintf(App.Msg, App.MsgSize, "[Reading %d Serial Device(s) ...\n", machine.numSerialSlots);
        vDisplayMessage(App.Msg);


        for (int i = 0; i < machine.numSerialSlots; i++) {

            /*
            snprintf(App.Msg, App.MsgSize, "Serial Board %s - %d.%d - %d %d-%c-%d...", machine.serialSlots[i].device, machine.serialSlots[i].boardId, machine.serialSlots[i].stationId ,machine.serialSlots[i].baud, machine.serialSlots[i].data_bit, machine.serialSlots[i].parity, machine.serialSlots[i].stop_bit);
            vDisplayMessage(App.Msg);
             */
            if (machine.serialSlots[i].device && machine.serialSlots[i].baud) {
                res = handle_modbus_connect(machine.serialSlots, i, machine.serialSlots[i].device, machine.serialSlots[i].baud, machine.serialSlots[i].parity, machine.serialSlots[i].data_bit, machine.serialSlots[i].stop_bit);
            } else {
                res = 0;
            }


            if (res > 0) {
                machine.serialSlots[i].state = DEV_STATE_INIT;
                continue;

            } else if (res == 0) {
                continue;
                // break;

            } else if (res < 0) {
                //////////////////////////////////////
                // Generazione Emergenza Macchina
                //
                snprintf(str, sizeof (str), "Serial Board %d (%s) not detected", machine.serialSlots[i].boardId, machine.serialSlots[i].device);
                if (generate_alarm((char*) str, 5102, 0, (int) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                }
                retVal = -1;
            }
        }

        vDisplayMessage("]\n");

    } else {

        snprintf(App.Msg, App.MsgSize, "[Searching for Serial Device");
        vDisplayMessage(App.Msg);

        for (i = 0; i < n_device_list; i++) {

            vDisplayMessage(".");

            for (j = 0; j < n_baud_list; j++) {

                res = handle_modbus_connect(machine.serialSlots, machine.numSerialSlots, device_list[i], baud_list[j], parity, data_bit, stop_bit);
                if (res > 0) {
                    snprintf(App.Msg, App.MsgSize, "[%sDetected SERIAL Device %s updated%s]\n", (char*) ANSI_COLOR_GREEN, device_list[i], (char*) ANSI_COLOR_RESET);
                    vDisplayMessage(App.Msg);
                    machine.serialSlots[machine.numSerialSlots].state = DEV_STATE_INIT;
                    machine.numSerialSlots++;
                    break;

                } else if (res == 0) {
                    continue;

                } else if (res < 0) {
                    retVal = -1;
                }
            }
        }

        vDisplayMessage("]\n");



        if (update_serial_cfg() > 0) {
            snprintf(App.Msg, App.MsgSize, "[%sSERIAL file %s updated%s]\n", (char*) ANSI_COLOR_GREEN, SER_BOARD_CFG_FILE, (char*) ANSI_COLOR_RESET);
            vDisplayMessage(App.Msg);
        }

        //////////////////////////////////////////////////////////////////////////////////
        // Nessuna scheda dichiarata : Inizializzazione OK, a cura dell primo avvio
        //
        if (retVal > 0) {
            App.SEROK = true;
        } else {
            snprintf(App.Msg, App.MsgSize, "[%s Search SER failed %s]\n", (char*) ANSI_COLOR_RED, (char*) ANSI_COLOR_RESET);
            vDisplayMessage(App.Msg);
        }
    }



    ///////////////////////////////////////////////////
    // Inizializzazione delle seriali identificate
    //
    int rc;
    int addr = 0;
    int nb = 0;
    int numSerialSlotsProcessed = 0;
    uint8_t data_str[32] = {0};
    uint16_t data[256] = {0};
    uint32_t t1;



    while (numSerialSlotsProcessed < machine.numSerialSlots) {

        for (i = 0; i < machine.numSerialSlots; i++) {

            t1 = xTaskGetTickCount();

            switch (machine.serialSlots[i].state) {


                case DEV_STATE_UNINIT:
                    numSerialSlotsProcessed++;
                    break;


                case DEV_STATE_INIT:
                    // Avvio timer
                    machine.serialSlots[i].preTimeout = 0;
                    machine.serialSlots[i].start_time = t1;
                    machine.serialSlots[i].state = DEV_STATE_WAITING;

                    if (machine.serialSlots[i].modbus_ctx) {

                        // 
                        rc = modbus_purge_comm((modbus_t*) machine.serialSlots[i].modbus_ctx);
                        if (rc) {
                            fprintf(stdout, "[purge comm serial board:%d/%d - phase 1...:%d bytes]\n", i + 1, machine.numSerialSlots, rc);
                            fflush(stdout);
                        }

                        // 
                        rc = modbus_purge_comm((modbus_t*) machine.serialSlots[i].modbus_ctx);
                        if (rc) {
                            fprintf(stdout, "[%spurge comm serial board:%d/%d - phase 2...:%d bytes%s]\n", (char*) ANSI_COLOR_MAGENTA, i + 1, machine.numSerialSlots, rc, (char*) ANSI_COLOR_RESET);
                            fflush(stdout);
                            machine.serialSlots[i].state = -1;
                        }



                        // Debug mode OFF
                        modbus_set_debug((modbus_t*) machine.serialSlots[i].modbus_ctx, false);


                        uint32_t tv_sec = 0;
                        uint32_t tv_usec = MODBUS_STREAM_TIMEOUT_MS * 1000;
                        modbus_set_response_timeout((modbus_t*) machine.serialSlots[i].modbus_ctx, tv_sec, tv_usec);
                    }

                    machine.serialSlots[i].preTimeout = 0;
                    machine.serialSlots[i].start_time = 0;

                    machine.serialSlots[i].stopRequest = 0;
                    machine.serialSlots[i].setupRequest = 0;
                    machine.serialSlots[i].setupDone = 0;
                    machine.serialSlots[i].pendingCommand = 0;
                    machine.serialSlots[i].runningCommand = 0;
                    machine.serialSlots[i].doneCommand = 0;
                    break;



                case DEV_STATE_WAITING:

                    // Avvio comunicazione
                    if (t1 - machine.serialSlots[i].start_time > MODBUS_WAIT_BEFOR_RECONNECT_MS) {
                        machine.serialSlots[i].state = DEV_STATE_CONNECTING;
                    }
                    break;


                case DEV_STATE_CONNECTING:

                    if (!machine.serialSlots[i].modbus_ctx) {
                        snprintf(App.Msg, App.MsgSize, "[%s MODBUS connect failed on serial board:%d %s]\n", (char*) ANSI_COLOR_RED, machine.serialSlots[i].boardId, (char*) ANSI_COLOR_RESET);
                        vDisplayMessage(App.Msg);

                        machine.serialSlots[i].state = -1;

                    } else {
                        ///////////////////////////////////////////////////////////////////////////
                        // Imposta lo slave-id che deve corrispondere nel driver del motore
                        //
                        if ((rc = modbus_set_slave((modbus_t*) machine.serialSlots[i].modbus_ctx, (int) machine.serialSlots[i].stationId)) < 0) {
                            snprintf(App.Msg, App.MsgSize, "[%s Set slave failed on serial board:%d %s]\n", (char*) ANSI_COLOR_RED, machine.serialSlots[i].boardId, (char*) ANSI_COLOR_RESET);
                            vDisplayMessage(App.Msg);
                            handle_serial_borad_init_error(i);
                        } else {
                            // snprintf(App.Msg, App.MsgSize, "[%s Set slave on serial board#%d as %d %s]\n", (char*) ANSI_COLOR_GREEN, machine.serialSlots[i].boardId, machine.serialSlots[i].stationId, (char*) ANSI_COLOR_RESET);
                            // vDisplayMessage(App.Msg);
                        }

                        ///////////////////////////////////         
                        // link ID Driver all'attuatore
                        //
                        if (!machine.serialSlots[i].pActuator) {
                            bool actLinked = false;

                            // machine.serialSlots[i].stationId = data[0];

                            for (i_act = 0; i_act < machine.num_actuator; i_act++) {
                                if (machine.actuator[i_act].boardId) {
                                    if (machine.actuator[i_act].boardId == machine.serialSlots[i].boardId && machine.actuator[i_act].stationId == machine.serialSlots[i].stationId) {
                                        // snprintf(App.Msg, App.MsgSize, "[%s Linked at %s %s]\n", (char*) ANSI_COLOR_GREEN, machine.actuator[i_act].name, (char*) ANSI_COLOR_RESET);
                                        // vDisplayMessage(App.Msg);
                                        machine.serialSlots[i].pActuator = (void*) &machine.actuator[i_act];
                                        machine.actuator[i_act].pSerialSlot = (void*) &machine.serialSlots[i];
                                        // Asse da inizializzare
                                        machine.actuator[i_act].step = STEP_UNINITIALIZED;
                                        actLinked = true;
                                        break;
                                    }
                                }
                            }

                            if (!actLinked) {
                                snprintf(App.Msg, App.MsgSize, "[%s board:%d/%d - Device active (Station:%d) - %sNOT Linked! %s ]\n", (char*) ANSI_COLOR_YELLOW, i + 1, machine.numSerialSlots, (int) machine.serialSlots[i].stationId, (char*) ANSI_COLOR_RED, (char*) ANSI_COLOR_RESET);
                                vDisplayMessage(App.Msg);

                            } else {
                                snprintf(App.Msg, App.MsgSize, "[%s board:%d/%d - Device active (Station:%d) - Linked to %s%s ]\n", (char*) ANSI_COLOR_YELLOW, i + 1, machine.numSerialSlots, (int) machine.serialSlots[i].stationId, (char*) machine.actuator[i_act].name, (char*) ANSI_COLOR_RESET);
                                vDisplayMessage(App.Msg);
                            }


                            machine.serialSlots[i].messageState = 0;
                            machine.serialSlots[i].timeoutCount = 0;



                            // Stato lettura in streaming
                            machine.serialSlots[i].state = DEV_STATE_INIT_STREAM;

                            if (handle_modbus_startup(&machine.serialSlots[i]) < 0) {
                                handle_serial_borad_init_error(i);
                            }


                        } else {
                            // Già collegato : stato lettura in streaming
                            snprintf(App.Msg, App.MsgSize, "[%s board:%d/%d - Device already linked %s]\n", (char*) ANSI_COLOR_YELLOW, i + 1, machine.numSerialSlots, (char*) ANSI_COLOR_RESET);
                            vDisplayMessage(App.Msg);
                            machine.serialSlots[i].state = DEV_STATE_INIT_STREAM;
                            machine.serialSlots[i].timeoutCount = 0;
                            machine.serialSlots[i].messageState = 0;
                        }
                    }
                    
                    numSerialSlotsProcessed++;
                    
                    break;
            }
        }
    }



    ///////////////////////////////////////////
    // Verifica collegamento degli attuatori
    //
    if (machine.numSerialSlots) {
        snprintf(str, sizeof (str), "[%s Checking %d actuators...%s]", (char*) ANSI_COLOR_YELLOW, machine.num_actuator, (char*) ANSI_COLOR_RESET);
        vDisplayMessage(str);

        for (i_act = 0; i_act < machine.num_actuator; i_act++) {

            if (machine.actuator[i_act].boardId) {

                if (machine.actuator[i_act].protocol == MODBUS_AC_SERVO_LICHUAN || machine.actuator[i_act].protocol == MODBUS_AC_SERVO_DELTA) {

                    snprintf(str, sizeof (str), "[%s %s ...%s", (char*) ANSI_COLOR_YELLOW, machine.actuator[i_act].name, (char*) ANSI_COLOR_RESET);
                    vDisplayMessage(str);

                    if (!machine.actuator[i_act].pSerialSlot) {
                        //////////////////////////////////////
                        // Generazione Emergenza Macchina
                        //
                        snprintf(str, sizeof (str), "Actuator %s not linked", machine.actuator[i_act].name);
                        vDisplayMessage(str);
                        retVal = -1;
                        /*
                        if (generate_alarm((char*) str, 5103, 0, (int) ALARM_FATAL_ERROR) < 0) {
                        }
                         */
                    } else {
                        SerialSlot *pSerialSlots = (SerialSlot *) machine.actuator[i_act].pSerialSlot;

                        if (!pSerialSlots->stationId) {
                            //////////////////////////////////////
                            // Generazione Emergenza Macchina
                            //
                            snprintf(str, sizeof (str), "Actuator %s driver offline", machine.actuator[i_act].name);
                            vDisplayMessage(str);
                            retVal = -2;
                            /*
                            if (generate_alarm((char*) str, 5104, 0, (int) ALARM_FATAL_ERROR) < 0) {
                            }
                             */
                        } else {
                            if (machine.serialSlots[i].state < 0) {
                                //////////////////////////////////////
                                // Generazione Emergenza Macchina
                                //
                                snprintf(str, sizeof (str), "Actuator %s invalid serial state (%d)", machine.actuator[i_act].name, machine.serialSlots[i].state);
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


    if (retVal > 0) {
        ////////////////////////////////////////////////////////
        // Tutte le schede identificate : Inizializzazione OK
        //
        App.SEROK = true;
    } else {
        App.SERDetectError++;
    }


    return retVal;
}












///////////////////////////////////
// Legge dai dispositivi seriali
//

int dataExchangeLoopSerial(void) {
    int rc, retVal = 0;
    int i, i_act;
    char str[256];
    uint32_t t1;
    char hHigh = 0, bLow = 0;
    int addr = 0;
    int nb = 0;


    App.SerialRunning = machine.numSerialSlots;

    for (i = 0; i < machine.numSerialSlots; i++) {
        LP_ACTUATOR pActuator = (LP_ACTUATOR) machine.serialSlots[i].pActuator;

        t1 = xTaskGetTickCount();

        if (!machine.serialSlots[i].disabled) {

            if (pActuator) {

                switch (machine.serialSlots[i].state) {

                    case DEV_STATE_INIT:

                        // Reset dati sequenza IDLE
                        pActuator->IDLECounter = 0;
                        pActuator->IDLESeq = 0;

                        machine.serialSlots[i].streamErrorCount = 0;
                        machine.serialSlots[i].preTimeout = 0;
                        machine.serialSlots[i].start_time = 0;

                        // machine.serialSlots[i].stopRequest = 0;
                        machine.serialSlots[i].setupRequest = 0;
                        machine.serialSlots[i].setupDone = 0;
                        machine.serialSlots[i].pendingCommand = 0;
                        machine.serialSlots[i].runningCommand = 0;
                        machine.serialSlots[i].doneCommand = 0;


                        // Abilitazione Seriale
                        App.SEROK = INIT_AND_RUN;


                        //////////////////////////////////////////////
                        // Abilita il driver (comando Servo ON)
                        //
                        if (!pActuator->disabled) {
                            if (handle_actuator_servo_on((void *) pActuator) < 0) {
                                // return -1;
                                if (generate_alarm((char*) "MODBUS : handle_actuator_start() Error", 6101, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                                }
                            }
                        } else {
                            if (handle_actuator_servo_off((void *) pActuator) < 0) {
                                // return -1;
                                if (generate_alarm((char*) "MODBUS : handle_actuator_start() Error", 6101, 0, (int32_t) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                                }
                            }
                        }

                        if (machine.serialSlots[i].stopRequest) {
                            machine.serialSlots[i].state = DEV_STATE_SERVICE_STOP;
                        } else if (machine.serialSlots[i].setupRequest == 1) {
                            machine.serialSlots[i].state = DEV_STATE_SERVICE_SETUP;
                        } else if (machine.serialSlots[i].setupRequest == 1000) {
                            machine.serialSlots[i].state = DEV_STATE_SERVICE_FIRST_SETUP;
                        } else {
                            machine.serialSlots[i].state = DEV_STATE_INIT_STREAM;
                        }
                        break;



                    case DEV_STATE_HOMING_INIT:
                        // Homing Init...
                        handle_modbus_homing_init(&machine.serialSlots[i]);
                        break;

                    case DEV_STATE_HOMING_SEND:
                        // Homing Send...
                        handle_modbus_homing_send(&machine.serialSlots[i]);
                        break;

                    case DEV_STATE_HOMING_RECV:
                        // Homing Recv...
                        handle_modbus_homing_recv(&machine.serialSlots[i]);
                        break;

                    case DEV_STATE_HOMING_DONE:
                        // Homing Done...
                        handle_modbus_homing_done(&machine.serialSlots[i]);
                        break;





                    case DEV_STATE_INIT_STREAM:
                        // Init...
                        handle_modbus_streaming_init(&machine.serialSlots[i]);
                        break;



                    case DEV_STATE_STREAMING_SEND:
                        ////////////////////////////////////////////////////       
                        // Invio comando lettura in Streaming (ASINCRONA)
                        //
                        // Lettura posizione 
                        //
                        // N.B.: La lettura continua della poszione non è necessaria se l'asse non è in movimento
                        //
                        handle_modbus_streaming_send(&machine.serialSlots[i]);
                        break;


                    case DEV_STATE_STREAMING_RECV:

                        handle_modbus_streaming_recv(&machine.serialSlots[i]);
                        break;




                    case DEV_STATE_STREAMING_DONE:
                        ////////////////////////////////////////////////////
                        // Fine comunicazione : azione post lettura
                        //                        
                        handle_modbus_streaming_done(&machine.serialSlots[i]);
                        break;





                    case DEV_STATE_STREAMING_POST_ACT:
                        // Fine comunicazione, imposta la traccia temporale
                        machine.serialSlots[i].preTimeout = 0;
                        machine.serialSlots[i].start_time = t1;
                        machine.serialSlots[i].state = DEV_STATE_STREAMING_POST_WAIT;
                        break;



                    case DEV_STATE_STREAMING_POST_WAIT:
                        ///////////////////////////
                        // Attesa prossimo loop
                        //
                        if (t1 - machine.serialSlots[i].start_time >= MODBUS_LOOP_TIME_MS) {
                            ///////////////////////////
                            // Comando da eseguire ?
                            //
                            if (machine.serialSlots[i].pendingCommand) {
                                machine.serialSlots[i].state = DEV_STATE_CMD_INIT;
                            } else if (machine.serialSlots[i].stopRequest) {
                                machine.serialSlots[i].state = DEV_STATE_SERVICE_STOP;
                            } else if (machine.serialSlots[i].setupRequest == 1) {
                                machine.serialSlots[i].state = DEV_STATE_SERVICE_SETUP;
                            } else if (machine.serialSlots[i].setupRequest == 1000) {
                                machine.serialSlots[i].state = DEV_STATE_SERVICE_FIRST_SETUP;
                            } else {
                                // Ritorna alla sequenza di lettura
                                machine.serialSlots[i].state = DEV_STATE_STREAMING_SEND;
                            }
                        }
                        break;








                        ////////////////////////////////////////
                        // Comando di servizio (LIBERO)
                        //

                    case DEV_STATE_SERVICE:
                        /////////////////////////////////////////////////          
                        // Invio comando lettura alla seriale SINCRONO
                        // Lettura posizione
                        //
                        hHigh = 0, bLow = 18;
                        addr = MAKEWORD(bLow, hHigh);
                        nb = 8;
                        // Pr0.18 Instantaneous current value of Phase U
                        // Pr0.19 Instantaneous current value of Phase W / / A ▲ P S T
                        // Pr0.20 Busbar voltage / / V ▲ P S T
                        // Pr0.21 Ratio of inertias of current load / / % ▲ P S T
                        // Pr0.22 Current total inertia / / % ▲ P S T
                        // Pr0.23 DI input status / / / ▲ P S T
                        // Pr0.24 DO output status / / / ▲ P S T
                        // Pr0.25 Display the fault code P1-60 selects / /


                        if (modbus_read_registers((modbus_t*) machine.serialSlots[i].modbus_ctx, addr, nb, (uint16_t*) & machine.serialSlots[i].data[8]) < 0) {
                            ////////////////////////////
                            // Generazione Allarme
                            //
                            if (generate_alarm((char*) "Serial Communication Service Error", 6004, 0, (int) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                            }

                            // handle_serial_borad_error(i, (char*) "read_reg", machine.serialSlots[i].state);
                            machine.serialSlots[i].streamErrorCount++;

                            // reinizializa il flusso
                            machine.serialSlots[i].state = DEV_STATE_INIT_STREAM;

                        } else {
                            machine.serialSlots[i].readCount++;
                            // fine lettura
                            machine.serialSlots[i].state = DEV_STATE_STREAMING_DONE;
                        }

                        if (!App.SERRunLoop) {
                            machine.serialSlots[i].state = DEV_STATE_CLOSING;
                        }
                        break;




                    case DEV_STATE_SERVICE_SETUP_SPEED_ACC:

                        handle_modbus_service_setup_speed_acc(&machine.serialSlots[i]);
                        break;





                        ////////////////////////////////////////////
                        // Fase preparatoria del driver
                        //
                    case DEV_STATE_SERVICE_SETUP:
                        handle_modbus_service_setup(&machine.serialSlots[i]);
                        break;

                    case DEV_STATE_SERVICE_SETUP_I:
                        handle_modbus_service_setup_I(&machine.serialSlots[i]);
                        break;

                    case DEV_STATE_SERVICE_SETUP_II:
                        handle_modbus_service_setup_II(&machine.serialSlots[i]);
                        break;

                    case DEV_STATE_SERVICE_SETUP_III:
                        handle_modbus_service_setup_III(&machine.serialSlots[i]);
                        break;

                    case DEV_STATE_SERVICE_SETUP_IV:
                        handle_modbus_service_setup_IV(&machine.serialSlots[i]);
                        break;


                    case DEV_STATE_SERVICE_STOP:
                        handle_modbus_service_stop(&machine.serialSlots[i]);
                        break;


                    case DEV_STATE_SERVICE_RESET:
                        handle_modbus_service_reset(&machine.serialSlots[i]);
                        break;





                    case DEV_STATE_SERVICE_OUT:
                        App.SerialRunning--;

                        if (App.SERRunLoop) {
                            if (machine.serialSlots[i].setupRequest == 1) {
                                machine.serialSlots[i].state = DEV_STATE_SERVICE_SETUP;
                                snprintf(App.Msg, App.MsgSize, "[Restart SERIAL service]");
                                vDisplayMessage(App.Msg);
                            } else if (machine.serialSlots[i].setupRequest == 1000) {
                                machine.serialSlots[i].state = DEV_STATE_SERVICE_FIRST_SETUP;
                                snprintf(App.Msg, App.MsgSize, "[Setup SERIAL service]");
                                vDisplayMessage(App.Msg);
                            } else {
                                if (machine.serialSlots[i].pendingCommand) {
                                    // Ripresa del servizio su comando in pendenza
                                    machine.serialSlots[i].setupRequest = 1;
                                    snprintf(App.Msg, App.MsgSize, "[SER: entering setup by pending command]");
                                    vDisplayMessage(App.Msg);
                                }
                            }
                        }
                        break;






                    case DEV_STATE_CMD_INIT:
                        //////////////////////////////
                        // Preparazione del Comando 
                        //
                        handle_modbus_cmd_init(&machine.serialSlots[i]);
                        break;




                    case DEV_STATE_CMD_INIT_SEND:
                        //////////////////////////////////////
                        // Invio comando inizializzazione
                        //
                        handle_modbus_cmd_init_send(&machine.serialSlots[i]);
                        break;



                    case DEV_STATE_CMD_INIT_RECV:
                        /////////////////////////////////////
                        // lettura risultato Inizializzazione
                        //            
                        handle_modbus_cmd_init_recv(&machine.serialSlots[i]);
                        break;



                    case DEV_STATE_CMD_FEEDBACK_SEND:
                        /////////////////////////////////////
                        // Invio comando lettura posizione
                        //
                        handle_modbus_cmd_feedback_send(&machine.serialSlots[i]);
                        break;




                    case DEV_STATE_CMD_FEEDBACK_RECV:
                        /////////////////////////////////////
                        // lettura posizione
                        //            
                        handle_modbus_cmd_feedback_recv(&machine.serialSlots[i]);
                        break;



                    case DEV_STATE_CMD_DONE:
                        ///////////////////////
                        // Comando terminato
                        //
                        handle_modbus_cmd_done(&machine.serialSlots[i]);
                        break;



                    case DEV_STATE_CMD_DONE_FEEDBACK:
                        ////////////////////////////////////////////////////////
                        // Azione post Comando (svuota la seriale)
                        //
                        handle_modbus_cmd_done_feedback(&machine.serialSlots[i]);
                        break;


                    case DEV_STATE_CMD_ERROR:
                        ///////////////////////////
                        // Generazione Allarme
                        //                    
                        if (generate_alarm((char*) "Serial Communication Error", 6009, 0, (int) ALARM_FATAL_ERROR, 0 + 1) < 0) {
                        }

                        // reinizializza il flusso
                        machine.serialSlots[i].state = DEV_STATE_INIT_STREAM;
                        break;





                    case DEV_STATE_CLOSING:
                        machine.serialSlots[i].state = DEV_STATE_SERVICE_STOP;
                        snprintf(App.Msg, App.MsgSize, "[RTC] SERIAL #%d Stopped\n", (i + 1));
                        vDisplayMessage(App.Msg);
                        break;

                }



                // Interrizione servizio
                if (!App.SERRunLoop) {
                    if (machine.serialSlots[i].state != DEV_STATE_SERVICE_OUT && machine.serialSlots[i].state != DEV_STATE_SERVICE_STOP) {
                        machine.serialSlots[i].state = DEV_STATE_CLOSING;
                    }
                }
            }
        }
    }

    return retVal;
}

int dataExchangeIsRunningSerial() {
    for (uint32_t i = 0; i < machine.numSerialSlots; i++) {
        if (machine.serialSlots[i].state != DEV_STATE_SERVICE_OUT && machine.serialSlots[i].state != 0 && machine.serialSlots[i].state != -1) {
            return 1;
        }
    }
    return 0;
}

int test_serial() {

    int USB = open("/dev/ttyS4", O_RDWR | O_NOCTTY);

    struct termios tty;
    struct termios tty_old;
    memset(&tty, 0, sizeof tty);

    /* Error Handling */
    if (tcgetattr(USB, &tty) != 0) {
        return -1;
    }

    /* Save old tty parameters */
    tty_old = tty;

    /* Set Baud Rate */
    cfsetospeed(&tty, (speed_t) B9600);
    cfsetispeed(&tty, (speed_t) B9600);

    /* Setting other Port Stuff */
    tty.c_cflag &= ~PARENB; // Make 8n1
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;

    tty.c_cflag &= ~CRTSCTS; // no flow control
    tty.c_cc[VMIN] = 1; // read doesn't block
    tty.c_cc[VTIME] = 5; // 0.5 seconds read timeout
    tty.c_cflag |= CREAD | CLOCAL; // turn on READ & ignore ctrl lines

    /* Make raw */
    cfmakeraw(&tty);

    /* Flush Port, then applies attributes */
    tcflush(USB, TCIFLUSH);
    if (tcsetattr(USB, TCSANOW, &tty) != 0) {
        fprintf(stderr, "Error %d", errno);
    }


    unsigned char cmd[32] = {0};
    int n_send = 0, n_written = 0, spot = 0;

    cmd[0] = 1;
    cmd[1] = 3;
    cmd[2] = 0;
    cmd[3] = 0;
    cmd[4] = 0;
    cmd[5] = 1;
    cmd[6] = 0x84;
    cmd[7] = 0x0A;

    n_send = 8;


    for (;;) {

        do {
            n_written = write(USB, &cmd[spot], n_send - n_written);
            spot += n_written;
        } while (n_written < n_send);


        int n = 0;
        char buf = '\0';

        /* Whole response*/
        char response[1024];
        memset(response, '\0', sizeof response);

        spot = 0;

        do {

            ioctl(USB, FIONREAD, &n);
            if (n > 0) {
                fprintf(stdout, "[Read %d:", buf);
                n = read(USB, &buf, 1);
                sprintf(&response[spot], "%c", buf);
                fprintf(stdout, "%c", buf);
                fflush(stdout);
                // spot += n;
            }

        } while (n > 0);

        if (n < 0) {
            // std::cout << "Error reading: " << strerror(errno) << std::endl;
        } else if (n == 0) {
            // std::cout << "Read nothing!" << std::endl;
        } else {
            // std::cout << "Response: " << response << std::endl;
        }
    }


    close(USB);

    return 1;
}
