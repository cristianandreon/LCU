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



// #define DEBUG_PRINTF





///////////////////////////////////////////////////
// Dati da inviare per conto del servizio HTTP
//
//
// N.B.: Pronlemi anche con l'utilizzo del Task UI, che comunque rallenta la risposta degli I/O
//      Probabile causa listen/accept non multitasking
//      Soluzione : il servizio HTTP verr� utilizzato solo per manutenzione a macchina in Stop
//      Per la teleassistenza verr� fatto un tunnel con l'interfaccia utente
//

// #define USE_UI_TASK





// Esportazione in C
extern "C" {

}




extern int closeSocket(TcpSocket **newSocket);






// Timeout ciclo comunicazioni    
#define APP_TIMEOUT_SEC             30

// Keep Alive connessione
#define KEEPALIVE_TIMEOUT_MSEC      10000








////////////////////////////////
// Mode & 1 ->  Debug Mode
//

int HTTPServerInit(int Mode) {
    size_t msg_size = 256;
    char *msg = (char *) malloc(msg_size);


    if (Mode & 1) {
    }

    App.recvBufferSize = (uint32_t) RECV_BUFFER_SIZE-1;
    while (!App.recvBuffer) {
        App.recvBuffer = (uint8_t *) calloc(App.recvBufferSize, 1);
        if (!App.recvBuffer) {
            // printf( "[ERROR] Failed to allocate %d bytes!\n", App.recvBufferSize);
            if (App.recvBufferSize > 256) {
                App.recvBufferSize -= 256;
            } else {
                break;
            }
        }
    }

    App.sendBufferSize = 16*1024-1;
    while (!App.sendBuffer) {
        App.sendBuffer = (uint8_t*) calloc(App.sendBufferSize + 1, 1);
        if (!App.sendBuffer) {
            // printf( "[ERROR] Failed to allocate %d bytes!\n", App.recvBufferSize);
            if (App.sendBufferSize > 256) {
                App.sendBufferSize -= 256;
            } else {
                break;
            }
        }
    }



    // snprintf(msg, msg_size, "\n[HTTP] %dTK/s - IDLE TK:%d - KA:%d\n", (int)TIMER_MS_TO_TICKS( 1000 ), (int)TIMER_MS_TO_TICKS( APP_TIMEOUT_SEC * 1000 ), (int)TIMER_MS_TO_TICKS( machine.ui_timeout_ms>0?machine.ui_timeout_ms:KEEPALIVE_TIMEOUT_MSEC ) );
    // vDisplayMessage(msg);

    if (!App.recvBuffer) {
#ifdef DEBUG_PRINTF
        printf("[ERROR] Failed to allocate %d bytes!\n", App.recvBufferSize);
#endif
        return -1;
    }
    if (!App.sendBuffer) {
#ifdef DEBUG_PRINTF
        printf("[ERROR] Failed to allocate %d bytes!\n", App.sendBufferSize);
#endif
        return -1;
    }


    App.RTHttpInit = true;
    
    App.HTTPServerState = STATE_INIT;


    if (msg)
        free(msg);

    return 1;
}









//
// Mode & 2 ->  Modalit� ritorno
// Mode & 1 ->  Modalit� debug
//

int HTTPServerLoop(int Mode) {
    uint32_t nSend = 0, nSent = 0;
    uint32_t recivError = 0, maxRecivError = 0, indexRecvBuffer = 0;

    TickType_t xLastWakeTime = xTaskGetTickCount();

    clockTicks_t lastTimeout1 = xTaskGetTickCount(), lastTimeout2 = lastTimeout1;
    clockTicks_t td1 = 0, td2 = 0;

    uint32_t tickTimeout = (uint32_t) TIMER_MS_TO_TICKS((APP_TIMEOUT_SEC * 1000));
    uint32_t tickToc = (uint32_t) TIMER_MS_TO_TICKS((1000));
    uint32_t dtka = 0;

    // UI Keep alive
    uint32_t tickTocUIKeepAliveTimeout = (uint32_t) TIMER_MS_TO_TICKS((KEEPALIVE_TIMEOUT_MSEC));
    uint32_t tickTocUIKeepAlive = 0;


    TcpSocket httpSocket = 0;
    TcpSocket listeningSocket = 0;

    size_t msg_size = 256;
    char *msg = (char *) malloc(msg_size);

    int32_t nReciv = 0;
    int rc = 0;


    App.HTTPRunning = 1;
    App.HTTPRunLoop = 1;


    if (!(Mode & 2)) {
        // Modalit� ritorno
        snprintf(msg, msg_size, "\n[HTTP] %s%d.%d.%d.%d:%u%s - KA:%dmsec\n", (char*) ANSI_COLOR_CYAN, (int) App.MyIpAddr[0], (int) App.MyIpAddr[1], (int) App.MyIpAddr[2], (int) App.MyIpAddr[3], App.HTTPLListeningPort, (char*) ANSI_COLOR_RESET, tickTocUIKeepAliveTimeout);
        vDisplayMessage(msg);
    }


    while (App.HTTPRunLoop) {

        try {

            // N.B.: se non abbastanza veloce causa il riempimento del buffer (250 msec sembra ok)
            // Wait for the next cycle.
            if (!(Mode & 2)) {
                // Modalit� ritorno
                vTaskDelayUntil((uint32_t*) & xLastWakeTime, (uint32_t) 500);
            }


            switch (App.HTTPServerState) {

                
                case STATE_UNINIT:
                    // Attesa di inizializzazione
                    break;
                    
        
                case STATE_INIT:

                    // if(Mode & 1) {
                    // }

                    lastTimeout1 = TIMER_GET_CURRENT();
                    lastTimeout2 = TIMER_GET_CURRENT();

                    if (!listeningSocket) {
                        listeningSocket = xrt_socket();
                    }

                    if (!listeningSocket) {
                        fprintf(stderr, "[ERROR:!listeningSocket]\n");
                        sleep(10000);
                    } else {

                        // lose the pesky "address already in use" error message
                        int yes = 1;
                        if (setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &yes, sizeof (int)) < 0) 
                            perror("setsockopt failed\n");
                        
  
    
    
                        // Now bind the host address using bind() call
                        if (xrt_bind(listeningSocket, App.HTTPLListeningPort) < 0) {
                            fprintf(stderr, "[ERROR:");
                            perror("!xrt_bind:");
                            fprintf(stderr, " - port:%d]\n", App.HTTPLListeningPort);
                            usleep(1000 * 1000);
                            xrt_shutdown(&listeningSocket);

                        } else {
                            // Now start listening for the clients, here process will * go in sleep mode and will wait for the incoming connection

                            struct timeval timeout;      
                            timeout.tv_sec = 5;
                            timeout.tv_usec = 1000*1000*0;

                            if (setsockopt (listeningSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
                                perror("setsockopt failed\n");


                            timeout.tv_sec = 15;
                            timeout.tv_usec = 1000*1000*0;
                            if (setsockopt (listeningSocket, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
                                perror("setsockopt failed\n");       
                              
                              
                            rc = xrt_listen((int) listeningSocket, (int) 1);

                            if (rc == 0) {
                                // printf( "[INFO] listend done. waiting for accept"); fflush(stdout);
                                App.HTTPServerState = STATE_LISTEN;
                            } else {
                                // return -1;
                            }
                        }
                    }

                    break;


                case STATE_LISTEN:
                    td1 = (uint32_t) TIMER_GET_CURRENT() - lastTimeout1;
                    td2 = (uint32_t) TIMER_GET_CURRENT() - lastTimeout2;


                    // Check for client inactivity every xxx seconds                
                    if (td1 >= tickTimeout) {
                        App.HTTPIDLECounter++;
                        // Timeout
                        if (Mode & 1) {
                            if (td1 > tickTimeout) {
                                snprintf(msg, msg_size, "[HTTP>%d]", (uint32_t) (td1 - tickTimeout));
                            } else {
                                snprintf(msg, msg_size, "[HTTP]");
                            }
                            vDisplayMessage(msg);
                        }
                        lastTimeout1 = TIMER_GET_CURRENT();
                    }

                    if (td2 >= tickToc) {
                        if (Mode & 1) {
                            // Debug Mode
                            snprintf(msg, msg_size, "H");
                            vDisplayMessage(msg);
                        }
                        // printf("[HTTP] TimeDiff:%d] \n", (int)Timer_diff( lastTimeout2, TIMER_GET_CURRENT()) );
                        lastTimeout2 = TIMER_GET_CURRENT();
                    }


                    httpSocket = xrt_accept((int) listeningSocket, NULL, NULL);

                    if ((int)httpSocket > 0) {
                        
                        App.HTTPServerState = STATE_CONNETED;
                        
                        indexRecvBuffer = 0;
                        recivError = 0;
                        maxRecivError = 255;

                        // Chiude il socket di ascolto
                        xrt_shutdown(&listeningSocket);

#ifdef DEBUG_PRINTF
                        snprintf(msg, msg_size, "[HTTP <-> %u.%u.%u.%u:%u]\n", httpSocket->dstHost[0], httpSocket->dstHost[1], httpSocket->dstHost[2], httpSocket->dstHost[3], httpSocket->dstPort);
                        vDisplayMessage(msg);
#endif

                    } else {
                        // printf("."); fflush(stdout);
                        // printf("[%d].", Timer_diff( lastTimeout1, TIMER_GET_CURRENT())); fflush(stdout);
                    }
                    break;



                case STATE_CONNETED:

start_read_as_connected:
                    
                    lastTimeout1 = TIMER_GET_CURRENT();


#ifdef USE_UI_TASK

                    App.HTTPSocket = httpSocket;
                    App.HTTPBufferToRecv = &App.recvBuffer[indexRecvBuffer];
                    App.HTTPBufferToRecvSize = App.recvBufferSize - indexRecvBuffer;
                    App.HTTPBufferToRecvResult = 0;

                    // Attesa risposta
                    while (App.HTTPBufferToRecv);

                    nReciv = App.HTTPBufferToRecvResult;

#else                    

                    // Recezione sincrona : limita le risorce del thread
                    nReciv = xrt_recv((int) httpSocket, &App.recvBuffer[indexRecvBuffer], (App.recvBufferSize - indexRecvBuffer));

#endif


                    if (nReciv < 0) {

                        // Azzera la posizione corrente
                        indexRecvBuffer = 0;
                        
                        App.HTTPServerState = STATE_CLOSING;
                        // snprintf(msg, msg_size, "[INFO] recived data FAILED\n");
                        // vDisplayMessage(msg);

                    } else if (nReciv == 0) {

                        // Azzera la posizione corrente
                        indexRecvBuffer = 0;
                        

                    } else if (nReciv > 0) {

                        // reset watchDog
                        tickTocUIKeepAlive = 0;

                        if (nReciv <= App.recvBufferSize) {
                            App.recvBuffer[indexRecvBuffer+nReciv] = 0;
                        } else {
                            vDisplayMessage("[http out of buf]");
                        }


                        /// printf( "[HTTP] recived data : %s - %u\n", (char*)App.recvBuffer, nReciv);

                        /// snprintf(msg, msg_size, "[KA:%d]", (int)dtka);
                        /// vDisplayMessage(msg);




                        nSend = App.sendBufferSize;



                        switch (handle_http_request(httpSocket, (char*) App.recvBuffer, (uint32_t) (indexRecvBuffer+nReciv), (char *) App.sendBuffer, (uint32_t *) & nSend)) {
                            case -9:
                                indexRecvBuffer += nReciv;
                                goto start_read_as_connected;
                                break;
                            case -1:
                                indexRecvBuffer = 0;
                                snprintf(msg, msg_size, "[!HTTP request]\n");
                                vDisplayMessage(msg);
                                // Invio della risposta
                                App.HTTPServerState = STATE_REPLYNG;
                                break;
                            default:
                                indexRecvBuffer = 0;
                                // Invio della risposta
                                App.HTTPServerState = STATE_REPLYNG;
                                break;
                        }

                    }
                    break;



                    ///////////////////////////////////
                    // Risposta diretta
                    //
                case STATE_REPLYNG:

                    lastTimeout1 = TIMER_GET_CURRENT();

                    if (App.sendBuffer && nSend) {


#ifdef USE_UI_TASK

                        // Utilizzo del Task dell'interfaccia utente
                        App.HTTPSocket = httpSocket;
                        App.HTTPBufferToSend = App.sendBuffer;
                        App.HTTPBufferToSendSize = nSend;
                        // Attesa completamento invio
                        while (App.HTTPBufferToSend && App.HTTPBufferToSendSize);
                        App.HTTPSocket = NULL;

                        App.HTTPServerState = STATE_CLOSING;

#else

                        nSent = xrt_send((int) httpSocket, App.sendBuffer, nSend);

                        if (nSend != nSent) {
#ifdef DEBUG_PRINTF
                            snprintf(msg, msg_size, "[INFO] send data FAILED\n");
                            vDisplayMessage(msg);
#endif
                            App.HTTPServerState = STATE_CLOSING;
                        } else {
                            // printf( "[INFO] %d bytes sent\n", nSent);
                            // App.HTTPServerState = STATE_CONNETED;
                            App.HTTPServerState = STATE_CLOSING;
                        }

#endif


                    } else {
                        // App.HTTPServerState = STATE_CONNETED;
                        App.HTTPServerState = STATE_CLOSING;
                    }
                    break;




                case STATE_CLOSING:
                    xrt_shutdown(&httpSocket);
                    xrt_shutdown(&listeningSocket);
                    if (App.HTTPRunLoop) {
                        App.HTTPServerState = STATE_INIT;
                    } else {
                        break;
                    }

                    break;
                    
                default:
                    // Stato indeterminato
                    break;

            }


            /*
            PACKET_PROCESS_SINGLE;
            // PACKET_PROCESS_MULT(12);
            Arp::driveArp( );
            Tcp::drivePackets( );
             */

            ////////////////////////////////////////
            // Processa l'input dalla tastiera
            //
            // process_keyboard_input(msg, msg_size);



            if (App.HTTPRunLoop <= 0) {
                if (httpSocket) {
                    xrt_shutdown(&httpSocket);
                }
                if (listeningSocket) {
                    xrt_shutdown(&listeningSocket);
                }
            }



        } catch (int) {
        }

        if (Mode & 2) {
            // Modalit� ritorno
            if (msg)
                free(msg);

            App.HTTPRunning = 0;

            return 0;
        }
    }


    if (!(Mode & 2)) {
        // Modalit� ritorno
        snprintf(msg, msg_size, "[HTTP] Exit...\n");
        vDisplayMessage(msg);
    }


    App.HTTPRunning = 0;

    if (msg)
        free(msg);

    return 1;
}

int HTTPServerClose(void) {
    App.HTTPRunLoop = 0;
    return 1;
}

