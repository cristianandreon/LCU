//============================================================================
// Name        : dataExchangeSCR.cpp
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
#include <fenv.h>


#define READ_I2C_INTERVAL_MS        100
#define READ_SCR_INTERVAL_MS        5000

static uint16_t GLRunLoop = 1;

int dataExchangeDumpSCR(char *msg, size_t msg_size) {
    SCRBoard *pSCRBoard;
    for (int i = 0; i < App.NumSCRBoard; i++) {
        snprintf(msg, msg_size, "[SCR#%d - MAC:%02x.%02x.%02x.%02x.%02x.%02x - IP:%d.%d.%d.%d - tout:%d - state:%d]\n", (int) GLSCRBoard[i].id
                , (int) GLSCRBoard[i].mac[0], (int) GLSCRBoard[i].mac[1], (int) GLSCRBoard[i].mac[2], (int) GLSCRBoard[i].mac[3], (int) GLSCRBoard[i].mac[4], (int) GLSCRBoard[i].mac[5]
                , (int) GLSCRBoard[i].ip[0], (int) GLSCRBoard[i].ip[1], (int) GLSCRBoard[i].ip[2], (int) GLSCRBoard[i].ip[3]
                , (int) GLSCRBoard[i].timeout_count
                , (int) GLSCRBoard[i].commState
                // %Fp/%Fp/%Fp
                // ,(unsigned long)&GLSCRBoard[i],(unsigned long)&GLSCRBoard[i].tickTocSCRWatchDogCurrent, (unsigned long)&GLSCRBoard[i].tickTocSCRWatchDog
                );
        vDisplayMessage(msg);
    }
    return 1;
}

int dataExchangeInitSCR(int Mode) {
    int retVal = 1;
    char str[256];
    int rc = 0;

    int shouldUpdateSCRCfg = 0;



    GLNumSCRBoardAllocated = 16;
    App.NumSCRBoard = 0;
    GLSCRBoard = (SCRBoard*) calloc((sizeof (SCRBoard) * GLNumSCRBoardAllocated) + 1, 1);

    if (!GLSCRBoard) {
        return -999;            
    }

    App.numNewSCR = 0;
    App.numUnresolvedSCR = 0;
    App.numDuplicateIpSCR  = 0;
    
    for (int i = 0; i < GLNumSCRBoardAllocated; i++) {

        GLSCRBoard[i].id = 0;
        GLSCRBoard[i].Mode = Mode;
        GLSCRBoard[i].port = GLSCRIpAddrPort;
        // GLSCRBoard[i].ip[0] = 192; GLSCRBoard[i].ip[1] = 168; GLSCRBoard[i].ip[2] = 1; GLSCRBoard[i].ip[3] = ...;

        GLSCRBoard[i].numRows = 8;
        // GLSCRBoard[i].Rows[] = ...

        // SCR WatchDOG
        GLSCRBoard[i].tickTocWatchDogTimeout = (uint32_t) TIMER_MS_TO_TICKS((machine.scr_timeout_ms > 0 ? machine.scr_timeout_ms : SCR_WATCHDOG_TIMEOUT_MSEC));

        GLSCRBoard[i].tickTocWatchDogCurrent = 0;

        // IO pending timer
        GLSCRBoard[i].tickTocPendingTimeout = (uint32_t) TIMER_MS_TO_TICKS((SCR_PENDING_TIMEOUT_MSEC));

        GLSCRBoard[i].tickTocPendingCounter = 0;

    }
    




    /////////////////////////////////////////
    // Lettura del file di configurazione
    //

    int curSCRBoard = 0;

    int res_cfg = read_scr_cfg();



    if (res_cfg > 0) {

        /////////////////////////////////////////////////////
        // Risoluzione dell'ip sulla base del mac address
        //

        snprintf(App.Msg, App.MsgSize, "[Reading %d SCR cfg...", App.NumSCRBoard);
        vDisplayMessage(App.Msg);

        App.numSCROK = 0;

        for (int j = START_SCR_ADDR; j < END_SCR_ADDR; j++) {
            IpAddr_t target_ip = {192, 168, 1, (uint8_t) j};
            EthAddr_t eth_dest = {0};
            int n_attemp = (j - START_SCR_ADDR) < App.NumSCRBoard ? 3 : 1;

            vDisplayMessage(".");

            // Richiesta mac addr da ip
            // rc = get_mac_addr_from_ip( target_ip, &eth_dest, 0 );

            for (int attemp = 0; attemp < n_attemp; attemp++) {

                rc = get_MAC_addr(target_ip, (uint16_t) 7371, &eth_dest, 0);

                if (rc > 0) {

                    int res = 0;

                    if (eth_dest[0] == 0x73 && eth_dest[1] == 0x71) {
                        ///////////////////////
                        // Scheda di SCR
                        //

                        snprintf(App.Msg, App.MsgSize, "[IP:%s%d.%d.%d.%d%s"
                                , (char*) ANSI_COLOR_BLUE
                                , (int) target_ip[0], (int) target_ip[1], (int) target_ip[2], (int) target_ip[3]
                                , (char*) ANSI_COLOR_RESET
                                );
                        vDisplayMessage(App.Msg);

                        for (int i = 0; i < App.NumSCRBoard; i++) {
                            SCRBoard *pSCRBoard = &GLSCRBoard[i];

                            if (GLSCRBoard[i].mac[0] == eth_dest[0] && GLSCRBoard[i].mac[1] == eth_dest[1] && GLSCRBoard[i].mac[2] == eth_dest[2] &&
                                    GLSCRBoard[i].mac[3] == eth_dest[3] && GLSCRBoard[i].mac[4] == eth_dest[4] && GLSCRBoard[i].mac[5] == eth_dest[5]) {

                                // L'id � assegnato dalla lettura del file di SCR
                                // GLSCRBoard[i].id

                                memcpy(&GLSCRBoard[i].ip, &target_ip, 4);
                                
                                GLSCRBoard[i].commState = STATE_INIT;

                                GLSCRBoard[i].buf_size = sizeof (GLSCRBoard[i].buf);

                                App.numSCROK++;
                                
                                res++;
                            }
                        }

                        if (res == 0) {
                            //////////////////////////////////////
                            /// Sostituzione o aggiunta scheda
                            //
                            snprintf(App.Msg, App.MsgSize, "[New SCR Board...Please update %s]", SCR_BOARD_CFG_FILE);
                            vDisplayMessage(App.Msg);

                            if (App.NumSCRBoard < GLNumSCRBoardAllocated) {

                                App.NumSCRBoard++;

                                // Assegnamento ID
                                GLSCRBoard[App.NumSCRBoard - 1].id = App.NumSCRBoard;

                                memcpy(&GLSCRBoard[App.NumSCRBoard - 1].mac, &eth_dest, 6);
                                memcpy(&GLSCRBoard[App.NumSCRBoard - 1].ip, &target_ip, 4);

                                GLSCRBoard[App.NumSCRBoard - 1].commState = STATE_INIT;
                                
                                GLSCRBoard[App.NumSCRBoard - 1].buf_size = sizeof (GLSCRBoard[App.NumSCRBoard - 1].buf);

                                shouldUpdateSCRCfg++;

                            }


                            ///////////////////////////////////////////////////////////////////////////
                            // N.B.: Impossibile avviare la logica : Campbio Delle schede di SCR
                            //
                            App.numNewSCR++;

                        } else if (res == 1) {
                            snprintf(App.Msg, App.MsgSize, ":%sOK%s].", (char*) ANSI_COLOR_GREEN, (char*) ANSI_COLOR_RESET);
                            vDisplayMessage(App.Msg);
                        } else {
                            snprintf(App.Msg, App.MsgSize, ":%sduplicated%s].", (char*) ANSI_COLOR_RED, (char*) ANSI_COLOR_RESET);
                            vDisplayMessage(App.Msg);
                        }

                        usleep(50 * 1000);


                    } else if (eth_dest[0] == 0x73 && eth_dest[1] == 0x62) {
                        /////////////////////////////////////
                        // Scheda IO alla porta 7371 ???
                        //

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


end_read_scr:

        if (shouldUpdateSCRCfg) {
            if (update_scr_cfg() < 0) {
                snprintf(App.Msg, App.MsgSize, "[%sSCR file %s updated%s]\n", (char*) ANSI_COLOR_GREEN, SCR_BOARD_CFG_FILE, (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);
            }
        }


        // snprintf(App.Msg, App.MsgSize, "]\n");
        // vDisplayMessage(App.Msg);



    } else {

        ////////////////////////////////////////////////////////////
        // File non trovato : inizializzazione legengo gli ip
        //

        snprintf(App.Msg, App.MsgSize, "[Searching for SCR on %s...\n", App.ETHInterfaceName);
        vDisplayMessage(App.Msg);


        // snprintf(App.Msg, App.MsgSize, "[Reading ARP...]\n");
        // vDisplayMessage(App.Msg);


        for (int i = START_SCR_ADDR; i < END_SCR_ADDR; i++) {

            // Richiesta mac addr
            IpAddr_t target_ip = {192, 168, 1, (uint8_t) i};
            EthAddr_t eth_dest = {0};

            // rc = get_mac_addr_from_ip( target_ip, &eth_dest, 1 );

            rc = get_MAC_addr(target_ip, (uint16_t) 7371, &eth_dest, 0);

            if (rc > 0) {

                if (eth_dest[0] == 0x73 && eth_dest[1] == 0x71) {
                    ///////////////////////
                    // Scheda SCR
                    //

                    if (curSCRBoard < GLNumSCRBoardAllocated) {
                        SCRBoard *pSCRBoard = &GLSCRBoard[curSCRBoard];
                        curSCRBoard++;

                        // Assegnamento ID
                        pSCRBoard->id = curSCRBoard;

                        memcpy(&pSCRBoard->mac, &eth_dest, 6);
                        memcpy(&pSCRBoard->ip, &target_ip, 4);

                        pSCRBoard->commState = STATE_INIT;
                        
                        pSCRBoard->buf_size = sizeof (pSCRBoard->buf);


                        snprintf(App.Msg, App.MsgSize, "[SCR #%d > IP:%s%d.%d.%d.%d%s - MAC:%02x.%02x.%02x.%02x.%02x.%02x]\n"
                                , (int) pSCRBoard->id
                                , (char*) ANSI_COLOR_BLUE
                                , (int) pSCRBoard->ip[0], (int) pSCRBoard->ip[1], (int) pSCRBoard->ip[2], (int) pSCRBoard->ip[3]
                                , (char*) ANSI_COLOR_RESET
                                , (int) pSCRBoard->mac[0], (int) pSCRBoard->mac[1], (int) pSCRBoard->mac[2], (int) pSCRBoard->mac[3], (int) pSCRBoard->mac[4], (int) pSCRBoard->mac[5]
                                );
                        vDisplayMessage(App.Msg);
                    } else {
                        snprintf(App.Msg, App.MsgSize, "[too many SCR boards]\n");
                        vDisplayMessage(App.Msg);
                    }

                    usleep(50 * 1000);


                } else if (eth_dest[0] == 0x73 && eth_dest[1] == 0x62) {

                    ///////////////////////
                    // Scheda SCR
                    //
                    snprintf(App.Msg, App.MsgSize, "[FILTERED OUT IO IP:%d.%d.%d.%d - MAC:%02x.%02x.%02x.%02x.%02x.%02x]\n"
                            , (int) target_ip[0], (int) target_ip[1], (int) target_ip[2], (int) target_ip[3]
                            , (int) eth_dest[0], (int) eth_dest[1], (int) eth_dest[2], (int) eth_dest[3], (int) eth_dest[4], (int) eth_dest[5]
                            );
                    vDisplayMessage(App.Msg);

                } else {
#ifdef DEBUG_PRINTF
                    snprintf(App.Msg, App.MsgSize, "[FILTERED OUT IP:%d.%d.%d.%d - MAC:%s%02x.%02x.%02x.%02x.%02x.%02x%s]\n"
                            , (int) target_ip[0], (int) target_ip[1], (int) target_ip[2], (int) target_ip[3]
                            , (char*) ANSI_COLOR_RED
                            , (int) eth_dest[0], (int) eth_dest[1], (int) eth_dest[2], (int) eth_dest[3], (int) eth_dest[4], (int) eth_dest[5]
                            , (char*) ANSI_COLOR_RESET
                            );
                    vDisplayMessage(App.Msg);
#endif
                }


            } else if (rc < 0) {
                snprintf(App.Msg, App.MsgSize, "[%sSCR #%d > IP:%d.%d.%d.%d - mac error!%s]\n"
                        , (char*) ANSI_COLOR_RED
                        , (int) GLSCRBoard[i].id
                        , (int) target_ip[0], (int) target_ip[1], (int) target_ip[2], (int) target_ip[3]
                        , (char*) ANSI_COLOR_RESET
                        );
                vDisplayMessage(App.Msg);
            }
        }


        App.NumSCRBoard = curSCRBoard;

        if (App.NumSCRBoard) {
            if (update_scr_cfg() > 0) {
                snprintf(App.Msg, App.MsgSize, "[%sSCR file %s updated%s]\n", (char*) ANSI_COLOR_GREEN, SCR_BOARD_CFG_FILE, (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);
            }
        }
        
        //////////////////////////////////////////////////////////////////////////////////
        // Nessuna scheda dichiarata : Inizializzazione OK, a cura dell primo avvio
        //
        App.SCROK = true;
    }






    ////////////////////////////
    // Verifiche finali
    //
    for (int i = 0; i < App.NumSCRBoard; i++) {
        if (GLSCRBoard[i].id > 0) {
            if (GLSCRBoard[i].ip[0] > 0 && GLSCRBoard[i].ip[1] > 0 && GLSCRBoard[i].ip[2] > 0 && GLSCRBoard[i].ip[3] > 0) {
                for (int j = 0; j < App.NumSCRBoard; j++) {
                    SCRBoard *pSCRBoard2 = &GLSCRBoard[j];
                    if (i != j) {
                        if (GLSCRBoard[i].ip[0] == pSCRBoard2->ip[0] && GLSCRBoard[i].ip[1] == pSCRBoard2->ip[1] && GLSCRBoard[i].ip[2] == pSCRBoard2->ip[2] && GLSCRBoard[i].ip[3] == pSCRBoard2->ip[3]) {
                            pSCRBoard2->id = 0;
                            memset(&pSCRBoard2->ip, 0, 4);
                            App.numDuplicateIpSCR++;
                        }
                    }
                }

            } else {
                App.numUnresolvedSCR++;
            }
        }
    }






    if (App.NumSCRBoard > 0) {
        char *pANSI_COLOR = (char *) ((App.NumSCRBoard > 0 && App.numNewSCR == 0 && App.numUnresolvedSCR == 0 && App.numDuplicateIpSCR == 0) ? (ANSI_COLOR_GREEN) : (ANSI_COLOR_RED));
        snprintf(App.Msg, App.MsgSize, "[%s%d SCR Board detected, %d new, %d unresolved, %d duplicated%s]\n", (char*) pANSI_COLOR, (int) App.NumSCRBoard, (int) App.numNewSCR, (int) App.numUnresolvedSCR, (int) App.numDuplicateIpSCR, (char*) ANSI_COLOR_RESET);
        vDisplayMessage(App.Msg);

        //////////////////////////////////////
        // Generazione Emergenza Macchina
        //
        if (App.numUnresolvedSCR > 0 || App.numDuplicateIpSCR > 0) {
            snprintf(App.Msg, App.MsgSize, "[%sSCR Board error : %d unresolved, %d duplicated%s]", (char*) ANSI_COLOR_RED, App.numUnresolvedSCR, App.numDuplicateIpSCR, (char*) ANSI_COLOR_RESET);
            vDisplayMessage(App.Msg);
            retVal = -1;
            /*
            if (generate_alarm((char*) "No SCR Board detected", 5022, 0, (int) ALARM_FATAL_ERROR) < 0) {             
            }
            */
        } else {
        }

    } else {
        //////////////////////////////////////
        // Generazione Emergenza Macchina
        //
        if (res_cfg > 0) {
            snprintf(App.Msg, App.MsgSize, "[%sNO SCR Board detected%s]\n", (char*) ANSI_COLOR_RED, (char*) ANSI_COLOR_RESET);
            vDisplayMessage(App.Msg);
            retVal = -1;
            /*
            if (generate_alarm((char*) App.Msg, 5023, 0, (int) ALARM_FATAL_ERROR) < 0) {
            }
            */
        }
    }






    if (App.numNewSCR > 0 || App.numUnresolvedSCR > 0 || App.numDuplicateIpSCR > 0) {
        if (App.numNewSCR == 1 && App.numUnresolvedSCR == 1 && App.numDuplicateIpSCR == 0) {
            // sostituzione scheda SCR
        } else {
            // Impossibile determinare lo slot delle schede : necessario riconfigurazione manuale
            retVal = -1;
        }

    } else {
    }
    
    
    ///////////////////////////////
    // Registra la callback UDP
    //
    if (retVal > 0) {
        if (App.NumSCRBoard > 0) {
            if (xrt_callback_udp((uint16_t) GLSCRIpAddrPort, &udpSCRHandler) <= 0) {
                //////////////////////////////////////
                // Generazione Emergenza Macchina
                //
                if (generate_alarm((char*) "No IO UDP callback", 5010, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                }
                retVal = -1;
            } else {
                // snprintf(App.Msg, App.MsgSize, "[CANBUS Callback Installed]\n"); vDisplayMessage(App.Msg);            
            }        
        }
    }
    
    if (retVal > 0) {
        ////////////////////////////////////////////////////////
        // Tutte le schede identificate : Inizializzazione OK
        //
        App.SCROK = true;            
    } else {
        App.SCRDetectError++;
    }

    return retVal;    
}

int read_scr_cfg() {
    int retVal = 0;
    uint8_t MacAddr[6];
    FILE *f = fopen(SCR_BOARD_CFG_FILE, "r");

    App.NumSCRBoard = 0;

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
                    if (boardId >= 1 && boardId <= GLNumSCRBoardAllocated) {
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
                        // SCRBoard *pSCRBoard = &GLSCRBoard[boardId-1];

                        GLSCRBoard[boardId - 1].id = boardId;
                        memcpy(&GLSCRBoard[boardId - 1].mac, &MacAddr, 6);

                        GLSCRBoard[boardId - 1].commState = STATE_UNINIT;
                                                       
                        /*
                        snprintf(App.Msg, App.MsgSize, "[CFG #%d : MAC:%02x.%02x.%02x.%02x.%02x.%02x]\n"
                                ,(int)GLSCRBoard[i].id
                                ,(int)GLSCRBoard[i].mac[0], (int)GLSCRBoard[i].mac[1], (int)GLSCRBoard[i].mac[2], (int)GLSCRBoard[i].mac[3], (int)GLSCRBoard[i].mac[4], (int)GLSCRBoard[i].mac[5]
                                );
                        vDisplayMessage(App.Msg);
                         */

                        if (boardId > App.NumSCRBoard) {
                            App.NumSCRBoard = boardId;
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

int update_scr_cfg() {
    int retVal = 0;

    if (App.NumSCRBoard) {
        char str[256];
        FILE *f = fopen(SCR_BOARD_CFG_FILE, "w");
        if (f) {
            for (int i = 0; i < App.NumSCRBoard; i++) {
                SCRBoard *pSCRBoard = &GLSCRBoard[i];

                snprintf(str, sizeof (str), "#%d=%02x.%02x.%02x.%02x.%02x.%02x\n", GLSCRBoard[i].id, GLSCRBoard[i].mac[0], GLSCRBoard[i].mac[1], GLSCRBoard[i].mac[2], GLSCRBoard[i].mac[3], GLSCRBoard[i].mac[4], GLSCRBoard[i].mac[5]);
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
        snprintf(App.Msg, App.MsgSize, "[update_scr_cfg : No SCR boards]\n");
        vDisplayMessage(App.Msg);
        retVal = -1;
    }

    return retVal;
}

int dataEchangeResetSCR() {
    snprintf(App.Msg, App.MsgSize, "\nDelete SCR file (Yes/No) ?\n");
    vDisplayMessage(App.Msg);
    if (getch() == 'Y') {
#ifdef WATCOM
        delete(SCR_BOARD_CFG_FILE);
#else
        unlink(SCR_BOARD_CFG_FILE);
#endif
        snprintf(App.Msg, App.MsgSize, "\n File %s deleted. Please Reboot...\n", SCR_BOARD_CFG_FILE);
        vDisplayMessage(App.Msg);
        return 1;
    }
    return 0;
}

int dataEchangeStopSCR() {
    GLRunLoop = 0;
}

int dataExchangeLoopSCR() {
    int retVal = 0;
    uint32_t dtka = 0;
    int rc = 0;

    try {

        for (uint16_t i = 0; i < App.NumSCRBoard; i++) {

            if (GLSCRBoard[i].id) {
                
                if (!GLSCRBoard[i].disabled) {

                    switch (GLSCRBoard[i].commState) {


                    case STATE_INIT:
                        GLSCRBoard[i].commState = STATE_START;
                        GLSCRBoard[i].tickTocWatchDog = 0;
                        GLSCRBoard[i].SessionID = 0;
                        // Abilitazione SCR                        
                        App.SCROK = INIT_AND_RUN;                        
                        break;


                    case STATE_START:
                        // Utilizzo del protocollo UDP per la velocit�
                        // strcpy(INIT_COMM_KEY_STR, "?SCR");
                        // uint16_t udp_req_len = (uint16_t)strlen(INIT_COMM_KEY_STR);

                        GLSCRBoard[i].tickTocWatchDog = 0;

                        if (GLSCRBoard[i].ip[0] == 0 && GLSCRBoard[i].ip[1] == 0 && GLSCRBoard[i].ip[2] == 0 && GLSCRBoard[i].ip[3] == 0) {

#ifdef DEBUG_PRINTF
                            snprintf(App.Msg, App.MsgSize, "[SCR] unresolved ip on #%d\n", (int) GLSCRBoard[i].id);
                            vDisplayMessage(App.Msg);
#endif

                            GLSCRBoard[i].commState = STATE_PENDING;

                        } else {

                            // Attiva la sessione
                            rc = xrt_send_udp(GLSCRBoard[i].ip, (uint16_t) GLSCRBoard[i].port, (uint16_t) GLSCRBoard[i].port, (uint8_t *) "!S", (uint16_t) 2, (uint8_t) 0);
                            if (rc > 0) {
                            } else {
                                snprintf(App.Msg, App.MsgSize, "[%s START SCR SESSION %s]\n", (char*) ANSI_COLOR_RED, (char*) ANSI_COLOR_RESET);
                                vDisplayMessage(App.Msg);
                            }

                            GLSCRBoard[i].ShouldInit = true;

                            rc = xrt_send_udp(GLSCRBoard[i].ip, (uint16_t) GLSCRBoard[i].port, (uint16_t) GLSCRBoard[i].port, (uint8_t *) "?SCR", (uint16_t) 4, (uint8_t) 0);

                            if (rc > 0) {

                                GLSCRBoard[i].commState = STATE_WAITING_FOR_STREAM;

                                if (GLSCRBoard[i].Mode & 1) {
                                    snprintf(App.Msg, App.MsgSize, "[SCR#%d] %d.%d.%d.%d:%u - WD:%dmsec\n", (int) (i + 1), (int) GLSCRBoard[i].ip[0], (int) GLSCRBoard[i].ip[1], (int) GLSCRBoard[i].ip[2], (int) GLSCRBoard[i].ip[3], (int) GLSCRBoard[i].port, GLSCRBoard[i].tickTocWatchDogTimeout);
                                    vDisplayMessage(App.Msg);
                                }

                            } else {

                                perror("xtr_udp_send():");

                                // GLSCRBoard[i].SCRCommState = STATE_CLOSING;
                                GLSCRBoard[i].commState = STATE_PENDING;

                                GLSCRBoard[i].tickTocWatchDog = 0;

                            }
                        }
                        if (!GLRunLoop) {
                            GLSCRBoard[i].commState = STATE_CLOSING;
                        }
                        break;



                    case STATE_PENDING:
                        GLSCRBoard[i].tickTocWatchDogCurrent = (uint32_t) xTaskGetTickCount();

                        if (GLSCRBoard[i].tickTocWatchDog == 0) {
                            // Avvio temporizzatore attesa ARP
                            GLSCRBoard[i].tickTocWatchDog = (uint32_t) xTaskGetTickCount();
                        } else {
                            if ((uint32_t) GLSCRBoard[i].tickTocWatchDogCurrent - GLSCRBoard[i].tickTocWatchDog > GLSCRBoard[i].tickTocPendingTimeout) {
                                // GLSCRBoard[i].commState = ...;
                                // GLSCRBoard[i].tickTocPendingCounter++;
                                GLSCRBoard[i].tickTocWatchDog = 0;
                            }
                        }
                        if (!GLRunLoop) {
                            GLSCRBoard[i].commState = STATE_CLOSING;
                        }
                        break;




                    case STATE_WAITING_FOR_STREAM:
                        /////////////////////////////////////////////////////
                        // Attesa della recezione dati
                        //

                        // Lettura dati dallo stream udp
                        rc = xrt_recive_udp(GLSCRBoard[i].ip, GLSCRBoard[i].port, GLSCRBoard[i].port, GLSCRBoard[i].buf, GLSCRBoard[i].buf_size, 0);
                        if (rc > 0) {
                            // snprintf(App.Msg, App.MsgSize, "[SCR Board in data:%d...]\n", (int) rc);  vDisplayMessage(App.Msg);
                        }

                        //
                        GLSCRBoard[i].tickTocWatchDogCurrent = (uint32_t) xTaskGetTickCount();

                        if (GLSCRBoard[i].tickTocWatchDog == 0) {
                            // Avvio temporizzatore attesa ARP
                            GLSCRBoard[i].tickTocWatchDog = (uint32_t) xTaskGetTickCount();
                        } else {
                            if ((uint32_t) GLSCRBoard[i].tickTocWatchDogCurrent - GLSCRBoard[i].tickTocWatchDog > GLSCRBoard[i].tickTocPendingTimeout) {
                                GLSCRBoard[i].commState = STATE_INIT;
                                // GLSCRBoard[i].tickTocPendingCounter++;
                                GLSCRBoard[i].tickTocWatchDog = 0;
                                // vDisplayMessage("-");
                            }
                        }
                        if (!GLRunLoop) {
                            GLSCRBoard[i].commState = STATE_CLOSING;
                        }
                        break;



                    case STATE_CONNETED:
                        ///////////////////////////////////////////////////////
                        // Comunicazione con gli I/O 
                        //
                        // Lettura dati dallo stream udp



                        rc = xrt_recive_udp(GLSCRBoard[i].ip, GLSCRBoard[i].port, GLSCRBoard[i].port, GLSCRBoard[i].buf, GLSCRBoard[i].buf_size, 0);

                        if (rc > 0) {
                            ////////////////////////////////
                            // Messaggio dalla scheda SCR
                            //
                            // snprintf(App.Msg, App.MsgSize, "[SCR Board in data:%d...]\n", (int) rc); 
                            // vDisplayMessage(App.Msg);
                            //
                        }



                        /////////////////////////////////////
                        // Analisi variazioni sulla logica
                        //

                    {
                        float *powen_row = (float *) NULL;
                        float *powen_row_bk = (float *) NULL;
                        uint16_t startRow = 0, endRow = 0;


#ifdef xBM_COMPILE
                        if (GLSCRBoard[i].id == 1) {
                            powen_row = (float*) &machine.workSet.owen1_row[0];
                            powen_row_bk = (float*) &machine.workSet.owen1_row_bk[0];
                            startRow = 0;
                            endRow = 8;
                        } else if (GLSCRBoard[i].id == 2) {
                            powen_row = (float*) &machine.workSet.owen1_row[0];
                            powen_row_bk = (float*) &machine.workSet.owen1_row_bk[0];
                            startRow = 8;
                            endRow = 16;
                        } else if (GLSCRBoard[i].id == 3) {
                            powen_row = (float*) &machine.workSet.owen2_row[0];
                            powen_row_bk = (float*) &machine.workSet.owen2_row_bk[0];
                            startRow = 0;
                            endRow = 8;
                        } else if (GLSCRBoard[i].id == 4) {
                            powen_row = (float*) &machine.workSet.owen2_row[0];
                            powen_row_bk = (float*) &machine.workSet.owen2_row_bk[0];
                            startRow = 8;
                            endRow = 16;
                        }
#endif

                        if (powen_row && powen_row_bk) {
                            char str[256], sendingCMD[512];
                            uint32_t sendingCMDSize, changeCount = 0;

                            sendingCMD[0] = '!';
                            sendingCMD[1] = 0;
                            sendingCMDSize = 1;

                            for (uint16_t j = startRow; j < endRow; j++) {
                                float powen_row_j = powen_row[j];

#ifdef xBM_COMPILE
                                if (machine.App.heat_on) {
                                } else {
                                    powen_row_j = 0.0f;
                                }
#endif
                                if (fabs(powen_row_j - powen_row_bk[j]) > 0.001f || GLSCRBoard[i].ShouldInit) {

                                    if (changeCount) {
                                        strncat((char*) sendingCMD, App.RowSep, sizeof (sendingCMD) - 1);
                                        sendingCMDSize++;
                                    }

                                    unsigned char value1b = (unsigned char) ((int) roundf(powen_row_j * 100.0f) + 1);

                                    snprintf((char*) str, sizeof (str), "R%c=%c", (unsigned char) (j - startRow + 1), (unsigned char) value1b);

                                    strncat((char*) sendingCMD, (char*) str, sizeof (sendingCMD) - strlen((char*) str) - 1);

                                    // snprintf(App.Msg, App.MsgSize, "[ROW%d=%d(%0.3f)]", (int)str[1], (int)str[3], (powen_row_j*100.0f)); vDisplayMessage(App.Msg);


                                    sendingCMDSize += 4;

                                    changeCount++;

                                    // snprintf(App.Msg, App.MsgSize, "[SCR#%d from %0.3f to %0.3f]\n", (int)j, (float)powen_row_bk[j], (float)powen_row[j]); vDisplayMessage(App.Msg);

                                    powen_row_bk[j] = powen_row_j;
                                }
                            }


                            if (changeCount) {
                                rc = xrt_send_udp(GLSCRBoard[i].ip, (uint16_t) GLSCRBoard[i].port, (uint16_t) GLSCRBoard[i].port, (uint8_t *) sendingCMD, (uint16_t) sendingCMDSize, (uint8_t) 0);
                                if (rc > 0) {
                                    // snprintf(App.Msg, App.MsgSize, "[%s UPDATE %d SCR {%s} %dbytes %s]\n", (char*) ANSI_COLOR_GREEN, changeCount, (char*) sendingCMD, (int)sendingCMDSize, (char*) ANSI_COLOR_RESET); vDisplayMessage(App.Msg);                                      
                                    GLSCRBoard[i].ShouldInit = false;
                                } else {
                                    snprintf(App.Msg, App.MsgSize, "[%s UPDATED SCR FAILED %s]\n", (char*) ANSI_COLOR_RED, (char*) ANSI_COLOR_RESET);
                                    vDisplayMessage(App.Msg);
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
                        if (xTaskGetTickCount() - GLSCRBoard[i].tickTocReadi2c >= READ_I2C_INTERVAL_MS) {
                            GLSCRBoard[i].tickTocReadi2c = xTaskGetTickCount();
                            if (xrt_send_udp(GLSCRBoard[i].ip, (uint16_t) GLSCRBoard[i].port, (uint16_t) GLSCRBoard[i].port, (uint8_t *) "I", (uint16_t) 1, (uint8_t) 0) == 0) {
                                // vDisplayMessage("[SCR I2C read]");
                            } else {
                                // vDisplayMessage("[!IO]");
                            }
                        }



                        if (xTaskGetTickCount() - GLSCRBoard[i].tickTocReadScr >= READ_SCR_INTERVAL_MS) {
                            GLSCRBoard[i].tickTocReadScr = xTaskGetTickCount();
                            if (xrt_send_udp(GLSCRBoard[i].ip, (uint16_t) GLSCRBoard[i].port, (uint16_t) GLSCRBoard[i].port, (uint8_t *) "?SCR", (uint16_t) 4, (uint8_t) 0) == 0) {
                                // vDisplayMessage("[SCR I2C read]");
                            } else {
                                // vDisplayMessage("[!IO]");
                            }
                        }



                        //
                        // watchDog SCR                        
                        //
                        GLSCRBoard[i].tickTocWatchDogCurrent = (uint32_t) xTaskGetTickCount();


                        if (GLSCRBoard[i].tickTocWatchDog == 0) {
                            // Avvio watchDog
                            GLSCRBoard[i].tickTocWatchDog = xTaskGetTickCount();
                        } else {
                            if ((uint32_t) GLSCRBoard[i].tickTocWatchDogCurrent - GLSCRBoard[i].tickTocWatchDog > GLSCRBoard[i].tickTocWatchDogTimeout) {
                                //////////////////////
                                // Scheda Offline
                                //

                                GLSCRBoard[i].commState = STATE_INIT;
                                GLSCRBoard[i].tickTocWatchDog = 0;

#ifdef xBM_COMPILE
                                // Macchina in produzione ?
                                if (machine.App.heat_on == false) {
                                    if (generate_alarm((char*) "SCR Board Communication Timeout", 5102, 0, (int) ALARM_WARNING, 0+1) < 0) {
                                    }
                                } else {
                                    if (generate_alarm((char*) "SCR Board Communication Timeout", 5102, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                                    }
                                }
#endif
                                
                            }
                        }

                        if (!GLRunLoop) {
                            GLSCRBoard[i].commState = STATE_CLOSING;
                        }
                        break;



                    case STATE_CLOSING:
                        if (GLRunLoop) {
                            GLSCRBoard[i].commState = STATE_START;
                        } else {
                            GLSCRBoard[i].commState = STATE_CLOSED;
                            rc = xrt_send_udp(GLSCRBoard[i].ip, (uint16_t) GLSCRBoard[i].port, (uint16_t) GLSCRBoard[i].port, (uint8_t *) "!X", (uint16_t) 2, (uint8_t) 0);
                            if (rc > 0) {
                            }
                            snprintf(App.Msg, App.MsgSize, "[RTC] SCR#%d Stopped\n", (i + 1));
                            vDisplayMessage(App.Msg);
                        }
                        break;

                    case STATE_CLOSED:
                        break;

                }
                
                }
                
            } else {
                // snprintf(App.Msg, App.MsgSize, "[SCR board %d invalid]\n", (int)i);
                // vDisplayMessage(App.Msg);
            }
        }

    } catch (int) {
    }

    return retVal;
}

int dataExchangeIsRunningSCR() {
    for (uint16_t i = 0; i < App.NumSCRBoard; i++) {
        if (GLSCRBoard[i].id) {
            if (GLSCRBoard[i].commState != STATE_CLOSED) {
                return 1;
            }
        }
    }
    return 0;
}





/////////////////////////////////////////////
// callback paccketto UDP (schede SCR)
//

void udpSCRHandler(IpAddr_t ip, uint16_t sport, uint16_t dport, int8_t *buf, uint16_t buf_size) {

    if (buf) {

        if (buf_size) {
            uint16_t udpLen = xrt_ntohs((uint16_t) buf_size);

            uint8_t *pData = (uint8_t *) (buf);
            uint8_t SCRBoardFound = 0;

            // GLSCRDataPerSec++;

            // snprintf(App.Msg, App.MsgSize, "[SCR Data recived : {%s} - %d bytes]\n", (char*) buf, buf_size); vDisplayMessage(App.Msg);

            for (int i = 0; i < App.NumSCRBoard; i++) {

                if (GLSCRBoard[i].id) {

                    if (GLSCRBoard[i].ip[0] == ip[0] && GLSCRBoard[i].ip[1] == ip[1] &&
                            GLSCRBoard[i].ip[2] == ip[2] && GLSCRBoard[i].ip[3] == ip[3]) {

                        // Reset watch dog
                        // GLSCRBoard[i].tickTocSCRWatchDog = 0;
                        GLSCRBoard[i].tickTocWatchDog = (uint32_t) xTaskGetTickCount();


                        //////////////  //////////////////////////////////
                        // Avvio dello stream -> cambio di stato
                        //
                        if (GLSCRBoard[i].commState == STATE_WAITING_FOR_STREAM) {
                            GLSCRBoard[i].commState = STATE_CONNETED;

                            snprintf(App.Msg, App.MsgSize, "[%sSCR #%d <-> %d.%d.%d.%d%s]\n", (char*) ANSI_COLOR_GREEN, GLSCRBoard[i].id, GLSCRBoard[i].ip[0], GLSCRBoard[i].ip[1], GLSCRBoard[i].ip[2], GLSCRBoard[i].ip[3], (char*) ANSI_COLOR_RESET);
                            vDisplayMessage(App.Msg);

                        } else if (GLSCRBoard[i].commState == STATE_CONNETED) {
                            // Reset watch dog
                            GLSCRBoard[i].tickTocWatchDog = 0;
                        }

#define MAX_ROWS    16

                        int j = 0;
                        uint16_t numRows = 0;

                        if (buf_size >= 2 &&
                                pData[j] == '=') {
                            j++;

                            memcpy(&GLSCRBoard[i].SessionID, &pData[j], 2);
                            j += 2;

                            memcpy(&GLSCRBoard[i].AnalogVoltage[0], &pData[j], 4);
                            j += 4;
                            memcpy(&GLSCRBoard[i].AnalogVoltage[1], &pData[j], 4);
                            j += 4;
                            memcpy(&GLSCRBoard[i].AnalogVoltage[2], &pData[j], 4);
                            j += 4;

                            memcpy(&numRows, &pData[j], 2);
                            j += 2;

                            if (numRows > GLSCRBoard[i].numRows) {

                                snprintf(App.Msg, App.MsgSize, "[SCR Data: Invalid numRows:%d/%d]\n", (int) numRows, GLSCRBoard[i].numRows);
                                vDisplayMessage(App.Msg);

                            } else {

                                for (int iRow = 0; iRow < numRows; iRow++) {
                                    GLSCRBoard[i].Rows[iRow] = (int) pData[j + iRow];
                                }
                                j += numRows;


                                memcpy(&GLSCRBoard[i].KALoopTimeMs, &pData[j], 2);
                                j += 2;
                                memcpy(&GLSCRBoard[i].TollRefChange, &pData[j], 4);
                                j += 4;

                                memcpy(&GLSCRBoard[i].MaxErr, &pData[j], 2);
                                j += 2;


                                memcpy(&GLSCRBoard[i].StatusCode, &pData[j], 2);
                                j += 2;
                                memcpy(&GLSCRBoard[i].StatusDataSize, &pData[j], 2);
                                j += 2;
                                memcpy(&GLSCRBoard[i].StatusData, &pData[j], GLSCRBoard[i].StatusDataSize);
                                j += GLSCRBoard[i].StatusDataSize;


                                if (GLSCRBoard[i].tickTocWatchDogTimeout < GLSCRBoard[i].KALoopTimeMs + 10 && GLSCRBoard[i].KALoopTimeMs > 0) {
                                    GLSCRBoard[i].tickTocWatchDogTimeout == GLSCRBoard[i].KALoopTimeMs + 10;
                                    snprintf(App.Msg, App.MsgSize, "[SCR Data: Invalid timeout:%d/%d]\n", (int) GLSCRBoard[i].tickTocWatchDogTimeout, GLSCRBoard[i].KALoopTimeMs);
                                    vDisplayMessage(App.Msg);
                                }


                                /*
                                snprintf(App.Msg, App.MsgSize, "[SCR Data: AnalogVolt:[%0.3f][%0.3f][%0.3f] - numRows:%d - Rows:[%d][%d][%d] -  Toll:%0.3f - StatusCode:%d - StatusDataSize:%d]\n"
                                        , (float) AnalogVoltage[0], (float) AnalogVoltage[1], (float) AnalogVoltage[2]
                                        , (int) numRows
                                        , (int) CHANEL_VALUES[0], (int) CHANEL_VALUES[1], (int) CHANEL_VALUES[2]
                                        , (float) TollRefChange
                                        , (int) StatusCode, (int) StatusDataSize
                                        );
                                vDisplayMessage(App.Msg);
                                 */

                            }


                            if (GLSCRBoard[i].SessionID == 0) {
                                // Scheda riavviata
                                GLSCRBoard[i].commState = STATE_INIT;
                            }

                        } else if (buf_size >= 2 &&
                                pData[j] == '@') {
                            ///////////////////////
                            // Segnale Keep Alive
                            //
                            // Lettura periodica dello stato delgli SCR
                            xrt_send_udp(GLSCRBoard[i].ip, (uint16_t) GLSCRBoard[i].port, (uint16_t) GLSCRBoard[i].port, (uint8_t *) "?SCR", (uint16_t) 4, (uint8_t) 0);


                        } else if (buf_size >= 2 &&
                                (GLSCRBoard[i].buf[0] == -1 || GLSCRBoard[i].buf[0] == 255) && (GLSCRBoard[i].buf[1] == -1 || GLSCRBoard[i].buf[1] == 255)
                                ) {

                            //////////////////////////////////
                            // DIspositivo su Indirizzo i2c
                            //

                            unsigned char *pData = &GLSCRBoard[i].buf[2];

                            long int tAmbient = 0, tObject = 0;

                            memcpy(&tAmbient, &pData[0], sizeof (long int));
                            pData += sizeof (long int);

                            memcpy(&tObject, &pData[0], sizeof (long int));
                            pData += sizeof (long int);

                            // ...
                            GLSCRBoard[i].i2cValues[0] = tAmbient;
                            GLSCRBoard[i].i2cValues[1] = tObject;
                            GLSCRBoard[i].i2cNumValues = 2;


                            for (int i_act = 0; i_act < machine.num_actuator; i_act++) {
                                if (machine.actuator[i_act].auxSCRId == GLSCRBoard[i].id) {
                                    if (machine.actuator[i_act].auxI2C == 0) {
                                        machine.actuator[i_act].cur_rpos = (float) GLSCRBoard[i].i2cValues[0] / 1000.0f;
                                        // posizione virtuale (0...1000)
                                        update_actuator_virtual_pos(&machine.actuator[i_act]);
                                    } else if (machine.actuator[i_act].auxI2C == 1) {
                                        machine.actuator[i_act].cur_rpos = (float) GLSCRBoard[i].i2cValues[1] / 1000.0f;
                                        // posizione virtuale (0...1000)
                                        update_actuator_virtual_pos(&machine.actuator[i_act]);
                                    } else {
                                    }
                                }
                            }

                            // snprintf(App.Msg, App.MsgSize, "[SCR#%d - tAmbient:%0.3f - tObject:%0.3f]\n", GLSCRBoard[i].id, (float)tAmbient / 1000.0f, (float)tObject / 1000.0f);
                            // vDisplayMessage(App.Msg);

                        } else {
                            // Messaggio inatteso
                        }


                        SCRBoardFound++;


                        if ((int) GLSCRBoard[i].id > 0 && GLSCRBoard[i].id < GLNumSCRBoardAllocated) {

                        } else {
                            snprintf(App.Msg, App.MsgSize, "[SCR#%d out of index:%d/%d]\n", (int) (i + 1), (int) GLSCRBoard[i].id, (int) GLNumSCRBoardAllocated);
                            vDisplayMessage(App.Msg);
                        }
                    }
                }
            }

            if (!SCRBoardFound) {
                snprintf(App.Msg, App.MsgSize, "[!SCRxIP:%d.%d.%d.%d]\n", (int) ip[0], (int) ip[1], (int) ip[2], (int) ip[3]);
                vDisplayMessage(App.Msg);

                for (int i = 0; i < App.NumSCRBoard; i++) {
                    snprintf(App.Msg, App.MsgSize, "[SCR%d - ID:%d - IP:%d.%d.%d.%d]\n", i + 1, GLSCRBoard[i].id, GLSCRBoard[i].ip[0], GLSCRBoard[i].ip[1], GLSCRBoard[i].ip[2], GLSCRBoard[i].ip[3]);
                    vDisplayMessage(App.Msg);
                }

                //////////////////////////////////////
                // Generazione Emergenza Macchina
                //
                if (generate_alarm((char*) "SCR Communication board not found", 5024, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
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
