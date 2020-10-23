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


#include <math.h>






#define MAX_USB_MESSAGE_SIZE    256

typedef struct tag_USB_CTX {
    int portfd;
} USB_CTX, *LP_USB_CTX;





static uint16_t GLRunLoop = 1;

int dataExchangeDumpUSB(char *msg, size_t msg_size) {
    for (int i = 0; i < machine.numUSBSlots; i++) {
        snprintf(msg, msg_size, "[SER#%d - ID:%d - cycles:%d - tout:%d - state:%d - {pend:%d,run:%d,done:%d,stop:%d,setup:%d} - Pos.:%d - Speed:%d]\n"
                , (int) machine.USBSlots[i].boardId
                , (int) machine.USBSlots[i].stationId
                , (int) machine.USBSlots[i].readCount
                , (int) machine.USBSlots[i].timeoutCount
                , (int) machine.USBSlots[i].state
                , (int) machine.USBSlots[i].pendingCommand
                , (int) machine.USBSlots[i].runningCommand
                , (int) machine.USBSlots[i].doneCommand
                , (int) machine.USBSlots[i].stopRequest
                , (int) machine.USBSlots[i].setupRequest
                , (int) machine.USBSlots[i].driverPosition
                , (int) machine.USBSlots[i].driverSpeed
                );
        vDisplayMessage(msg);
    }
    return 1;
}

int usb_purge_comm(void * usb_ctx) {
    LP_USB_CTX pUSBCtx = (LP_USB_CTX) usb_ctx;
    if (pUSBCtx) {
        return 1;
    }
    return 0;
}

int usb_free(void * usb_ctx) {
    LP_USB_CTX pUSBCtx = (LP_USB_CTX) usb_ctx;
    if (pUSBCtx) {
        close(pUSBCtx->portfd);
        free(pUSBCtx);
        return 1;
    }
    return 0;
}

void *usb_connect(USBSlot *pUSBSlot, int numUSBSlots, char *device_addr) {
    int portfd = -1;
    char str[256];

    if (device_addr) {

        portfd = open(device_addr, O_RDWR);

        if (portfd >= 0) {
            struct termios oldtio, newtio;

            // save existing attributes
            tcgetattr(portfd, &oldtio);  

            // set attributes - these flags may change for your device
            #define BAUDRATE B115200 

            memset(&newtio, 0x00, sizeof(newtio));  
            newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;   
            newtio.c_iflag = IGNPAR | ICRNL;          
            newtio.c_oflag = 0;  

            tcflush(portfd, TCIFLUSH);  
            tcsetattr(portfd, TCSANOW, &newtio); 

            //reset attributes
            tcsetattr(portfd, TCSANOW, &oldtio);
        }

        if (portfd < 0) {
            sprintf(str, "cannot open %s. Sorry.\n", device_addr);
            // perror(str);
        }

        LP_USB_CTX pUSBCtx = (LP_USB_CTX) calloc(sizeof (USB_CTX), 0);
        if (pUSBCtx) {
            pUSBCtx->portfd = portfd;
            return (void *) pUSBCtx;
        }
    }
    
    return (void *) NULL;
}

int usb_data_available(void *usb_ctx) {
}

int usb_set_slave(void *usb_ctx, int i) {
}

int usb_report_slave_id(void *usb_ctx, char *out) {
}

int usb_read_registers(void *usb_ctx, int addr, int nb, char *data, int data_size) {
}

int usb_param_update(void *usb_ctx, int addr, int nb, int value, bool debug) {
}

int xrt_usb_message_send(void *usb_ctx, int usb_id, int usb_dlc, char *data) {
    LP_USB_CTX pUSBCtx = (LP_USB_CTX) usb_ctx;
    if (pUSBCtx) {

        /*
        int retval = write(pUSBCtx->sock, &frame, sizeof (struct usb_frame));
        if (retval != sizeof (struct usb_frame)) {
            return (-1);
        } else {
            return (0);
        }
        */
    }
    return -1;
}


int xrt_usb_message_recv(void *usb_ctx, char *data , int data_size) {
    LP_USB_CTX pUSBCtx = (LP_USB_CTX) usb_ctx;
    if (pUSBCtx) {        
       
    }
}




char *get_USB_status(void *pvUSBSlots) {

    if (pvUSBSlots) {
        USBSlot *USBSlots = (USBSlot *) pvUSBSlots;

        switch (USBSlots->state) {
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






int dataExchangeResetUSB() {
    int i;

    for (i = 0; i < machine.numUSBSlots; i++) {
        machine.USBSlots[i].state = -1;
    }

    usleep(1 * 1000);

    for (i = 0; i < machine.numUSBSlots; i++) {
        if (machine.USBSlots[i].usb_ctx) {
            // usb_close((void *) machine.USBSlots[i].usb_ctx);
        }
    }

    for (i = 0; i < machine.numUSBSlots; i++) {
        if (machine.USBSlots[i].usb_ctx) {
            machine.USBSlots[i].timeoutCount = 0;
            // usb_connect((void *) machine.USBSlots[i].usb_ctx);
        }
    }

    return 0;
}

int dataEchangeStopUSB() {
    GLRunLoop = 0;
}


// Ritorna 0 se lo stato passa a bloccato (-1)

int handle_USB_borad_init_error(int i) {
    int retVal = 0;
    machine.USBSlots[i].timeoutCount++;
    if (machine.USBSlots[i].timeoutCount > MAX_SERIAL_ATTEMPS) {
        snprintf(App.Msg, App.MsgSize, "[%s board:%d - stopping communication (%s) %s]\n", (char*) ANSI_COLOR_MAGENTA, i + 1, (char*) modbus_strerror(errno), (char*) ANSI_COLOR_RESET);
        vDisplayMessage(App.Msg);
        machine.USBSlots[i].state = -1;
        retVal = 0;
    } else {
        // Ripete il ciclo
        machine.USBSlots[i].state = 0;
        retVal = 1;
    }
    return retVal;
}

int handle_USB_borad_error(int i, char *msg, int state) {
    int retVal = 0;

    machine.USBSlots[i].timeoutCount++;

    if (machine.USBSlots[i].timeoutCount > MAX_SERIAL_LOOP_ERROR) {

        if (state >= 0) {
            snprintf(App.Msg, App.MsgSize, "[%s board:%d - stream error (%s) (%s) state:%d %s]\n", (char*) ANSI_COLOR_MAGENTA, i + 1, (char*) msg, (char*) modbus_strerror(errno), state, (char*) ANSI_COLOR_RESET);
        } else {
            snprintf(App.Msg, App.MsgSize, "[%s board:%d - %s %s]\n", (char*) ANSI_COLOR_MAGENTA, i + 1, (char*) msg, (char*) ANSI_COLOR_RESET);
        }
        vDisplayMessage(App.Msg);

        dataExchangeResetUSB();


        if (usb_purge_comm((void*) machine.USBSlots[i].usb_ctx)) {
        }


        machine.USBSlots[i].state = DEV_STATE_STREAMING_SEND;
        retVal = 0;

        /////////////////////////
        // Generazione Allarme
        //
        if (generate_alarm((char*) "USB Communication Timeout", 6001, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
        }


    } else {
        // Ripete il ciclo

        if (state >= 0) {
            snprintf(App.Msg, App.MsgSize, "[%s board:%d - (%s)(%s) state:%d %s]\n", (char*) ANSI_COLOR_RED, i + 1, (char*) msg, (char*) modbus_strerror(errno), state, (char*) ANSI_COLOR_RESET);
        } else {
            snprintf(App.Msg, App.MsgSize, "[%s board:%d - %s %s]\n", (char*) ANSI_COLOR_RED, i + 1, (char*) msg, (char*) ANSI_COLOR_RESET);
        }
        vDisplayMessage(App.Msg);

        machine.USBSlots[i].state = DEV_STATE_STREAMING_SEND;
        retVal = 1;
    }
    return retVal;
}

int read_USB_cfg() {
    int retVal = 0;
    FILE *f = fopen(USB_BOARD_CFG_FILE, "r");

    machine.numUSBSlots = 0;

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
                    if (boardId >= 1 && boardId <= machine.numUSBSlotsAllocated) {
                        if (pSer) {
                            LP_USBSlot USBSlot = (LP_USBSlot) & machine.USBSlots[boardId - 1];

                            USBSlot->boardId = boardId;

                            // Device
                            pstr = strstr(pSer, ".");
                            if (pstr) {
                                pstr[0] = 0;
                                strncpy(USBSlot->device, pSer, sizeof(USBSlot->device));
                                pSer = pstr + 1;
                                strncpy(USBSlot->device_addr, pSer, sizeof(USBSlot->device_addr));
                            }



                            if (boardId > machine.numUSBSlots) {
                                machine.numUSBSlots = boardId;
                            }

                            /*
                            snprintf(App.Msg, App.MsgSize, "[CFG USB : ID:%d - %s @%d %d-%c-%d]\n", USBSlot->boardId, USBSlot->device, USBSlot->baud, USBSlot->data_bit, (char)USBSlot->parity, USBSlot->stop_bit );
                            vDisplayMessage(App.Msg);
                            */
                        }




                    } else {
                        // Numero scheda non valido
                        snprintf(App.Msg, App.MsgSize, "[CFG !USB addr]\n");
                        vDisplayMessage(App.Msg);
                    }

                } else {
                    // Indirizzo board non valido
                    snprintf(App.Msg, App.MsgSize, "[CFG !USB #]\n");
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



int update_USB_cfg() {
    int retVal = 0;

    if (machine.numUSBSlots) {
        char str[256];
        FILE *f = fopen(USB_BOARD_CFG_FILE, "w");
        if (f) {
            for (int i = 0; i < machine.numUSBSlots; i++) {
                USBSlot *USBSlot = &machine.USBSlots[i];

                snprintf(str, sizeof (str), "#%d=%s.%s\n", USBSlot->boardId, USBSlot->device, USBSlot->device_addr);
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
        snprintf(App.Msg, App.MsgSize, "[update_USB_cfg : No USB boards]\n");
        vDisplayMessage(App.Msg);
        retVal = 0;
    }

    return retVal;
}




int handle_usb_connect(USBSlot *USBSlots, uint32_t slotIndex, char *device_addr) {

    int i, j, i_act, res;
    void *ctx = NULL;
    int retVal = 0;
    char str[32];

    ctx = usb_connect(USBSlots, slotIndex, device_addr);

    if (ctx) {

        // Assegnamento della slot
        if (slotIndex < machine.numUSBSlotsAllocated) {


            USBSlots->usb_ctx = ctx;

            {

                struct timeval timeout;

                timeout.tv_sec = MODBUS_TIMEOUT_SEC;
                timeout.tv_usec = MODBUS_TIMEOUT_USEC;
                // modbus_get_byte_timeout(ctx, &timeout);


                timeout.tv_sec = MODBUS_TIMEOUT_SEC;
                timeout.tv_usec = MODBUS_TIMEOUT_USEC;
                // modbus_set_response_timeout(ctx, &timeout);




                // int rts = modbus_rtu_get_rts(ctx);

                // int res = modbus_rtu_set_rts(ctx, MODBUS_RTU_RTS_NONE);




                /// fprintf(stderr, "[%s%s - %d-%d%c%d OK%s]", (char*) ANSI_COLOR_GREEN, device, baud, data_bit, parity, stop_bit, (char*) ANSI_COLOR_RESET);

                // id impostato dalle lettura dello slave
                // machine.USBSlots[slotIndex].board_id = slotIndex;

                USBSlots[slotIndex].usb_ctx = ctx;
                
                strncpy(USBSlots[slotIndex].device_addr, device_addr, sizeof(USBSlots[slotIndex].device_addr));
                
                snprintf(str, sizeof(str), "can%d", slotIndex);
                strncpy(USBSlots[slotIndex].device, str, sizeof(USBSlots[slotIndex].device));

                // Lettura dell' Id del Driver a cura del loop
                USBSlots[slotIndex].boardId = slotIndex + 1;
                USBSlots[slotIndex].pActuator = NULL;
                USBSlots[slotIndex].state = 0;
                USBSlots[slotIndex].start_time = 0;
                USBSlots[slotIndex].messageState = 0;
                USBSlots[slotIndex].timeoutCount = 0;

    
                USBSlots[slotIndex].data_size = MAX_USB_MESSAGE_SIZE;
                USBSlots[slotIndex].data = (char *)calloc(USBSlots[slotIndex].data_size, 1);
                if (!USBSlots[slotIndex].data) {
                    USBSlots[slotIndex].data_size = 0;
                    retVal = -1;
                }

                if (slotIndex + 1 > machine.numUSBSlots)
                    machine.numUSBSlots = slotIndex + 1;

                ctx = NULL;

                retVal = 1;

            }
        }

        if (ctx)
            usb_free(ctx);

    } else {
        fprintf(stderr, "[%s%s - FAILED:", (char*) ANSI_COLOR_RED, device_addr);
        fprintf(stderr, "%s]\n", (char*) ANSI_COLOR_RESET);
        retVal = -1;
    }

    return retVal;
}

int dataExchangeInitUSB(int Mode) {
    int retVal = 1;
    char *device_list[] = {
         (char *) "/dev/ttyUSB0"
        ,(char *) "/dev/ttyUSB1"
        ,(char *) "/dev/ttyUSB2"
        ,(char *) "/dev/ttyUSB3"
    };
    int n_device_list = sizeof (device_list) / sizeof (device_list[0]);

    int i, j, i_act, res;
    char str[256];



    machine.numUSBSlotsAllocated = 16;
    machine.USBSlots = (USBSlot*) calloc(sizeof (IOBoardSlot) * machine.numUSBSlotsAllocated + 1, 1);
    machine.numUSBSlots = 0;

    if (!machine.USBSlots) {
        machine.numUSBSlotsAllocated = 0;
        return -1;
    }

    /*
    snprintf(App.Msg, App.MsgSize, "[Testing USB Port");
    vDisplayMessage(App.Msg);
    test_USB();
     */


    res = read_USB_cfg();

    if (res > 0) {
        snprintf(App.Msg, App.MsgSize, "[Reading %d USB Device(s) ", machine.numUSBSlots);
        vDisplayMessage(App.Msg);


        for (int i = 0; i < machine.numUSBSlots; i++) {

            /*
            if(!machine.USBSlots[i].device[0])
                snprintf(machine.USBSlots[i].device, sizeof(machine.USBSlots[i].device), "can%d", i);
            */
           
            res = handle_usb_connect(machine.USBSlots, i, machine.USBSlots[i].device_addr);
            if (res > 0) {
                if (retVal>=0)
                    retVal++;

            } else if (res == 0) {

            } else if (res < 0) {
                //////////////////////////////////////
                // Generazione Emergenza Macchina
                //
                snprintf(str, sizeof (str), "USB Board %d (%s) not detected", machine.USBSlots[i].boardId, machine.USBSlots[i].device);
                if (generate_alarm((char*) str, 5102, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                }
                retVal = -1;
            }
        }

        vDisplayMessage("]\n");

    } else {


        snprintf(App.Msg, App.MsgSize, "[Searching for USB Device");
        vDisplayMessage(App.Msg);

        for (i = 0; i < n_device_list; i++) {

            vDisplayMessage(".");

            res = handle_usb_connect(machine.USBSlots, machine.numUSBSlots, device_list[i]);
            if (res > 0) {
                if (retVal>=0)
                    retVal++;

            } else if (res == 0) {

            } else if (res < 0) {
                retVal = -1;
            }
        }

        vDisplayMessage("]\n");


        if (update_USB_cfg() > 0) {
            snprintf(App.Msg, App.MsgSize, "[%sUSB file %s updated%s]\n", (char*) ANSI_COLOR_GREEN, USB_BOARD_CFG_FILE, (char*) ANSI_COLOR_RESET);
            vDisplayMessage(App.Msg);
        }
    }




    ///////////////////////////////////////////////////
    // Inizializzazione delle USB identificate
    //
    int rc;
    int addr = 0;
    int nb = 0;
    int numUSBSlotsProcessed = 0;
    uint8_t data_str[32] = {0};
    uint16_t data[256] = {0};
    uint32_t t1;



    while (numUSBSlotsProcessed < machine.numUSBSlots) {

        for (i = 0; i < machine.numUSBSlots; i++) {

            t1 = xTaskGetTickCount();

            if (machine.USBSlots[i].state == DEV_STATE_INIT) {

                if (machine.USBSlots[i].usb_ctx) {


                    /*
                    modbus_set_error_recovery((void*) machine.USBSlots[i].usb_ctx, (modbus_error_recovery_mode) (MODBUS_ERROR_RECOVERY_LINK | MODBUS_ERROR_RECOVERY_PROTOCOL));
                     */

                    // 
                    rc = usb_purge_comm((void*) machine.USBSlots[i].usb_ctx);
                    if (rc) {
                        // fprintf(stdout, "[purge comm USB board:%d/%d - phase 1...:%d bytes]\n", i + 1, machine.numUSBSlots, rc);
                        // fflush(stdout);
                    }

                    // 
                    rc = usb_purge_comm((void*) machine.USBSlots[i].usb_ctx);
                    if (rc) {
                        // fprintf(stdout, "[%spurge comm USB board:%d/%d - phase 2...:%d bytes%s]\n", (char*) ANSI_COLOR_MAGENTA, i + 1, machine.numUSBSlots, rc, (char*) ANSI_COLOR_RESET);
                        // fflush(stdout);
                        machine.USBSlots[i].state = -1;
                    }
                }

                // Avvio timer
                machine.USBSlots[i].start_time = t1;
                machine.USBSlots[i].state = DEV_STATE_WAITING;


            } else if (machine.USBSlots[i].state == DEV_STATE_WAITING) {

                // Avvio comunicazione
                if (t1 - machine.USBSlots[i].start_time > MODBUS_WAIT_BEFOR_RECONNECT_MS) {
                    machine.USBSlots[i].state = DEV_STATE_CONNECTING;
                }


            } else if (machine.USBSlots[i].state == DEV_STATE_CONNECTING) {

                if (!machine.USBSlots[i].usb_ctx) {
                    if (machine.USBSlots[i].messageState <= 0) {
                        // fprintf(stderr, "[%s USB board:%d/%d - no ctx %s]\n", (char*) ANSI_COLOR_RED, i + 1, machine.numUSBSlots, (char*) ANSI_COLOR_RESET);
                    }
                    machine.USBSlots[i].messageState++;

                    machine.USBSlots[i].state = -1;

                } else {

                    ///////////////////////////////////         
                    // link ID Driver all'attuatore
                    //
                    if (!machine.USBSlots[i].pActuator) {


                        if ((rc = usb_set_slave((void*) machine.USBSlots[i].usb_ctx, (int) (i + 1))) < 0) {

                            if (machine.USBSlots[i].messageState <= 0) {
                                snprintf(App.Msg, App.MsgSize, "[%s Set slave failed on USB board:%d/%d %s]\n", (char*) ANSI_COLOR_RED, i + 1, machine.numUSBSlots, (char*) ANSI_COLOR_RESET);
                                vDisplayMessage(App.Msg);
                            }
                            machine.USBSlots[i].messageState++;
                            handle_USB_borad_init_error(i);

                        } else {

                            rc = usb_report_slave_id((void*) machine.USBSlots[i].usb_ctx, (char*) data_str);
                            if (rc < 0) {
                            } else {
                                fprintf(stdout, "[%s USB: %d/%d - Device ID:%d.%d %s]\n", (char*) ANSI_COLOR_YELLOW, i + 1, machine.numUSBSlots, (int) data_str[0], (int) data_str[1], (char*) ANSI_COLOR_RESET);
                                machine.USBSlots[i].stationId = (int) data_str[0];
                            }

                            bool actLinked = false;

                            machine.USBSlots[i].stationId = -1;

                            for (i_act = 0; i_act < machine.num_actuator; i_act++) {
                                if (machine.actuator[i_act].boardId) {
                                    if (machine.actuator[i_act].boardId == machine.USBSlots[i].stationId) {
                                        machine.USBSlots[i].pActuator = (void*) &machine.actuator[i_act];
                                        machine.actuator[i_act].pUSBSlot = (void*) &machine.USBSlots[i];
                                        // snprintf(App.Msg, App.MsgSize, "[%s Linked at %s %s]\n", (char*) ANSI_COLOR_GREEN, machine.actuator[i_act].name, (char*) ANSI_COLOR_RESET);
                                        // vDisplayMessage(App.Msg);
                                        actLinked = true;
                                        break;
                                    }
                                }
                            }

                            if (!actLinked) {
                                snprintf(App.Msg, App.MsgSize, "[%s board:%d/%d - Device active (Station:%d) - %sNOT Linked! %s ]\n", (char*) ANSI_COLOR_YELLOW, i + 1, machine.numUSBSlots, (int) machine.USBSlots[i].stationId, (char*) ANSI_COLOR_RED, (char*) ANSI_COLOR_RESET);
                                vDisplayMessage(App.Msg);

                            } else {
                                snprintf(App.Msg, App.MsgSize, "[%s board:%d/%d - Device active (Station:%d) - Linked to %s%s ]\n", (char*) ANSI_COLOR_YELLOW, i + 1, machine.numUSBSlots, (int) machine.USBSlots[i].stationId, (char*) machine.actuator[i_act].name, (char*) ANSI_COLOR_RESET);
                                vDisplayMessage(App.Msg);
                            }

                            // Stato lettura in streaming
                            machine.USBSlots[i].state = DEV_STATE_INIT_STREAM;

                            machine.USBSlots[i].messageState = 0;
                            machine.USBSlots[i].timeoutCount = 0;
                        }





                        bool dumpParams = false;

                        if (dumpParams) {
                            ///////////////////////////////////////////////
                            // Lettura : Pr0.20 Busbar voltage
                            //
                            // ...
                        }


                    } else {
                        // Già collegato : stato lettura in streaming
                        snprintf(App.Msg, App.MsgSize, "[%s board:%d/%d - Device already linked %s]\n", (char*) ANSI_COLOR_YELLOW, i + 1, machine.numUSBSlots, (char*) ANSI_COLOR_RESET);
                        vDisplayMessage(App.Msg);
                        machine.USBSlots[i].state = DEV_STATE_INIT_STREAM;
                        machine.USBSlots[i].timeoutCount = 0;
                        machine.USBSlots[i].messageState = 0;
                    }
                }


                numUSBSlotsProcessed++;
            }
        }
    }


    ///////////////////////////////////////////
    // Verifica collegamento degli attuatori
    //
    snprintf(str, sizeof (str), "[%s Checking %d actuators...%s]\n", (char*) ANSI_COLOR_YELLOW, machine.num_actuator, (char*) ANSI_COLOR_RESET);
    vDisplayMessage(str);

    for (i_act = 0; i_act < machine.num_actuator; i_act++) {

        if (machine.actuator[i_act].boardId) {

            snprintf(str, sizeof (str), "[%s %s ...%s", (char*) ANSI_COLOR_YELLOW, machine.actuator[i_act].name, (char*) ANSI_COLOR_RESET);
            vDisplayMessage(str);

            if (!machine.actuator[i_act].pUSBSlot) {
                //////////////////////////////////////
                // Generazione Emergenza Macchina
                //
                snprintf(str, sizeof (str), "Actuator %s not linked", machine.actuator[i_act].name);
                if (generate_alarm((char*) str, 5103, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                }
            } else {
                USBSlot *pUSBSlots = (USBSlot *) machine.actuator[i_act].pUSBSlot;

                if (!pUSBSlots->stationId) {
                    //////////////////////////////////////
                    // Generazione Emergenza Macchina
                    //
                    snprintf(str, sizeof (str), "Actuator %s driver offline", machine.actuator[i_act].name);
                    if (generate_alarm((char*) str, 5104, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                    }
                } else {
                    if (machine.USBSlots[i].state < 0) {
                        //////////////////////////////////////
                        // Generazione Emergenza Macchina
                        //
                        snprintf(str, sizeof (str), "Actuator %s invalid USB state (%d)", machine.actuator[i_act].name, machine.USBSlots[i].state);
                        if (generate_alarm((char*) str, 5105, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                        } else {
                        }
                    } else {
                        snprintf(str, sizeof (str), "%sOK%s]\n", (char*) ANSI_COLOR_GREEN, (char*) ANSI_COLOR_RESET);
                        vDisplayMessage(str);
                    }
                }
            }
        }
    }

    
    if (retVal < 0) {
        App.USBDetectError++;
    }
    
    return retVal;
}






/*
 * 
 * DELTA 
 * 
 * 
 
P1-01 = 0B  (USBopen)

P3-00   ADR     Address Setting         0x7C N/A O O O O 9.2
P3-01   BRT     Transmission Speed      0x0203 (2 = 500 Kbit/s CAMopen, 3 = 38400 Modbus)
P3-02   PTL     Communication Protocol  6 N/A O O O O 9.2
P3-03   FLT     Communication Error Disposal 0 N/A O O O O 9.2
P3-04   CWD     Communication Timeout 0 sec O O O O 9.2
P3-05   CMM     Communication Mechanism 0 N/A O O O O 9.2
P3-06   SDI     Control Switch of Digital Input (DI) 0 N/A O O O O 9.2
P3-07   CDT     Communication Response Delay Time 0 1ms O O O O 9.2
P3-08   MNS     Monitor Mode 0000 N/A O O O O 9.2
P3-09   SYC     USBopen Synchronize Setting 0x57A1 N/A O O O O 9.2 

 * 
 * 
 */


///////////////////////////////////
// Legge dai dispositivi USB
//

static bool GLReadPosIDLE = true;

uint32_t GLUSBMeasurePostTimeout = 0;

int GLUSBRunning = 0;

int dataExchangeLoopUSB(void) {
    int rc, retVal = 0;
    int i, i_act;
    int usb_id = 0;
    int usb_dlc = 0;
    char str[256], *data = NULL;
    uint32_t t1;
    char hHigh = 0, bLow = 0;
    int avaiable_bytes = 0;
    float newPosition = 0.0;


    GLUSBRunning = machine.numUSBSlots;

    if (!machine.numUSBSlots)
        App.USBOK = INIT_AND_RUN;
    
    for (i = 0; i < machine.numUSBSlots; i++) {
        LP_ACTUATOR pActuator = (LP_ACTUATOR) machine.USBSlots[i].pActuator;

        t1 = xTaskGetTickCount();

        
        if (!machine.USBSlots[i].disabled) {

            if (machine.USBSlots[i].state == DEV_STATE_INIT) {

                // Abilitazione USB            
                App.USBOK = INIT_AND_RUN;

                machine.USBSlots[i].state = DEV_STATE_INIT_STREAM;

            } else if (machine.USBSlots[i].state == DEV_STATE_INIT_STREAM) {
                // Init...

                // Debug mode OFF
                // usb_set_debug((void*) machine.USBSlots[i].usb_ctx, false);

                struct timeval timeout;
                timeout.tv_sec = 0;
                timeout.tv_usec = MODBUS_STREAM_TIMEOUT_MS * 1000;
                // usb_set_response_timeout((void*) machine.USBSlots[i].usb_ctx, &timeout);


                rc = usb_purge_comm((void*) machine.USBSlots[i].usb_ctx);
                if (rc) {
                    // fprintf(stdout, "[purge comm USB board#%d - %d bytes]\n", i + 1, rc);
                    // fflush(stdout);
                    /////////////////////////////////////////////////////////////
                    // Misura il tempo della risposta (x regolare il timeout
                    //
                    if (GLUSBMeasurePostTimeout) {
                        snprintf(App.Msg, App.MsgSize, "[%s Post timeout response : %dmsec %s]\n", (char*) ANSI_COLOR_RED, (xTaskGetTickCount() - GLUSBMeasurePostTimeout), (char*) ANSI_COLOR_RESET);
                        vDisplayMessage(App.Msg);
                        GLUSBMeasurePostTimeout = 0;
                    }
                }


                machine.USBSlots[i].start_time = t1;
                machine.USBSlots[i].tStat = xTaskGetTickCount();

                machine.USBSlots[i].stopRequest = 0;
                machine.USBSlots[i].setupRequest = 0;
                machine.USBSlots[i].pendingCommand = 0;
                machine.USBSlots[i].runningCommand = 0;
                machine.USBSlots[i].doneCommand = 0;


                ///////////////////////////////
                // resetta l'uscita ausiliaria
                //
                actuator_set_aux_io((void *) pActuator, (int) 0, (int) 0);




                ///////////////////////////////////////////////
                // Avvio loop
                //
                if (machine.USBSlots[i].stopRequest) {
                    machine.USBSlots[i].state = DEV_STATE_SERVICE_STOP;
                } else if (machine.USBSlots[i].setupRequest == 1) {
                    machine.USBSlots[i].state = DEV_STATE_SERVICE_SETUP;
                } else if (machine.USBSlots[i].setupRequest == 1000) {
                    machine.USBSlots[i].state = DEV_STATE_SERVICE_FIRST_SETUP;
                } else {
                    machine.USBSlots[i].state = DEV_STATE_STREAMING_SEND;
                }

                if (!GLRunLoop) {
                    machine.USBSlots[i].state = DEV_STATE_CLOSING;
                }



            } else if (machine.USBSlots[i].state == DEV_STATE_STREAMING_SEND) {

                ////////////////////////////////////////////////////       
                // Invio comando lettura in Streaming (ASINCRONA)
                //
                // Lettura posizione 
                //
                // N.B.: La lettura continua della poszione non è necessaria se l'asse non è in movimento
                //

                if (machine.USBSlots[i].runningCommand || GLReadPosIDLE) {

                    usb_id = 0;
                    usb_dlc = 0;
                    data[0] = 0;

                    // Lunghezza risposta attesa
                    machine.USBSlots[i].waitingRespSize = 8; // ???

                    if (xrt_usb_message_send((void*) machine.USBSlots[i].usb_ctx, usb_id, usb_dlc, data) < 0) {
                        /////////////////////////
                        // Generazione Allarme
                        //
                        if (generate_alarm((char*) "USB Communication Send Error", 6002, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                        }

                        // handle_USB_borad_error(i, (char*) "send", machine.USBSlots[i].state);
                        machine.USBSlots[i].streamErrorCount++;

                        // reinizializza il flusso
                        machine.USBSlots[i].state = DEV_STATE_INIT_STREAM;

                    } else {
                        machine.USBSlots[i].state = DEV_STATE_STREAMING_RECV;
                    }


                } else {
                    /////////////////////////////////////////
                    // IDLE : nessun comando in esecuzione
                    //                
                    // fine lettura
                    machine.USBSlots[i].state = DEV_STATE_STREAMING_DONE;
                }



                ////////////////////////////////////////////////////       
                // Verifica variazione paramtri degli assi
                //
                //
                if (pActuator) {
                    LP_ACTUATOR pActuatorMirror = (LP_ACTUATOR) pActuator->actuator_mirror;
                    BOOL reinitActuator = false;

                    // N.B.: Le decellerazione sono uguali alla acc x imposizione del driver
                    pActuator->dec_auto1 = pActuator->acc_auto1;
                    pActuator->dec_auto2 = pActuator->acc_auto2;

                    if (pActuatorMirror) {
                        if (fabs(pActuatorMirror->speed_auto1 - pActuator->speed_auto1) > 0.001) {
                            // snprintf(App.Msg, App.MsgSize, "[%s %s change speed1 %s]", (char*) ANSI_COLOR_GREEN, pActuator->name, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);
                            pActuatorMirror->speed_auto1 = pActuator->speed_auto1;
                            machine.USBSlots[i].paramToUpdate[SPEED_1] = TRUE;
                            reinitActuator = true;
                        }
                        if (fabs(pActuatorMirror->speed_auto2 - pActuator->speed_auto2) > 0.001) {
                            // snprintf(App.Msg, App.MsgSize, "[%s %s change speed2 %s]", (char*) ANSI_COLOR_GREEN, pActuator->name, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);
                            pActuatorMirror->speed_auto2 = pActuator->speed_auto2;
                            machine.USBSlots[i].paramToUpdate[SPEED_2] = TRUE;
                            reinitActuator = true;
                        }
                        if (fabs(pActuatorMirror->acc_auto1 - pActuator->acc_auto1) > 0.001) {
                            // snprintf(App.Msg, App.MsgSize, "[%s %s change acc_auto1 %s]", (char*) ANSI_COLOR_GREEN, pActuator->name, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);
                            pActuatorMirror->acc_auto1 = pActuator->acc_auto1;
                            machine.USBSlots[i].paramToUpdate[ACC_1] = TRUE;
                            reinitActuator = true;
                        }
                        if (fabs(pActuatorMirror->acc_auto2 - pActuator->acc_auto2) > 0.001) {
                            // snprintf(App.Msg, App.MsgSize, "[%s %s change acc_auto2 %s]", (char*) ANSI_COLOR_GREEN, pActuator->name, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);
                            pActuatorMirror->acc_auto2 = pActuator->acc_auto2;
                            machine.USBSlots[i].paramToUpdate[ACC_2] = TRUE;
                            reinitActuator = true;
                        }
                        if (fabs(pActuatorMirror->dec_auto1 - pActuator->dec_auto1) > 0.001) {
                            // snprintf(App.Msg, App.MsgSize, "[%s %s change dec_auto1 %s]", (char*) ANSI_COLOR_GREEN, pActuator->name, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);
                            pActuatorMirror->dec_auto1 = pActuator->dec_auto1;
                            machine.USBSlots[i].paramToUpdate[ACC_1] = TRUE;
                            reinitActuator = true;
                        }
                        if (fabs(pActuatorMirror->dec_auto2 - pActuator->dec_auto2) > 0.001) {
                            // snprintf(App.Msg, App.MsgSize, "[%s %s change dec_auto2 %s]", (char*) ANSI_COLOR_GREEN, pActuator->name, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);
                            pActuatorMirror->dec_auto2 = pActuator->dec_auto2;
                            machine.USBSlots[i].paramToUpdate[ACC_2] = TRUE;
                            reinitActuator = true;
                        }

                        // Forse dev'essere asincrono
                        if (reinitActuator) {
                            machine.USBSlots[i].state = DEV_STATE_SERVICE_SETUP_SPEED_ACC;
                        }
                    }
                }

            } else if (machine.USBSlots[i].state == DEV_STATE_STREAMING_RECV) {
                avaiable_bytes = usb_data_available((void*) machine.USBSlots[i].usb_ctx);
                if (avaiable_bytes >= machine.USBSlots[i].waitingRespSize) {
                    // Dato disponibile sulla USBe ...Lettura risposta ...
                    if (xrt_usb_message_recv((void*) machine.USBSlots[i].usb_ctx, (char*) &machine.USBSlots[i].data[0], machine.USBSlots[i].data_size) < 0) {
                        //
                        // Errore lettura dato : Emergenza Macchina
                        /////////////////////////
                        // Generazione Allarme
                        //
                        if (generate_alarm((char*) "USB Communication Recv Error", 6002, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                        }
                        // handle_USB_borad_error(i, (char*) "recv", machine.USBSlots[i].state);
                        machine.USBSlots[i].streamErrorCount++;

                        // reinizializa il flusso
                        machine.USBSlots[i].state = DEV_STATE_INIT_STREAM;

                    } else {

                        machine.USBSlots[i].driverPosition = (int) machine.USBSlots[i].data[4 + 0] + (uint32_t) machine.USBSlots[i].data[4 + 1]*10000;
                        machine.USBSlots[i].driverSpeed = (int) ((int16_t) machine.USBSlots[i].data[0]);
                        machine.USBSlots[i].driverAcc = (uint32_t) MAKEDWORD(0, 0);

                        machine.USBSlots[i].readCount++;

                        // Passa il dato alla logica
                        if (pActuator) {

                            actuator_encoder_to_position((void *) pActuator, (int32_t) machine.USBSlots[i].data[4 + 1], (int32_t) machine.USBSlots[i].data[4 + 0], &newPosition);

                            // posizione reale
                            pActuator->cur_rpos = (float) newPosition;

                            // velocita
                            pActuator->speed = (float) machine.USBSlots[i].driverSpeed;


                            if (pActuator->speed > pActuator->max_speed)
                                pActuator->max_speed = pActuator->speed;
                            if (pActuator->speed < pActuator->min_speed)
                                pActuator->min_speed = pActuator->speed;

                            // posizione virtuale (0...1000)
                            update_actuator_virtual_pos(pActuator);
                        }

                        // Comando in esecuzione ?
                        if (machine.USBSlots[i].runningCommand) {
                        }


                        // fine lettura
                        machine.USBSlots[i].state = DEV_STATE_STREAMING_DONE;
                    }

                } else {
                    if (t1 - machine.USBSlots[i].start_time > MODBUS_STREAM_TIMEOUT_MS) {
                        /////////////////////////////
                        // Comunicazione in timeout 
                        //
                        // Generazione Allarme
                        //

                        GLUSBMeasurePostTimeout = t1;

                        snprintf(str, sizeof (str), "USB Communication Streaming timeout (%d/%d)", avaiable_bytes, machine.USBSlots[i].waitingRespSize);
                        if (generate_alarm((char*) str, 6003, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                        }

                        // handle_USB_borad_error(i, (char*) "timeout!", machine.USBSlots[i].state);

                        // reinizializza il flusso
                        machine.USBSlots[i].state = DEV_STATE_INIT_STREAM;
                    }
                }




            } else if (machine.USBSlots[i].state == DEV_STATE_STREAMING_DONE) {


                ////////////////////////////////////////////////////
                // Fine comunicazione : azione post lettura
                //                        
                if (t1 - machine.USBSlots[i].tStat >= 5000) {
                    ////////////////////////////////////
                    // Debug : stampa ad ogno secondo
                    //
                    if (machine.USBSlots[i].state >= 0) {
                        if (machine.USBSlots[i].state >= 10 && machine.USBSlots[i].state <= 900) {
                            GLUSBDataPerSec = machine.USBSlots[i].readCount / 5;

                            // Statistiche IO/sec
                            if (GLUSBDataPerSec > GLUSBMaxDataPerSec)
                                GLUSBMaxDataPerSec = GLUSBDataPerSec;
                            if (GLUSBDataPerSec < GLUSBMinDataPerSec && GLUSBDataPerSec > 0)
                                GLUSBMinDataPerSec = GLUSBDataPerSec;

                            GLUSBLastDataPerSec = GLUSBDataPerSec;

                        }
                    }

                    machine.USBSlots[i].readCount = 0;
                    machine.USBSlots[i].streamErrorCount = 0;

                    machine.USBSlots[i].tStat = t1;
                }



                //////////////////////////////////////////
                // Esecuzione Comando di test / debug      
                //
                /*
                if (GLTestUSB == 1) { // pos test
                    GLTestUSB = 0;
                } else if (GLTestUSB == 2) { // speed test
                    GLTestUSB = 0;
                } else if (GLTestUSB == 3) { // reset
                    GLTestUSB = 0;
                } else if (GLTestUSB == 4) { // stop motor
                    GLTestUSB = 0;
                } else if (GLTestUSB == 5) { // reset position table
                    GLTestUSB = 0;
                } else if (GLTestUSB == 6) { // standby motor
                    GLTestUSB = 0;
                }
                */

                ///////////////////////////////////////////////
                // Richiesta di chiusura o rilettura
                //
                if (machine.USBSlots[i].stopRequest) {
                    machine.USBSlots[i].state = DEV_STATE_SERVICE_STOP;
                } else if (machine.USBSlots[i].setupRequest == 1) {
                    machine.USBSlots[i].state = DEV_STATE_SERVICE_SETUP;
                } else if (machine.USBSlots[i].setupRequest == 1000) {
                    machine.USBSlots[i].state = DEV_STATE_SERVICE_FIRST_SETUP;
                } else {
                    machine.USBSlots[i].state = DEV_STATE_STREAMING_POST_ACT;
                }


                if (!GLRunLoop) {
                    machine.USBSlots[i].state = DEV_STATE_CLOSING;
                }



            } else if (machine.USBSlots[i].state == DEV_STATE_STREAMING_POST_ACT) {
                // Fine comunicazione, imposta la traccia temporale
                machine.USBSlots[i].start_time = t1;
                machine.USBSlots[i].state = DEV_STATE_STREAMING_POST_WAIT;



            } else if (machine.USBSlots[i].state == DEV_STATE_STREAMING_POST_WAIT) {
                ///////////////////////////
                // Attesa prossimo loop
                //
                if (t1 - machine.USBSlots[i].start_time >= MODBUS_LOOP_TIME_MS) {
                    ///////////////////////////
                    // Comando da eseguire ?
                    //
                    if (machine.USBSlots[i].pendingCommand) {
                        machine.USBSlots[i].state = DEV_STATE_CMD_INIT;
                    } else if (machine.USBSlots[i].stopRequest) {
                        machine.USBSlots[i].state = DEV_STATE_SERVICE_STOP;
                    } else if (machine.USBSlots[i].setupRequest == 1) {
                        machine.USBSlots[i].state = DEV_STATE_SERVICE_SETUP;
                    } else if (machine.USBSlots[i].setupRequest == 1000) {
                        machine.USBSlots[i].state = DEV_STATE_SERVICE_FIRST_SETUP;
                    } else {
                        // Ritorna alla sequenza di lettura
                        machine.USBSlots[i].state = DEV_STATE_STREAMING_SEND;
                    }
                }


                if (!GLRunLoop) {
                    machine.USBSlots[i].state = DEV_STATE_CLOSING;
                }






                ////////////////////////////////////////
                // Comando di servizio
                //

            } else if (machine.USBSlots[i].state == DEV_STATE_SERVICE) {
                /////////////////////////////////////////////////          
                // Invio comando lettura alla USBe SINCRONO
                // Lettura posizione
                //
                usb_id = 0;
                usb_dlc = 0;
                data[0] = 0;

                if (usb_read_registers((void*) machine.USBSlots[i].usb_ctx, usb_id, usb_dlc, (char*)&machine.USBSlots[i].data[0], machine.USBSlots[i].data_size) < 0) {
                    ////////////////////////////
                    // Generazione Allarme
                    //
                    if (generate_alarm((char*) "USB Communication Service Error", 6004, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                    }

                    // handle_USB_borad_error(i, (char*) "read_reg", machine.USBSlots[i].state);
                    machine.USBSlots[i].streamErrorCount++;

                    // reinizializa il flusso
                    machine.USBSlots[i].state = DEV_STATE_INIT_STREAM;

                } else {
                    machine.USBSlots[i].readCount++;
                    // fine lettura
                    machine.USBSlots[i].state = DEV_STATE_STREAMING_DONE;
                }

                if (!GLRunLoop) {
                    machine.USBSlots[i].state = DEV_STATE_CLOSING;
                }




            } else if (machine.USBSlots[i].state == DEV_STATE_SERVICE_SETUP_SPEED_ACC) {
                int Error = 0;
                int speed_rpm = (int) pActuator->speed_auto1 > 0 ? (int) pActuator->speed_auto1 : 1;
                int acc_ms = pActuator->acc_auto1 >= 0.0f ? (int) pActuator->acc_auto1 : 0, dec_ms = acc_ms;
                int speed_rpm2 = (int) pActuator->speed_auto2 > 0 ? (int) pActuator->speed_auto2 : 1;
                int acc_ms2 = pActuator->acc_auto2 >= 0.0f ? (int) pActuator->acc_auto2 : 0, dec_ms2 = acc_ms2;


                machine.USBSlots[i].start_time = t1;

                if (machine.USBSlots[i].paramToUpdate[SPEED_1]) {
                    if (usb_param_update((void*) machine.USBSlots[i].usb_ctx, 2, 02, speed_rpm, false) < 0) {
                        Error++;
                    }
                    if (usb_purge_comm((void*) machine.USBSlots[i].usb_ctx)) {
                    }
                    machine.USBSlots[i].paramToUpdate[SPEED_1] = 0;
                    snprintf(App.Msg, App.MsgSize, "[%s update %s param 2.02 { speed1:%d } [%dmsec] Err:%d %s]", (char*) ANSI_COLOR_GREEN, pActuator->name, speed_rpm, (xTaskGetTickCount() - machine.USBSlots[i].start_time), Error, (char*) ANSI_COLOR_RESET);
                    vDisplayMessage(App.Msg);
                }

                if (machine.USBSlots[i].paramToUpdate[ACC_1] || machine.USBSlots[i].paramToUpdate[ACC_2]) {
                    if (usb_param_update((void*) machine.USBSlots[i].usb_ctx, 1, 39, acc_ms, false) < 0) {
                        Error++;
                    }
                    if (usb_purge_comm((void*) machine.USBSlots[i].usb_ctx)) {
                    }
                    machine.USBSlots[i].paramToUpdate[ACC_1] = 0;
                    machine.USBSlots[i].paramToUpdate[ACC_2] = 0;
                    snprintf(App.Msg, App.MsgSize, "[%s update %s param 1.39 { acc:%d } [%dmsec] Err:%d %s]", (char*) ANSI_COLOR_GREEN, pActuator->name, acc_ms, (xTaskGetTickCount() - machine.USBSlots[i].start_time), Error, (char*) ANSI_COLOR_RESET);
                    vDisplayMessage(App.Msg);
                }
                if (machine.USBSlots[i].paramToUpdate[SPEED_2]) {
                    if (usb_param_update((void*) machine.USBSlots[i].usb_ctx, 2, 06, speed_rpm2, false) < 0) {
                        Error++;
                    }
                    if (usb_purge_comm((void*) machine.USBSlots[i].usb_ctx)) {
                    }
                    machine.USBSlots[i].paramToUpdate[SPEED_2] = 0;
                    snprintf(App.Msg, App.MsgSize, "[%s update %s param 2.06 { speed2:%d } [%dmsec] Err:%d %s]", (char*) ANSI_COLOR_GREEN, pActuator->name, speed_rpm2, (xTaskGetTickCount() - machine.USBSlots[i].start_time), Error, (char*) ANSI_COLOR_RESET);
                    vDisplayMessage(App.Msg);
                }

                // N.B.: Per modificare il parametro acc di ritorno è necessario cambiarla la volo
                // if (usb_param_update((void*) machine.USBSlots[i].usb_ctx, 1, 39, acc_ms, true) < 0) {
                // }

                /*
                snprintf(App.Msg, App.MsgSize, "[%s update %s param {speed1:%d - speed2:%d - acc:%d} [%dmsec] Err:%d %s]", (char*) ANSI_COLOR_GREEN, pActuator->name, speed_rpm, speed_rpm2, acc_ms, (xTaskGetTickCount() - machine.USBSlots[i].start_time), Error, (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);
                 * */

                // fine lettura
                machine.USBSlots[i].state = DEV_STATE_STREAMING_DONE;


            } else if (machine.USBSlots[i].state == DEV_STATE_SERVICE_SETUP) {
                int32_t targetTurns = 0, targetPulses = 0, targetTurns2 = 0, targetPulses2 = 0;
                float rpos = 0.0f;


                machine.USBSlots[i].setupRequest = 0;

                if (pActuator) {
                    int speed_rpm = pActuator->speed_auto1;
                    int acc_ms = pActuator->acc_auto1, dec_ms = acc_ms;
                    int speed_rpm2 = pActuator->speed_auto2;
                    int acc_ms2 = pActuator->acc_auto2, dec_ms2 = acc_ms2;


                    actuator_position_to_encoder(pActuator, pActuator->end_rpos, &targetTurns, &targetPulses);


                    ///////////////////////////////
                    // resetta l'uscita ausiliaria
                    //
                    actuator_set_aux_io((void *) pActuator, (int) 0, (int) 0);

                    usleep(1 * 1000);

                    // Debug mode                        
                    // modbus_set_debug((void*)((USBSlot*)pActuator->pUSBSlots)->usb_ctx, true);

                    // if (pActuator->step == STEP_UNINITIALIZED) {
                    if (handle_actuator_position_mode((void *) pActuator, targetTurns, targetPulses, speed_rpm, acc_ms, dec_ms, targetTurns2, targetPulses2, speed_rpm2, acc_ms2, dec_ms2, 0, 0, 0) < 0) {
                        // return -1;
                        if (generate_alarm((char*) "handle_actuator_position_mode() Error", 6100, 0, (int) /*ALARM_FATAL_ERROR*/ALARM_WARNING, 0+1) < 0) {
                        }
                    }

                    usleep(10 * 1000);

                    // Avvio driver (comando Servo ON)
                    if (!pActuator->disabled) {
                        if (handle_actuator_servo_on((void *) pActuator) < 0) {
                            // return -1;
                            if (generate_alarm((char*) "handle_actuator_start() Error", 6101, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                            }
                        }
                    } else {
                        if (handle_actuator_servo_off((void *) pActuator) < 0) {
                            // return -1;
                            if (generate_alarm((char*) "handle_actuator_start() Error", 6101, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                            }
                        }                    
                    }

                    // Lettura posizione
                    usb_id = 0;
                    usb_dlc = 0;
                    data[0] = 0;

                    if (usb_read_registers((void*) machine.USBSlots[i].usb_ctx, usb_id, usb_dlc, (char*) &machine.USBSlots[i].data[0], machine.USBSlots[i].data_size) < 0) {
                        if (generate_alarm((char*) "usb_read_registers() Error", 6102, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                        }
                    } else {
                        // Passa il dato alla logica
                        if (pActuator) {

                            actuator_encoder_to_position((void *) pActuator, (int32_t) machine.USBSlots[i].data[5], (int32_t) machine.USBSlots[i].data[4], &newPosition);

                            if (fabs(pActuator->cur_rpos - newPosition) > 0.01) {
                                // snprintf(App.Msg, App.MsgSize, "[%s %s Initial POS %0.3f->%0.3f%s]", (char*) ANSI_COLOR_GREEN, pActuator->name, pActuator->cur_rpos, newPosition, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);
                                pActuator->cur_rpos = newPosition;
                            } else {
                                // snprintf(App.Msg, App.MsgSize, "[%s %s Initial POS %0.3f OK%s]", (char*) ANSI_COLOR_GREEN, pActuator->name, pActuator->cur_rpos, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);
                            }

                            if (fabs(pActuator->cur_rpos - pActuator->end_rpos) < 0.01) {
                                pActuator->position = ON;
                                snprintf(App.Msg, App.MsgSize, "[%s %s Initial POS=ON %s]", (char*) ANSI_COLOR_GREEN, pActuator->name, (char*) ANSI_COLOR_RESET);
                                vDisplayMessage(App.Msg);
                            } else if (fabs(pActuator->cur_rpos - pActuator->start_rpos) < 0.01) {
                                pActuator->position = OFF;
                                snprintf(App.Msg, App.MsgSize, "[%s %s Initial POS=OFF %s]", (char*) ANSI_COLOR_GREEN, pActuator->name, (char*) ANSI_COLOR_RESET);
                                vDisplayMessage(App.Msg);
                            }

                            // velocita
                            pActuator->speed = (float) machine.USBSlots[i].data[0];


                        } else {
                            if (generate_alarm((char*) "USB: Actuator not linked", 6103, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                            }
                        }
                    }

                    // Debug mode                        
                    // modbus_set_debug((void*)((USBSlot*)pActuator->pUSBSlots)->usb_ctx, false);
                }

                // reinizializa il flusso
                machine.USBSlots[i].state = DEV_STATE_INIT_STREAM;

                if (!GLRunLoop) {
                    machine.USBSlots[i].state = DEV_STATE_CLOSING;
                }



            } else if (machine.USBSlots[i].state == DEV_STATE_SERVICE_STOP) {

                machine.USBSlots[i].stopRequest = 0;

                ///////////////////////////////////////////////
                // Scrittura : Pr6.01 -> 0 (Stop motor)
                //
                if (usb_param_update((void*) machine.USBSlots[i].usb_ctx, 6, 1, 0, false) < 0) {
                    // return -1;
                }
                machine.USBSlots[i].state = DEV_STATE_SERVICE_OUT;

            } else if (machine.USBSlots[i].state == DEV_STATE_SERVICE_OUT) {

                GLUSBRunning--;

                if (GLRunLoop) {
                    if (machine.USBSlots[i].setupRequest == 1) {
                        machine.USBSlots[i].state = DEV_STATE_SERVICE_SETUP;
                        machine.USBSlots[i].setupRequest = 0;
                        snprintf(App.Msg, App.MsgSize, "[Restart USB service]");
                        vDisplayMessage(App.Msg);
                    } else if (machine.USBSlots[i].setupRequest == 1000) {
                        machine.USBSlots[i].state = DEV_STATE_SERVICE_FIRST_SETUP;
                    } else {
                        if (machine.USBSlots[i].pendingCommand) {
                            // Ripresa del servizio su comando in pendenza
                            machine.USBSlots[i].setupRequest = 1;
                        }
                    }
                }





                /////////////////////////////////////////////////////
                // Invio del Comando Movimento e attesa esecuzione
                //

            } else if (machine.USBSlots[i].state == DEV_STATE_CMD_INIT) {

                if (pActuator) {
                    float acc = 0.0f, dec = 0.0f, speed = 0.0f;
                    if (machine.USBSlots[i].pendingCommand == 1) {
                        acc = pActuator->acc_auto1;
                        dec = pActuator->dec_auto1;
                        speed = pActuator->speed_auto1;
                    } else if (machine.USBSlots[i].pendingCommand == -1) {
                        acc = pActuator->acc_auto2;
                        dec = pActuator->dec_auto2;
                        speed = pActuator->speed_auto2;
                    } else {
                    }

                    pActuator->start_time1 = 0;
                    pActuator->start_time2 = 0;
                    pActuator->start_time3 = 0;


                    machine.USBSlots[i].doneCommand = 0;

                    machine.USBSlots[i].start_time = t1;
                    machine.USBSlots[i].state = DEV_STATE_CMD_INIT_SEND;


                } else {
                    if (generate_alarm((char*) "DEV_STATE_CMD_INIT:no actuator linked", 6014, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                    }
                    // reinizializa il flusso
                    machine.USBSlots[i].state = DEV_STATE_INIT_STREAM;
                }




            } else if (machine.USBSlots[i].state == DEV_STATE_CMD_INIT_SEND) {

                //////////////////////       
                // Invio comando 
                //

                usb_id = 0;
                usb_dlc = 0;
                data[0] = 0;


                // Lunghezza risposta attesa
                machine.USBSlots[i].waitingRespSize = 8;

                if (xrt_usb_message_send((void*) machine.USBSlots[i].usb_ctx, usb_id, usb_dlc, data) < 0) {
                    ////////////////////////////
                    // Generazione Allarme                
                    //
                    if (generate_alarm((char*) "USB Communication CMD INIT Error", 6005, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                    }

                    // handle_USB_borad_error(i, (char*) "send", machine.USBSlots[i].state);
                    machine.USBSlots[i].streamErrorCount++;

                    // reinizializa il flusso
                    machine.USBSlots[i].state = DEV_STATE_INIT_STREAM;

                } else {
                    machine.USBSlots[i].start_time = t1;
                    machine.USBSlots[i].state = DEV_STATE_CMD_INIT_RECV;
                }


            } else if (machine.USBSlots[i].state == DEV_STATE_CMD_INIT_RECV) {
                avaiable_bytes = usb_data_available((void*) machine.USBSlots[i].usb_ctx);
                if (avaiable_bytes >= machine.USBSlots[i].waitingRespSize) {
                    // Dato disponibile sulla USBe ...Lettura risposta ...
                    if (xrt_usb_message_recv((void*) machine.USBSlots[i].usb_ctx, (char*) &machine.USBSlots[i].data[0], machine.USBSlots[i].data_size) < 0) {
                        //
                        // Errore lettura dato : Emergenza Macchina
                        /////////////////////////
                        // Generazione Allarme
                        //
                        if (generate_alarm((char*) "USB Communication CMD INIT Recv Error", 6002, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                        }

                        // handle_USB_borad_error(i, (char*) "recv", machine.USBSlots[i].state);
                        machine.USBSlots[i].streamErrorCount++;

                        // reinizializa il flusso
                        machine.USBSlots[i].state = DEV_STATE_INIT_STREAM;

                    } else {

                        // snprintf(App.Msg, App.MsgSize, "[%s board:%d - recived %dbytes :[%2x] - starting feedback read %s]\n", (char*) ANSI_COLOR_MAGENTA, i + 1, (int) avaiable_bytes, (int) machine.USBSlots[i].data[0], (char*) ANSI_COLOR_RESET);
                        // vDisplayMessage(App.Msg);


                        ////////////////////////////////////
                        // Attivazione Uscite ausiliarie
                        //
                        if (actuator_set_aux_io((void *) pActuator, (int) 1, (int) 0) > 0) {
                            // snprintf(App.Msg, App.MsgSize, "[%s board:%d - AUX DO activated %s]\n", (char*) ANSI_COLOR_MAGENTA, i + 1, (char*) ANSI_COLOR_RESET);
                            // vDisplayMessage(App.Msg);
                        } else {
                            if (generate_alarm((char*) "AUX DO no activated", 6019, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                            }
                        }

                        /*
                         *
                         * 
                         *           
                          Software
                          trigger
                          P5-07 Directly write the procedure number into P5-07 and
                          trigger command.
                          Both panel and communication (RS-232/485／
                          USBopen) can do.
                          Application: PC or PLC that issues command via
                          communication.
                        * 
                        * 
                        * 
                        */

                        // Stato di Comando in esecuzione
                        machine.USBSlots[i].runningCommand = machine.USBSlots[i].pendingCommand;
                        machine.USBSlots[i].pendingCommand = 0;

                        // Ciclo lettura posizione
                        machine.USBSlots[i].state = DEV_STATE_CMD_FEEDBACK_SEND;
                    }

                } else {

                    if (t1 - machine.USBSlots[i].start_time > MODBUS_STREAM_TIMEOUT_MS) {
                        /////////////////////////////
                        // Comunicazione in timeout 
                        //
                        // Generazione Allarme
                        //                    

                        GLUSBMeasurePostTimeout = t1;

                        if (generate_alarm((char*) "USB Communication CMD Timeout", 6006, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                        }
                        // handle_USB_borad_error(i, (char*) "timeout!", -1);

                        // reinizializa il flusso
                        machine.USBSlots[i].state = DEV_STATE_INIT_STREAM;
                    }
                }



            } else if (machine.USBSlots[i].state == DEV_STATE_CMD_FEEDBACK_SEND) {

                /////////////////////////////////////
                // Invio comando lettura posizione
                //
                usb_id = 0;
                usb_dlc = 0;
                data[0] = 0;


                // Lunghezza risposta attesa
                machine.USBSlots[i].waitingRespSize = 8; // ???

                if (xrt_usb_message_send((void*) machine.USBSlots[i].usb_ctx, usb_id, usb_dlc, data) < 0) {
                    //////////////////////////////
                    // Generazione Allarme        
                    //
                    if (generate_alarm((char*) "USB Communication CMD FEEDBACK Send Error", 6007, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                    }


                } else {
                    machine.USBSlots[i].start_time = t1;
                    machine.USBSlots[i].state = DEV_STATE_CMD_FEEDBACK_RECV;
                }



            } else if (machine.USBSlots[i].state == DEV_STATE_CMD_FEEDBACK_RECV) {

                if (usb_data_available((void*) machine.USBSlots[i].usb_ctx) >= machine.USBSlots[i].waitingRespSize) {
                    // Dato disponibile sulla USBe ...Lettura risposta ...
                    if (xrt_usb_message_recv((void*) machine.USBSlots[i].usb_ctx, (char*) &machine.USBSlots[i].data[0], machine.USBSlots[i].data_size) < 0) {
                        //////////////////////////////
                        // Generazione Allarme        
                        //
                        if (generate_alarm((char*) "USB Communication CMD FEEDBACK Recv Error", 6008, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                        }

                        // reinizializa il flusso
                        machine.USBSlots[i].state = DEV_STATE_INIT_STREAM;

                    } else {

                        machine.USBSlots[i].driverPosition = (int) machine.USBSlots[i].data[4 + 0] + (uint32_t) machine.USBSlots[i].data[4 + 1]*10000;
                        machine.USBSlots[i].driverSpeed = (int) ((int16_t) machine.USBSlots[i].data[0]);
                        machine.USBSlots[i].driverAcc = (uint32_t) MAKEDWORD(0, 0);

                        machine.USBSlots[i].readCount++;


                        // snprintf(App.Msg, App.MsgSize, "[%s board:%d - feedback read pos:%d %s]\n", (char*) ANSI_COLOR_MAGENTA, i + 1, (int)machine.USBSlots[i].driverPosition, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);




                        // Passa il dato alla logica
                        if (pActuator) {
                            bool positionReached = false;

                            actuator_encoder_to_position((void *) pActuator, (int32_t) machine.USBSlots[i].data[4 + 1], (int32_t) machine.USBSlots[i].data[4 + 0], &newPosition);

                            if (fabs(pActuator->cur_rpos - newPosition) > 0.01) {
                                if (!pActuator->start_time1)
                                    pActuator->start_time1 = t1;
                            }

                            // velocita
                            pActuator->speed = (float) machine.USBSlots[i].driverSpeed;


                            if (pActuator->speed > pActuator->max_speed)
                                pActuator->max_speed = pActuator->speed;
                            if (pActuator->speed < pActuator->min_speed)
                                pActuator->min_speed = pActuator->speed;

                            // posizione reale
                            pActuator->cur_rpos = newPosition;
                            pActuator->readCounter++;


                            // posizione virtuale (0...1000)
                            update_actuator_virtual_pos(pActuator);


                            // Debug
                            /*
                            if (machine.USBSlots[i].readCount % 10 == 0 || pActuator->positionReached) {
                                snprintf(App.Msg, App.MsgSize, "[%d.%d->%s%0.3f%s]", (int) (int16_t) machine.USBSlots[i].data[4+0], (int) (int16_t) machine.USBSlots[i].data[4+1], (char*) ANSI_COLOR_YELLOW, newPosition, (char*) ANSI_COLOR_RESET);
                                vDisplayMessage(App.Msg);
                            }
                             */



                            // Ancora dati nel buffer ?
                            int available_data;
                            if ((available_data = usb_data_available((void*) machine.USBSlots[i].usb_ctx)) > 0) {
                                snprintf(App.Msg, App.MsgSize, "[%d bytes pending]", (int) available_data);
                                vDisplayMessage(App.Msg);
                            }



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

                                if (machine.USBSlots[i].runningCommand) {
                                    machine.USBSlots[i].doneCommand = machine.USBSlots[i].runningCommand;
                                    machine.USBSlots[i].runningCommand = 0;
                                } else {
                                }
                                // posizione raggiunta fine sequenza
                                machine.USBSlots[i].state = DEV_STATE_CMD_DONE;

                            } else {

                                if (machine.USBSlots[i].stopRequest) {
                                    machine.USBSlots[i].state = DEV_STATE_SERVICE_STOP;
                                } else {
                                    // Ripete la lettura
                                    machine.USBSlots[i].state = DEV_STATE_CMD_FEEDBACK_SEND;
                                }
                            }
                        } else {
                            // nessun attuattore collegato : fine lettura
                            snprintf(App.Msg, App.MsgSize, "[%s No actuator linked...exit seq %s]\n", (char*) ANSI_COLOR_YELLOW, (char*) ANSI_COLOR_RESET);
                            vDisplayMessage(App.Msg);
                            machine.USBSlots[i].state = DEV_STATE_STREAMING_DONE;
                        }
                    }

                } else {
                    if (t1 - machine.USBSlots[i].start_time > MODBUS_STREAM_TIMEOUT_MS) {
                        /////////////////////////////
                        // Comunicazione in timeout 
                        //
                        // Generazione Allarme
                        //

                        GLUSBMeasurePostTimeout = t1;

                        /*
                        snprintf((char*) str, sizeof (str), (char*) "USB Communication CMD FEEDBACK Recv timeout (%d) - Available data:%d/%d"
                                , (t1 - machine.USBSlots[i].start_time)
                                , usb_data_available((void*) machine.USBSlots[i].usb_ctx)
                                , machine.USBSlots[i].waitingRespSize);
                         */

                        if (generate_alarm((char*) str, 6009, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                        }
                        // handle_USB_borad_error(i, (char*) "timeout!", -1);

                        // reinizializa il flusso
                        machine.USBSlots[i].state = DEV_STATE_INIT_STREAM;
                    }
                }



                //////////////////////////////////////////
                // Controllo del timeout del movimento
                //
                if (machine.USBSlots[i].runningCommand) {
                    if (pActuator) {
                        uint timeout_ms = 0;
                        float target_pos = 0.0f;

                        // Controllo stato dell'attuatore
                        if (pActuator->step == STEP_DONE || pActuator->step == STEP_ERROR || pActuator->step == STEP_STOPPED) {
                            // reinizializa il flusso
                            machine.USBSlots[i].state = DEV_STATE_INIT_STREAM;
                        }

                        // snprintf(App.Msg, App.MsgSize, "[%sActuator3 t1:%d - start time:%d%s]", (char*) ANSI_COLOR_GREEN, t1, pActuator->start_time, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);

                        if (machine.USBSlots[i].runningCommand == 1) {
                            timeout_ms = pActuator->timeout1_ms;
                            target_pos = pActuator->end_rpos;
                        } else if (machine.USBSlots[i].runningCommand == -1) {
                            timeout_ms = pActuator->timeout2_ms;
                            target_pos = pActuator->start_rpos;
                        }
                        if (timeout_ms > 0) {
                            if (t1 - pActuator->start_time > timeout_ms) {
                                if (!(pActuator->rtOptions & 2)) {
                                    pActuator->rtOptions += 2;
                                }
                                snprintf(str, sizeof (str), "Actuator %s timeout (side:%d time:%0.3fsec) - Position:%0.3f/%0.3f - DriverPos.:%d"
                                        , pActuator->name
                                        , machine.USBSlots[i].runningCommand
                                        , (float) timeout_ms / 1000.0f
                                        , pActuator->cur_rpos
                                        , target_pos
                                        , machine.USBSlots[i].driverPosition
                                        );
                                if (generate_alarm((char*) str, 6011, 0, (int) ALARM_ERROR, 0+1) < 0) {
                                }
                                // reinizializa il flusso
                                machine.USBSlots[i].state = DEV_STATE_INIT_STREAM;
                            }
                        }
                    }
                }



            } else if (machine.USBSlots[i].state == DEV_STATE_CMD_DONE) {

                // Comando terminato
                // snprintf(App.Msg, App.MsgSize, "[%s CMD done...exit seq %s]\n", (char*) ANSI_COLOR_YELLOW, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);


                if (pActuator) {
                    uint timewarn_ms = 0;

                    ///////////////////////////////
                    // Verifica tempo esecuzione
                    //
                    if (machine.USBSlots[i].runningCommand == 1) {
                        timewarn_ms = pActuator->timewarn1_ms;
                    } else if (machine.USBSlots[i].runningCommand == -1) {
                        timewarn_ms = pActuator->timewarn2_ms;
                    }
                    if (timewarn_ms > 0) {
                        if (t1 - pActuator->start_time > timewarn_ms) {
                            if (!(pActuator->rtOptions & 1)) {
                                pActuator->rtOptions += 1;
                                snprintf(str, sizeof (str), "Actuator %s time warning (side:%d time:%0.3fsec)", pActuator->name, machine.USBSlots[i].runningCommand, (float) timewarn_ms / 1000.0f);
                                if (generate_alarm((char*) str, 6010, 0, (int) ALARM_WARNING, 0+1) < 0) {
                                }
                            }
                        }
                    }
                }


                ///////////////////////////////
                // resetta l'uscita ausiliaria
                //
                actuator_set_aux_io((void *) pActuator, (int) 0, (int) 0);


                // posizione raggiunta fine lettura ???
                machine.USBSlots[i].state = DEV_STATE_STREAMING_DONE;

                // reinizializza : il purge non dovrebbe trovare dati
                machine.USBSlots[i].state = DEV_STATE_INIT_STREAM;




            } else if (machine.USBSlots[i].state == DEV_STATE_CMD_ERROR) {
                ///////////////////////////
                // Generazione Allarme
                //                    
                if (generate_alarm((char*) "USB Communication Error", 6009, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                }

                // reinizializza il flusso
                machine.USBSlots[i].state = DEV_STATE_INIT_STREAM;





            } else if (machine.USBSlots[i].state == DEV_STATE_CLOSING) {
                machine.USBSlots[i].state = DEV_STATE_SERVICE_STOP;
                snprintf(App.Msg, App.MsgSize, "[RTC] USB #%d Stopped\n", (i + 1));
                vDisplayMessage(App.Msg);
            }
        }
    }


    return retVal;
}

int dataExchangeIsRunningUSB() {
    for (uint32_t i = 0; i < machine.numUSBSlots; i++) {
        if (machine.USBSlots[i].state != DEV_STATE_SERVICE_OUT && machine.USBSlots[i].state != 0 && machine.USBSlots[i].state != -1) {
            return 1;
        }
    }
    return 0;
}

int test_USB() {

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
