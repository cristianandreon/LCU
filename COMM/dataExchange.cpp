#define EXTERN

/////////////////////////
// RT Kernel includes
//
#ifdef WATCOM
#include "FreeRTOS.h"
#include "task.h"
#else
#include <iostream>

#include "./../RTLinux/RTLinux.h"
#endif




// Macro attivazione debug
// #define MODBUS_DEBUG
// #define DEBUG_PRINTF


// #define DISABLE_UI_RESPONDER
// #define DISABLE_UI_RESPONDER_SEND


// Attesa prima di ritentare l'init delle periperiche
#define WAIT_BEFORE_REINIT_BOARDS_MSEC 3000





// IO WatchDog (GLOBALE per essere resettato dalla callback)
uint32_t tickTocIOWatchDog = 0;
uint32_t tickTocIOKeepAlive = 0;










#define MAX_SEND_ERROR  3




// Stringa avvio streaming
char *INIT_COMM_KEY_STR = (char *) calloc(12, 1);

char *dataExchangeGetStat(int32_t state) {
    switch (state) {
        case STATE_UNINIT:
            return (char*) "UNINIT";
            break;
        case STATE_INIT:
            return (char*) "INIT";
            break;
        case STATE_PENDING:
            return (char*) "PENDING";
            break;
        case STATE_WAITING_FOR_STREAM:
            return (char*) "WAITING STREAM";
            break;
        case STATE_LISTEN:
            return (char*) "LISTEN";
            break;
        case STATE_CONNETED:
            return (char*) "CONNETED";
            break;
        case STATE_WEBSOCKET_PARSING:
            return (char*) "WS PARSING";
            break;
        case STATE_REPLYNG:
            return (char*) "REPLYNG";
            break;
        case STATE_WEBSOCKET_REPLYNG:
            return (char*) "WS REPLYNG";
            break;
        case STATE_WEBSOCKET_SINGLE_REPLYNG:
            return (char*) "WS SINGLE REPLYNG";
            break;
        case STATE_WEBSOCKET_REPLYNG_DATA:
            return (char*) "WS REPLYNG DATA";
            break;
        case STATE_WEBSOCKET_SINGLE_REPLYNG_DATA:
            return (char*) "WS SINGLE REPLYNG DATA";
            break;
        case STATE_CLOSING:
            return (char*) "CLOSING";
            break;
        case STATE_CLOSED:
            return (char*) "CLOSED";
            break;
        case STATE_TESTING:
            return (char*) "TESTING";
            break;
        case STATE_MAX:
            return (char*) "OUT OF RANGE";
            break;
        default:
            return (char*) "???";
            break;
    }

    return (char*) "";
}

int32_t dataExchangeDump(char *msg, size_t msg_size) {

    //////////////////////////////////////
    // Generazione Log Macchina
    //
    /*
    if (generate_alarm((char*) "Help request", 2002, 1, (int32_t) ALARM_LOG) < 0) {
    }
    if (generate_alarm((char*) "Warning", 2002, 2, (int32_t) ALARM_WARNING) < 0) {
    }
    if (generate_alarm((char*) "Error", 2002, 3, (int32_t) ALARM_ERROR) < 0) {
    }
     */
    /*
    if (generate_alarm((char*)"Fatal error", 2002, 4, (int32_t)ALARM_FATAL_ERROR ) < 0) {   
    }
     */

    if (msg) {
        snprintf(msg, msg_size, "[UI State:%s][HTTP State:%s]\n", dataExchangeGetStat((int32_t) App.UICommState), dataExchangeGetStat((int32_t) App.HTTPServerState));
        vDisplayMessage(msg);

        snprintf(msg, msg_size, "[IO KAto:%d][IO WD:%d][IO Pending:%d]\n", (int32_t) GLIOBoard[0].tickTocIOKeepAliveTimeout, (int32_t) GLIOBoard[0].tickTocIOWatchDogTimeout, (int32_t) GLIOBoard[0].tickTocIOPendingTimeout);
        vDisplayMessage(msg);

        snprintf(msg, msg_size, "[IO/s:%d/%d/%d][UI/s:%d/%d/%d][SER/s:%d/%d/%d][CAN/s:%d/%d/%d]\n"
                , GLIOMinDataPerSec, GLIOLastDataPerSec, GLIOMaxDataPerSec
                , GLUIMinDataPerSec, GLUILastDataPerSec, GLUIMaxDataPerSec
                , GLSERMinDataPerSec, GLSERLastDataPerSec, GLSERMaxDataPerSec
                , GLCANMinDataPerSec, GLCANLastDataPerSec, GLCANMaxDataPerSec
                );
        vDisplayMessage(msg);

        snprintf(msg, msg_size, "[RTerr:%d][MaxCycle:%dms]\n", GLLogicErr, GLmaxtOut);
        vDisplayMessage(msg);

        snprintf(msg, msg_size, "[COMM run time:%d/%d/%d %d/%d/%d ms][COMM Max run time:%d/%d/%d %d/%d/%d ms]\n", App.UIMaxTime[1], App.UIMaxTime[2], App.UIMaxTime[3], App.UIMaxTime[4], App.UIMaxTime[5], App.UIMaxTime[6], App.UIMaxStatTime[1], App.UIMaxStatTime[2], App.UIMaxStatTime[3], App.UIMaxStatTime[4], App.UIMaxStatTime[5], App.UIMaxStatTime[6]);
        vDisplayMessage(msg);

        snprintf(msg, msg_size, "[ErrorList: %d/%d/%d]", machine.curAlarmList, machine.numAlarmList, machine.numAlarmListAllocated);
        vDisplayMessage(msg);

        snprintf(msg, msg_size, " - [lastAlarmListId/lastAlarmId: %d/%d]\n", machine.lastAlarmListId, machine.lastAlarmId);
        vDisplayMessage(msg);

    }
    return 1;
}









////////////////////////////////
// Mode & 1 ->  Debug Mode
//

int32_t dataExchangeInit(int32_t Mode) {
    char msg[256];
    size_t msg_size = sizeof (msg);


    // Compatibilita fra i due tipi
    if (sizeof (clockTicks_t) != sizeof (portTickType)) {
        fprintf(stderr, "[ERROR] Wrong structure size!\n");
        return -1;
    }

    if (sizeof (clockTicks_t) != 4) {
        fprintf(stderr, "[ERROR] Wrong structure size!\n");
        return -1;
    }

    if (sizeof (unsigned) != sizeof (uint32_t)) {
        fprintf(stderr, "[ERROR] Wrong (unsigned/unsigned int32_t)!\n");
        return -1;
    }

    if (sizeof (size_t) != sizeof (uint32_t)) {
        fprintf(stderr, "[ERROR] Wrong (size_t/unsigned int32_t)\n");
        return -1;
    }


    if (sizeof(EthAddr_t) != 6) {
        fprintf(stderr, "[ERROR] EthAddr_t size!\n" );
        return -1;
    }
    if (sizeof(IpAddr_t) != 4) {
        fprintf(stderr, "[ERROR] IpAddr_t size!\n" );
        return -1;
    }





    if (!App.UIRecvBuffer) {
        fprintf(stderr, "[ERROR] Failed to allocate %d bytes!\n", App.UIRecvBufferSize);
        return -1;
    }
    if (!App.UISendBuffer) {
        fprintf(stderr, "[ERROR] Failed to allocate %d bytes!\n", App.UISendBufferSize);
        return -1;
    }




    // Contatore tempo massimo IO
    GLMaxIOTime = 0;
    GLMStatIOTime = 0;



    ////////////////////////////
    // Scheda IO
    //

    GLNumIOBoardAllocated = (uint32_t) 0;
    GLIOBoard = (IOBoard *) NULL;





    ////////////////////////////
    // Scheda SCR
    //

    GLNumSCRBoardAllocated = (uint32_t) 0;
    GLSCRBoard = (SCRBoard *) NULL;




    ///////////////////////////////////
    // Porta UI (1..1024 = privileged)
    //
    GLListeningPort = 7362;


    ////////////////////////////
    // Porta Scheda IO
    //
    GLIOIpAddrPort = (uint32_t) 7373;



    ////////////////////////////
    // Porta Scheda SCR
    //
    GLSCRIpAddrPort = (uint32_t) 7371;


    ////////////////////////////
    // Porta Scheda CANBUS
    //
    GLCANIpAddrPort = (uint32_t) 7372;






    App.UIRecvBuffer[0] = '!';
    App.UIRecvBuffer[1] = 0;



    // Gateway
    App.Gateway[0] = 192;
    App.Gateway[1] = 168;
    App.Gateway[2] = 1;
    App.Gateway[3] = 1;

    // Local IP
    App.MyIpAddr[0] = 192;
    App.MyIpAddr[1] = 168;
    App.MyIpAddr[2] = 1;
    App.MyIpAddr[3] = CU_ADDR;

    // Subnet Mask
    App.Netmask[0] = 255;
    App.Netmask[1] = 255;
    App.Netmask[2] = 255;
    App.Netmask[3] = 0;






    // Initialize TCP/IP stack


    if (xrt_get_ip(&App.MyIpAddr) < 0) {
        fprintf(stderr, "%s*** [ERROR] IP failed! Check networkcard Installed...%s\n", ANSI_COLOR_RED, ANSI_COLOR_RESET);
        return -12;
    }

    if (xrt_get_mac_addr((char *) App.MacAddress) < 0) {
        fprintf(stderr, "%s*** [ERROR] MAC failed! Check networkcard Installed...%s\n", ANSI_COLOR_RED, ANSI_COLOR_RESET);
        return -12;
    }

    fprintf(stderr, "[CU at %s%x.%x.%x.%x.%x.%x - %d.%d.%d.%d%s]\n",
            ANSI_COLOR_CYAN,
            App.MacAddress[0], App.MacAddress[1], App.MacAddress[2], App.MacAddress[3], App.MacAddress[4], App.MacAddress[5],
            App.MyIpAddr[0], App.MyIpAddr[1], App.MyIpAddr[2], App.MyIpAddr[3],
            ANSI_COLOR_RESET
            );




#ifdef xBM_COMPILE

    fprintf(stderr, "   Reading %sxBM%s Config files :\n"
                    "   I/O    : %s%s%s  - IP from %d to %d\n",
            ANSI_COLOR_CYAN,
            ANSI_COLOR_RESET,
            ANSI_COLOR_BLUE,
            IO_BOARD_CFG_FILE,
            ANSI_COLOR_RESET, 
            START_IO_ADDR, END_IO_ADDR
            );

    fprintf(stderr, "   SCR    : %s%s%s - IP from %d to %d \n",
            ANSI_COLOR_BLUE,
            SCR_BOARD_CFG_FILE,
            ANSI_COLOR_RESET,
            START_SCR_ADDR, END_SCR_ADDR
            );

    fprintf(stderr, "   CANBUS : %s%s%s - IP from %d to %d \n",
            ANSI_COLOR_BLUE,
            CAN_BOARD_CFG_FILE,
            ANSI_COLOR_RESET,
            START_CAN_ADDR, END_CAN_ADDR
            );
    
    fprintf(stderr, "   SERIAL : %s%s%s \n",
            ANSI_COLOR_BLUE,
            SER_BOARD_CFG_FILE,
            ANSI_COLOR_RESET
            );

        

#elif xCNC_COMPILE

    fprintf(stderr, "Reading %sxCNC%s Config files :\n    I/O    :  %s%s%s - IP from %d to %d \n",
            ANSI_COLOR_CYAN,
            ANSI_COLOR_RESET,
            ANSI_COLOR_BLUE,
            IO_BOARD_CFG_FILE,
            ANSI_COLOR_RESET, 
            START_IO_ADDR, END_IO_ADDR
            );

    fprintf(stderr, "   CANBUS    : %s%s%s - IP from %d to %d \n",
            ANSI_COLOR_BLUE,
            CAN_BOARD_CFG_FILE,
            ANSI_COLOR_RESET,
            START_CAN_ADDR, END_CAN_ADDR
            );
    
    fprintf(stderr, "   SERIAL    : %s%s%s \n",
            ANSI_COLOR_BLUE,
            SER_BOARD_CFG_FILE,
            ANSI_COLOR_RESET
            );
    
#endif

    
    
    
    

    //////////////////////////////////////////////////////////////////////
    //
    // Inizializzazione periferiche I/O, SERIAL, CANBUS, SCR, USB ...
    //    

    while (!App.CANOK || !App.SEROK || !App.SCROK || !App.USBOK || !App.IOOK) {

        ////////////////////////////////////
        // Inizializza le schede CANOpen
        //
        if (!App.CANOK) {
            if (dataExchangeInitCAN(Mode) <= -999) {
                fprintf(stderr, "[ERROR] CAN init failed!\n");
                return -1;
            }
        }


        /////////////////////////////////////////
        // Inizializza i dispositivi seriali
        //
        if (!App.SEROK) {
            if (dataExchangeInitSerial(Mode) <= -999) {
                fprintf(stderr, "[ERROR] Serial init failed!\n");
                return -1;
            }
        }




        ////////////////////////////////////
        // Inizializza le schede di IO
        //
        if (!App.IOOK) {
            if (dataExchangeInitIO(Mode) <= -999) {
                fprintf(stderr, "[ERROR] IO init failed!\n");
                return -1;
            }
        }






        ////////////////////////////////////
        // Inizializza le schede SCR
        //
        if (!App.SCROK) {
            if (dataExchangeInitSCR(Mode) <= -999) {
                fprintf(stderr, "[ERROR] SCR init failed!\n");
                return -1;
            }
        }






        ////////////////////////////////////
        // Inizializza le schede USB
        //
        App.USBOK = true;
        /*
         * WORK IN PROGRESS
         * 
        if (dataExchangeInitUSB(Mode) <= 0) {
            fprintf(stderr, "[ERROR] USB init failed!\n");
        }
         */





        if (App.TerminateProgram) {
            App.TerminateProgram = false;
            break;
        }

        // Debug : termina la ricerca delle schede
        if (App.DebugMode) {
            if (App.MyIpAddr[3] != 200) {
                fprintf(stderr, "[INIT] Breaking boards init by IP Address:%d\n", App.MyIpAddr[3]);
                App.CANOK = INIT_AND_RUN;
                App.SEROK = INIT_AND_RUN;
                App.SCROK = INIT_AND_RUN;
                App.USBOK = INIT_AND_RUN;
                App.IOOK = INIT_AND_RUN;                
                break;
            }
        }


        if (!App.CANOK || !App.SEROK || !App.SCROK || !App.USBOK || !App.IOOK) {
            // Ritenta dopo attesa ...
            usleep(WAIT_BEFORE_REINIT_BOARDS_MSEC * 1000);
        }

        ////////////////////////////////
        // Lettura input tastiera
        //
        process_keyboard_input(msg, msg_size);

    }








#ifdef DEBUG_PRINTF
    snprintf(App.Msg, App.MsgSize, "\n[RTC] OK\n");
    vDisplayMessage(App.Msg);
#endif

    App.Initialized = true;

    return 1;
}




// Debug
// int32_t App.TrackPacketRecived = 0;

//
// Mode & 2 ->  Modalit� ritorno
// Mode & 1 ->  Modalit� debug
//

int32_t dataExchangeLoop(int32_t Mode) {
    uint32_t in = 0, r = 0, nSend = 0, nSent = 0, nReciv = 0;


    TickType_t xLastWakeTime = xTaskGetTickCount();

    clockTicks_t lastTimeout1 = xTaskGetTickCount(), lastTimeout2 = lastTimeout1;
    clockTicks_t td1 = 0, td2 = 0;

    uint32_t tickTimeout = (uint32_t) TIMER_MS_TO_TICKS((APP_TIMEOUT_SEC * 1000));
    uint32_t tickToc = (uint32_t) TIMER_MS_TO_TICKS((1000));
    uint32_t dtka = 0;

    // UI Keep alive
    uint32_t tickTocUIKeepAliveTimeout = (uint32_t) TIMER_MS_TO_TICKS((KEEPALIVE_TIMEOUT_MSEC));
    uint32_t tickTocUIKeepAlive = 0;



    char msg[256];
    size_t msg_size = sizeof (msg);



    SOCKET uiSocket = 0;
    SOCKET listeningSocket = 0;

    int32_t rc = 0;








    App.UICommState = STATE_INIT;


    App.RTCRunning = 1;
    App.UIRunLoop = 1;

    // tickToc = (uint32_t)(1000)/(1ul);


    ///////////////////////////////
    //
    // Avvio recezione pacchetti
    //
    // Buffer_startReceiving( );




    TickType_t xStatTime[16];


    while (App.UIRunLoop) {

        xStatTime[1] = xTaskGetTickCount();


        try {


            // N.B.: Piena potenza agli scudi anteriori
            // Wait for the next cycle.
            // vTaskDelayUntil( &xLastWakeTime, 1 );



            if (App.UICommState == STATE_CONNETED || App.UICommState == STATE_REPLYNG) {
                lastTimeout1 = xTaskGetTickCount();
            }




            ////////////////////////////////
            // Lettura input tastiera
            //
            process_keyboard_input(msg, msg_size);



            switch (App.UICommState) {



                case STATE_TESTING:
                    break;

                case STATE_INIT:

                    // if(Mode & 1) {
                    snprintf(msg, msg_size, "\n[RTC] %s%d.%d.%d.%d:%d%s - KA:%dmsec\n", ANSI_COLOR_BLUE, (int32_t) App.MyIpAddr[0], (int32_t) App.MyIpAddr[1], (int32_t) App.MyIpAddr[2], (int32_t) App.MyIpAddr[3], GLListeningPort, ANSI_COLOR_RESET, tickTocUIKeepAliveTimeout);
                    vDisplayMessage(msg);
                    // }

                    lastTimeout2 = lastTimeout1 = xTaskGetTickCount();

                    tickTocUIKeepAlive = 0;

                    if (!listeningSocket)
                        listeningSocket = xrt_socket();

                    if (!listeningSocket) {
                        fprintf(stderr, "[ERROR:!listeningSocket]\n");
                        usleep(1000 * 1000);
                    } else {

                        // lose the pesky "address already in use" error message
                        int32_t yes = 1;
                        if (setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &yes, sizeof (int32_t)) < 0) {
                            perror("!setsockopt:");
                        }

                        // Now bind the host address using bind() call
                        if (xrt_bind(listeningSocket, GLListeningPort) < 0) {
                            fprintf(stderr, "[ERROR:");
                            perror("!xrt_bind:");
                            fprintf(stderr, " - port:%d]\n", GLListeningPort);

                            usleep(3 * 1000 * 1000);
                            xrt_shutdown(&listeningSocket);
                            usleep(1 * 1000 * 1000);

                        } else {
                            // Now start listening for the clients, only one connection before refuse 
                            xrt_listen(listeningSocket, 1);

                            // App.UICommState = STATE_TESTING;
                            App.UICommState = STATE_LISTEN;
                        }
                    }

                    break;


                case STATE_LISTEN:
                    td1 = (uint32_t) xTaskGetTickCount() - lastTimeout1;
                    td2 = (uint32_t) xTaskGetTickCount() - lastTimeout2;

                    // printf("[%d] \r", Timer_diff( lastTimeout2, xTaskGetTickCount() );
                    // fflush(stdout);

                    // Check for client inactivity every xxx seconds                
                    if (td1 >= tickTimeout) {
                        App.UIIDLECounter++;
                        // Timeout
                        if (Mode & 1) {
                            if (td1 > tickTimeout) {
                                snprintf(msg, msg_size, "[RTC>%d]\n", (uint32_t) (td1 - tickTimeout));
                            } else {
                                snprintf(msg, msg_size, "[RTC]");
                            }
                            vDisplayMessage(msg);
                        }
                        lastTimeout1 = xTaskGetTickCount();
                    }

                    if (td2 >= tickToc) {
                        if (Mode & 1) {
                            // Debug Mode
                            snprintf(msg, msg_size, ".");
                            vDisplayMessage(msg);
                        }
                        // printf("[RTC] TimeDiff:%d] \n", (int32_t)Timer_diff( lastTimeout2, xTaskGetTickCount()) );
                        lastTimeout2 = xTaskGetTickCount();
                    }



                    fd_set read_fds, write_fds, except_fds;
                    struct timeval tv;

                    // N.B.: Un tempo eccessico penalizza gli I/O e
                    // tv.tv_usec = 500;
                    tv.tv_usec = 250;
                    tv.tv_sec = 0;

                    FD_ZERO(&read_fds); // clear the master and temp sets
                    FD_ZERO(&write_fds);
                    FD_ZERO(&except_fds);

                    FD_SET(listeningSocket, &read_fds);
                    FD_SET(listeningSocket, &write_fds);
                    FD_SET(listeningSocket, &except_fds);

                    if (xrt_select(listeningSocket + 1, &read_fds, &write_fds, &except_fds, &tv) > 0) {

                        if (FD_ISSET(listeningSocket, &read_fds) || FD_ISSET(listeningSocket, &write_fds) || FD_ISSET(listeningSocket, &except_fds)) {

                            // Accept actual connection from the client
                            struct sockaddr_in remote_addr = {0};
                            socklen_t remote_addr_len = sizeof (remote_addr);
                            uiSocket = xrt_accept(listeningSocket, (struct sockaddr*) &remote_addr, &remote_addr_len);

                            if (uiSocket) {
                                App.UICommState = STATE_CONNETED;



                                // Chiude il socket di ascolto
                                xrt_shutdown(&listeningSocket);

#ifdef DEBUG_PRINTF
#endif

                                App.UIConnectedPort = ntohs(remote_addr.sin_port);

                                inet_ntop(AF_INET, &remote_addr.sin_addr, App.UIConnectedIp, sizeof (App.UIConnectedIp));

                                snprintf(msg, msg_size, "[%sUI <-> %s:%u%s]\n", (char*) ANSI_COLOR_GREEN, App.UIConnectedIp, App.UIConnectedPort, (char*) ANSI_COLOR_RESET);
                                vDisplayMessage(msg);

                            } else {
                                // printf("."); fflush(stdout);
                                // printf("[%d].", Timer_diff( lastTimeout1, xTaskGetTickCount()); fflush(stdout);
                            }
                        }
                    }
                    break;



                case STATE_CONNETED:

                    // Recezione asincrona
                    nReciv = xrt_recv_async(uiSocket, &App.UIRecvBuffer[App.UIIndexRecvBuffer], (App.UIRecvBufferSize - App.UIIndexRecvBuffer));

                    if (nReciv < 0) {
                        App.UICommState = STATE_CLOSING;

                        sprintf(msg, "[INFO] recived data FAILED\n");
                        vDisplayMessage(msg);

                    } else if (nReciv == 0) {
                        ////////////////////////
                        // Keep Alive/watchDog
                        //
                        uint32_t tickTocUIKeepAliveCurrent = (uint32_t) xTaskGetTickCount();


                        if (tickTocUIKeepAlive == 0) {
                            // Avvio watchDog
                            tickTocUIKeepAlive = (uint32_t) xTaskGetTickCount();
                        } else {
                            // Controllo watchDog
                            if ((uint32_t) tickTocUIKeepAliveCurrent >= (uint32_t) tickTocUIKeepAlive) {
                                dtka = (uint32_t) tickTocUIKeepAliveCurrent - tickTocUIKeepAlive;
                                if (dtka > tickTocUIKeepAliveTimeout) {

                                    // Reset connessione
                                    App.UICommState = STATE_CLOSING;

                                    // Conteggio timeout
                                    machine.ui_timeout_count++;

                                    sprintf(msg, "[RTC] UI timeout [%d/%d]\n", (int32_t) dtka, (int32_t) tickTocUIKeepAliveTimeout);
                                    vDisplayMessage(msg);
                                }

                            } else {
                                // rotazione del contatore a 32bit...reset
                                snprintf(App.Msg, App.MsgSize, "[RTC] UI ROT [%u/%u]\n", tickTocUIKeepAliveCurrent, tickTocUIKeepAlive);
                                vDisplayMessage(App.Msg);
                                tickTocUIKeepAlive = tickTocUIKeepAliveCurrent;
                            }
                        }



                    } else if (nReciv > 0) {

                        // reset watchDog
                        tickTocUIKeepAlive = (uint32_t) xTaskGetTickCount();

                        GLUIDataPerSec++;

                        App.UIRecvBuffer[App.UIIndexRecvBuffer + nReciv] = 0;

                        // printf( "[RTC] recived data : %s - %u\n", (char*)App.UIRecvBuffer, nReciv);

                        /// sprintf(msg, "[KA:%d]", (int32_t)dtka);
                        /// vDisplayMessage(msg);


                        if (App.UIRecvBuffer[App.UIIndexRecvBuffer + 0] == '!') {
                            ////////////////////////////
                            // xProject direct command
                            //
                            // printf("HANDLE xProject command...\n");

                            // DEBUG 
#ifdef DISABLE_UI_RESPONDER
                            strcpy((char*) App.UISendBuffer, (char*) ".");
                            nSend = strlen((char*) App.UISendBuffer);
#else                                    
                            nSend = App.UISendBufferSize;
                            handle_xproject_command((uint8_t*) & App.UIRecvBuffer[1], nReciv - 1, App.UISendBuffer, &nSend);
#endif
                            if (nSend > 0) {
                                // printf( "REPLING xProject command %d bytes...\n", nSend);
                                // printf( "'%s'\n", App.UISendBuffer);
                                App.UICommState = STATE_REPLYNG;
                            }

                        } else if (strnicmp((char*) &App.UIRecvBuffer[App.UIIndexRecvBuffer], (char*) "GET ", 4) == 0/* || strstr(App.UIRecvBuffer, "GET ")*/) {
                            /////////////////////////////////////////////
                            // webSocket handshake
                            //
#ifdef DEBUG_PRINTF
                            snprintf(msg, msg_size, "HANDLE webSocket handshake...\n");
                            vDisplayMessage(msg);
#endif

                            nSend = App.UISendBufferSize;
                            if (handle_websocket_handshake((uint8_t*) & App.UIRecvBuffer[App.UIIndexRecvBuffer], (uint32_t) nReciv, (uint8_t *) App.UISendBuffer, (uint32_t *) & nSend) < 0) {
                                App.UIIndexRecvBuffer = 0;
                                App.UICommState = STATE_CLOSING;
                            }

#ifdef DEBUG_PRINTF
                            snprintf(msg, msg_size, "REPLING webSocket handhsake %d bytes...\n", nSend);
                            vDisplayMessage(msg);

                            printf("'%s'\n", App.UISendBuffer);
                            vDisplayMessage((char*) App.UISendBuffer);
#endif


                            App.UICommState = STATE_REPLYNG;


                        } else {
                            ///////////////////////////
                            // WebSocket protocol
                            //
                            App.UICommState = STATE_WEBSOCKET_PARSING;

                        }
                    }
                    break;


                    ////////////////////////////////////////
                    // Processo del protocollo websocket
                    //
                case STATE_WEBSOCKET_PARSING:
                    /// printf( "[INFO] parsing websocket %d/%d\n", App.UIIndexRecvBuffer, nReciv);
                    // Imposta la dimensione del buffer
                    nSend = App.UISendBufferSize;
                    App.SendWebSocketError = 0;

                    if (App.UIIndexWebSocketBuffer > App.UIRecvBufferSize) {
                        sprintf(msg, "[Internal error]\n");
                        vDisplayMessage(msg);
                        App.UICommState = STATE_CLOSING;
                        App.UIRecvBufferLastResp = -1;
                        App.UIIndexWebSocketBuffer = 0;
                        App.UIIndexRecvBuffer = 0;

                    } else {

                        switch (handle_websocket_data((uint8_t *) & App.UIRecvBuffer[App.UIIndexWebSocketBuffer], (uint32_t) (App.UIIndexRecvBuffer + nReciv), (uint32_t *) & App.UIIndexWebSocketBuffer, App.UISendBuffer, &nSend)) {
                            case -1: // Error
                                sprintf(msg, "[RTC ws error]\n");
                                vDisplayMessage(msg);
                                App.UICommState = STATE_CLOSING;
                                App.UIRecvBufferLastResp = -1;
                                App.UIIndexWebSocketBuffer = 0;
                                App.UIIndexRecvBuffer = 0;
                                break;
                            case 0:
                                // Done : Invia la risposta e termina il loop
                                // printf( "[INFO] websocket continue->STATE_REPLYNG:%dbytes\n", nSend);
                                App.UICommState = STATE_WEBSOCKET_SINGLE_REPLYNG;
                                App.UIRecvBufferLastResp = 0;
                                // N.B.: Invia la risposta e riparte il processo dati WebSocket da zezo
                                App.UIIndexWebSocketBuffer = 0;
                                App.UIIndexRecvBuffer = 0;
                                break;
                            case 1:
                                // Continue : Invia la risposta e continua il parsing senza leggere dal socket
                                // sprintf(msg, "[RTC feedback and continue from %d...]\n", App.UIIndexWebSocketBuffer); vDisplayMessage(msg);
                                App.UICommState = STATE_WEBSOCKET_REPLYNG;
                                App.UIRecvBufferLastResp = 1;
                                // N.B.: Continua il processo dati WebSocket dal punto assegnato
                                // App.UIIndexWebSocketBuffer;
                                break;
                            case 2: // Disconnect
                                // sprintf(msg, "[RTC ws disconnect]\n"); vDisplayMessage(msg);
                                App.UICommState = STATE_CLOSING;
                                App.UIRecvBufferLastResp = 2;
                                App.UIIndexWebSocketBuffer = 0;
                                break;
                            case 3: // Continua la lettura senza inviare la risposta (lettura parziale)
                                // sprintf(msg, "[RTC continue process from %d, read from %d...]\n", App.UIIndexWebSocketBuffer, App.UIIndexRecvBuffer+nReciv); vDisplayMessage(msg);
                                App.UICommState = STATE_CONNETED;
                                App.UIRecvBufferLastResp = 3;

                                // N.B.: Riparte il processo dati WebSocket da zezo e continua la lettura da App.UIIndexWebSocketBuffer
                                if (App.UIIndexRecvBuffer + nReciv > App.UIIndexWebSocketBuffer && App.UIIndexWebSocketBuffer) {
                                    uint32_t shiftLeftSize = App.UIIndexWebSocketBuffer;
                                    uint32_t unparsedDataSize = (App.UIIndexRecvBuffer + nReciv) - App.UIIndexWebSocketBuffer;
                                    sprintf(msg, "[RTC shifting %d data of %d bytes...]\n", unparsedDataSize, App.UIIndexWebSocketBuffer);
                                    memcpy((void*) App.UIRecvBuffer, (void*) &App.UIRecvBuffer[App.UIIndexWebSocketBuffer], unparsedDataSize);
                                    App.UIIndexRecvBuffer = unparsedDataSize;
                                    App.UIIndexWebSocketBuffer = 0;
                                } else {
                                    App.UIIndexRecvBuffer += nReciv;
                                    App.UIIndexWebSocketBuffer = App.UIIndexWebSocketBuffer;
                                }
                                break;
                        }

                    }

                    break;





                    ////////////////////////////////////////
                    // Header Risposta protocollo websocket
                    //
                case STATE_WEBSOCKET_REPLYNG:
                case STATE_WEBSOCKET_SINGLE_REPLYNG:
                {

                    App.webSocketHeaderSize = sizeof (App.webSocketHeader);


                    if (handle_websocket_response(App.UISendBuffer, nSend, App.webSocketHeader, &App.webSocketHeaderSize) >= 0) {
                        nSent = xrt_send(uiSocket, App.webSocketHeader, App.webSocketHeaderSize);
                        if (nSent != App.webSocketHeaderSize) {
                            // Riprova ad re-inviare
                            if (App.SendWebSocketError < MAX_SEND_ERROR) {
                                App.SendWebSocketError++;
                            } else {

                                sprintf(msg, "%s[INFO] sent webSocket header FAILED\n%s", (char*) ANSI_COLOR_RED, (char*) ANSI_COLOR_RESET);
                                vDisplayMessage(msg);

                                // App.UICommState = STATE_CLOSING;

                                if (App.UICommState == STATE_WEBSOCKET_REPLYNG) {
                                    App.UICommState = STATE_WEBSOCKET_PARSING;
                                } else {
                                    App.UIIndexRecvBuffer = 0;
                                    App.UICommState = STATE_CONNETED;
                                }
                            }
                        } else {
                            /// printf( "[INFO] sent webSocket data %s - %dbytes\n", App.UISendBuffer, nSend);
                            App.SendWebSocketError = 0;
                            if (App.UICommState == STATE_WEBSOCKET_REPLYNG) {
                                App.UICommState = STATE_WEBSOCKET_REPLYNG_DATA;
                            } else {
                                App.UICommState = STATE_WEBSOCKET_SINGLE_REPLYNG_DATA;
                            }
                        }
                    } else {
                        sprintf(msg, "[INFO] handle_websocket_response FAILED\n");
                        vDisplayMessage(msg);
                        App.UICommState = STATE_CLOSING;
                    }
                    break;
                }


                    ////////////////////////////////////////
                    // Body Risposta protocollo websocket
                    //
                case STATE_WEBSOCKET_REPLYNG_DATA:
                case STATE_WEBSOCKET_SINGLE_REPLYNG_DATA:
                {

                    nSent = xrt_send(uiSocket, App.UISendBuffer, nSend);
                    if (nSent != nSend) {
                        // Riprova ad re-inviare
                        if (App.SendWebSocketError < MAX_SEND_ERROR) {
                            App.SendWebSocketError++;
                        } else {
                            sprintf(msg, "[INFO] sent webSocket data FAILED\n");
                            vDisplayMessage(msg);
                            // App.UICommState = STATE_CLOSING;
                            if (App.UICommState == STATE_WEBSOCKET_REPLYNG_DATA) {
                                App.UICommState = STATE_WEBSOCKET_PARSING;
                            } else {
                                App.UIIndexRecvBuffer = 0;
                                App.UICommState = STATE_CONNETED;
                            }
                        }
                    } else {
                        /// printf( "[INFO] sent webSocket data %s - %dbytes\n", App.UISendBuffer, nSend);

                        if (App.UICommState == STATE_WEBSOCKET_REPLYNG_DATA) {
                            App.UICommState = STATE_WEBSOCKET_PARSING;

                            if (App.dumpWebSocket) {
                                vDisplayMessage("<");
                                vDisplayMessage((char*) App.UISendBuffer);
                                vDisplayMessage(">\n");
                                App.dumpWebSocket--;
                            }

                        } else {

                            if (App.dumpWebSocket) {
                                vDisplayMessage("(");
                                vDisplayMessage((char*) App.UISendBuffer);
                                vDisplayMessage(")\n");
                                App.dumpWebSocket--;
                            }

                            App.UIIndexRecvBuffer = 0;
                            App.UICommState = STATE_CONNETED;
                        }
                    }
                    break;
                }



                    ///////////////////////////////////
                    // Risposta diretta
                    //
                case STATE_REPLYNG:
                    if (App.UISendBuffer && nSend) {
                        /// printf( "[INFO] SENDING:%s\n", App.UISendBuffer);
                        nSent = xrt_send(uiSocket, App.UISendBuffer, nSend);
                        if (nSend != nSent) {
#ifdef DEBUG_PRINTF
                            sprintf(msg, "[INFO] send data FAILED\n");
                            vDisplayMessage(msg);
#endif
                            App.UICommState = STATE_CLOSING;
                        } else {
                            // printf( "[INFO] %d bytes sent\n", nSent);
                            App.UICommState = STATE_CONNETED;
                        }
                    } else {
                        App.UICommState = STATE_CONNETED;
                    }
                    break;




                case STATE_CLOSING:
                    xrt_shutdown(&uiSocket);
                    xrt_shutdown(&listeningSocket);
                    if (App.UIRunLoop) {
                        App.UICommState = STATE_INIT;
                    } else {
                        break;
                    }

                    break;

            }



            xStatTime[2] = xTaskGetTickCount();


            ////////////////////////////////////////
            // Comunicazione con le Schede di I/O
            //

            dataExchangeLoopIO();

            xStatTime[3] = xTaskGetTickCount();



            ////////////////////////////////////////
            // Comunicazione i dispositivi seriali
            //

            dataExchangeLoopSerial();

            xStatTime[4] = xTaskGetTickCount();



            ////////////////////////////////////////
            // Comunicazione i dispositivi CANOpen
            //

            dataExchangeLoopCAN();

            xStatTime[5] = xTaskGetTickCount();



            ////////////////////////////////////////
            // Comunicazione i dispositivi USB
            //

            dataExchangeLoopUSB();

            xStatTime[6] = xTaskGetTickCount();


            ////////////////////////////////////////
            // Comunicazione con le Schede SCR
            //

            dataExchangeLoopSCR();

            xStatTime[7] = xTaskGetTickCount();



            // Statistiche tempi esecuzione
            App.UIMaxTime[1] = xStatTime[2] - xStatTime[1];
            App.UIMaxTime[2] = xStatTime[3] - xStatTime[2];
            App.UIMaxTime[3] = xStatTime[4] - xStatTime[3];
            App.UIMaxTime[4] = xStatTime[5] - xStatTime[4];
            App.UIMaxTime[5] = xStatTime[6] - xStatTime[5];
            App.UIMaxTime[6] = xStatTime[7] - xStatTime[6];


            if (App.UIMaxTime[1] > App.UIMaxStatTime[1]) {
                App.UIMaxStatTime[1] = App.UIMaxTime[1];
            }
            if (App.UIMaxTime[2] > App.UIMaxStatTime[2]) {
                App.UIMaxStatTime[2] = App.UIMaxTime[2];
            }
            if (App.UIMaxTime[3] > App.UIMaxStatTime[3]) {
                App.UIMaxStatTime[3] = App.UIMaxTime[3];
            }
            if (App.UIMaxTime[4] > App.UIMaxStatTime[4]) {
                App.UIMaxStatTime[4] = App.UIMaxTime[4];
            }
            if (App.UIMaxTime[5] > App.UIMaxStatTime[5]) {
                App.UIMaxStatTime[5] = App.UIMaxTime[5];
            }
            if (App.UIMaxTime[6] > App.UIMaxStatTime[6]) {
                App.UIMaxStatTime[6] = App.UIMaxTime[6];
            }






            if (App.UIRunLoop <= 0) {
                if (uiSocket) {
                    xrt_shutdown(&uiSocket);
                }
                if (listeningSocket) {
                    xrt_shutdown(&listeningSocket);
                }
            }


        } catch (std::exception& e) {
            // std::cerr << "Exception catched : " << e.what() << std::endl;
            //////////////////////////////////////
            // Generazione Warning
            //
            char msg[512];
            snprintf(msg, sizeof (msg), "dataExchangeLoop() :  Exception : %s", e.what());
            if (generate_alarm((char*) msg, 8888, 0, (int32_t) ALARM_WARNING, 0 + 1) < 0) {
            }

        } catch (...) {
            // std::cerr << "Exception catched : " << e.what() << std::endl;
            //////////////////////////////////////
            // Generazione Warning
            //
            char msg[512];
            snprintf(msg, sizeof (msg), "dataExchangeLoop() :  Unk Exception");
            if (generate_alarm((char*) msg, 8888, 0, (int32_t) ALARM_WARNING, 0 + 1) < 0) {
            }
        }

        if (Mode & 2) {
            // Modalit� ritorno

            App.RTCRunning = 0;

            return 0;
        }
    }


    snprintf(msg, msg_size, "[RTC] Exit...\n");
    vDisplayMessage(msg);


    App.RTCRunning = 0;

    return 1;
}

int32_t dataExchangeStop(void) {

    dataEchangeStopIO();
    dataEchangeStopSCR();
    dataEchangeStopSerial();
    dataEchangeStopCAN();
    dataEchangeStopUSB();

    usleep(100);

    uint32_t xStatTime = xTaskGetTickCount();

    while (dataExchangeIsRunningIO() || dataExchangeIsRunningSCR() || dataExchangeIsRunningSerial()) {
        dataExchangeLoopIO();
        dataExchangeLoopSerial();
        dataExchangeLoopSCR();
        if (xTaskGetTickCount() - xStatTime > 3000) {
            break;
        }
    }

    App.UIRunLoop = 0;
    return 1;
}

int32_t dataExchangeReset(void) {

    App.UICommState = STATE_CLOSING;
    machine.ui_timeout_count = 0;
    machine.io_timeout_count = 0;

    GLIOMaxDataPerSec = 0;
    GLIOMinDataPerSec = NOT_SET_HIGH_VALUE;
    GLIODataPerSec = 0;

    GLUIMaxDataPerSec = 0;
    GLUIMinDataPerSec = NOT_SET_HIGH_VALUE;
    GLUIDataPerSec = 0;

    GLSERMaxDataPerSec = 0;
    GLSERMinDataPerSec = NOT_SET_HIGH_VALUE;
    GLSERDataPerSec = 0;

    return 1;
}

int32_t dataExchangeClose(void) {
    char msg[256];


    dataExchangeStop();


    // App.UICommState = STATE_CLOSING;
#ifdef DEBUG_PRINTF
    sprintf(msg, "[RTC] ending stack...\n");
    vDisplayMessage((char*) msg);
#endif

    // Utils::endStack( );

    // Packet_release_type();


    return 1;
}




#define CONSOLE_ROW_SIZE    1024

int32_t GLConsoleFD = 0;
int32_t GLConsoleBufferMaxLines = 64;
int32_t GLConsoleBufferNunLines = 0;
int32_t GLConsoleBufferCurLine = 0;
char **GLConsoleBuffer = (char **) calloc(sizeof (char*)*GLConsoleBufferMaxLines, 1);

int32_t GLConsoleBufferSerializedSize = (CONSOLE_ROW_SIZE + 256) * GLConsoleBufferMaxLines;
char *GLConsoleBufferSerialized = (char *) calloc(GLConsoleBufferSerializedSize, 1);

int32_t setupConsole() {
    struct termios options;

    if (GLConsoleBuffer) {

        for (int32_t i = 0; i < GLConsoleBufferMaxLines; i++) {
            GLConsoleBuffer[i] = (char*) calloc(CONSOLE_ROW_SIZE, 1);
        }

        return (0);

    } else {
        return (-1);
    }
}

int32_t addToConsole(char *Message) {
    if (Message) {
        if (Message[0]) {
            int32_t MessageLen = strlen(Message);
            if (Message[strlen(Message) - 1] == 10 || Message[strlen(Message) - 1] == 13) {
                if (GLConsoleBufferNunLines < GLConsoleBufferMaxLines) {
                    // riempimento buffer
                    strncat(GLConsoleBuffer[GLConsoleBufferNunLines], Message, CONSOLE_ROW_SIZE - strlen(GLConsoleBuffer[GLConsoleBufferNunLines]) - MessageLen);
                    GLConsoleBufferNunLines++;
                    // Reset riga a seguire
                    if (GLConsoleBufferNunLines < GLConsoleBufferMaxLines) {
                        GLConsoleBuffer[GLConsoleBufferNunLines][0] = 0;
                    } else {
                        GLConsoleBuffer[0][0] = 0;
                    }
                } else {
                    // rotazione buffer
                    strncat(GLConsoleBuffer[GLConsoleBufferCurLine], Message, CONSOLE_ROW_SIZE - strlen(GLConsoleBuffer[GLConsoleBufferCurLine]) - MessageLen);
                    GLConsoleBufferCurLine++;
                    // Reset riga a seguire
                    if (GLConsoleBufferCurLine >= GLConsoleBufferMaxLines)
                        GLConsoleBufferCurLine = 0;
                    GLConsoleBuffer[GLConsoleBufferCurLine][0] = 0;
                }
            } else {
                // Aggiunga a riga corrente
                if (GLConsoleBufferNunLines < GLConsoleBufferMaxLines) {
                    strncat(GLConsoleBuffer[GLConsoleBufferNunLines], Message, CONSOLE_ROW_SIZE - strlen(GLConsoleBuffer[GLConsoleBufferNunLines]) - MessageLen);
                } else {
                    strncat(GLConsoleBuffer[GLConsoleBufferCurLine], Message, CONSOLE_ROW_SIZE - strlen(GLConsoleBuffer[GLConsoleBufferCurLine]) - MessageLen);
                }
            }
        }
    }
    return 1;
}

char *readConsole() {
    int32_t nAdded = 0;

    GLConsoleBufferSerialized[0] = 0;

    for (int32_t i = GLConsoleBufferCurLine; i < GLConsoleBufferNunLines; i++) {
        if (GLConsoleBuffer[i]) {
            if (nAdded)
                strncat(GLConsoleBufferSerialized, "</br>", GLConsoleBufferSerializedSize - 5);
            strncat(GLConsoleBufferSerialized, GLConsoleBuffer[i], GLConsoleBufferSerializedSize - strlen(GLConsoleBufferSerialized));
            nAdded++;
        }
    }

    for (int32_t i = 0; i < GLConsoleBufferCurLine; i++) {
        if (GLConsoleBuffer[i]) {
            if (nAdded)
                strncat(GLConsoleBufferSerialized, "</br>", GLConsoleBufferSerializedSize - 5);
            strncat(GLConsoleBufferSerialized, GLConsoleBuffer[i], GLConsoleBufferSerializedSize - strlen(GLConsoleBufferSerialized));
            nAdded++;
        }
    }


    return (char*) GLConsoleBufferSerialized;
}


