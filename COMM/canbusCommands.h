#ifndef CANBUS_COMMANDS_H
   
    #ifdef EXTERN
        #define CANBUS_COMMANDS_H extern
    #else
        #define CANBUS_COMMANDS_H 
    #endif
    

    #define MAX_CAN_MESSAGE_SIZE    256

    #define CONTROL_WORD_RESET  -1

    #define CONTROL_WORD_QUICK_STOP -2


    CANBUS_COMMANDS_H int32_t xrt_can_message_send(void *pvCANSlots, uint8_t CANID, int16_t Index, int16_t SubIndex, int8_t *Data, int8_t DataSize, bool writeMode);
    CANBUS_COMMANDS_H int32_t xrt_can_message_recv(void *pvCANSlots, char *Data, int32_t DataSize);
    CANBUS_COMMANDS_H int32_t canbus_data_available( void* pCANSlot);
    CANBUS_COMMANDS_H char *get_CAN_status(void *pvCANSlots);
    
    CANBUS_COMMANDS_H int32_t handle_canbus_send_sinlge_command(void *pvCANSlot, LP_ACTUATOR pActuator, int32_t tposition, int32_t cspeed, int32_t acc_ms, int32_t dec_ms, int32_t folowingErrPulses);
    CANBUS_COMMANDS_H int32_t handle_canbus_start_multiple_command(void *pvCANSlot);
    CANBUS_COMMANDS_H int32_t handle_canbus_add_multiple_command(void *pvCANSlot, LP_ACTUATOR pActuator, int32_t tposition, float start_angle, float end_angle, float radius, int32_t axisSinCosLin, int32_t direction, int32_t precision_nsteps, int32_t period_msec, float feed_pulses_min, int32_t cspeed, int32_t acc_ms, int32_t dec_ms, int32_t folowingErrPulses);
    CANBUS_COMMANDS_H int32_t handle_canbus_send_multiple_command(void *pvCANSlot);
    
    
    CANBUS_COMMANDS_H int32_t handle_canbus_homing_init(void *pvCANSlot);
    CANBUS_COMMANDS_H int32_t handle_canbus_homing_send(void *pvCANSlot);
    CANBUS_COMMANDS_H int32_t handle_canbus_homing_recv(void *pvCANSlot);
    CANBUS_COMMANDS_H int32_t handle_canbus_homing_done(void *pvCANSlot);
    
    CANBUS_COMMANDS_H int32_t handle_canbus_service_first_setup(void *pvCANSlot);
    CANBUS_COMMANDS_H int32_t handle_canbus_service_setup_speed_acc(void *pvCANSlot);
    CANBUS_COMMANDS_H int32_t handle_canbus_service_setup(void *pvCANSlot);    
    CANBUS_COMMANDS_H int32_t handle_canbus_service_stop(void *pvCANSlot);
    CANBUS_COMMANDS_H int32_t handle_canbus_service_out(void *pvCANSlot);
    CANBUS_COMMANDS_H int32_t handle_canbus_service_reset(void *pvCANSlot);
    CANBUS_COMMANDS_H int32_t handle_canbus_service(void *pvCANSlot);
    
    CANBUS_COMMANDS_H int32_t handle_canbus_cmd_init(void *pvCANSlot);
    CANBUS_COMMANDS_H int32_t handle_canbus_cmd_init_send(void *pvCANSlot);
    CANBUS_COMMANDS_H int32_t handle_canbus_cmd_init_recv(void *pvCANSlot);
    CANBUS_COMMANDS_H int32_t handle_canbus_cmd_feedback_send(void *pvCANSlot);
    CANBUS_COMMANDS_H int32_t handle_canbus_cmd_feedback_recv(void *pvCANSlot);
    CANBUS_COMMANDS_H int32_t handle_canbus_cmd_done(void *pvCANSlot);
    
    CANBUS_COMMANDS_H int32_t handle_canbus_streaming_done(void *pvCANSlot);
    CANBUS_COMMANDS_H int32_t handle_canbus_streaming_recv(void *pvCANSlot);
    CANBUS_COMMANDS_H int32_t handle_canbus_streaming_send(void *pvCANSlot);
    CANBUS_COMMANDS_H int32_t handle_canbus_streaming_init(void *pvCANSlot);
    CANBUS_COMMANDS_H int32_t handle_canbus_streaming_post_act(void *pvCANSlot);
    CANBUS_COMMANDS_H int32_t handle_canbus_streaming_post_wait(void *pvCANSlot);

        
    CANBUS_COMMANDS_H int32_t handle_canbus_init(void *pvCANSlot);
    CANBUS_COMMANDS_H int32_t handle_canbus_startup (void *pvCANSlot);

    
    CANBUS_COMMANDS_H char *get_emc_error( int errorData );

    // Esportazione in C
    
    #ifdef __cplusplus
        extern "C" {
    #endif
    


    #ifdef __cplusplus
        }
    #endif
        


#endif
    
