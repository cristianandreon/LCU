#ifndef DATA_EXCHANGE_IO_H
   
    #ifdef EXTERN
        #define DATA_EXCHANGE_IO_H extern
    #else
        #define DATA_EXCHANGE_IO_H 
    #endif
    


    DATA_EXCHANGE_IO_H int dataExchangeDumpIO ( char *msg, size_t msg_size );
    DATA_EXCHANGE_IO_H int dataExchangeInitIO ( int );
    DATA_EXCHANGE_IO_H int dataExchangeLoopIO ();
    DATA_EXCHANGE_IO_H int dataEchangeResetIO ();
    DATA_EXCHANGE_IO_H int dataEchangeStopIO();
    DATA_EXCHANGE_IO_H int dataExchangeIsRunningIO();

    DATA_EXCHANGE_IO_H int read_io_cfg();
    DATA_EXCHANGE_IO_H int update_io_cfg();

    DATA_EXCHANGE_IO_H void udpIOHandler(IpAddr_t ip, uint16_t sport, uint16_t dport, int8_t *buf, uint16_t buf_size);
    
    
    
    // Esportazione in C
    
    #ifdef __cplusplus
        extern "C" {
    #endif
    


    #ifdef __cplusplus
        }
    #endif
        


#endif
    
