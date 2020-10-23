#ifndef RTLINUX_UTILITY_H_

    #ifdef EXTERN
    	#ifdef __cplusplus
        	#define RTLINUX_UTILITY_H_ extern "C"
    	#else
        	#define RTLINUX_UTILITY_H_ extern
    	#endif
    #else
        #define RTLINUX_UTILITY_H_
    #endif


    RTLINUX_UTILITY_H_ int get_cur_module_path ( char *pBuf, int pBufSize );
    RTLINUX_UTILITY_H_ void strip_file_name ( char *source, char *dir, uint32_t dir_size, char *file, uint32_t file_size, int Mode );
    RTLINUX_UTILITY_H_ void urldecode2(const char *src, char *dst, int dst_size);
    RTLINUX_UTILITY_H_ int http_request(char *url, char *method, char *post, char *filename, int *outSize, char *outErr, int outErrSize);
    RTLINUX_UTILITY_H_ float cal_arc_length( float StartAngleRad, float EndAngleRad, float Radius, int Direction);

    #endif
