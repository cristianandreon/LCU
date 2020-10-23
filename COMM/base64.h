//
#ifndef BASE64_ENCODER_H

	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <stdbool.h>


    #ifdef EXTERN
    	#ifdef __cplusplus
        	#define BASE64_ENCODER_H extern "C"
    	#else
        	#define BASE64_ENCODER_H extern
    	#endif
    #else
        #define BASE64_ENCODER_H
    #endif

    BASE64_ENCODER_H void *NewBase64Decode( const char *inputBuffer,
                                              size_t length,
                                              size_t *outputLength);

    BASE64_ENCODER_H char *NewBase64Encode( const void *inputBuffer,
                                              size_t length,
                                              bool separateLines,
                                              size_t *outputLength);

#endif

