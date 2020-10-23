
#ifndef XPROJECT_COMMAND_H
#ifdef EXTERN
    	#ifdef __cplusplus
        	#define XPROJECT_COMMAND_H extern "C"
    	#else
        	#define XPROJECT_COMMAND_H extern
    	#endif
    #else
        #define XPROJECT_COMMAND_H
    #endif




        ///////////////////////////
        // Cache delle variabili
        //
        XPROJECT_COMMAND_H cu_vars_cache *GLCUVars;
        XPROJECT_COMMAND_H unsigned int GLCUNumVarsAllocated;
        XPROJECT_COMMAND_H unsigned int GLCUNumVars;


#ifdef __cplusplus
    extern "C" {
#endif
        
        int handle_xproject_command ( uint8_t *recvBuffer, uint32_t nReciv, uint8_t *out_str, uint32_t *out_str_size );
        int logic_get_value ( uint8_t *var_name, uint8_t *out_str, uint32_t *out_str_size, int Mode );

        int initVarToCache ( );
        int addVarToCache ( void *varAddr, char typeOf, int *Id );
        int getVarFromCache ( int Id, char **str, uint32_t *str_size );

        int updateVar ( void *varAddr, char typeOf, char *newValue);
        int handleUpdateVarFailed ( char *newValue);
        int auto_size_string(char *str, int nDigits);

        

#ifdef __cplusplus
    }
#endif

    
#endif
