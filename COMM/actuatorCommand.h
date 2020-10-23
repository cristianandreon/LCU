
#ifndef ACTUATOR_COMMAND_H

    #ifdef EXTERN
        #ifdef __cplusplus
            #define ACTUATOR_COMMAND_H extern "C"
        #else
            #define ACTUATOR_COMMAND_H extern
        #endif
    #else
        #define ACTUATOR_COMMAND_H
    #endif


    // Numero letture consecutive zero
    #define NUM_ZERO_SPEED_TO_HOMING    25


    ACTUATOR_COMMAND_H int process_actuators_initialize (int32_t doHoming);
    ACTUATOR_COMMAND_H int process_actuator_initialize(void *pvActuator, int32_t doHoming);
    
    ACTUATOR_COMMAND_H int process_actuators_terminate(void);
    ACTUATOR_COMMAND_H int process_actuator_terminate(void *pvActuator);
    ACTUATOR_COMMAND_H int process_actuator_homing_request(void *pvActuator);
        
    ACTUATOR_COMMAND_H void do_acctuator_mirror ();
    
    ACTUATOR_COMMAND_H void update_actuator_virtual_pos(void *pvActuator);    
    ACTUATOR_COMMAND_H int actuator_position_to_encoder (void *pvActuator, float rpos, int32_t *pTargetTurns, int32_t *pTargetPulses);
    ACTUATOR_COMMAND_H int actuator_encoder_to_position (void *pvActuator, int32_t targetTurns, int32_t targetPulses, float *rpos);
    
    ACTUATOR_COMMAND_H int actuator_delta_pulse(void *pvActuator, int32_t turns2, int32_t turns, int32_t pulses2, int32_t pulses, int32_t *turnsOut, int32_t *pulsesOut, int Mode);
    ACTUATOR_COMMAND_H int actuator_add_pulse_ppt(void *pvActuator, int32_t turns, int32_t pulses, int32_t *turnsOut, int32_t *pulsesOut, int Mode);
    
    ACTUATOR_COMMAND_H int actuator_ppt_to_ppo(void *pvActuator, int32_t turnsPPT, int32_t pulsesPPT, int32_t *tpurnsPPO, int32_t *pulsesPPO);
    ACTUATOR_COMMAND_H int actuator_ppo_to_ppt(void *pvActuator, int32_t turnsPPO, int32_t pulsesPPO, int32_t *tpurnsPPT, int32_t *pulsesPPT);

    
    ACTUATOR_COMMAND_H int actuator_ppt_to_ppo(void *pvActuator, int32_t pulsesPPT, int32_t turnsPPT, int32_t *pulsesPPO, int32_t *tpurnsPPO);
    ACTUATOR_COMMAND_H int actuator_ppo_to_ppt(void *pvActuator, int32_t pulsesPPO, int32_t turnsPPO, int32_t *pulsesPPT, int32_t *tpurnsPPT);

    ACTUATOR_COMMAND_H int actuator_set_working_offset(void *pvActuator, float workingOffset);
    
    ACTUATOR_COMMAND_H int actuator_handle_read_position(void *pvActuator, float newPosition, bool initialRead);
    ACTUATOR_COMMAND_H int actuator_speed_to_linear(void *pvActuator, float value, float *newValue);
    ACTUATOR_COMMAND_H int actuator_linear_to_speed(void *pvActuator, float value, float *newValue);

    ACTUATOR_COMMAND_H char *get_actuator_step(void *pvActuator);
    ACTUATOR_COMMAND_H char *get_actuator_driver_status(void *pvActuator);
    ACTUATOR_COMMAND_H char *get_actuator_pos_desc(void *pvActuator, int position);
    ACTUATOR_COMMAND_H int actuator_set_aux_io(void *pvActuator, int newValue1, int newValue2);
    
    ACTUATOR_COMMAND_H int process_actuators_move_loop ( void );

    ACTUATOR_COMMAND_H float get_timeout_by_stroke ( void *pvActuator, float stroke_mm, float speed_rpm );
    
    ACTUATOR_COMMAND_H int handle_set_as_pending_command(void *pvActuator, bool *okToNextStep);
    
    ACTUATOR_COMMAND_H int handle_actuator_send_move_cmd ( void *pActuator, int Mode );
    
    ACTUATOR_COMMAND_H int32_t handle_actuator_position_reached ( void *pvActuator );
    
    ACTUATOR_COMMAND_H int handle_actuator_position_mode (void *pvActuator, int32_t targetTurns, int32_t targetPulses, int32_t speed_rpm, int32_t acc_ms, int32_t dec_ms, int32_t targetTurns2, int32_t targetPulses2, int32_t speed_rpm2, int32_t acc_ms2, int32_t dec_ms2 , int32_t homingTurns, int32_t homingPulses, int32_t homing_speed_rpm);
    ACTUATOR_COMMAND_H int handle_actuator_speed_mode ( void *pvActuator, int speed );
    ACTUATOR_COMMAND_H int handle_actuator_servo_on ( void *pvActuator );
    ACTUATOR_COMMAND_H int handle_actuator_servo_off ( void *pvActuator );
    ACTUATOR_COMMAND_H int handle_actuator_servo_stop ( void *pvActuator );
    ACTUATOR_COMMAND_H int handle_actuator_prepare_for_run(void *pvActuator, int position_in_table );

    // debug
    ACTUATOR_COMMAND_H int handle_actuator_test (void *pvActuator, bool resetAll, bool posTest, bool speedTest , bool stopMotor, bool standbyMotor);

    ACTUATOR_COMMAND_H int handle_actuator_sevo_on_off ( void *pvActuator, int mode );
    ACTUATOR_COMMAND_H int handle_actuator_reset_pos_table ( void *pvActuator, int mode );
    ACTUATOR_COMMAND_H int get_actuator_control_mode(void *pvActuator);


        
    
#endif
