#ifndef DATA_EXCHANGE_USB_H
   
    #ifdef EXTERN
        #define DATA_EXCHANGE_USB_H extern
    #else
        #define DATA_EXCHANGE_USB_H 
    #endif
    


    DATA_EXCHANGE_USB_H int dataExchangeDumpUSB ( char *msg, size_t msg_size );
    DATA_EXCHANGE_USB_H int dataExchangeInitUSB ( int Mode );
    DATA_EXCHANGE_USB_H int dataExchangeLoopUSB ( );
    DATA_EXCHANGE_USB_H int dataEchangeStopUSB();
    DATA_EXCHANGE_USB_H int dataExchangeIsRunningUSB();

    DATA_EXCHANGE_USB_H char *get_USB_status(void *pvUSBlots);

    DATA_EXCHANGE_USB_H int read_USB_cfg();
    DATA_EXCHANGE_USB_H int update_USB_cfg();

    // Esportazione in C
    
    #ifdef __cplusplus
        extern "C" {
    #endif
    


    #ifdef __cplusplus
        }
    #endif
        


#endif
    
