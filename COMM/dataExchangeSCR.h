#ifndef DATA_EXCHANGE_SCR_H
   
    #ifdef EXTERN
        #define DATA_EXCHANGE_SCR_H extern
    #else
        #define DATA_EXCHANGE_SCR_H 
    #endif
    


    DATA_EXCHANGE_SCR_H int dataExchangeDumpSCR ( char *msg, size_t msg_size );
    DATA_EXCHANGE_SCR_H int dataExchangeInitSCR ( int Mode );
    DATA_EXCHANGE_SCR_H int dataExchangeLoopSCR ( );
    DATA_EXCHANGE_SCR_H int dataEchangeStopSCR();
    DATA_EXCHANGE_SCR_H int dataExchangeIsRunningSCR();

    DATA_EXCHANGE_SCR_H int read_scr_cfg();
    DATA_EXCHANGE_SCR_H int update_scr_cfg();

    DATA_EXCHANGE_SCR_H void udpSCRHandler(IpAddr_t ip, uint16_t sport, uint16_t dport, int8_t *buf, uint16_t buf_size);

    
    
    // Esportazione in C
    
    #ifdef __cplusplus
        extern "C" {
    #endif
    


    #ifdef __cplusplus
        }
    #endif
        


#endif
    
