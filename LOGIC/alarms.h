#ifndef __ALARMS
    #ifdef EXTERN
        #ifdef __cplusplus
            #if defined ( __WATCOMC__ ) || defined ( __WATCOM_CPLUSPLUS__ )
                #define __ALARMS extern
            #else
                #define __ALARMS 
            #endif
        #else
            #define __ALARMS extern
        #endif
    #else
        #define __ALARMS 
    #endif

                
       
#ifdef __cplusplus
    extern "C" {
#endif


    __ALARMS int reset_alarms(void);
    __ALARMS int generate_alarm(char *Desc, int Code, int actuatorId, int Type, int checkDupMode);
    __ALARMS int add_alarm(LP_ALARM pAlarm, int Mode);
    __ALARMS char *serialize_alarm(uint32_t startAlarm, int typeOf);

                
       
#ifdef __cplusplus
    }
#endif    
    
#endif
