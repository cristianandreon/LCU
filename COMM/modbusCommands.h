#ifndef MODBUS_COMMANDS_H
   
    #ifdef EXTERN
        #define MODBUS_COMMANDS_H extern
    #else
        #define MODBUS_COMMANDS_H 
    #endif
    
    // Posizione neutrale in tabella movimenti nel driver
    #define AC_SERVO_FORWARD_POSITION_IN_TABLE      1
    #define AC_SERVO_BACKWARD_POSITION_IN_TABLE     2
    #define AC_SERVO_NEUTRAL_POSITION_IN_TABLE      3
    #define AC_SERVO_HOMING_POSITION_IN_TABLE       4

    // USO TEMPORANEO : Rele statico normalmente chiuso
    #define SSR_STATE_OFF   0
    #define SSR_STATE_ON    1

    // Trigger Auto OFF (N.B.: 30ms per i rele meccanici, 60ms per gli statici)
    // >=2 <= 121 Auto trigger OFF
    // >=122 <= 242 Auto trigger ON
    #define AUTO_OFF_TIME_MS        30
    #define AUTO_ON_TIME_MS         60
    #define SSR_STATE_ON_AND_OFF    (1+AUTO_OFF_TIME_MS)
    #define SSR_STATE_OFF_AND_ON    (122+AUTO_ON_TIME_MS)

    
    MODBUS_COMMANDS_H int32_t modbus_get_bits_by_pable_pos( void *pvActuator, int32_t position_in_table);
    MODBUS_COMMANDS_H int modbus_param_update(modbus_t *ctx, int hParam, int lParam, int newValue, bool check_mode, int sizeOf, bool PrintParam);
    
    MODBUS_COMMANDS_H int handle_modbus_homing_init(void *pvSerialSlots);
    MODBUS_COMMANDS_H int handle_modbus_homing_send(void *pvSerialSlots);
    MODBUS_COMMANDS_H int handle_modbus_homing_recv(void *pvSerialSlots);
    MODBUS_COMMANDS_H int handle_modbus_homing_done(void *pvSerialSlots);
    
    MODBUS_COMMANDS_H int handle_modbus_service_reset(void *pvSerialSlots);
    MODBUS_COMMANDS_H int handle_modbus_service_setup(void *pSerialSlots);
    MODBUS_COMMANDS_H int handle_modbus_service_setup_I(void *pSerialSlots);
    MODBUS_COMMANDS_H int handle_modbus_service_setup_II(void *pSerialSlots);
    MODBUS_COMMANDS_H int handle_modbus_service_setup_III(void *pSerialSlots);
    MODBUS_COMMANDS_H int handle_modbus_service_setup_IV(void *pSerialSlots);
    MODBUS_COMMANDS_H int handle_modbus_service_stop(void *pvSerialSlots);
    MODBUS_COMMANDS_H int handle_modbus_service_setup_speed_acc(void *pvSerialSlots);
    
    MODBUS_COMMANDS_H int handle_modbus_cmd_init(void *pvSerialSlots);
    MODBUS_COMMANDS_H int handle_modbus_cmd_init_send(void *pvSerialSlots);
    MODBUS_COMMANDS_H int handle_modbus_cmd_init_recv(void *pvSerialSlots);
    MODBUS_COMMANDS_H int handle_modbus_cmd_feedback_send(void *pvSerialSlots);
    MODBUS_COMMANDS_H int handle_modbus_cmd_feedback_recv(void *pvSerialSlots);
    MODBUS_COMMANDS_H int handle_modbus_cmd_done(void *pvSerialSlots);
    MODBUS_COMMANDS_H int handle_modbus_cmd_done_feedback(void *pvSerialSlots);
    
    MODBUS_COMMANDS_H int handle_modbus_streaming_done(void *pvSerialSlots);
    MODBUS_COMMANDS_H int handle_modbus_streaming_recv(void *pvSerialSlots);
    MODBUS_COMMANDS_H int handle_modbus_streaming_send(void *pvSerialSlots);
    MODBUS_COMMANDS_H int handle_modbus_streaming_init(void *pvSerialSlots);
    
    MODBUS_COMMANDS_H int handle_modbus_init(void *pvSerialSlots);
    MODBUS_COMMANDS_H int handle_modbus_startup (void *pvSerialSlots);

    MODBUS_COMMANDS_H int dump_ac_servo_params(void *pvSerialSlots);

    // Esportazione in C
    
    #ifdef __cplusplus
        extern "C" {
    #endif
    


    #ifdef __cplusplus
        }
    #endif
        


#endif
    
