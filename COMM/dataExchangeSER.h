#ifndef DATA_EXCHANGE_SER_H
   
    #ifdef EXTERN
        #define DATA_EXCHANGE_SER_H extern
    #else
        #define DATA_EXCHANGE_SER_H 
    #endif
    


    DATA_EXCHANGE_SER_H int dataExchangeDumpSerial ( char *msg, size_t msg_size );
    DATA_EXCHANGE_SER_H int dataExchangeInitSerial ( int Mode );
    DATA_EXCHANGE_SER_H int dataExchangeLoopSerial ( );
    DATA_EXCHANGE_SER_H int dataEchangeStopSerial();
    DATA_EXCHANGE_SER_H int dataExchangeIsRunningSerial();

    DATA_EXCHANGE_SER_H int handle_modbus_connect(SerialSlot *serialSlots, uint32_t slotIndex, char *device, int baud, char parity, int data_bit, int stop_bit);
    DATA_EXCHANGE_SER_H int handle_serial_borad_init_error(int i);

    DATA_EXCHANGE_SER_H char *get_serial_status(void *pvSerialSlots);

    DATA_EXCHANGE_SER_H int read_serial_cfg();
    DATA_EXCHANGE_SER_H int update_serial_cfg();

    DATA_EXCHANGE_SER_H uint32_t GLSERMeasurePostTimeout;
    
    // Esportazione in C
    
    #ifdef __cplusplus
        extern "C" {
    #endif
    


    #ifdef __cplusplus
        }
    #endif
        


#endif
    
