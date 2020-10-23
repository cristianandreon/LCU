#ifndef __GCODE_KEY


    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <stddef.h>
    #include <stdint.h>




    #ifdef __cplusplus
        #if defined ( __WATCOMC__ ) || defined ( __WATCOM_CPLUSPLUS__ )
            extern "C" {
        #endif
    #endif



    #ifdef EXTERN
        #ifdef __cplusplus
            #if defined ( __WATCOMC__ ) || defined ( __WATCOM_CPLUSPLUS__ )
                #define __GCODE_KEY extern
            #else
                #define __GCODE_KEY  extern "C"
            #endif
        #else
            #define __GCODE_KEY extern
        #endif
    #else
        #define __GCODE_KEY 
    #endif

       
#ifdef __cplusplus
    // extern "C" {
#endif
                
    __GCODE_KEY bool gcode_init_display_rows ( uint32_t nRows );

    __GCODE_KEY bool gcode_update_start_row ( uint32_t newStartRow );
    __GCODE_KEY bool gcode_update_no_view_row ( uint32_t newNoViewRow );
    __GCODE_KEY bool gcode_update_content ( char *GCodeRowContent );
    __GCODE_KEY bool gcode_insert_content ( char *GCodeRowContent, uint32_t rowPos );
    __GCODE_KEY bool gcode_add_content ( char *GCodeRowContent );
    __GCODE_KEY bool gcode_update_row ( char *GCodeRowContent, uint32_t rowPos );
    __GCODE_KEY bool gcode_delete_content ( uint32_t rowPos );
    __GCODE_KEY bool gcode_reset_content ( void );
    
    __GCODE_KEY bool gcode_srep_content ();
    
    __GCODE_KEY int32_t process_gcode_row ( char *GCodeRowContent, int32_t *rowOptions, uint32_t *index, void *pvGcodeCmd );
    __GCODE_KEY int32_t process_gcode_item ( char *GCodeItem, int32_t *rowOptions, void *pvGcodeCmd );
    __GCODE_KEY int32_t process_gcode_xproject ( char *GCodeItem, int32_t *rowOptions, void *pvGcodeCmd );
    __GCODE_KEY int32_t process_gcode_comment ( char *GCodeItem, int32_t *rowOptions, void *pvGcodeCmd );

    __GCODE_KEY int32_t gcode_rebuld_content_from_rows ( void );
    
    __GCODE_KEY int32_t gcode_act_linear_move ( void *pvCANSlot, void *pvActuatorX, void *pvActuatorY, void *pvActuatorZ, float targetX, float targetY, float targetZ, float feedMMMin, float precisionMM, int32_t FreeMode );   

    __GCODE_KEY int32_t gcode_act_free_linear_move(void *pvCANSlot, void *pvActuatorX, void *pvActuatorY, void *pvActuatorZ, float targetX, float targetY, float targetZ, float feedMMMin, float precisionMM );
    __GCODE_KEY int32_t gcode_act_interpolated_linear_move(void *pvCANSlot, void *pvActuatorX, void *pvActuatorY, void *pvActuatorZ, float targetX, float targetY, float targetZ, float feedMMMin, float precisionMM );
    __GCODE_KEY int32_t gcode_act_circular_move ( void *pvCANSlot, void *pvActuatorX, void *pvActuatorY, void *pvActuatorZ, float centerX, float centerY, float RadiusMM, float startAngRad, float endAngRad, float PrecisionMM, uint8_t moveType, float targetX, float targetY, float targetZ, float feedXY, float feedZ );


    __GCODE_KEY float gcode_get_mill_feed ( float default_min_feed, void *pvGcodeCmd );
    __GCODE_KEY float gcode_get_rapid_feed ( uint8_t Axis, void *pvGcodeCmd  );
    __GCODE_KEY float get_timeout_by_feed ( float deltaXYZ, float feedMMMin );
    
    __GCODE_KEY int32_t gcode_get_mapped_actuator ( uint8_t Axis, uint8_t defaultAxis, uint32_t *AxisIndex, void **ppActuator);
    
#ifdef __cplusplus
        // }
#endif

#endif
