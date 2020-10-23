#ifndef DATA_EXCHANGE_CAN_H
   
    #ifdef EXTERN
        #define DATA_EXCHANGE_CAN_H extern
    #else
        #define DATA_EXCHANGE_CAN_H 
    #endif
    

    DATA_EXCHANGE_CAN_H int xrt_can_message_send(void *pvCANSlots, uint8_t CANID, int16_t Index, int16_t SubIndex, int8_t *Data, int8_t DataSize, bool writeMode);

    DATA_EXCHANGE_CAN_H char *get_canbus_status(void *pvCANSlot);
    
    DATA_EXCHANGE_CAN_H int dataExchangeDumpCAN ( char *msg, size_t msg_size );
    DATA_EXCHANGE_CAN_H int dataExchangeInitCAN ( int Mode );
    DATA_EXCHANGE_CAN_H int dataExchangeLoopCAN ( );
    DATA_EXCHANGE_CAN_H int dataEchangeStopCAN();
    DATA_EXCHANGE_CAN_H int dataExchangeIsRunningCAN();

    DATA_EXCHANGE_CAN_H char *get_CAN_status(void *pvCANlots);

    DATA_EXCHANGE_CAN_H int read_canbus_cfg();
    DATA_EXCHANGE_CAN_H int update_canbus_cfg();

    DATA_EXCHANGE_CAN_H void udpCANHandler(IpAddr_t ip, uint16_t sport, uint16_t dport, int8_t *buf, uint16_t buf_size);
    
    DATA_EXCHANGE_CAN_H char *get_interpolation_error_desc( int32_t errorCode, int32_t targetPosX, int32_t targetPosY, int32_t targetPosZ);
    
    // Esportazione in C
    
    #ifdef __cplusplus
        extern "C" {
    #endif
    


    #ifdef __cplusplus
        }
    #endif
        


#endif
    
