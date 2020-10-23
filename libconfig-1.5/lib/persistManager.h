
#ifndef PERSIST_MANAGER_H
   
    #ifdef EXTERN
        #define PERSIST_MANAGER_H extern
    #else
        #define PERSIST_MANAGER_H 
    #endif




    #ifdef __cplusplus
        extern "C" {
    #endif
    
    int read_settings(void);
    int write_settings(void);


    #ifdef __cplusplus
        }
    #endif
    

#endif