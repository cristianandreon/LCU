#ifndef HTTP_SERVER_H

        
    #ifdef EXTERN        
        #define HTTP_SERVER_H extern
    #else
        #define HTTP_SERVER_H 
    #endif

        HTTP_SERVER_H int HTTPServerInit ( int );
        HTTP_SERVER_H int HTTPServerLoop ( int );
        HTTP_SERVER_H int HTTPServerClose ();
        HTTP_SERVER_H int HTTPServerLoop2( int Mode );
    

        #ifdef __cplusplus
            extern "C" {
        #endif
        

        #ifdef __cplusplus
                }
        #endif

#endif
