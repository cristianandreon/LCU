
#ifndef HTTP_REQUEST_H

#ifdef EXTERN
#define HTTP_REQUEST_H extern
#else
#define HTTP_REQUEST_H
#endif



#if (__cplusplus && WATCOM)
extern "C" {
#endif

    extern int handle_http_request(TcpSocket httpSocket, char *recvBuffer, uint32_t nReciv, char *out_str, uint32_t *out_str_size);
    extern int handle_http_default_header(char *out_str, uint32_t *out_str_size, char *PtrOrigin);
    extern int handle_http_default_header_response(char *out_str, uint32_t *out_str_size, char *PtrOrigin, char *PtrHeaderMessage);
    extern int handle_http_empty_header_response(char *out_str, uint32_t *out_str_size);
    extern int handle_http_update_firmware_form(char *out_str, uint32_t *out_str_size);
    
    extern int handle_http_options_request(TcpSocket httpSocket, char *ptrURL, uint32_t nReciv, char *out_str, uint32_t *out_str_size, char* PtrOrigin);
    extern int handle_http_get_request(TcpSocket httpSocket, char *ptrURL, uint32_t nReciv, char *out_str, uint32_t *out_str_size, char *PtrOrigin);
    extern int handle_http_put_request(TcpSocket httpSocket, char *HTTPRequest, char *requesBody, uint32_t nReciv, char *out_str, uint32_t *out_str_size, char *PtrOrigin);
    extern int handle_http_post_request(TcpSocket httpSocket, char *HTTPRequest, char *requesBody, uint32_t nReciv, char *out_str, uint32_t *out_str_size, char *PtrOrigin, char *RequestingHost, char *ContentType, char *Boundary, char *ContentLen);

#if (__cplusplus && WATCOM)
}
#endif


#endif
