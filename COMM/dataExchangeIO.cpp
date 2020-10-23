//============================================================================
// Name        : dataExchangeIO.cpp
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


#define READ_I2C_INTERVAL_MS    5000


static uint16_t GLRunLoop = 1;



int dataExchangeDumpIO(char *msg, size_t msg_size) {
    for (int32_t i = 0; i < App.NumIOBoard; i++) {
        snprintf(msg, msg_size, "[IO#%d - MAC:%02x.%02x.%02x.%02x.%02x.%02x - IP:%d.%d.%d.%d - tout:%d - state:%d]\n", (int) (i + 1)
                , (int) GLIOBoard[i].mac[0], (int) GLIOBoard[i].mac[1], (int) GLIOBoard[i].mac[2], (int) GLIOBoard[i].mac[3], (int) GLIOBoard[i].mac[4], (int) GLIOBoard[i].mac[5]
                , (int) GLIOBoard[i].ip[0], (int) GLIOBoard[i].ip[1], (int) GLIOBoard[i].ip[2], (int) GLIOBoard[i].ip[3]
                , (int) GLIOBoard[i].timeout_count
                , (int) GLIOBoard[i].commState
                // %Fp/%Fp/%Fp
                // ,(unsigned long)&GLIOBoard[i],(unsigned long)&GLIOBoard[i].tickTocIOWatchDogCurrent, (unsigned long)&GLIOBoard[i].tickTocIOWatchDog
                );
        vDisplayMessage(msg);
    }
    return 1;
}


int dataExchangeInitIO(int Mode) {
    int retVal = 1;
    char str[256];
    int rc = 0;
    
    int shouldUpdateIoCfg = 0;



    GLNumIOBoardAllocated = 16;
    App.NumIOBoard = 0;
    GLIOBoard = (IOBoard*) calloc((sizeof (IOBoard) * GLNumIOBoardAllocated) + 1, 1);

    if (!GLIOBoard) {
        GLNumIOBoardAllocated = 0;
        return -999;
    }

    App.numNewIO = 0;
    App.numUnresolvedIO = 0;
    App.numDuplicateIpIO  = 0;
        
    

    for (int i = 0; i < GLNumIOBoardAllocated; i++) {

        GLIOBoard[i].id = 0;
        GLIOBoard[i].Mode = Mode;
        GLIOBoard[i].port = GLIOIpAddrPort;
        // GLIOBoard[i].ip[0] = 192; GLIOBoard[i].ip[1] = 168; GLIOBoard[i].ip[2] = 1; GLIOBoard[i].ip[3] = ...;


        // IO KeepAlive
        GLIOBoard[i].tickTocIOKeepAliveTimeout = (uint32_t) TIMER_MS_TO_TICKS((IO_KEEPALIVE_TIMEOUT_MSEC));

        // IO WatchDOG
        GLIOBoard[i].tickTocIOWatchDogTimeout = (uint32_t) TIMER_MS_TO_TICKS((machine.io_timeout_ms > 0 ? machine.io_timeout_ms : IO_WATCHDOG_TIMEOUT_MSEC));

        // IO pending timer
        GLIOBoard[i].tickTocIOPendingTimeout = (uint32_t) TIMER_MS_TO_TICKS((IO_PENDING_TIMEOUT_MSEC));

        GLIOBoard[i].tickTocIOWatchDogCurrent = 0;
        GLIOBoard[i].tickTocIOPendingCounter = 0;

    }


    
    

    /////////////////////////////////////////
    // Lettura del file di configurazione
    //

    int curIOBoard = 0;

    int res_cfg = read_io_cfg();


    if (res_cfg > 0) {

        /////////////////////////////////////////////////////
        // Risoluzione dell'ip sulla base del mac address
        //

        snprintf(App.Msg, App.MsgSize, "[Reading %d IO cfg...", App.NumIOBoard);
        vDisplayMessage(App.Msg);

        App.numIOOK = 0;

        for (int j = START_IO_ADDR; j < END_IO_ADDR; j++) {
            IpAddr_t target_ip = {192, 168, 1, (uint8_t)j};
            EthAddr_t eth_dest = {0};
            int n_attemp = (j - START_IO_ADDR) < App.NumIOBoard ? 3 : 1;

            vDisplayMessage(".");

            // Richiesta mac addr da ip
            // rc = get_mac_addr_from_ip( target_ip, &eth_dest, 0 );

            for (int attemp = 0; attemp < n_attemp; attemp++) {

                rc = get_MAC_addr(target_ip, (uint16_t)7373, &eth_dest, 0);

                if (rc > 0) {

                    int res = 0;

                    if (eth_dest[0] == 0x73 && eth_dest[1] == 0x62) {
                        ///////////////////////
                        // Scheda di I/O
                        //

                        snprintf(App.Msg, App.MsgSize, "[IP:%s%d.%d.%d.%d%s"
                                , (char*) ANSI_COLOR_BLUE
                                , (int) target_ip[0], (int) target_ip[1], (int) target_ip[2], (int) target_ip[3]
                                , (char*) ANSI_COLOR_RESET
                                );
                        vDisplayMessage(App.Msg);

                        for (int i = 0; i < App.NumIOBoard; i++) {
                            IOBoard *pIOBoard = &GLIOBoard[i];

                            if (GLIOBoard[i].mac[0] == eth_dest[0] && GLIOBoard[i].mac[1] == eth_dest[1] && GLIOBoard[i].mac[2] == eth_dest[2] &&
                                    GLIOBoard[i].mac[3] == eth_dest[3] && GLIOBoard[i].mac[4] == eth_dest[4] && GLIOBoard[i].mac[5] == eth_dest[5]) {

                                // L'id � assegnato dalla lettura del file di IO
                                // GLIOBoard[i].id

                                memcpy(&GLIOBoard[i].ip, &target_ip, 4);
                                
                                GLIOBoard[i].commState = STATE_INIT;
                                
                                GLIOBoard[i].buf_size = sizeof (GLIOBoard[i].buf);
                                
                                App.numIOOK++;

                                res++;
                            }
                        }

                        if (res == 0) {
                            //////////////////////////////////////
                            /// Sostituzione o aggiunta scheda
                            //
                            snprintf(App.Msg, App.MsgSize, ":New IO Board...Please update %s]", IO_BOARD_CFG_FILE);
                            vDisplayMessage(App.Msg);

                            if (App.NumIOBoard < GLNumIOBoardAllocated) {

                                App.NumIOBoard++;

                                // Assegnamento ID
                                GLIOBoard[App.NumIOBoard - 1].id = App.NumIOBoard;

                                memcpy(&GLIOBoard[App.NumIOBoard - 1].mac, &eth_dest, 6);
                                memcpy(&GLIOBoard[App.NumIOBoard - 1].ip, &target_ip, 4);
                                
                                GLIOBoard[App.NumIOBoard - 1].commState  = STATE_INIT;

                                GLIOBoard[App.NumIOBoard - 1].buf_size = sizeof (GLIOBoard[App.NumIOBoard - 1].buf);

                                shouldUpdateIoCfg++;

                            }


                            ///////////////////////////////////////////////////////////////////////////
                            // N.B.: Impossibile avviare la logica : Campbio Delle schede di IO
                            //
                            App.numNewIO++;

                        } else if (res == 1) {
                            snprintf(App.Msg, App.MsgSize, ":%sOK%s].", (char*) ANSI_COLOR_GREEN, (char*) ANSI_COLOR_RESET);
                            vDisplayMessage(App.Msg);
                        } else {
                            snprintf(App.Msg, App.MsgSize, ":%sduplicated%s].", (char*) ANSI_COLOR_RED, (char*) ANSI_COLOR_RESET);
                            vDisplayMessage(App.Msg);
                        }

                        usleep(50 * 1000);

                    
                    } else if (eth_dest[0] == 0x73 && eth_dest[1] == 0x71) {
                        ///////////////////////////////
                        // Scheda SCR : Porta errata
                        //
                        snprintf(App.Msg, App.MsgSize, "[FILTERED OUT SCR IP:%d.%d.%d.%d - MAC:%02x.%02x.%02x.%02x.%02x.%02x]\n"
                                , (int) target_ip[0], (int) target_ip[1], (int) target_ip[2], (int) target_ip[3]
                                , (int) eth_dest[0], (int) eth_dest[1], (int) eth_dest[2], (int) eth_dest[3], (int) eth_dest[4], (int) eth_dest[5]
                                );
                        vDisplayMessage(App.Msg);
                        
                    } else if (eth_dest[0] == 0x73 && eth_dest[1] == 0x73) {
                        //////////////////////////////////
                        // Scheda CANBUS : porta errata
                        //
                        snprintf(App.Msg, App.MsgSize, "[FILTERED OUT CANBUS IP:%d.%d.%d.%d - MAC:%02x.%02x.%02x.%02x.%02x.%02x]\n"
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
            if (update_io_cfg() < 0) {
                snprintf(App.Msg, App.MsgSize, "[%sIO file %s updated%s]\n", (char*) ANSI_COLOR_GREEN, IO_BOARD_CFG_FILE, (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);
            }
        }


        // snprintf(App.Msg, App.MsgSize, "]\n");
        // vDisplayMessage(App.Msg);



    } else {

        ////////////////////////////////////////////////////////////
        // File non trovato : inizializzazione legengo gli ip
        //

        snprintf(App.Msg, App.MsgSize, "[Searching for IO on %s...\n", App.ETHInterfaceName);
        vDisplayMessage(App.Msg);


        // snprintf(App.Msg, App.MsgSize, "[Reading ARP...]\n");
        // vDisplayMessage(App.Msg);


        for (int i = START_IO_ADDR; i < END_IO_ADDR; i++) {

            // Richiesta mac addr
            IpAddr_t target_ip = {192, 168, 1, (uint8_t)i};
            EthAddr_t eth_dest = {0};

            // rc = get_mac_addr_from_ip( target_ip, &eth_dest, 1 );

            rc = get_MAC_addr(target_ip, (uint16_t)7373, &eth_dest, 0);

            if (rc > 0) {

                if (eth_dest[0] == 0x73 && eth_dest[1] == 0x62) {
                    ////////////////////
                    // Scheda di I/O
                    //

                    if (curIOBoard < GLNumIOBoardAllocated) {
                        IOBoard *pIOBoard = &GLIOBoard[curIOBoard];
                        curIOBoard++;

                        // Assegnamento ID
                        pIOBoard->id = curIOBoard;

                        memcpy(&pIOBoard->mac, &eth_dest, 6);
                        memcpy(&pIOBoard->ip, &target_ip, 4);
                        
                        pIOBoard->commState  = STATE_INIT;

                        pIOBoard->buf_size = sizeof (pIOBoard->buf);


                        snprintf(App.Msg, App.MsgSize, "[IO #%d > IP:%s%d.%d.%d.%d%s - MAC:%02x.%02x.%02x.%02x.%02x.%02x]\n"
                                , (int) pIOBoard->id
                                , (char*) ANSI_COLOR_BLUE
                                , (int) pIOBoard->ip[0], (int) pIOBoard->ip[1], (int) pIOBoard->ip[2], (int) pIOBoard->ip[3]
                                , (char*) ANSI_COLOR_RESET
                                , (int) pIOBoard->mac[0], (int) pIOBoard->mac[1], (int) pIOBoard->mac[2], (int) pIOBoard->mac[3], (int) pIOBoard->mac[4], (int) pIOBoard->mac[5]
                                );
                        vDisplayMessage(App.Msg);
                    } else {
                        snprintf(App.Msg, App.MsgSize, "[IO too many IO boards]\n");
                        vDisplayMessage(App.Msg);
                    }

                    usleep(50 * 1000);

                
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


            } else if (rc < 0) {
                snprintf(App.Msg, App.MsgSize, "[%sIO #%d > IP:%d.%d.%d.%d - mac error!%s]\n"
                        , (char*) ANSI_COLOR_RED
                        , (int) i
                        , (int) target_ip[0], (int) target_ip[1], (int) target_ip[2], (int) target_ip[3]
                        , (char*) ANSI_COLOR_RESET
                        );
                vDisplayMessage(App.Msg);
            }
        }


        App.NumIOBoard = curIOBoard;

        if (App.NumIOBoard) {
            if (update_io_cfg() > 0) {
                snprintf(App.Msg, App.MsgSize, "[%sIO file %s updated%s]\n", (char*) ANSI_COLOR_GREEN, IO_BOARD_CFG_FILE, (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);
            }
        }
        
        //////////////////////////////////////////////////////////////////////////////////
        // Nessuna scheda dichiarata : Inizializzazione OK, a cura dell primo avvio
        //
        App.IOOK = true;
    }






    ////////////////////////////
    // Verifiche finali
    //
    curIOBoard = 0;

    for (int i = 0; i < App.NumIOBoard; i++) {
        if (GLIOBoard[i].id > 0) {
            if (GLIOBoard[i].ip[0] > 0 && GLIOBoard[i].ip[1] > 0 && GLIOBoard[i].ip[2] > 0 && GLIOBoard[i].ip[3] > 0) {
                for (int j = 0; j < App.NumIOBoard; j++) {
                    IOBoard *pIOBoard2 = &GLIOBoard[j];
                    if (i != j) {
                        if (GLIOBoard[i].ip[0] == pIOBoard2->ip[0] && GLIOBoard[i].ip[1] == pIOBoard2->ip[1] && GLIOBoard[i].ip[2] == pIOBoard2->ip[2] && GLIOBoard[i].ip[3] == pIOBoard2->ip[3]) {
                            pIOBoard2->id = 0;
                            memset(&pIOBoard2->ip, 0, 4);
                            App.numDuplicateIpIO++;
                        }
                    }
                }

                curIOBoard = i + 1;
            } else {
                App.numUnresolvedIO++;
            }
        }
    }


    // Numero di Shede di IO
    App.NumIOBoard = curIOBoard;

    // Numero di Shede di IO lato Logica
    machine.numIOBoardSlots = (unsigned int) App.NumIOBoard;
    if (machine.numIOBoardSlots > machine.numIOBoardSlotsAllocated) {
        snprintf(App.Msg, App.MsgSize, "[%s Too many IO Board: %d/%d%s]\n", (char*) ANSI_COLOR_RED, (int) machine.numIOBoardSlots, (int) machine.numIOBoardSlotsAllocated, (char*) ANSI_COLOR_RESET);
        vDisplayMessage(App.Msg);
        machine.numIOBoardSlots = machine.numIOBoardSlotsAllocated;        
    }



    if (App.NumIOBoard > 0) {
        char *pANSI_COLOR = (char *) ((App.NumIOBoard > 0 && App.numNewIO == 0 && App.numUnresolvedIO == 0 && App.numDuplicateIpIO == 0) ? (ANSI_COLOR_GREEN) : (ANSI_COLOR_RED));
        snprintf(App.Msg, App.MsgSize, "[%s%d IO Board detected, %d new, %d unresolved, %d duplicated%s]\n", (char*) pANSI_COLOR, (int) App.NumIOBoard, (int) App.numNewIO, (int) App.numUnresolvedIO, (int) App.numDuplicateIpIO, (char*) ANSI_COLOR_RESET);
        vDisplayMessage(App.Msg);

        //////////////////////////////////////
        // Generazione Emergenza Macchina
        //
        if (App.numUnresolvedIO > 0 || App.numDuplicateIpIO > 0) {
            snprintf(App.Msg, App.MsgSize, "[%sIO Board error : %d unresolved, %d duplicated%s]\n", (char*) ANSI_COLOR_RED, App.numUnresolvedIO, App.numDuplicateIpIO, (char*) ANSI_COLOR_RESET);
            vDisplayMessage(App.Msg);
            retVal = -1;
            /*
            if (generate_alarm((char*) "IO Board NOT detected", 5002, 0, (int) ALARM_FATAL_ERROR) < 0) {
            }
            */
        } else {        
        }

    } else {
        //////////////////////////////////////
        // Generazione Emergenza Macchina
        //
        if (res_cfg > 0) {
            snprintf(App.Msg, App.MsgSize, "[%sNO IO Board detected%s]\n", (char*) ANSI_COLOR_RED, (char*) ANSI_COLOR_RESET);
            vDisplayMessage(App.Msg);
            retVal = -1;
        }
    }


    
    ///////////////////////////////
    // Lettura periodica I2C...
    //    
    for (int i = 0; i < App.NumIOBoard; i++) {
        if (GLIOBoard[i].id > 0) {
            GLIOBoard[i].Readi2cIntervalMs = 0;
        }
    }
    
    
    




    if (App.numNewIO > 0 || App.numUnresolvedIO > 0 || App.numDuplicateIpIO > 0) {
        if (App.numNewIO == 1 && App.numUnresolvedIO == 1 && App.numDuplicateIpIO == 0) {
            // sostituzione scheda IO
        } else {
            // Impossibile determinare lo slot delle schede : necessario riconfigurazione manuale
            retVal -1;
        }

    } else {
    }
    
    

    ///////////////////////////////
    // Registra la callback UDP
    //
    if (retVal > 0) {
        if (App.NumIOBoard) {
            if (xrt_callback_udp((uint16_t) GLIOIpAddrPort, &udpIOHandler) <= 0) {
                //////////////////////////////////////
                // Generazione Emergenza Macchina
                //
                if (generate_alarm((char*) "No IO UDP callback", 5010, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                }
                retVal = -1;
            } else {
                // snprintf(App.Msg, App.MsgSize, "[IO Callback Installed]\n"); vDisplayMessage(App.Msg);            
            }
        }
    }

    if (retVal > 0) {
        ////////////////////////////////////////////////////////
        // Tutte le schede identificate : Inizializzazione OK
        //
        App.IOOK = true;            
    }

    return retVal;    
}

int read_io_cfg() {
    int retVal = 0;
    uint8_t MacAddr[6];
    FILE *f = fopen(IO_BOARD_CFG_FILE, "r");

    App.NumIOBoard = 0;

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
                    if (boardId >= 1 && boardId <= GLNumIOBoardAllocated) {
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
                                    snprintf(App.Msg, App.MsgSize, "[CFG !mac value]\n");
                                    vDisplayMessage(App.Msg);
                                }
                            }
                        }

                        /////////////////////////////////////
                        // Assegnamento nella struttura
                        //
                        // IOBoard *pIOBoard = &GLIOBoard[boardId-1];

                        GLIOBoard[boardId - 1].id = boardId;
                        memcpy(&GLIOBoard[boardId - 1].mac, &MacAddr, 6);

                        /*
                        snprintf(App.Msg, App.MsgSize, "[CFG #%d : MAC:%02x.%02x.%02x.%02x.%02x.%02x]\n"
                                ,(int)GLIOBoard[i].id
                                ,(int)GLIOBoard[i].mac[0], (int)GLIOBoard[i].mac[1], (int)GLIOBoard[i].mac[2], (int)GLIOBoard[i].mac[3], (int)GLIOBoard[i].mac[4], (int)GLIOBoard[i].mac[5]
                                );
                        vDisplayMessage(App.Msg);
                         */

                        if (boardId > App.NumIOBoard) {
                            App.NumIOBoard = boardId;
                        }

                    } else {
                        // Numero scheda non valido
                        snprintf(App.Msg, App.MsgSize, "[CFG !board addr]\n");
                        vDisplayMessage(App.Msg);
                    }

                } else {
                    // Indirizzo board non valido
                    snprintf(App.Msg, App.MsgSize, "[CFG !board #]\n");
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



int update_io_cfg() {
    int retVal = 0;

    if (App.NumIOBoard) {
        char str[256];
        FILE *f = fopen(IO_BOARD_CFG_FILE, "w");
        if (f) {
            for (int i = 0; i < App.NumIOBoard; i++) {
                IOBoard *pIOBoard = &GLIOBoard[i];

                snprintf(str, sizeof (str), "#%d=%02x.%02x.%02x.%02x.%02x.%02x\n", GLIOBoard[i].id, GLIOBoard[i].mac[0], GLIOBoard[i].mac[1], GLIOBoard[i].mac[2], GLIOBoard[i].mac[3], GLIOBoard[i].mac[4], GLIOBoard[i].mac[5]);
                if (fputs(str, f) != 0) {
                    if (retVal == 0)
                        retVal++;
                } else {
                    retVal = -1;
                    snprintf(App.Msg, App.MsgSize, "[CFG update error]\n");
                    vDisplayMessage(App.Msg);
                }
            }

            fclose(f);
            f = NULL;

        } else {
            snprintf(App.Msg, App.MsgSize, "[CFG file not open]\n");
            vDisplayMessage(App.Msg);
            retVal = -1;
        }

    } else {
        snprintf(App.Msg, App.MsgSize, "[update_io_cfg : No IO boards]\n");
        vDisplayMessage(App.Msg);
        retVal = -1;
    }

    return retVal;
}

int dataEchangeResetIO() {
    snprintf(App.Msg, App.MsgSize, "\nDelete I/O file (Yes/No) ?\n");
    vDisplayMessage(App.Msg);
    if (getch() == 'Y') {
#ifdef WATCOM
        delete(IO_BOARD_CFG_FILE);
#else
        unlink(IO_BOARD_CFG_FILE);
#endif
        snprintf(App.Msg, App.MsgSize, "\n File %s deleted. Please Reboot...\n", IO_BOARD_CFG_FILE);
        vDisplayMessage(App.Msg);
        return 1;
    }
    return 0;
}


int dataEchangeStopIO() {
    GLRunLoop = 0;
}



unsigned char *GLIODataOut = (unsigned char *) NULL;
unsigned int GLNumIODataOutSize = 0;
unsigned int GLNumIODataOutAllocated = 0;
unsigned int GLNumIODataOut = 0;

int dataExchangeLoopIO() {
    int retVal = 0;
    uint32_t dtka = 0;
    int rc = 0, connectedCount = 0;
    char str[256];

    try {
        
        for (int32_t i_board = 0; i_board < App.NumIOBoard; i_board++) {
            IOBoard *pIOBoard = (IOBoard*)&GLIOBoard[i_board];

            if (pIOBoard->id) {
                
                if (!pIOBoard->disabled) {

                    switch (pIOBoard->commState) {


                    case STATE_INIT:
                        pIOBoard->commState = STATE_START;
                        pIOBoard->tickTocIOPendingCounter = 0;
                        pIOBoard->tickTocIOWatchDog = 0;
                        pIOBoard->tickTocIOKeepAlive = 0;
                        // Abilitazione IO
                        App.IOOK = INIT_AND_RUN;
                        break;


                    case STATE_START:
                        // N.B.: Utilizzo del protocollo UDP

                        pIOBoard->tickTocIOWatchDog = 0;
                        pIOBoard->tickTocIOKeepAlive = 0;

                        if (pIOBoard->ip[0] == 0 && pIOBoard->ip[1] == 0 && pIOBoard->ip[2] == 0 && pIOBoard->ip[3] == 0) {

#ifdef DEBUG_PRINTF
                            snprintf(App.Msg, App.MsgSize, "[IO] unresolved ip on #%d\n", (int) pIOBoard->id);
                            vDisplayMessage(App.Msg);
#endif
                            
                            pIOBoard->commState = STATE_PENDING;

                        } else {

                            rc = xrt_send_udp(pIOBoard->ip, (uint16_t) pIOBoard->port, (uint16_t) pIOBoard->port, (uint8_t *) "?IO", (uint16_t) 3, (uint8_t) 0);

                            if (rc == 3) {

                                pIOBoard->commState = STATE_WAITING_FOR_STREAM;

                                if (pIOBoard->Mode & 1) {
                                    snprintf(App.Msg, App.MsgSize, "[IO#%d] %d.%d.%d.%d:%u - WD:%dmsec\n", (int) pIOBoard->id, (int) pIOBoard->ip[0], (int) pIOBoard->ip[1], (int) pIOBoard->ip[2], (int) pIOBoard->ip[3], (int) pIOBoard->port, pIOBoard->tickTocIOWatchDogTimeout);
                                    vDisplayMessage(App.Msg);
                                }

                                ////////////////////////////////////////
                                // Verifica / Archivia il mac adress
                                //
                                EthAddr_t eth_dest = {0};

                                // rc = xrt_arp_resolve( pIOBoard->ip, (EthAddr_t *)&eth_dest );
                                // rc = get_mac_addr_from_ip ( pIOBoard->ip, (EthAddr_t *)&eth_dest, 0 );
                                rc = 1;

                                if (rc > 0) {

                                    if (memcmp(&pIOBoard->mac, &eth_dest, 6) != 0) {

                                        /*
                                         * vDisplayMessage(App.Msg);
                                        snprintf(App.Msg, App.MsgSize, "[IO Board #%d changed...]\n", (int)pIOBoard->id);
                                         * */

                                        /*
                                        snprintf(App.Msg, App.MsgSize, "[MAC:%02x.%02x.%02x.%02x.%02x.%02x -> %02x.%02x.%02x.%02x.%02x.%02x]\n"
                                                ,(int)pIOBoard->mac[0], (int)pIOBoard->mac[1], (int)pIOBoard->mac[2], (int)pIOBoard->mac[3], (int)pIOBoard->mac[4], (int)pIOBoard->mac[5]
                                                ,(int)Arp::arpTable[rc].ethAddr[0],(int)Arp::arpTable[rc].ethAddr[1],(int)Arp::arpTable[rc].ethAddr[2],(int)Arp::arpTable[rc].ethAddr[3],(int)Arp::arpTable[rc].ethAddr[4],(int)Arp::arpTable[rc].ethAddr[5]
                                                );
                                        vDisplayMessage(App.Msg);
                                         */

                                        /*
                                         * memcpy(&pIOBoard->mac, &eth_dest, 6);
                                         * */

                                    } else {
                                        // OK : max confermato
                                        // snprintf(App.Msg, App.MsgSize, "[IO #%d MAC ok]\n", pIOBoard->id);
                                        // vDisplayMessage(App.Msg);
                                    }

                                } else {
                                    // should never appens
                                    pIOBoard->commState = STATE_PENDING;
                                    snprintf(App.Msg, App.MsgSize, "[!ARP]\n");
                                    vDisplayMessage(App.Msg);
                                }



                            } else {

#ifdef DEBUG
                                perror("xrt_send_udp():");
#endif

                                if (pIOBoard->Mode & 1) {
                                    if (pIOBoard->tickTocIOPendingCounter > 2) {
                                        snprintf(App.Msg, App.MsgSize, "[I/O %d waiting]\n", pIOBoard->id);
                                        vDisplayMessage(App.Msg);
                                    }
                                }

                                // pIOBoard->IOCommState = STATE_CLOSING;
                                pIOBoard->commState = STATE_PENDING;

                                pIOBoard->tickTocIOWatchDog = 0;
                                pIOBoard->tickTocIOKeepAlive = 0;
                            }
                        }
                        
                        if (!GLRunLoop) {
                            pIOBoard->commState = STATE_CLOSING;
                        }
                        break;



                    case STATE_PENDING:
                        pIOBoard->tickTocIOWatchDogCurrent = (uint32_t) xTaskGetTickCount();

                        if (pIOBoard->tickTocIOWatchDog == 0) {
                            // Avvio temporizzatore attesa ARP
                            pIOBoard->tickTocIOWatchDog = (uint32_t) xTaskGetTickCount();
                        } else {
                            if ((uint32_t) pIOBoard->tickTocIOWatchDogCurrent - pIOBoard->tickTocIOWatchDog > pIOBoard->tickTocIOPendingTimeout) {
                                // Ricollegamento dopo risoluzione ARP
                                pIOBoard->commState = STATE_START;
                                pIOBoard->tickTocIOPendingCounter++;
                            }
                        }
                        if (!GLRunLoop) {
                            pIOBoard->commState = STATE_CLOSING;
                        }
                        break;




                    case STATE_WAITING_FOR_STREAM:
                        /////////////////////////////////////////////////////
                        // Attesa della recezione dati
                        //

                        // Lettura dati dallo stream udp
                        xrt_recive_udp(pIOBoard->ip, pIOBoard->port, pIOBoard->port, pIOBoard->buf, pIOBoard->buf_size, 0);

                        //
                        pIOBoard->tickTocIOWatchDogCurrent = (uint32_t) xTaskGetTickCount();


                        if (pIOBoard->tickTocIOKeepAlive == 0) {
                            // Avvio watchDog
                            pIOBoard->tickTocIOKeepAlive = (uint32_t) xTaskGetTickCount();
                        } else {
                            dtka = (uint32_t) pIOBoard->tickTocIOWatchDogCurrent - pIOBoard->tickTocIOKeepAlive;
                            if (dtka > pIOBoard->tickTocIOPendingTimeout) {
                                ////////////////////////////////////////////////////////////////
                                //
                                // Ivio del comando per ristabilire la connessione ... che non arriva
                                //
                                pIOBoard->commState = STATE_START;
                                pIOBoard->tickTocIOKeepAlive = 0;
                            }
                        }
                        
                        if (!GLRunLoop) {
                            pIOBoard->commState = STATE_CLOSING;
                        }
                        
                        break;



                    case STATE_CONNETED:
                        ///////////////////////////////////////////////////////
                        // Comunicazione con gli I/O 
                        //
                        
                        connectedCount++;
                        
                        // Lettura dati dallo stream udp
                        if (xTaskGetTickCount() > pIOBoard->tickTocIOWatchDogCurrent) {
                            
                            rc = xrt_recive_udp(pIOBoard->ip, pIOBoard->port, pIOBoard->port, pIOBoard->buf, pIOBoard->buf_size, 0);


                            //
                            // watchDog IO                        
                            //
                            pIOBoard->tickTocIOWatchDogCurrent = (uint32_t) xTaskGetTickCount();


                            if (pIOBoard->tickTocIOWatchDog == 0) {
                                // Avvio watchDog
                                pIOBoard->tickTocIOWatchDog = xTaskGetTickCount(); // (uint32_t)xTaskGetTickCountFromISR();


                            } else {
                                // Controllo watchDog
                                if ((uint32_t) pIOBoard->tickTocIOWatchDogCurrent >= (uint32_t) pIOBoard->tickTocIOWatchDog) {
                                    dtka = (uint32_t) pIOBoard->tickTocIOWatchDogCurrent - pIOBoard->tickTocIOWatchDog;
                                    if (dtka > pIOBoard->tickTocIOWatchDogTimeout) {

                                        if (pIOBoard->tickTocIOWatchDogCurrent - pIOBoard->tickTocIOWatchDog != dtka) {
                                            snprintf(App.Msg, App.MsgSize, "[timer error]\n");
                                            vDisplayMessage(App.Msg);
                                        }

                                        // Reset connessione
                                        pIOBoard->commState = STATE_CLOSING;

                                        // Conteggio timeout
                                        machine.io_timeout_count++;
                                        pIOBoard->timeout_count++;

                                        snprintf(str, sizeof(str), "[RTC] IO#%d Timeout [%u/%u]", (int)pIOBoard->id, (uint32_t) dtka, (uint32_t) pIOBoard->tickTocIOWatchDogTimeout);

                                        //////////////////////////////////////
                                        // Generazione Emergenza Macchina
                                        //
                                        if (generate_alarm((char*) str, 5003, pIOBoard->id, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                                        }

                                    } else {
                                        if (dtka > GLMaxIOTime) {
                                            GLMaxIOTime = dtka;
                                            snprintf(App.Msg, App.MsgSize, "[RTC IO>%dms]", (uint32_t) dtka);
                                            vDisplayMessage(App.Msg);
                                        }
                                        // Statistica ciclo lettura IO
                                        if (dtka > GLMStatIOTime) {
                                            GLMStatIOTime = dtka;
                                        }
                                    }
                                } else {
                                    // rotazione del contatore a 32bit...reset
                                    snprintf(App.Msg, App.MsgSize, "[RTC] IO#%d ROT [%u/%u]\n"
                                            , (i_board + 1)
                                            , pIOBoard->tickTocIOWatchDogCurrent, pIOBoard->tickTocIOWatchDog
                                            );
                                    vDisplayMessage(App.Msg);
                                    pIOBoard->tickTocIOWatchDog = pIOBoard->tickTocIOWatchDogCurrent;

                                    ////////////////////////////////////////
                                    // Registra la rotazione del contatore
                                    //
                                    if (generate_alarm((char*) "IO Counter rotate", 2001, pIOBoard->id, ALARM_LOG, 0+1) < 0) {
                                    }
                                }
                            }



                            ///////////////////////////////////////////////////
                            // Invio delle UScite Digitali/ Analogiche
                            //
                            //
                            ///////////////////////////////////////////////////////////////////////////////////
                            // Preparazione stringa da inviare sulla base delle variazioni degli stati
                            //
                            // comando !
                            // D = uscita digitale...1 char per il valore   0..1
                            // A = uscita analogica..2 char per il valore   0..65535
                            //

                            for (uint32_t i = 0; i < machine.numIOBoardSlots; i++) {

                                if (machine.ioBoardSlots[i].id == pIOBoard->id) {

                                    uint32_t cODataOut = (uint32_t) 0;
                                    uint32_t numDigitOut = (uint32_t) 0;
                                    uint32_t numAnalogOut = (uint32_t) 0;

                                    machine.ioBoardSlots[i].NumIODataOut = 0;
                                    machine.ioBoardSlots[i].IODataOut[0] = '!';
                                    machine.ioBoardSlots[i].IODataOut[1] = 0; // Num Digital Output
                                    machine.ioBoardSlots[i].IODataOut[2] = 0; // Num Analog Output
                                    cODataOut = 3;

                                    if (sizeof(machine.ioBoardSlots[i].IODataOut) < machine.ioBoardSlots[i].numDigitalOUT + machine.ioBoardSlots[i].numAnalogOUT*2+3+1) {
                                        vDisplayMessage("[!IO MEMORY BUFFER]");
                                    } else {

                                        for (uint32_t j = 0; j < machine.ioBoardSlots[i].numDigitalOUT; j++) {
                                            if (machine.ioBoardSlots[i].digitalOUT[j] != machine.ioBoardSlots[i].digitalOUTBK[j]) {
                                                machine.ioBoardSlots[i].digitalOUTBK[j] = machine.ioBoardSlots[i].digitalOUT[j];

                                                // GLIODataOut[cODataOut] = 'D';
                                                // cODataOut++;

                                                machine.ioBoardSlots[i].IODataOut[cODataOut] = (unsigned char) j;
                                                cODataOut++;

                                                machine.ioBoardSlots[i].IODataOut[cODataOut] = (unsigned char) machine.ioBoardSlots[i].digitalOUT[j];
                                                cODataOut++;

                                                numDigitOut++;
                                                machine.ioBoardSlots[i].NumIODataOut++;
                                                
                                                // Auto trigger OFF
                                                if (machine.ioBoardSlots[i].digitalOUT[j] > 1 && machine.ioBoardSlots[i].digitalOUT[j] <= 121) {
                                                    // N.B.: La scheda spegne l'uscita automaticamente
                                                    machine.ioBoardSlots[i].digitalOUT[j] = 0;
                                                    machine.ioBoardSlots[i].digitalOUTBK[j] = 0;
                                                } else if (machine.ioBoardSlots[i].digitalOUT[j] >= 122 && machine.ioBoardSlots[i].digitalOUT[j] <= 242) {
                                                    // N.B.: La scheda accende l'uscita automaticamente
                                                    machine.ioBoardSlots[i].digitalOUT[j] = 1;
                                                    machine.ioBoardSlots[i].digitalOUTBK[j] = 1;                                                    
                                                }
                                            }
                                        }
                                        
                                        // Number of Digital Output
                                        machine.ioBoardSlots[i].IODataOut[1] = (unsigned char) numDigitOut;

                                        for (uint32_t j = 0; j < machine.ioBoardSlots[i].numAnalogOUT; j++) {
                                            if (machine.ioBoardSlots[i].analogOUT[j] != machine.ioBoardSlots[i].analogOUTBK[j]) {
                                                machine.ioBoardSlots[i].analogOUTBK[j] = machine.ioBoardSlots[i].analogOUT[j];

                                                // GLIODataOut[cODataOut] = 'A';
                                                // cODataOut++;

                                                machine.ioBoardSlots[i].IODataOut[cODataOut] = (unsigned char) j;
                                                cODataOut++;

                                                memcpy(&machine.ioBoardSlots[i].IODataOut[cODataOut], &machine.ioBoardSlots[i].analogOUT[j], 2);
                                                cODataOut += 2;

                                                numAnalogOut++;
                                                machine.ioBoardSlots[i].NumIODataOut++;
                                            }
                                        }
                                        
                                        // Number of AnalogIO Output
                                        machine.ioBoardSlots[i].IODataOut[2] = (unsigned char) numAnalogOut; 

                                        if (machine.ioBoardSlots[i].NumIODataOut) {
                                            int sent = xrt_send_udp(pIOBoard->ip, (uint16_t) pIOBoard->port, (uint16_t) pIOBoard->port, (uint8_t *) machine.ioBoardSlots[i].IODataOut, (uint16_t) cODataOut, (uint8_t) 0);
                                            if (sent != cODataOut) {
                                                vDisplayMessage("[!IO SEND FAILED]");
                                            } else {
                                                // vDisplayMessage("[Sent IO UPD]");
                                            }
                                        }
                                    }
                                }
                            }





                            
                            

                            ///////////////////////////////////////////////////
                            // Invio richiesta di lettura i2c
                            //
                            // Uso : ['I'][Addr dispositivo]...
                            //
                            // test : usa il tribber del timeout
                            //
                            if (pIOBoard->Readi2cIntervalMs > 0) {
                                if (xTaskGetTickCount() - pIOBoard->tickTocReadi2c >= pIOBoard->Readi2cIntervalMs) {
                                    int send = xrt_send_udp(pIOBoard->ip, (uint16_t) pIOBoard->port, (uint16_t) pIOBoard->port, (uint8_t *) "I", (uint16_t) 1, (uint8_t) 0);
                                    
                                    pIOBoard->tickTocReadi2c = xTaskGetTickCount();
                                    
                                    if (send == 1) {
                                        // vDisplayMessage("[IO-I2C read]");
                                    } else {
                                        vDisplayMessage("[!IO-I2C read]");
                                    }
                                }
                            }
                            

                            //////////////////////////////////
                            // Segnale Keep alive IO
                            //
                            if (pIOBoard->tickTocIOKeepAlive == 0) {
                                // Avvio watchDog
                                pIOBoard->tickTocIOKeepAlive = (uint32_t) xTaskGetTickCount();
                            } else {
                                dtka = (uint32_t) pIOBoard->tickTocIOWatchDogCurrent - pIOBoard->tickTocIOKeepAlive;
                                if (dtka > pIOBoard->tickTocIOKeepAliveTimeout) {
                                    // Ivio del comando per tenere alta la connessione
                                    int send = xrt_send_udp(pIOBoard->ip, (uint16_t) pIOBoard->port, (uint16_t) pIOBoard->port, (uint8_t *) ".", (uint16_t) 1, (uint8_t) 0);
                                    if (send != 1) {
                                        snprintf(App.Msg, App.MsgSize, "%s[!KA]%s", (char*) ANSI_COLOR_RED, (char*) ANSI_COLOR_RESET);
                                        vDisplayMessage(App.Msg);
                                    } else {
                                        // vDisplayMessage("[KA]");
                                    }
                                    pIOBoard->tickTocIOKeepAlive = 0;
                                }
                            }
                        }
                        
                        if (!GLRunLoop) {
                            pIOBoard->commState = STATE_CLOSING;
                        }
                        break;


                    case STATE_CLOSING:
                        if (GLRunLoop) {
                            pIOBoard->commState = STATE_START;
                        } else {
                            pIOBoard->commState = STATE_CLOSED;
                            snprintf(App.Msg, App.MsgSize, "[RTC] IO#%d Stopped\n", (i_board+1));
                            vDisplayMessage(App.Msg);
                        }
                        break;
                        
                    case STATE_CLOSED:
                        break;

                }
                
                }
                
            } else {
                // snprintf(App.Msg, App.MsgSize, "[IO board %d invalid]\n", (int)i_board);
                // vDisplayMessage(App.Msg);
            }
        }

        
        
    } catch (int) {
    }
    
    
    if (retVal < 0) {
        App.IODetectError++;
    }
    

    return retVal;
}




int dataExchangeIsRunningIO() {
    for (uint32_t i = 0; i < App.NumIOBoard; i++) {
        if (GLIOBoard[i].id) {
            if (GLIOBoard[i].commState != STATE_CLOSED) {
                return 1;
            }
        }
    }
    return 0;
}





/////////////////////////////////////////////
// callback pacchetto UDP (schede I/O)
//

void udpIOHandler(IpAddr_t ip, uint16_t sport, uint16_t dport, int8_t *buf, uint16_t buf_size) {

    if (buf) {

        if (buf_size) {
            uint16_t udpLen = xrt_ntohs((uint16_t) buf_size);

            // IpHeader *ip = (IpHeader *)(packet + sizeof(EthHeader));
            // UdpHeader *udp2 = (UdpHeader *)(ip->payloadPtr());
            // uint8_t *pData = (uint8_t *)(udp + sizeof(UdpHeader));

            uint8_t *pData = (uint8_t *) (buf);
            uint8_t IOBoardFound = 0;

            GLIODataPerSec++;

            for (int32_t i_board = 0; i_board < App.NumIOBoard; i_board++) {
                IOBoard *pIOBoard = (IOBoard*)&GLIOBoard[i_board];


                if (pIOBoard->id) {

                    if (pIOBoard->ip[0] == ip[0] && pIOBoard->ip[1] == ip[1] &&
                            pIOBoard->ip[2] == ip[2] && pIOBoard->ip[3] == ip[3]) {

                        // Reset watch dog
                        // pIOBoard->tickTocIOWatchDog = 0;
                        pIOBoard->tickTocIOWatchDog = (uint32_t) xTaskGetTickCount();


                        //////////////  //////////////////////////////////
                        // Avvio dello stream -> cambio di stato
                        //
                        if (pIOBoard->commState == STATE_WAITING_FOR_STREAM) {
                            pIOBoard->commState = STATE_CONNETED;
                            snprintf(App.Msg, App.MsgSize, "[%sIO #%d <-> %d.%d.%d.%d%s]\n", (char*) ANSI_COLOR_GREEN, pIOBoard->id, pIOBoard->ip[0], pIOBoard->ip[1], pIOBoard->ip[2], pIOBoard->ip[3], (char*) ANSI_COLOR_RESET);
                            vDisplayMessage(App.Msg);
                        }


                        unsigned int nDigitIO = (unsigned int) pData[0]; // udpLen / 2;
                        unsigned int nAnalogIO = (unsigned int) pData[1];
                        unsigned char ioAddr = 0, dVal = 0;
                        unsigned int aVal = 0;

                        pData += 2;

                        IOBoardFound++;


                        if ((int) pIOBoard->id <= (int) machine.numIOBoardSlots && pIOBoard->id > 0) {
                            int32_t iBoard = pIOBoard->id - 1;
        
                            if (nDigitIO || nAnalogIO) {
                                // snprintf(App.Msg, App.MsgSize, "[nDigitIO:%d - nAnalogIO:%d]\n", nDigitIO, nAnalogIO);
                                // vDisplayMessage(App.Msg);
                            }

                            ////////////////////////////////////////////////////////////////////////////////////////
                            //
                            // N.B.: Identificazione del messaggio sulla base di valori chiave del N° di Ingress
                            //
                            
                            if ( (nDigitIO == -1 || nDigitIO == 255) && (nAnalogIO == -1 || nAnalogIO == 255) ) {
                                //////////////////////////////////
                                // Dispositivo su Indirizzo i2c
                                //
                                long int tAmbient = 0, tObject = 0;
                                
                                memcpy (&tAmbient, &pData[0], sizeof(long int));
                                pData += sizeof(long int);

                                memcpy (&tObject, &pData[0], sizeof(long int));
                                pData += sizeof(long int);
                                
                                // ...
                                
                                GLIOBoard[iBoard].i2cValues[0] = tAmbient;
                                GLIOBoard[iBoard].i2cValues[1] = tObject;
                                GLIOBoard[iBoard].i2cNumValues = 2;
                                
                                // snprintf(App.Msg, App.MsgSize, "[IO#%d - tAmbient:%0.3f - tObject:%0.3f]\n", pIOBoard->id, (float)tAmbient / 1000.0f, (float)tObject / 1000.0f);
                                // vDisplayMessage(App.Msg);

                                
                            } else {
                            
                                //////////////////////////////////
                                // Ingressi digitali (1byte)
                                //

                                for (int iIO = 0; iIO < nDigitIO; iIO++) {

                                    ioAddr = pData[0];
                                    pData++;
                                    dVal = pData[0];
                                    pData++;

                                    if ((int) ioAddr < machine.ioBoardSlots[iBoard].numDigitalIN) {
                                        machine.ioBoardSlots[iBoard].digitalIN[ioAddr] = (char) dVal;

                                        //    snprintf(App.Msg, App.MsgSize, "[DIG IN%d:%d]", (int)ioAddr, (int)dVal);
                                        //    vDisplayMessage(App.Msg);

                                    } else {
                                        // snprintf(App.Msg, App.MsgSize, "[!ioAddr:%d]", (int)ioAddr);
                                        // vDisplayMessage(App.Msg);
                                    }
                                }


                                //////////////////////////////////
                                // Ingressi Analogici (2byte)
                                //
                                for (int iIO = 0; iIO < nAnalogIO; iIO++) {

                                    ioAddr = pData[0];
                                    pData++;
                                    memcpy((void*) &aVal, (void*) &pData[0], 2);
                                    pData += 2;

                                    if ((int) ioAddr < machine.ioBoardSlots[iBoard].numDigitalIN) {
                                        machine.ioBoardSlots[iBoard].analogIN[ioAddr] = aVal;
                                        // snprintf(App.Msg, App.MsgSize, "[%d:%d]", (int)ioAddr, (int)aVal);
                                        // vDisplayMessage(App.Msg);
                                    } else {
                                        // snprintf(App.Msg, App.MsgSize, "[!ioAddr:%d]", (int)ioAddr);
                                        // vDisplayMessage(App.Msg);
                                    }
                                }
                            }
                            
                        } else {
                            snprintf(App.Msg, App.MsgSize, "[!IO index:%d/%d]\n", (int) pIOBoard->id, (int) machine.numIOBoardSlots);
                            vDisplayMessage(App.Msg);
                        }
                    }
                }
            }

            if (!IOBoardFound) {
                snprintf(App.Msg, App.MsgSize, "[!IOxIP:%d.%d.%d.%d]\n", (int) ip[0], (int) ip[1], (int) ip[2], (int) ip[3]);
                vDisplayMessage(App.Msg);

                for (int32_t i_board = 0; i_board < App.NumIOBoard; i_board++) {
                    IOBoard *pIOBoard = (IOBoard*)&GLIOBoard[i_board];
                    snprintf(App.Msg, App.MsgSize, "[IO%d - ID:%d - IP:%d.%d.%d.%d]\n", (int)(i_board + 1), pIOBoard->id, pIOBoard->ip[0], pIOBoard->ip[1], pIOBoard->ip[2], pIOBoard->ip[3]);
                    vDisplayMessage(App.Msg);
                }

                //////////////////////////////////////
                // Generazione Emergenza Macchina
                //
                if (generate_alarm((char*) "IO Communication board not found", 5004, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
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
