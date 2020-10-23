
#ifndef __XRT_MODBUS
    #ifdef EXTERN
        #ifdef __cplusplus
            #if defined ( __WATCOMC__ ) || defined ( __WATCOM_CPLUSPLUS__ )
                #define __XRT_MODBUS extern
            #else
                #define __XRT_MODBUS 
            #endif
        #else
            #define __XRT_MODBUS extern
        #endif
    #else
        #define __XRT_MODBUS 
    #endif

                
       
#ifdef __cplusplus
    extern "C" {
#endif

    __XRT_MODBUS int modbus_data_available(modbus_t *ctx);
    __XRT_MODBUS int modbus_purge_comm(modbus_t *ctx);
    __XRT_MODBUS int xrt_modbus_read_registers_send(modbus_t *ctx, int addr, int nb);
    __XRT_MODBUS int xrt_modbus_read_registers_recv(modbus_t *ctx, uint16_t *dest);
    __XRT_MODBUS int xrt_modbus_write_register_send(modbus_t *ctx, int addr, int value);
    __XRT_MODBUS int xrt_modbus_write_register_recv(modbus_t *ctx, uint16_t *dest);

    __XRT_MODBUS int8_t *xrt_modbus_get_last_req(modbus_t *ctx);
    __XRT_MODBUS int xrt_modbus_get_last_req_len(modbus_t *ctx);
    __XRT_MODBUS int8_t *xrt_modbus_get_last_res(modbus_t *ctx);
    __XRT_MODBUS int xrt_make_socket_non_blocking (modbus_t *ctx);

#ifdef __cplusplus
    }
#endif    
    
#endif