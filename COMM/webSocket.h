
#ifndef WEB_SOCKET_H
    #define WEB_SOCKET_H


#ifdef __cplusplus
    extern "C" {
#endif

    int handle_websocket_handshake ( uint8_t *recvBuffer, uint32_t nReciv, uint8_t *out_str, uint32_t *out_str_size );
    int handle_websocket_data ( uint8_t *recvBuffer, uint32_t nReciv, uint32_t *indexRecvBuffer, uint8_t *out_str, uint32_t *out_str_size );
    int handle_websocket_response ( uint8_t *PayLoad, uint32_t PayLoadSize, uint8_t *webSocketHeader, uint32_t *webSocketHeaderSize );

#ifdef __cplusplus
    }
#endif

        
#endif
