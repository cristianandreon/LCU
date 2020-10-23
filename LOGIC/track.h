#ifndef __TRACK
    #ifdef EXTERN
        #ifdef __cplusplus
            #if defined ( __WATCOMC__ ) || defined ( __WATCOM_CPLUSPLUS__ )
                #define __TRACK extern
            #else
                #define __TRACK 
            #endif
        #else
            #define __TRACK extern
        #endif
    #else
        #define __TRACK 
    #endif

                
       
#ifdef __cplusplus
    extern "C" {
#endif


    __TRACK int start_track ( LP_ACTUATOR pActuator, int resolution_ms, uint32_t max_tick );
    __TRACK int record_track ( LP_ACTUATOR pActuator );
    __TRACK int end_track ( LP_ACTUATOR pActuator );


       
#ifdef __cplusplus
    }
#endif    
    
#endif