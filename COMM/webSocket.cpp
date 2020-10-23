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


// #define DEBUG_WEBSOCKET_PACKET


/// #define DEBUG_PRINT
static char *GL_SECURITY_KEY = (char *) "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
static char *GL_SECURITY_NAME = (char *) "Sec-WebSocket-Key";

int handle_websocket_handshake(uint8_t *recvBuffer, uint32_t nReciv, uint8_t *out_str, uint32_t *out_str_size) {

    if (recvBuffer && out_str_size) {
        char *sSec_WebSocket_Key = NULL, *Sec_WebSocket_Key = strstr((char*) recvBuffer, (char*) GL_SECURITY_NAME);
        if (Sec_WebSocket_Key) {
            unsigned char *sha_str = (unsigned char *) calloc(128, 1), *sha_out_str = (unsigned char *) calloc(32, 1);
            unsigned char *sha_str_enc = NULL;
            uint32_t sha_str_enc_len = 0;
            size_t lc_out_str_size = out_str_size != NULL ? out_str_size[0] : 0;

            if (Sec_WebSocket_Key) {
                while (*Sec_WebSocket_Key != ':' && *Sec_WebSocket_Key != 0)
                    Sec_WebSocket_Key++;
            }
            if (*Sec_WebSocket_Key == ':') {
                Sec_WebSocket_Key++;
                while (*Sec_WebSocket_Key == ' ' && *Sec_WebSocket_Key != 0)
                    Sec_WebSocket_Key++;
                sSec_WebSocket_Key = Sec_WebSocket_Key;
                while (*Sec_WebSocket_Key != '\r' && *Sec_WebSocket_Key != '\n')
                    Sec_WebSocket_Key++;
                if (*Sec_WebSocket_Key == '\r' || *Sec_WebSocket_Key == '\n')
                    *Sec_WebSocket_Key = 0;
            }


            // {   char *sSec_WebSocket_Protocol = NULL, *Sec_WebSocket_Protocol = strstr(recvBuffer, "Sec-WebSocket-Protocol"); }


            // snprintf((char*)sha_str, sizeof(sha_str), "%s%s", (char*)(sSec_WebSocket_Key!=NULL?sSec_WebSocket_Key:""), (char*)"258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
            sha_str[0] = 0;
            if (sSec_WebSocket_Key)
                strncat((char*) sha_str, (char*) sSec_WebSocket_Key, (size_t) (sizeof (sha_str) - strlen((char*) sSec_WebSocket_Key)));

            strncat((char*) sha_str, (char*) GL_SECURITY_KEY, (uint32_t)sizeof (sha_str) - 40);

            sha1((const unsigned char *) sha_str, (size_t) strlen((char*) sha_str), (unsigned char*) sha_out_str);
            sha_out_str[20] = 0;

            // if (!sha_out_str[0]) printf("handle_websocket_handshake() : sha1 error!\n");

            sha_str_enc = (unsigned char*) NewBase64Encode((const void *) sha_out_str, (size_t) strlen((char*) sha_out_str), (bool)0, (size_t *) & sha_str_enc_len);


            // if (!sha_str_enc[0]) printf("handle_websocket_handshake() : NewBase64Encode error!\n");

            /// printf("sha_str_enc:%s\n", sha_str_enc);

            if (sha_out_str)
                free(sha_out_str);

            if (sha_str)
                free(sha_str);

            if (strlen((char*) sha_str_enc) + 130 > lc_out_str_size) {
                // Memoria insufficiente
                // printf("handle_websocket_handshake() : NOT enough memory!\n");
                return -1;

            } else {

                if (out_str) {
                    strncpy((char*) out_str, "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: ", lc_out_str_size);
                    strncat((char*) out_str, (char*) sha_str_enc, (lc_out_str_size - strlen((char*) sha_str_enc) - strlen((char*) out_str)));
                    strncat((char*) out_str, "\r\n\r\n", (lc_out_str_size - 4 - strlen((char*) out_str)));
                }

                if (out_str_size)
                    *out_str_size = strlen((char*) out_str);
            }


        } else {
            if (out_str)
                out_str[0] = 0;
            if (out_str_size)
                *out_str_size = 0;
        }

        return 1;
    } else {
        return -1;
    }
}







// uint8_t __far PayLoad[4096];





// Valori di Ritorno :
//  -1  ->  Errore / disconnessione
//  0   ->  Buffer completato
//  1   ->  Buffer non compleabile (continua con il cmd reciv)
//  2   ->  Disconnessione 

int handle_websocket_data(uint8_t *recvBuffer, uint32_t nReciv, uint32_t *indexRecvBuffer, uint8_t *out_str, uint32_t *out_str_size) {
    int retVal = -1;

    // Lunghezza totale dato
    if (nReciv > App.UIRecvBufferSize) {
        snprintf(App.Msg, App.MsgSize, "[Internal error on message size:%d]\n", nReciv);
        vDisplayMessage(App.Msg);
        return -1;
    }                
    
    if (nReciv >= 4) {
        uint8_t *PayLoad = NULL;
        uint16_t PayLoadSize = 0;
        int16_t websocketHeader = 0;
        int8_t *pwebsocketHeader = (int8_t*) & websocketHeader;
        uint8_t HeaderOpcode = 0;
        uint8_t HeaderPayloadLen = 0, Mask = 0;
        uint8_t Len = 0;
        uint16_t Len16 = 0;
        uint32_t Len32 = 0;
        uint8_t MaskingKey[4] = {0};
        uint8_t Opcode = 0, ReversedBits = 0;
        uint32_t originalIndexRecvBuffer = indexRecvBuffer[0], startIndex = 0;
        int i, j;
        int supportedCase = 0;


#ifdef DEBUG_PRINT
        printf("*** [INFO] handle_websocket_data:'%s' - %u\n", recvBuffer, nReciv);
#endif



        // N.B.: bit speculari rispetto alle specifiche

        memcpy(&HeaderOpcode, &recvBuffer[startIndex++], 1);
        memcpy(&HeaderPayloadLen, &recvBuffer[startIndex++], 1);
        indexRecvBuffer[0] += 2;

        Opcode = ((HeaderOpcode << 4) & 0xFF) >> 4;
        if (Opcode & 8) {
            // disconnect
        }


        /*  Da specifica RFC 6455
        
        Opcode:  4 bits

            Defines the interpretation of the "Payload data".  If an unknown
            opcode is received, the receiving endpoint MUST _Fail the
            WebSocket Connection_.  The following values are defined.

         *  %x0 denotes a continuation frame
         *  %x1 denotes a text frame
         *  %x2 denotes a binary frame
         *  %x3-7 are reserved for further non-control frames
         *  %x8 denotes a connection close
         *  %x9 denotes a ping
         *  %xA denotes a pong
         *  %xB-F are reserved for further control frames
              
         */


        Len = ((HeaderPayloadLen << 1) & 0xFF) >> 1;
        Len16 = (uint16_t) Len;

        // Mask = HeaderPayloadLen & 0x80;
        Mask = HeaderPayloadLen & 128;




        if (Opcode == 2) {
            snprintf(App.Msg, App.MsgSize, "%s[WEBSOCKET] binary frame unsupported!%s\n", (char*) ANSI_COLOR_RED, (char*) ANSI_COLOR_RESET);
            vDisplayMessage(App.Msg);
            // disconnect
            indexRecvBuffer[0] = 0;
            return 2;
        }

        if (Opcode & 8) {
            // disconnect
            indexRecvBuffer[0] = 0;
            return 2;
        }



        if (Len == 126 || Len == 127) {
            memcpy(&Len16, &recvBuffer[startIndex], 2);
            startIndex += 2;
            indexRecvBuffer[0] += 2;
            Len16 = xrt_ntohs(Len16);
            // fprintf( stderr, "[RTC] 16bit size:%d\n", Len16);
        }
        if (Len == 127) {
            memcpy(&Len32, &recvBuffer[startIndex], 4);
            startIndex += 4;
            indexRecvBuffer[0] += 4;

            // NON Supportato
            snprintf(App.Msg, App.MsgSize, "%s[WEBSOCKET] unsupported 32bit size:%d%s\n", (char*) ANSI_COLOR_RED, Len32, (char*) ANSI_COLOR_RESET);
            vDisplayMessage(App.Msg);
            Len16 = 0;

            // NOT SUPPORTED : Disconnect
            if (supportedCase = 0) {
                indexRecvBuffer[0] = 0;
                return 2;
            }
        }



        if (Mask) {
            memcpy(MaskingKey, &recvBuffer[startIndex], 4);
            startIndex += 4;
            indexRecvBuffer[0] += 4;
        }





        if (HeaderOpcode & 2 || HeaderOpcode & 4 || HeaderOpcode & 8) {

            /*  Da specifica RFC 6455
            
            RSV1, RSV2, RSV3:  1 bit each

                MUST be 0 unless an extension is negotiated that defines meanings
                for non-zero values.  If a nonzero value is received and none of
                the negotiated extensions defines the meaning of such a nonzero
                value, the receiving endpoint MUST _Fail the WebSocket
                Connection_.
             
             */
              
            // return 3;
            snprintf(App.Msg, App.MsgSize, "%s[WEBSOCKET] streaming failed at:%d -nRecv:%d - OPCODE:%d - LEN:%d LEN16:%d - MASK:%d %s\n", (char*) ANSI_COLOR_RED, originalIndexRecvBuffer, nReciv, (int) Opcode, (int) Len, (int) Len16, (int) Mask, (char*) ANSI_COLOR_RESET);
            vDisplayMessage(App.Msg);

dump_header:

            snprintf(App.Msg, App.MsgSize, " F  r  r  r  OP_CODE    - M  PAYLOAD LEN\n");
            vDisplayMessage(App.Msg);
            snprintf(App.Msg, App.MsgSize, "[0][1][2][3][4][5][6][7]-[0][1][2][3][4][5][6][7]\n");
            vDisplayMessage(App.Msg);
            snprintf(App.Msg, App.MsgSize, "[%d] %d  %d  %d [%d][%d][%d][%d]-[%d] %d  %d  %d  %d  %d  %d  %d \n",
                    HeaderOpcode & 1 ? 1 : 0, HeaderOpcode & 2 ? 1 : 0, HeaderOpcode & 4 ? 1 : 0, HeaderOpcode & 8 ? 1 : 0, HeaderOpcode & 16 ? 1 : 0, HeaderOpcode & 32 ? 1 : 0, HeaderOpcode & 64 ? 1 : 0, HeaderOpcode & 128 ? 1 : 0,
                    HeaderPayloadLen & 1 ? 1 : 0, HeaderPayloadLen & 2 ? 1 : 0, HeaderPayloadLen & 4 ? 1 : 0, HeaderPayloadLen & 8 ? 1 : 0, HeaderPayloadLen & 16 ? 1 : 0, HeaderPayloadLen & 32 ? 1 : 0, HeaderPayloadLen & 64 ? 1 : 0, HeaderPayloadLen & 128 ? 1 : 0
                    );
            vDisplayMessage(App.Msg);

            // Disconnect
            if (supportedCase = 0) {
                indexRecvBuffer[0] = 0;
                return 2;
            }

        } else {

#ifdef DEBUG_WEBSOCKET_PACKET
            if (indexRecvBuffer[0] + Len16 > nReciv) {
                snprintf(App.Msg, App.MsgSize, "%s[WEBSOCKET] partial data at:%d -nRecv:%d - OPCODE:%d - LEN:%d LEN16:%d - MASK:%d %s\n", (char*) ANSI_COLOR_RED, originalIndexRecvBuffer, nReciv, (int) Opcode, (int) Len, (int) Len16, (int) Mask, (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);
                supportedCase = 1;
                goto dump_header;
            }
#endif
        }




        if (indexRecvBuffer[0] + Len16 <= nReciv) {
            // memcpy (PayLoad, &recvBuffer[indexRecvBuffer], max(sizeof(PayLoad), Len));


            PayLoadSize = Len16 + 1;
            PayLoad = (uint8_t *) calloc(PayLoadSize, 1);
            if (!PayLoad)
                PayLoadSize = 0;


            if (Len16 <= PayLoadSize) {

                for (i = startIndex, j = 0; j < (Len16); i++, j++) {
                    if (Mask) {
                        PayLoad[j] = recvBuffer[i] ^ MaskingKey[j % 4];
                    } else {
                        PayLoad[j] = recvBuffer[i];
                    }
                }

                PayLoad[Len16] = 0;
                startIndex += Len16;
                indexRecvBuffer[0] += Len16;

                if (indexRecvBuffer[0] > App.UIRecvBufferSize) {
                    snprintf(App.Msg, App.MsgSize, "[Internal error adding Len16:%d]\n", Len16);
                    vDisplayMessage(App.Msg);
                    return -1;
                }                


                /*
                if (PayLoad[0] == '?')
                    printf("*** [INFO] handle_websocket_data PAYLOAD %dbytes:'%s'\n", (int)Len16, PayLoad);
                */

                if (handle_xproject_command((uint8_t *) & PayLoad[0], (uint16_t) Len16, out_str, out_str_size) <= 0) {
                    if (!Mask) {
                        snprintf(App.Msg, App.MsgSize, "%s[WEBSOCKET] unmasked data - size:%dbytes %s\n", (char*) ANSI_COLOR_RED, Len16, (char*) ANSI_COLOR_RESET);
                        // snprintf(App.Msg, App.MsgSize, "%s[WEBSOCKET] unmasked data - size:%dbytes - data:%s%s\n", (char*)ANSI_COLOR_RED, Len16, PayLoad, (char*)ANSI_COLOR_RESET);
                        vDisplayMessage(App.Msg);
                    }
                }


                // Gia fatto da handle_xproject_command
                // Per ottimizzazione viene sanificata solo ti tipo stringa ???
                /*
                if (out_str) sanitizeString((char*) out_str);
                */



            } else {
                snprintf(App.Msg, App.MsgSize, "%s[WEBSOCKET] undetected case %s\n", (char*) ANSI_COLOR_RED, (char*) ANSI_COLOR_RESET);
                vDisplayMessage(App.Msg);                
                *out_str_size = 0;
                retVal = -1;
            }

            if (PayLoad)
                free(PayLoad);
            PayLoad = NULL;


            PayLoadSize = 0;






            if (indexRecvBuffer[0] == nReciv) {
                // Buffer completato
                indexRecvBuffer[0] = 0;
                retVal = 0;

            } else {
                ////////////////////////////////////////////////////////////////////////////////////////////////////////////
                // Continua il parsing, NON la lettura, partendo dall'indice correntemente assegnato (indexRecvBuffer[0])
                // invia la risposta di quanto letto
                //
                *out_str_size = 0;
                retVal = 1;
            }


        } else {
            /////////////////////////////////////////////////////////////////////////
            // Dati parziali : continua la lettura ripartendo dall'indice originale
            //
            indexRecvBuffer[0] = originalIndexRecvBuffer;
            *out_str_size = 0;
            return 3;
        }


        /// printf( "*** [DEBUG] after loop %d/%d\n", indexRecvBuffer[0], nReciv);



    } else {
        /////////////////////////////////////////////////////////////////
        // Dati parziali : continua la lettura senza inviare risposta
        //

#ifdef DEBUG_PRINTF
        printf("*** [DEBUG] Partial data %d\n", nReciv);
#endif

        indexRecvBuffer[0] = nReciv;
        *out_str_size = 0;
        retVal = 3;

    }


    return retVal;
}

int handle_websocket_response(uint8_t *PayLoad, uint32_t PayLoadSize, uint8_t *webSocketHeader, uint32_t *webSocketHeaderSize) {
    if (PayLoad) {
        uint8_t HeaderOpcode = 0;
        uint8_t LengthMask = 0, Mask = 0;
        uint8_t Len = ((LengthMask << 1) >> 1);
        uint16_t extPayload = 0;
        uint32_t extPayload2 = 0;
        uint8_t MaskingKey[4] = {1, 1, 1, 1};
        uint8_t OPcode = 0x1; // string
        uint32_t cOutData = 0;


        HeaderOpcode = (1 << 7) + (0 << 6) + (0 << 5) + (0 << 4) + (OPcode << 0);
        memcpy(&webSocketHeader[cOutData], &HeaderOpcode, 1);
        cOutData += 1;

        if (PayLoadSize <= 125) {
            LengthMask = (Mask << 7) + (PayLoadSize);
            memcpy(&webSocketHeader[cOutData], &LengthMask, 1);
            cOutData += 1;
                        
        } else if (PayLoadSize > 125 && PayLoadSize < (uint16_t) 0xFFFF) {
            LengthMask = (Mask << 7) + (126);
            memcpy(&webSocketHeader[cOutData], &LengthMask, 1);
            cOutData += 1;
            extPayload = xrt_ntohs(PayLoadSize);
            memcpy(&webSocketHeader[cOutData], &extPayload, 2);
            cOutData += 2;
            /*
            } else if (PayLoadSize >= 65535) {
                LengthMask = (Mask<<7) + (127);
                memcpy(&webSocketHeader[cOutData], &LengthMask, 1); cOutData+=1;
                extPayload = 0;
                memcpy(&webSocketHeader[cOutData], &extPayload, 2); cOutData+=2;
                extPayload2 = PayLoadSize;
                memcpy(&webSocketHeader[cOutData], &extPayload2, 4); cOutData+=4;
             */
        }

        if (Mask) {
            memcpy(&webSocketHeader[cOutData], MaskingKey, 4);
            cOutData += 4;
        }

        if (*webSocketHeaderSize <= cOutData) {
            return -1;
        } else {
            *webSocketHeaderSize = cOutData;
            return PayLoadSize;
        }

    } else {
        return 0;
    }
}

