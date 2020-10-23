#define EXTERN


/////////////////////////
// RT Kernel includes
//
#ifdef WATCOM
#include "FreeRTOS.h"
#include "task.h"
#else
#include "./../RTLinux/RTLinux.h"
#endif




#define DEBUG_PRINTF

enum http_method_enum {
    METHOD_GET = 1,
    METHOD_POST,
    METHOD_HEAD,
    METHOD_PUT,
    METHOD_DELETE,
    METHOD_LINK,
    METHOD_UNLINK,
    METHOD_SHARING,
    METHOD_OPTIONS
};

typedef struct _tagMethodTable {
    enum http_method_enum Method;
    MDB_CHAR *String;

} HTTP_METHOD_TABLE;


static HTTP_METHOD_TABLE MethodTable[] = {
    { METHOD_GET, (char*) "GET"},
    { METHOD_POST, (char*) "POST"},
    { METHOD_HEAD, (char*) "HEAD"},
    { METHOD_PUT, (char*) "PUT"},
    { METHOD_DELETE, (char*) "DELETE"},
    { METHOD_LINK, (char*) "LINK"},
    { METHOD_UNLINK, (char*) "UNLINK"},
    { METHOD_SHARING, (char*) "SHARE"},
    { METHOD_OPTIONS, (char*) "OPTIONS"},
};

/*
HTTP/1.0

Method         = "OPTIONS"                ; Section 9.2
                        | "GET"                    ; Section 9.3
                        | "HEAD"                   ; Section 9.4
                        | "POST"                   ; Section 9.5
                        | "PUT"                    ; Section 9.6
                        | "DELETE"                 ; Section 9.7
                        | "TRACE"                  ; Section 9.8
                        | "CONNECT"                ; Section 9.9
                        | extension-method
 */

static const uint32_t MethodTableLen = sizeof (MethodTable) / sizeof (MethodTable[0]);


#define IDS_400_MESSAGE         "<HEAD><TITLE>400 Badly Formed Request</TITLE></HEAD>\n<BODY><H1>400 Badly Formed Request</H1>\nThe request had bad syntax or was inherently impossible to be satisfied.<BR>\n</BODY>\r\n"
#define IDS_500_MESSAGE         "<HEAD><TITLE>500 Unknown Server Error</TITLE></HEAD>\n<BODY><H1>500 Unknown Server Error</H1>\nUnknown server error.<BR>\n</BODY>\r\n"
#define IDS_404_MESSAGE         "<HEAD><TITLE>404 Not Found</TITLE></HEAD>\n<BODY><H1>404 Not Found</H1>\nThe requested URL was not found on this server.<BR>\n</BODY>\r\n"
#define IDS_405_MESSAGE         "<HEAD><TITLE>405 Unknown Method</TITLE></HEAD>\n<BODY><H1>405 Unknown Method</H1>\nThe requested method is not supported on this server.<BR>\n</BODY>\r\n"
#define IDS_503_MESSAGE         "<HEAD><TITLE>503 Server Capacity Reached</TITLE></HEAD>\n<BODY><H1>503 Server Capacity Reached</H1>\nServer capacity reached. Try again in a few seconds..<BR>\n</BODY>\r\n"
#define IDS_TAGSTRING           "<H2></H2><HR><P>\nSent via the LS HTTP Server.\r\n"
#define IDS_TAGCOMMENT          "<! Sent via the MultiDB HTTP Server.>\r\n"

static struct _tagMsgTable {
    int id;
    char *Desc;
    char *idStr;

} MsgTable[] = {
    { 100, (char*) "Continue", (char*) ""},
    { 101, (char*) "Switching Protocols", (char*) ""},
    { 200, (char*) "OK", (char*) ""},
    { 201, (char*) "Created", (char*) ""},
    { 202, (char*) "Accepted", (char*) ""},
    { 203, (char*) "Non-Authoritative Information", (char*) ""},
    { 204, (char*) "No Content", (char*) ""},
    { 205, (char*) "Reset Content", (char*) ""},
    { 206, (char*) "Partial Content", (char*) ""},
    { 300, (char*) "Multiple Choices", (char*) ""},
    { 301, (char*) "Moved Permanently", (char*) ""},
    { 302, (char*) "Found", (char*) ""},
    { 303, (char*) "See Other", (char*) ""},
    { 304, (char*) "Not Modified", (char*) ""},
    { 305, (char*) "Use Proxy", (char*) ""},
    { 307, (char*) "Temporary Redirect", (char*) ""},
    { 400, (char*) "Badly Formed Request", (char*) IDS_400_MESSAGE},
    { 401, (char*) "Unauthorized", (char*) ""},
    { 402, (char*) "Payment Required", (char*) ""},
    { 403, (char*) "Forbidden", (char*) ""},
    { 404, (char*) "Not Found", (char*) IDS_404_MESSAGE},
    { 405, (char*) "Unknown Method", (char*) IDS_405_MESSAGE},
    { 406, (char*) "Not Acceptable", (char*) ""},
    { 407, (char*) "Proxy Authentication Required", (char*) ""},
    { 408, (char*) "Request Time-out", (char*) ""},
    { 409, (char*) "Conflict", (char*) ""},
    { 410, (char*) "Gone", (char*) ""},
    { 411, (char*) "Length Required", (char*) ""},
    { 412, (char*) "Precondition Failed", (char*) ""},
    { 413, (char*) "Request Entity Too Large", (char*) ""},
    { 414, (char*) "Request-URI Too Large", (char*) ""},
    { 415, (char*) "Unsupported Media Type", (char*) ""},
    { 416, (char*) "Requested range not satisfiable", (char*) ""},
    { 417, (char*) "Expectation Failed", (char*) ""},
    { 501, (char*) "Not Implemented", (char*) ""},
    { 502, (char*) "Bad Gateway", (char*) ""},
    { 504, (char*) "Gateway Time-out", (char*) ""},
    { 505, (char*) "HTTP Version not supported", (char*) ""},
    { 500, (char*) "Internal Server Error", (char*) IDS_500_MESSAGE},
    { 503, (char*) "Server Capacity Reached", (char*) IDS_503_MESSAGE}
};


static const uint32_t MsgTableSize = sizeof (MsgTable) / sizeof (MsgTable[0]);

static struct _tagMIME_Table {
    MDB_CHAR *ext;
    MDB_CHAR *type;

} MIMETable[] = {


    { (char*) ".txt", (char*) "text/plain"},
    { (char*) ".ls", (char*) "application/ls"},
    { (char*) ".xml", (char*) "text/xml"},

    { (char*) ".htm", (char*) "text/html"},
    { (char*) ".html", (char*) "text/html"},
    { (char*) ".gif", (char*) "image/gif"},
    { (char*) ".pdf", (char*) "application/pdf"},

    { (char*) ".pot", (char*) "application/vnd.ms-powerpoint"},
    { (char*) ".ppm", (char*) "image/x-portable-pixmap"},
    { (char*) ".pps", (char*) "application/vnd.ms-powerpoint"},
    { (char*) ".ppt", (char*) "application/vnd.ms-powerpoint"},
    { (char*) ".tif", (char*) "image/tiff"},
    { (char*) ".tiff", (char*) "image/tiff"},
    { (char*) ".jpe", (char*) "image/jpeg"},
    { (char*) ".jpeg", (char*) "image/jpeg"},
    { (char*) ".jpg", (char*) "image/jpeg"},
    { (char*) ".jpgx", (char*) "image/jpeg"},
    { (char*) ".doc", (char*) "application/msword"},
    { (char*) ".dot", (char*) "application/msword"},
    { (char*) ".bmp", (char*) "image/bmp"},
    { (char*) ".png", (char*) "image/png"},

    { (char*) ".xla", (char*) "application/vnd.ms-excel"},
    { (char*) ".xlc", (char*) "application/vnd.ms-excel"},
    { (char*) ".xlm", (char*) "application/vnd.ms-excel"},
    { (char*) ".xls", (char*) "application/vnd.ms-excel"},
    { (char*) ".xlt", (char*) "application/vnd.ms-excel"},
    { (char*) ".xlw", (char*) "application/vnd.ms-excel"},

    { (char*) ".323", (char*) "text/h323"},
    { (char*) ".acx", (char*) "application/internet-property-stream"},
    { (char*) ".ai", (char*) "application/postscript"},
    { (char*) ".aif", (char*) "audio/x-aiff"},
    { (char*) ".aifc", (char*) "audio/x-aiff"},
    { (char*) ".aiff", (char*) "audio/x-aiff"},
    { (char*) ".asf", (char*) "video/x-ms-asf"},
    { (char*) ".asr", (char*) "video/x-ms-asf"},
    { (char*) ".asx", (char*) "video/x-ms-asf"},
    { (char*) ".au", (char*) "audio/basic"},
    { (char*) ".avi", (char*) "video/x-msvideo"},
    { (char*) ".axs", (char*) "application/olescript"},
    { (char*) ".bas", (char*) "text/plain"},
    { (char*) ".bcpio", (char*) "application/x-bcpio"},
    { (char*) ".bin", (char*) "application/octet-stream"},
    { (char*) ".c", (char*) "text/plain"},
    { (char*) ".cat", (char*) "application/vnd.ms-pkiseccat"},
    { (char*) ".cdf", (char*) "application/x-cdf"},
    { (char*) ".cer", (char*) "application/x-x509-ca-cert"},
    { (char*) ".class", (char*) "application/octet-stream"},
    { (char*) ".clp", (char*) "application/x-msclip"},
    { (char*) ".cmx", (char*) "image/x-cmx"},
    { (char*) ".cod", (char*) "image/cis-cod"},
    { (char*) ".cpio", (char*) "application/x-cpio"},
    { (char*) ".crd", (char*) "application/x-mscardfile"},
    { (char*) ".crl", (char*) "application/pkix-crl"},
    { (char*) ".crt", (char*) "application/x-x509-ca-cert"},
    { (char*) ".csh", (char*) "application/x-csh"},
    { (char*) ".css", (char*) "text/css"},
    { (char*) ".dcr", (char*) "application/x-director"},
    { (char*) ".der", (char*) "application/x-x509-ca-cert"},
    { (char*) ".dir", (char*) "application/x-director"},
    { (char*) ".dll", (char*) "application/x-msdownload"},
    { (char*) ".dms", (char*) "application/octet-stream"},
    { (char*) ".dvi", (char*) "application/x-dvi"},
    { (char*) ".dxr", (char*) "application/x-director"},
    { (char*) ".eps", (char*) "application/postscript"},
    { (char*) ".etx", (char*) "text/x-setext"},
    { (char*) ".evy", (char*) "application/envoy"},
    { (char*) ".exe", (char*) "application/octet-stream"},
    { (char*) ".fif", (char*) "application/fractals"},
    { (char*) ".flr", (char*) "x-world/x-vrml"},
    { (char*) ".gtar", (char*) "application/x-gtar"},
    { (char*) ".gz", (char*) "application/x-gzip"},
    { (char*) ".h", (char*) "text/plain"},
    { (char*) ".hdf", (char*) "application/x-hdf"},
    { (char*) ".hlp", (char*) "application/winhlp"},
    { (char*) ".hqx", (char*) "application/mac-binhex40"},
    { (char*) ".hta", (char*) "application/hta"},
    { (char*) ".htc", (char*) "text/x-component"},
    { (char*) ".htt", (char*) "text/webviewhtml"},
    { (char*) ".ico", (char*) "image/x-icon"},
    { (char*) ".ief", (char*) "image/ief"},
    { (char*) ".iii", (char*) "application/x-iphone"},
    { (char*) ".ins", (char*) "application/x-internet-signup"},
    { (char*) ".isp", (char*) "application/x-internet-signup"},
    { (char*) ".jfif", (char*) "image/pipeg"},
    { (char*) ".js", (char*) "application/x-javascript"},
    { (char*) ".latex", (char*) "application/x-latex"},
    { (char*) ".lha", (char*) "application/octet-stream"},
    { (char*) ".lsf", (char*) "video/x-la-asf"},
    { (char*) ".lsx", (char*) "video/x-la-asf"},
    { (char*) ".lzh", (char*) "application/octet-stream"},
    { (char*) ".m13", (char*) "application/x-msmediaview"},
    { (char*) ".m14", (char*) "application/x-msmediaview"},
    { (char*) ".m3u", (char*) "audio/x-mpegurl"},
    { (char*) ".man", (char*) "application/x-troff-man"},
    { (char*) ".mdb", (char*) "application/x-msaccess"},
    { (char*) ".me", (char*) "application/x-troff-me"},
    { (char*) ".mht", (char*) "message/rfc822"},
    { (char*) ".mhtml", (char*) "message/rfc822"},
    { (char*) ".mid", (char*) "audio/mid"},
    { (char*) ".mny", (char*) "application/x-msmoney"},
    { (char*) ".mov", (char*) "video/quicktime"},
    { (char*) ".movie", (char*) "video/x-sgi-movie"},
    { (char*) ".mp2", (char*) "video/mpeg"},
    { (char*) ".mp3", (char*) "audio/mpeg"},
    { (char*) ".mpa", (char*) "video/mpeg"},
    { (char*) ".mpe", (char*) "video/mpeg"},
    { (char*) ".mpeg", (char*) "video/mpeg"},
    { (char*) ".mpg", (char*) "video/mpeg"},
    { (char*) ".mpp", (char*) "application/vnd.ms-project"},
    { (char*) ".mpv2", (char*) "video/mpeg"},
    { (char*) ".ms", (char*) "application/x-troff-ms"},
    { (char*) ".mvb", (char*) "application/x-msmediaview"},
    { (char*) ".nws", (char*) "message/rfc822"},
    { (char*) ".oda", (char*) "application/oda"},
    { (char*) ".p10", (char*) "application/pkcs10"},
    { (char*) ".p12", (char*) "application/x-pkcs12"},
    { (char*) ".p7b", (char*) "application/x-pkcs7-certificates"},
    { (char*) ".p7c", (char*) "application/x-pkcs7-mime"},
    { (char*) ".p7m", (char*) "application/x-pkcs7-mime"},
    { (char*) ".p7r", (char*) "application/x-pkcs7-certreqresp"},
    { (char*) ".p7s", (char*) "application/x-pkcs7-signature"},
    { (char*) ".pbm", (char*) "image/x-portable-bitmap"},
    { (char*) ".pfx", (char*) "application/x-pkcs12"},
    { (char*) ".pgm", (char*) "image/x-portable-graymap"},
    { (char*) ".pko", (char*) "application/ynd.ms-pkipko"},
    { (char*) ".pma", (char*) "application/x-perfmon"},
    { (char*) ".pmc", (char*) "application/x-perfmon"},
    { (char*) ".pml", (char*) "application/x-perfmon"},
    { (char*) ".pmr", (char*) "application/x-perfmon"},
    { (char*) ".pmw", (char*) "application/x-perfmon"},
    { (char*) ".pnm", (char*) "image/x-portable-anymap"},
    { (char*) ".prf", (char*) "application/pics-rules"},
    { (char*) ".ps", (char*) "application/postscript"},
    { (char*) ".pub", (char*) "application/x-mspublisher"},
    { (char*) ".qt", (char*) "video/quicktime"},
    { (char*) ".ra", (char*) "audio/x-pn-realaudio"},
    { (char*) ".ram", (char*) "audio/x-pn-realaudio"},
    { (char*) ".ras", (char*) "image/x-cmu-raster"},
    { (char*) ".rgb", (char*) "image/x-rgb"},
    { (char*) ".rmi", (char*) "audio/mid"},
    { (char*) ".roff", (char*) "application/x-troff"},
    { (char*) ".rtf", (char*) "application/rtf"},
    { (char*) ".rtx", (char*) "text/richtext"},
    { (char*) ".scd", (char*) "application/x-msschedule"},
    { (char*) ".sct", (char*) "text/scriptlet"},
    { (char*) ".setpay", (char*) "application/set-payment-initiation"},
    { (char*) ".setreg", (char*) "application/set-registration-initiation"},
    { (char*) ".sh", (char*) "application/x-sh"},
    { (char*) ".shar", (char*) "application/x-shar"},
    { (char*) ".sit", (char*) "application/x-stuffit"},
    { (char*) ".snd", (char*) "audio/basic"},
    { (char*) ".spc", (char*) "application/x-pkcs7-certificates"},
    { (char*) ".spl", (char*) "application/futuresplash"},
    { (char*) ".src", (char*) "application/x-wais-source"},
    { (char*) ".sst", (char*) "application/vnd.ms-pkicertstore"},
    { (char*) ".stl", (char*) "application/vnd.ms-pkistl"},
    { (char*) ".stm", (char*) "text/html"},
    { (char*) ".svg", (char*) "image/svg+xml"},
    { (char*) ".sv4cpio", (char*) "application/x-sv4cpio"},
    { (char*) ".sv4crc", (char*) "application/x-sv4crc"},
    { (char*) ".swf", (char*) "application/x-shockwave-flash"},
    { (char*) ".t", (char*) "application/x-troff"},
    { (char*) ".tar", (char*) "application/x-tar"},
    { (char*) ".tcl", (char*) "application/x-tcl"},
    { (char*) ".tex", (char*) "application/x-tex"},
    { (char*) ".texi", (char*) "application/x-texinfo"},
    { (char*) ".texinfo", (char*) "application/x-texinfo"},
    { (char*) ".tgz", (char*) "application/x-compressed"},
    { (char*) ".tr", (char*) "application/x-troff"},
    { (char*) ".trm", (char*) "application/x-msterminal"},
    { (char*) ".tsv", (char*) "text/tab-separated-values"},
    { (char*) ".txt", (char*) "text/plain"},
    { (char*) ".uls", (char*) "text/iuls"},
    { (char*) ".ustar", (char*) "application/x-ustar"},
    { (char*) ".vcf", (char*) "text/x-vcard"},
    { (char*) ".vrml", (char*) "x-world/x-vrml"},
    { (char*) ".wav", (char*) "audio/x-wav"},
    { (char*) ".wcm", (char*) "application/vnd.ms-works"},
    { (char*) ".wdb", (char*) "application/vnd.ms-works"},
    { (char*) ".wks", (char*) "application/vnd.ms-works"},
    { (char*) ".wmf", (char*) "application/x-msmetafile"},
    { (char*) ".wps", (char*) "application/vnd.ms-works"},
    { (char*) ".wri", (char*) "application/x-mswrite"},
    { (char*) ".wrl", (char*) "x-world/x-vrml"},
    { (char*) ".wrz", (char*) "x-world/x-vrml"},
    { (char*) ".xaf", (char*) "x-world/x-vrml"},
    { (char*) ".xbm", (char*) "image/x-xbitmap"},
    { (char*) ".xof", (char*) "x-world/x-vrml"},
    { (char*) ".xpm", (char*) "image/x-xpixmap"},
    { (char*) ".xwd", (char*) "image/x-xwindowdump"},
    { (char*) ".z", (char*) "application/x-compress"},
    { (char*) ".zip", (char*) "application/zip"}
};


static const uint32_t MIMETableLen = sizeof (MIMETable) / sizeof (MIMETable[0]);






#define SOCKET_SEND_DATA()  \
    if ((uint32_t)strlen ((char*)out_str) >= App.sendBufferSize) {\
        snprintf(msg, msg_size, (char*)"[!http size:%u/%u]\n", (uint32_t)strlen ((char*)out_str), App.sendBufferSize);\
        vDisplayMessage((char*)msg);\
        return -1;\
    }\
    if (httpSocket) {\
        nSend = strlen((char*)out_str);\
        \
        nSent = xrt_send( (int)httpSocket, (void*)out_str, (uint32_t)nSend );\
        \
        if (nSend != nSent) {\
            return -2;\
        } else {\
            out_str[0] = 0;\
            out_str_size[0] = 0;\
        }\
    } 




/// #define DEBUG_PRINT

int handle_http_request(TcpSocket httpSocket, char *HTTPRequest, uint32_t nReciv, char *out_str, uint32_t *out_str_size) {
    MDB_CHAR **HttpRequestList = NULL;
    uint32_t NumHttpRequestList = 0, NumHttpRequestListAllocated = 0;

    MDB_CHAR **SubRequestList = NULL;
    uint32_t NumSubRequestList = 0, NumSubRequestListAllocated = 0;

    uint32_t j = 0, i = 0, i_req = 0, HTTPRequestSize = 0, NumHTTPRequest = 0, NumHTTPRequestAllocated = 0;
    uint32_t headerSize = 0, sizeContent = 0;


    MDB_CHAR *CGIString = NULL;
    uint32_t CGIStringAllocated = 0;

    MDB_CHAR **CGIStringArray = NULL;
    uint32_t NumCGIStringArray = 0, NumCGIStringArrayAllocated = 0;

    MDB_CHAR *PostContent = NULL;
    uint32_t PostContentAllocated = 0;

    MDB_CHAR **PostContentArray = NULL;
    uint32_t NumPostContentArray = 0, NumPostContentArrayAllocated = 0;

    MDB_CHAR PtrMethod[256];
    int RequestMethod = 0;

    MDB_CHAR HTTPResponse[256];

    BOOL bSendResponse = TRUE;

    MDB_CHAR UserAgent[256];
    MDB_CHAR RequestingHost[256];
    MDB_CHAR ContentType[256];
    MDB_CHAR ConnectionKey[256];
    MDB_CHAR KeepAliveKey[256];
    MDB_CHAR ContentLen[256];
    MDB_CHAR PtrVersion[256];
    MDB_CHAR PtrOrigin[256];

    MDB_CHAR LocalFullFileName[512];
    MDB_CHAR LocalFileName[512];
    MDB_CHAR LocalFileExt[256];
    MDB_CHAR Boundary[256];

    MDB_CHAR *PtrURI = NULL;
    int PtrURILen = 0;

    MDB_CHAR *BasePath = (MDB_CHAR *) "/share/";
    MDB_CHAR *requestBody = NULL;

    char msg[256];
    int msg_size = sizeof (msg);

    int retVal = 0;


    out_str[0] = 0;
    

    Boundary[0] = 0;

    if (HTTPRequest && out_str_size) {

        /*
        vDisplayMessage(ANSI_COLOR_CYAN);         
        vDisplayMessage(HTTPRequest);
        vDisplayMessage(ANSI_COLOR_RESET);
         */

        retVal = 1;

        if (out_str_size[0] < 6800) {
            // Memoria insufficiente
            // printf("handle_websocket_handshake() : NOT enough memory!\n");
            return -1;

        } else {



#define MAX_FILES   255

            if (out_str) {

                // Processa la richiesta
                if (HTTPRequest) {
                    MDB_CHAR *CurBuffer = (MDB_CHAR *) NULL;
                    MDB_CHAR *PtrBuffer = (MDB_CHAR *) HTTPRequest;
                    MDB_CHAR PrevBuffer = 0;

                    requestBody = strstr(HTTPRequest, "\r\n\r\n");
                    if (!requestBody)
                        requestBody = strstr(HTTPRequest, "\r\r\r\r");
                    if (!requestBody)
                        requestBody = strstr(HTTPRequest, "\n\n\n\n");
                    if (!requestBody)
                        requestBody = strstr(HTTPRequest, "\n\r\n\r");

                    if (requestBody) {
                        // *requestBody = 0;
                        requestBody += 4;
                    }


                    for (CurBuffer = strpbrk(PtrBuffer, (char*) "\r"); CurBuffer; CurBuffer = strpbrk(PtrBuffer, (char*) "\r")) {
                        PrevBuffer = *CurBuffer;
                        *CurBuffer = '\0';

                        // add to list
                        // Mode & 1	->	Aggiungi solo se inesistente
                        if (add_array_to_string_array(PtrBuffer, &HttpRequestList, &NumHttpRequestList, &NumHttpRequestListAllocated, NULL, 0 + 0) < 0) {
                        }

                        *CurBuffer = PrevBuffer;

                        *CurBuffer++;
                        *CurBuffer++;

                        // point to next text
                        PtrBuffer = CurBuffer;

                        if (PtrBuffer >= requestBody) {
                            break;
                        }
                    }
                }



                // Parse the primary, and most important, request line
                // Parse the request line into a list of tokens
                for (i_req = 0; i_req < NumHttpRequestList; i_req++) {

                    MDB_CHAR *pTempString = NULL;
                    MDB_CHAR *PtrRequestBuffer = NULL;
                    MDB_CHAR *CurRequestBuffer = NULL;



                    NumSubRequestList = 0;

                    COPY_POINTER(pTempString, HttpRequestList[i_req]);

                    PtrRequestBuffer = pTempString;

                    for (CurRequestBuffer = strpbrk(PtrRequestBuffer, (char*) " "); CurRequestBuffer; CurRequestBuffer = strpbrk(PtrRequestBuffer, (char*) " ")) {
                        *CurRequestBuffer = '\0';
                        if (add_array_to_string_array(PtrRequestBuffer, &SubRequestList, &NumSubRequestList, &NumSubRequestListAllocated, NULL, 0 + 0) < 0) {
                        }
                        *CurRequestBuffer++;
                        PtrRequestBuffer = CurRequestBuffer;
                    }

                    if (add_array_to_string_array(PtrRequestBuffer, &SubRequestList, &NumSubRequestList, &NumSubRequestListAllocated, NULL, 0 + 0) < 0) {
                    }



                    if (i_req == 0) {

                        if (!HttpRequestList[i_req]) {

                            // SendCanned Msg(0, 400, NULL);

                        } else {
                            // 1) parse the method
                            COPY_ARRAY(PtrMethod, SubRequestList[0]);
                            RequestMethod = 0;
                            for (i = 0; i < MethodTableLen; i++) {
                                if (check_stri(MethodTable[i].String, PtrMethod) == 0) {
                                    RequestMethod = MethodTable[i].Method;
                                    break;
                                }
                            }

                            // 2) parse the URI
                            if (NumSubRequestList > 1) {
                                uint32_t PtrURISize = strlen(SubRequestList[1]) + 32;
                                PtrURI = (char*) realloc(PtrURI, PtrURISize);

                                COPY_POINTER(PtrURI, SubRequestList[1]);

                                /*
                                if (InternetCanonicalizeUrl(SubRequestList[1], PtrURI, &PtrURISize, ICU_DECODE | ICU_NO_ENCODE)) {
                                } else {
                                }
                                 */

                                if (!PtrURI[0]) {
                                    // m_pDoc->DbgVMessage ( "Null request URI\nSending 400 error\n" ) ;
                                    // SendCannedMsg(0, 400, NULL);
                                }

                                // replace UNIX '/' with MS '\'
                                GET_POINTER_LEN(PtrURI, PtrURILen);
#ifndef IPAD_COMPILE
                                for (i = 0; i < PtrURILen; i++) {
                                    if (PtrURI[i] == '/') {
                                        PtrURI[i] = DEFAULT_FOLDER_SEP;
                                    }
                                }
#endif
                            } else {
                                FREE_POINTER(PtrURI);
                            }

                            if (NumSubRequestList > 2) {
                                COPY_ARRAY(PtrVersion, SubRequestList[2]);
                            } else {
                                COPY_ARRAY(PtrVersion, (char*) "HTTP/0.9");
                            }
                        }



                    } else if (i_req >= 1) {
                        if (NumSubRequestList > 0) {
                            MDB_CHAR *PtrRequestType = SubRequestList[0];
                            if (check_stri(PtrRequestType, (char*) "Accept:") == 0) {
                            } else if (check_stri(PtrRequestType, (char*) "Accept-Charset:") == 0) {
                            } else if (check_stri(PtrRequestType, (char*) "Accept-Encoding:") == 0) {
                            } else if (check_stri(PtrRequestType, (char*) "Accept-Language:") == 0) {
                            } else if (check_stri(PtrRequestType, (char*) "Authorization:") == 0) {
                            } else if (check_stri(PtrRequestType, (char*) "Expect:") == 0) {
                            } else if (check_stri(PtrRequestType, (char*) "From:") == 0) {
                            } else if (check_stri(PtrRequestType, (char*) "Host:") == 0) {
                                if (NumSubRequestList > 1) {
                                    COPY_ARRAY(RequestingHost, SubRequestList[1]);
                                }
                            } else if (check_stri(PtrRequestType, (char*) "If-Match:") == 0) {
                            } else if (check_stri(PtrRequestType, (char*) "If-Modified-Since:") == 0) {
                            } else if (check_stri(PtrRequestType, (char*) "If-Modified-Match:") == 0) {
                            } else if (check_stri(PtrRequestType, (char*) "If-Range:") == 0) {
                            } else if (check_stri(PtrRequestType, (char*) "If-Unmodified-Since:") == 0) {
                            } else if (check_stri(PtrRequestType, (char*) "Max-Forwards:") == 0) {
                            } else if (check_stri(PtrRequestType, (char*) "Proxy-Authorization:") == 0) {
                            } else if (check_stri(PtrRequestType, (char*) "Range:") == 0) {
                            } else if (check_stri(PtrRequestType, (char*) "Referer:") == 0) {
                            } else if (check_stri(PtrRequestType, (char*) "TE:") == 0) {
                            } else if (check_stri(PtrRequestType, (char*) "User-Agent:") == 0) {
                                if (NumSubRequestList > 1) {
                                    COPY_ARRAY(UserAgent, SubRequestList[1]);
                                    COPY_ARRAY(UserAgent, HttpRequestList[i_req]);
                                }
                            } else if (check_stri(PtrRequestType, (char*) "Connection:") == 0) {
                                if (NumSubRequestList > 1) {
                                    COPY_ARRAY(ConnectionKey, SubRequestList[1]);
                                }
                            } else if (check_stri(PtrRequestType, (char*) "Keep-Alive:") == 0) {
                                if (NumSubRequestList > 1) {
                                    COPY_ARRAY(KeepAliveKey, SubRequestList[1]);
                                }
                            } else if (check_stri(PtrRequestType, (char*) "Content-length:") == 0) {
                                COPY_ARRAY(ContentLen, SubRequestList[1]);
                            } else if (check_stri(PtrRequestType, (char*) "Content-Type:") == 0) {
                                if (NumSubRequestList > 1) {
                                    COPY_ARRAY(ContentType, SubRequestList[1]);
                                    if (NumSubRequestList > 2) {
                                        char *strBoundary = strstr(SubRequestList[2], "boundary");
                                        if (strBoundary) {
                                            while (strBoundary && *strBoundary != '=')
                                                strBoundary++;
                                            if (*strBoundary == '=')
                                                strBoundary++;
                                            while (*strBoundary == ' ')
                                                strBoundary++;
                                        }
                                        // COPY_ARRAY(Boundary, "--");
                                        ADD_ARRAY(Boundary, strBoundary);
                                    }
                                }
                            } else if (check_stri(PtrRequestType, (char*) "Origin:") == 0) {
                                COPY_ARRAY(PtrOrigin, SubRequestList[1]);
                            }
                        }
                    }
                }



                // Add base path
                COPY_ARRAY(LocalFullFileName, (char*) BasePath);
                APPEND_CHAR_IF_NOT(LocalFullFileName, DEFAULT_FOLDER_SEP, sizeof (BasePath));



                /*
                ___________________________________________

                Identificazione stringa CGI
                ___________________________________________																																																	*/

                COPY_ARRAY(LocalFileName, ((MDB_CHAR*) & PtrURI[0]));
                for (i = 0; i < PtrURILen; i++) {
                    if (PtrURI[i] == '?') {
                        PtrURI[i] = 0;
                        COPY_ARRAY(LocalFileName, ((MDB_CHAR*) & PtrURI[0]));
                        CpyStr(&CGIString, ((MDB_CHAR*) & PtrURI[i + 1]), &CGIStringAllocated);
                        PtrURI[i] = '?';
                    }
                }

                if (LocalFileName[1]) {
                    STRIP_FIRST_USER_CHAR(LocalFileName, DEFAULT_FOLDER_SEP, sizeof (LocalFileName));
                    STRIP_FIRST_USER_CHAR(LocalFileName, '/', sizeof (LocalFileName));
                    ADD_ARRAY(LocalFullFileName, LocalFileName);
                }

                
                
                

                ///////////////////////////////
                // Imposta la risposta HTTP
                //
                if (handle_http_default_header(out_str, out_str_size, PtrOrigin) <= 0) {
                }
                
                
                
                
                /////////////////////////////////////////////
                // Protocollo HTTP GET
                //
                if (RequestMethod == METHOD_GET) {
                    if (handle_http_get_request(httpSocket, (char*) LocalFileName, nReciv, out_str, out_str_size, PtrOrigin) < 0) {
                        retVal = -1;
                    }


                    /////////////////////////////////////////////
                    // Protocollo HTTP METHOD_POST
                    //
                } else if (RequestMethod == METHOD_POST) {

                    /*
                    vDisplayMessage(ANSI_COLOR_CYAN);
                    vDisplayMessage(HTTPRequest);
                    vDisplayMessage(ANSI_COLOR_RESET);
                     */

                    switch (handle_http_post_request(httpSocket, HTTPRequest, requestBody, nReciv, out_str, out_str_size, PtrOrigin, RequestingHost, ContentType, Boundary, ContentLen)) {
                        case -9:
                            retVal = -9;
                            break;
                        case -1:
                            retVal = -1;
                            break;
                        case 0:
                            retVal = 0;
                            break;
                        default:
                            retVal = 1;
                            break;
                    }


                    /////////////////////////////////////////////
                    // Protocollo HTTP PUT
                    //
                } else if (RequestMethod == METHOD_PUT) {

                    /*
                    vDisplayMessage(ANSI_COLOR_CYAN);         
                    vDisplayMessage(HTTPRequest);
                    vDisplayMessage(ANSI_COLOR_RESET);
                     */

                    if (handle_http_put_request(httpSocket, HTTPRequest, requestBody, nReciv, out_str, out_str_size, PtrOrigin) < 0) {
                        retVal = -1;
                    }


                    /////////////////////////////////////////////
                    // Protocollo HTTP OPTIONS
                    //
                } else if (RequestMethod == METHOD_OPTIONS) {
                    if (handle_http_options_request(httpSocket, HTTPRequest, nReciv, out_str, out_str_size, PtrOrigin) < 0) {
                        retVal = -1;
                    }

                } else {
                    snprintf(msg, msg_size, (char*) "[http method not supported:%s]\n", PtrMethod);
                    vDisplayMessage(msg);
                }
            }



            // Imposta i dati da inviare                
            if (out_str_size)
                *out_str_size = strlen((char*) out_str);
        }




    } else {
        if (out_str)
            out_str[0] = 0;
        if (out_str_size)
            *out_str_size = 0;
    }



end_func:

    free_fields_array(&HttpRequestList, NumHttpRequestListAllocated);

    free_fields_array(&SubRequestList, NumSubRequestListAllocated);

    free_fields_array(&CGIStringArray, NumCGIStringArrayAllocated);

    free_fields_array(&PostContentArray, NumPostContentArrayAllocated);

    return retVal;
}

int handle_http_empty_header_response(char *out_str, uint32_t *out_str_size) {
    uint32_t sendBufferSize = out_str_size[0];
    char msg[512];
    uint32_t msg_size = sizeof (msg);

    strncpy((char*) out_str, (char*) "HTTP/1.1 200\nServer:xProject REALTIME\nConnection:Close\nContent-type:text/html; charset=utf-8\n\n", App.sendBufferSize);
}


int handle_http_default_header(char *out_str, uint32_t *out_str_size, char *PtrOrigin) {
    if (out_str_size) {
        uint32_t sendBufferSize = out_str_size[0];
        char msg[512];
        uint32_t msg_size = sizeof (msg);

        if (out_str) {
            strncpy((char*) out_str, (char*) "HTTP/1.1 200\nConnection:Close\nContent-type:text/html; charset=utf-8", App.sendBufferSize - 64);
            strncat((char*) out_str, (char*) "\r\nAllow: OPTIONS, GET, POST", (App.sendBufferSize - 64 - strlen((char*) out_str)));
            strncat((char*) out_str, (char*) "\r\nServer: xProject Realtime HTTP server", (App.sendBufferSize - 64 - strlen((char*) out_str)));
            strncat((char*) out_str, (char*) "\r\nAccess-Control-Allow-Origin: ", (App.sendBufferSize - 64 - strlen((char*) out_str)));
            strncat((char*) out_str, (char*) (PtrOrigin ? PtrOrigin : ""), (App.sendBufferSize - 64 - strlen((char*) out_str)));
            strncat((char*) out_str, (char*) "\r\nHeader set Access-Control-Allow-Headers:Origin, X-Requested-With, Content-Type, Accept", (App.sendBufferSize - 64 - strlen((char*) out_str)));
            strncat((char*) out_str, (char*) "\r\n\r\n", (App.sendBufferSize - 32 - strlen((char*) out_str)));
            strncat((char*) out_str, (char*) "<head><style type=\"text/css\">.cuHTTP { font-family:tahoma font-size:15px; } table { border: 1px solid #D0E0F9; cellpadding:0; cellspacing:0; } td { border: 1px solid #F7F7F7; padding:10px; font-family: 'Roboto', 'calibri', sans-serif; } .r {text-align:right;} .c {text-align:center;} .onHttpImg {background-color:darkGreen;} .offHttpImg {background-color:darkRed;} </style></head>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
        }
    }
    return 1;
}



int handle_http_default_header_response(char *out_str, uint32_t *out_str_size, char *PtrOrigin, char *PtrHeaderMessage) {
    uint32_t sendBufferSize = out_str_size[0];
    char msg[512];
    uint32_t msg_size = sizeof (msg);


    if (!App.Initialized) {
        strncat((char*) out_str, (char*) "<body onload=\"", (App.sendBufferSize - 32 - strlen((char*) out_str)));
        // Loop refresh pagina
        strncat((char*)out_str, (char*)"javascript:setTimeout('window.location.reload()',500)", (App.sendBufferSize - 32 - strlen((char*)out_str)) );
        strncat((char*)out_str, (char*)"\">", (App.sendBufferSize - 32 - strlen((char*)out_str)) );
    }

    if (PtrHeaderMessage)
        strncat((char*) out_str, (char*)PtrHeaderMessage, (App.sendBufferSize - strlen(PtrHeaderMessage) - strlen((char*) out_str)));

    strncat((char*) out_str, (char*) "<center><table style=\"padding:17px; width:--0%\" ><tr><td style=\"vertical-align: top;\">", (App.sendBufferSize - 255 - strlen((char*) out_str)));
    strncat((char*) out_str, (char*) "<table width=600 >", (App.sendBufferSize - 255 - strlen((char*) out_str)));
    strncat((char*) out_str, (char*) "<tr style=\"background-color:lightGray;\"><td colspan=3><h2>xProject Realtime - ver. ", (App.sendBufferSize - 255 - strlen((char*) out_str)));
    snprintf(msg, msg_size, (char*) "%d.%2d", App.MajVer, App.MinVer);
    strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
    strncat((char*) out_str, (char*) "</h2> ", (App.sendBufferSize - 4 - strlen((char*) out_str)));
    strncat((char*) out_str, (char*) __DATE__, (App.sendBufferSize - 16 - strlen((char*) out_str)));
    strncat((char*) out_str, (char*) "</td></tr>", (App.sendBufferSize - 32 - strlen((char*) out_str)));


    if (App.Initialized) {
        strncat((char*) out_str, (char*) "<tr><td><a href=\"/FIRMWARE\"><b>FIRMWARE</b></a></td><td>FIRMWARE Manager</td><td></td></tr>", (App.sendBufferSize - 32 - strlen((char*) out_str)));
        strncat((char*) out_str, (char*) "<tr><td><a href=\"/IO\"><b>IO</b></a></td><td>IO Board status</td><td></td></tr>", (App.sendBufferSize - 32 - strlen((char*) out_str)));
        strncat((char*) out_str, (char*) "<tr><td><a href=\"/STAT\"><b>STAT</b></a></td><td>Statistics</td><td></td></tr>", (App.sendBufferSize - 32 - strlen((char*) out_str)));
        strncat((char*) out_str, (char*) "<tr><td><a href=\"/RESET_RTC\"><b>RESET_RTC</b></a></td><td>Reset the RealTime Communicator</td><td></td></tr>", (App.sendBufferSize - 32 - strlen((char*) out_str)));
        strncat((char*) out_str, (char*) "<tr><td><a href=\"javascript:void(0)\" onClick=\"if (confirm('Warning : this command will reboot the Contol Unit...\\n\\nProceed now ?')) location.href='/REBOOT'; \"><b>REBOOT</b></a></td><td>Reboot the system (machine must be in STOP)</td><td></td></tr>", (App.sendBufferSize - 32 - strlen((char*) out_str)));
        strncat((char*) out_str, (char*) "<tr><td><a href=\"javascript:void(0)\" onClick=\"if (confirm('Warning : this command will remove canbus config CANBUS.cfg file...\\n\\nProceed now ?')) location.href='/RESET_CAN'; \"><b>RESET CANBUS</b></a></td><td>Reset CANBUS Boards config (machine must be in STOP)</td><td></td></tr>", (App.sendBufferSize - 32 - strlen((char*) out_str)));
        strncat((char*) out_str, (char*) "<tr><td><a href=\"javascript:void(0)\" onClick=\"if (confirm('Warning : this command will remove serial posrt config SER.cfg file...\\n\\nProceed now ?')) location.href='/RESET_SER'; \"><b>RESET SERIAL</b></a></td><td>Reset SERIAL Boards config (machine must be in STOP)</td><td></td></tr>", (App.sendBufferSize - 32 - strlen((char*) out_str)));
        strncat((char*) out_str, (char*) "<tr><td><a href=\"javascript:void(0)\" onClick=\"if (confirm('Warning : this command will remove I/O config IO.cfg file...\\n\\nProceed now ?')) location.href='/RESET_IO'; \"><b>RESET I/O</b></a></td><td>Reset I/O Boards config (machine must be in STOP)</td><td></td></tr>", (App.sendBufferSize - 32 - strlen((char*) out_str)));
        strncat((char*) out_str, (char*) "<tr><td><a href=\"javascript:void(0)\" onClick=\"if (confirm('Warning : this command will remove SCR config SCR.cfg file...\\n\\nProceed now ?')) location.href='/RESET_SCR'; \"><b>RESET SCR</b></a></td><td>Reset SCR Boards config (machine must be in STOP)</td><td></td></tr>", (App.sendBufferSize - 32 - strlen((char*) out_str)));
        strncat((char*) out_str, (char*) "<tr><td><a href=\"javascript:void(0)\" onClick=\"if (confirm('Warning : this command will remove USB config USB.cfg file...\\n\\nProceed now ?')) location.href='/RESET_USB'; \"><b>RESET USB</b></a></td><td>Reset USB Boards config (machine must be in STOP)</td><td></td></tr>", (App.sendBufferSize - 32 - strlen((char*) out_str)));
        strncat((char*) out_str, (char*) "<tr><td colspan=2 style=\"border-bottom:1px solid lightGray;\"></td></tr>", (App.sendBufferSize - 32 - strlen((char*) out_str)));
        strncat((char*) out_str, (char*) "<tr><td><a href=\"javascript:void(0)\" onClick=\"\"></a><b>Connected UI</b></td><td>", (App.sendBufferSize - 32 - strlen((char*) out_str)) );
        if (App.UIConnectedIp[0]) {
            snprintf(msg, msg_size, "<span style=\"color:darkGreen;\"><b>%s:%u</b></span>", App.UIConnectedIp, (uint16_t) App.UIConnectedPort);
        } else {
            snprintf(msg, msg_size, "<span style=\"color:darkGray;\">none</span>");
        }
        strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg)) - strlen((char*) out_str));
        strncat((char*) out_str, (char*) "</td><td></td></tr>", (App.sendBufferSize - 32 - strlen((char*) out_str)));
        
        
    } else {
        strncat((char*) out_str, (char*) "<tr><td colspan=2 class=\"c\" style=\"font-size:32px;\">Initializing Application</td></tr>", (App.sendBufferSize - 32 - strlen((char*) out_str)));
        if (!App.CANOK) {
            strncat((char*) out_str, (char*) "<tr><td class=\"r\">CANBUS</td><td style=\"color:Orange\">PENDING</td></tr>", (App.sendBufferSize - 32 - strlen((char*) out_str)));
        } else {
            strncat((char*) out_str, (char*) "<tr><td class=\"r\">CANBUS</td><td style=\"color:darkGreen\">OK</td></tr>", (App.sendBufferSize - 32 - strlen((char*) out_str)));            
        }
        if (!App.SEROK) {
            strncat((char*) out_str, (char*) "<tr><td class=\"r\">SERIAL</td><td style=\"color:Orange\">PENDING</td></tr>", (App.sendBufferSize - 32 - strlen((char*) out_str)));
        } else {
            strncat((char*) out_str, (char*) "<tr><td class=\"r\">SERIAL</td><td style=\"color:darkGreen\">OK</td></tr>", (App.sendBufferSize - 32 - strlen((char*) out_str)));            
        }        
        if (!App.IOOK) {
            strncat((char*) out_str, (char*) "<tr><td class=\"r\">I/O</td><td style=\"color:Orange\">PENDING</td></tr>", (App.sendBufferSize - 32 - strlen((char*) out_str)));
        } else {
            strncat((char*) out_str, (char*) "<tr><td class=\"r\">I/O</td><td style=\"color:darkGreen\">OK</td></tr>", (App.sendBufferSize - 32 - strlen((char*) out_str)));            
        }        
        if (!App.SCROK) {
            strncat((char*) out_str, (char*) "<tr><td class=\"r\">SCR</td><td style=\"color:Orange\">PENDING</td></tr>", (App.sendBufferSize - 32 - strlen((char*) out_str)));
        } else {
            strncat((char*) out_str, (char*) "<tr><td class=\"r\">SCR</td><td style=\"color:darkGreen\">OK</td></tr>", (App.sendBufferSize - 32 - strlen((char*) out_str)));            
        }        
        if (!App.USBOK) {
            strncat((char*) out_str, (char*) "<tr><td class=\"r\">USB</td><td style=\"color:Orange\">PENDING</td></tr>", (App.sendBufferSize - 32 - strlen((char*) out_str)));
        } else {
            strncat((char*) out_str, (char*) "<tr><td class=\"r\">USB</td><td style=\"color:darkGreen\">OK</td></tr>", (App.sendBufferSize - 32 - strlen((char*) out_str)));            
        }        
    }
    
    
    strncat((char*) out_str, (char*) "</table></center><br/>", (App.sendBufferSize - 32 - strlen((char*) out_str)));

    
    /*
    strncat((char*) out_str, (char*) "</td><td style=\"vertical-align: top;\">", (App.sendBufferSize - 255 - strlen((char*) out_str)));
    */
    
    strncat((char*) out_str, (char*) "</td></tr>", (App.sendBufferSize - 255 - strlen((char*) out_str)));

    strncat((char*) out_str, (char*) "</table>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
    
    return 1;
}




int handle_http_options_request(TcpSocket httpSocket, char *ptrURL, uint32_t nReciv, char *out_str, uint32_t *out_str_size, char* PtrOrigin) {

    uint32_t sendBufferSize = out_str_size[0];
    // char msg[256];

    // snprintf(msg, sizeof(msg), (char*) "%d.%d.%d.%d:%d", (int) MyIpAddr[0], (int) MyIpAddr[1], (int) MyIpAddr[2], (int) MyIpAddr[3], 8080);

    strncpy((char*) out_str, (char*) "HTTP/1.1 200 OK", (App.sendBufferSize - 32 - strlen((char*) out_str)));
    strncat((char*) out_str, (char*) "\r\nAllow: OPTIONS, GET, POST", (App.sendBufferSize - 64 - strlen((char*) out_str)));
    strncat((char*) out_str, (char*) "\r\nServer: xProject Realtime HTTP server", (App.sendBufferSize - 64 - strlen((char*) out_str)));
    strncat((char*) out_str, (char*) "\r\nAccess-Control-Allow-Origin: ", (App.sendBufferSize - 64 - strlen((char*) out_str)));
    strncat((char*) out_str, (char*) PtrOrigin ? PtrOrigin : "", (App.sendBufferSize - 64 - strlen((char*) out_str)));
    strncat((char*) out_str, (char*) "\r\nHeader set Access-Control-Allow-Headers:Origin, X-Requested-With, Content-Type, Accept", (App.sendBufferSize - 64 - strlen((char*) out_str)));

    // strncat((char*) out_str, (char*) "\r\nAccess-Control-Allow-Headers: X-PINGOTHER, Content-Type", (App.sendBufferSize - 64 - strlen((char*) out_str)));
    // strncat((char*) out_str, (char*) "\r\nAccess-Control-Allow-Methods: POST, GET, OPTIONS", (App.sendBufferSize - 64 - strlen((char*) out_str)));
    // strncat((char*) out_str, (char*) "\r\nContent-Length: 0", (App.sendBufferSize - 64 - strlen((char*) out_str)));
    strncat((char*) out_str, (char*) "\r\n\r\n", (App.sendBufferSize - 32 - strlen((char*) out_str)));

    return 1;
}

int handle_http_get_request(TcpSocket httpSocket, char *ptrURL, uint32_t nReciv, char *out_str, uint32_t *out_str_size, char *PtrOrigin) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint32_t sendBufferSize = out_str_size[0];
    uint32_t nSend = 0, nSent = 0;
    uint32_t msg_size = 512;
    char *msg = (char *) calloc(msg_size, 1);
    int i, j;

    int retVal = 1;


    if (strnicmp((char*) ptrURL, (char*) "FILE", 4) == 0) {

        DIR *dir = NULL;
        struct dirent *ent = NULL;



        // struct find_t fileinfo = {0};
        int nFiles = 0;

        int done = 0; // findfirst( "\\SHARE\\*.*", _A_NORMAL, &fileinfo);


        // Intestazione predefinita risposta
        handle_http_default_header_response(out_str, out_str_size, PtrOrigin, NULL);

        ///////////////////////////////////////////
        // Invio dati
        //
        SOCKET_SEND_DATA();

        if ((dir = opendir("/share/*.*")) != NULL) {

            strncat((char*) out_str, (char*) "<center><table width=600 >", (App.sendBufferSize - 255 - strlen((char*) out_str)));
            strncat((char*) out_str, (char*) "<tr style=\"background-color:#ECEAEA\"><td>File</td><td class=\"r\">Size</td><td class=\"c\">Date</td></tr>", (App.sendBufferSize - 255 - strlen((char*) out_str)));

            // vDisplayMessage("[HTTP FILES]\n");


            while ((ent = readdir(dir)) != NULL) {
                // printf ("%s\n", ent->d_name);
                // int len = strlen(fileinfo.name);
                int len = 0;

                // %02u:%02u:%02u ", (f.wr_time >> 11) & 0x1f, (f.wr_time >>  5) & 0x3f, (f.wr_time & 0x1f) * 2,

                strncat((char*) out_str, (char*) "<tr><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
                strncat((char*) out_str, (char*) ent->d_name, (App.sendBufferSize - len - strlen((char*) out_str)));
                strncat((char*) out_str, (char*) "</td><td class=\"r\">", (App.sendBufferSize - 16 - strlen((char*) out_str)));
                if (/*fileinfo.size*/0 > 1024)

                    snprintf(msg, msg_size, (char*) "%3u Kb", /*fileinfo.size / 1024 */ 0);
                else
                    snprintf(msg, msg_size, (char*) "%3u b", /*fileinfo.size*/ 0);

                strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));
                strncat((char*) out_str, (char*) "</td><td class=\"c\">", (App.sendBufferSize - 16 - strlen((char*) out_str)));
                snprintf(msg, msg_size, (char*) "%02u/%02u/%04u", /* (fileinfo.wr_date & 0x1f), (fileinfo.wr_date >>  5) & 0x0f, ((fileinfo.wr_date >> 9) & 0x7f) + 1980*/ 0, 0, 0);
                strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));
                strncat((char*) out_str, (char*) "</td></tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));

                done = 0; // findnext( &fileinfo );

                nFiles++;

                if (nFiles >= MAX_FILES) {
                    strncat((char*) out_str, (char*) "<tr><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
                    strncat((char*) out_str, (char*) "...", (App.sendBufferSize - len - strlen((char*) out_str)));
                    strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
                    strncat((char*) out_str, (char*) ".", (App.sendBufferSize - 1 - strlen((char*) out_str)));
                    strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
                    strncat((char*) out_str, (char*) ".", (App.sendBufferSize - 1 - strlen((char*) out_str)));
                    strncat((char*) out_str, (char*) "</td></tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
                    break;
                }
            }
            strncat((char*) out_str, (char*) "</table></center>", (App.sendBufferSize - 32 - strlen((char*) out_str)));

            closedir(dir);
        }



    } else if (strnicmp((char*) ptrURL, (char*) "FIRMWARE", 8)== 0) {
        ///////////////////////
        // FIRMWARE manager
        //

        handle_http_update_firmware_form(out_str, out_str_size);
        

    } else if (strnicmp((char*) ptrURL, (char*) "RESET_CAN", 9)== 0) {
        ///////////////////////
        // CANBUS reset
        //
        if (!remove(CAN_BOARD_CFG_FILE) && access(CAN_BOARD_CFG_FILE, F_OK) != -1) {
            strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkRed; padding:20px; background-color:rgb(255,235,235); font-size:36px;\">ERROR : unable to file ", (App.sendBufferSize - 64 - strlen((char*) out_str)));
            strncat((char*) out_str, (char*) CAN_BOARD_CFG_FILE, (App.sendBufferSize - 64 - strlen((char*) out_str)));
            strncat((char*) out_str, (char*) "</span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
        } else {
            if (dataExchangeInitCAN(0+0) <= -999) {
                strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkGReen; padding:20px; background-color:rgb(235,255,235); font-size:36px;\">File ", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                strncat((char*) out_str, (char*) CAN_BOARD_CFG_FILE, (App.sendBufferSize - 64 - strlen((char*) out_str)));
                strncat((char*) out_str, (char*) " deleted ...Reboot the system to take effect</span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));            
            } else {
                strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkGReen; padding:20px; background-color:rgb(235,255,235); font-size:36px;\">File ", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                strncat((char*) out_str, (char*) CAN_BOARD_CFG_FILE, (App.sendBufferSize - 64 - strlen((char*) out_str)));
                strncat((char*) out_str, (char*) " deleted ...CANBUS Relinked : ", (App.sendBufferSize - 64 - strlen((char*) out_str)));                            
                snprintf(msg, msg_size, (char*) "%d found", machine.numCANSlots);
                strncat((char*) out_str, (char*) msg, (App.sendBufferSize - 64 - strlen((char*) out_str)));                            
                strncat((char*) out_str, (char*) "</span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));                            
               
                        
            }
        }

    } else if (strnicmp((char*) ptrURL, (char*) "RESET_SER", 9)== 0) {
        ///////////////////////
        // SERIAL reset
        //
        if (!remove(SER_BOARD_CFG_FILE) && access(SER_BOARD_CFG_FILE, F_OK) != -1) {
            strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkRed; padding:20px; background-color:rgb(255,235,235); font-size:36px;\">ERROR : unable to file ", (App.sendBufferSize - 64 - strlen((char*) out_str)));
            strncat((char*) out_str, (char*) SER_BOARD_CFG_FILE, (App.sendBufferSize - 64 - strlen((char*) out_str)));
            strncat((char*) out_str, (char*) "</span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
        } else {
            strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkGReen; padding:20px; background-color:rgb(235,255,235); font-size:36px;\">File ", (App.sendBufferSize - 64 - strlen((char*) out_str)));
            strncat((char*) out_str, (char*) SER_BOARD_CFG_FILE, (App.sendBufferSize - 64 - strlen((char*) out_str)));
            strncat((char*) out_str, (char*) " deleted ...Reboot the system to take effect</span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));            
        }

    } else if (strnicmp((char*) ptrURL, (char*) "RESET_IO", 8)== 0) {
        ///////////////////////
        // I/O reset
        //
        if (!remove(IO_BOARD_CFG_FILE) && access(IO_BOARD_CFG_FILE, F_OK) != -1) {
            strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkRed; padding:20px; background-color:rgb(255,235,235); font-size:36px;\">ERROR : unable to file ", (App.sendBufferSize - 64 - strlen((char*) out_str)));
            strncat((char*) out_str, (char*) IO_BOARD_CFG_FILE, (App.sendBufferSize - 64 - strlen((char*) out_str)));
            strncat((char*) out_str, (char*) "</span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
        } else {
            strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkGReen; padding:20px; background-color:rgb(235,255,235); font-size:36px;\">File ", (App.sendBufferSize - 64 - strlen((char*) out_str)));
            strncat((char*) out_str, (char*) IO_BOARD_CFG_FILE, (App.sendBufferSize - 64 - strlen((char*) out_str)));
            strncat((char*) out_str, (char*) " deleted ...Reboot the system to take effect</span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));            
        }

    } else if (strnicmp((char*) ptrURL, (char*) "RESET_SCR", 9)== 0) {
        ///////////////////////
        // SCR reset
        //
        if (!remove(SCR_BOARD_CFG_FILE) && access(SCR_BOARD_CFG_FILE, F_OK) != -1) {
            strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkRed; padding:20px; background-color:rgb(255,235,235); font-size:36px;\">ERROR : unable to file ", (App.sendBufferSize - 64 - strlen((char*) out_str)));
            strncat((char*) out_str, (char*) SCR_BOARD_CFG_FILE, (App.sendBufferSize - 64 - strlen((char*) out_str)));
            strncat((char*) out_str, (char*) "</span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
        } else {
            strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkGReen; padding:20px; background-color:rgb(235,255,235); font-size:36px;\">File ", (App.sendBufferSize - 64 - strlen((char*) out_str)));
            strncat((char*) out_str, (char*) SCR_BOARD_CFG_FILE, (App.sendBufferSize - 64 - strlen((char*) out_str)));
            strncat((char*) out_str, (char*) " deleted ...Reboot the system to take effect</span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));            
        }

    } else if (strnicmp((char*) ptrURL, (char*) "RESET_USB", 9)== 0) {
        ///////////////////////
        // USB reset
        //
        if (!remove(USB_BOARD_CFG_FILE) && access(USB_BOARD_CFG_FILE, F_OK) != -1) {
            strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkRed; padding:20px; background-color:rgb(255,235,235); font-size:36px;\">ERROR : unable to file ", (App.sendBufferSize - 64 - strlen((char*) out_str)));
            strncat((char*) out_str, (char*) USB_BOARD_CFG_FILE, (App.sendBufferSize - 64 - strlen((char*) out_str)));
            strncat((char*) out_str, (char*) "</span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
        } else {
            strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkGReen; padding:20px; background-color:rgb(235,255,235); font-size:36px;\">File ", (App.sendBufferSize - 64 - strlen((char*) out_str)));
            strncat((char*) out_str, (char*) USB_BOARD_CFG_FILE, (App.sendBufferSize - 64 - strlen((char*) out_str)));
            strncat((char*) out_str, (char*) " deleted ...Reboot the system to take effect</span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));            
        }

        

    } else if (strnicmp((char*) ptrURL, (char*) "IO", 2) == 0) {
        ////////////////
        // IO
        //

        // vDisplayMessage("[HTTP IO]\n");

        // Intestazione predefinita risposta
        handle_http_default_header_response(out_str, out_str_size, PtrOrigin, NULL);


        ///////////////////////////////////////////
        // Invio dati
        //
        SOCKET_SEND_DATA();



        if (!App.NumIOBoard) {
            strncat((char*) out_str, (char*) "<center><br/><b><h2>No IO Board detected</h2></b><br/></center>\r\n", (App.sendBufferSize - 32 - strlen((char*) out_str)));
        }


        for (i = 0; i < App.NumIOBoard; i++) {

            strncat((char*) out_str, (char*) "<center><table width=90%% >", (App.sendBufferSize - 255 - strlen((char*) out_str)));
            strncat((char*) out_str, (char*) "<tr style=\"background-color:lightGray;\"><td colspan=5>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
            snprintf(msg, msg_size, (char*) "IO #%d - %d.%d.%d.%d - %.2x.%.2x.%.2x.%.2x.%.2x.%.2x", i + 1,
                    GLIOBoard[i].ip[0], GLIOBoard[i].ip[1], GLIOBoard[i].ip[2], GLIOBoard[i].ip[3],
                    GLIOBoard[i].mac[0], GLIOBoard[i].mac[1], GLIOBoard[i].mac[2], GLIOBoard[i].mac[3], GLIOBoard[i].mac[4], GLIOBoard[i].mac[5]
                    );
            strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
            strncat((char*) out_str, (char*) "</td></tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));



            if (i < machine.numIOBoardSlots) {

                if (machine.ioBoardSlots[i].numDigitalIN > 0) {

                    // Intestazione
                    strncat((char*) out_str, (char*) "<tr style=\"background-color:#ECEAEA\"><td>DigIN</td>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
                    for (j = 0; j < machine.ioBoardSlots[i].numDigitalIN; j++) {
                        snprintf(msg, msg_size, (char*) "<td class=\"c\">%d</td>", j + 1);
                        strncat((char*) out_str, msg, (App.sendBufferSize - strlen((char*) msg)));
                    }
                    strncat((char*) out_str, (char*) "</tr><tr><td></td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));



                    for (j = 0; j < machine.ioBoardSlots[i].numDigitalIN; j++) {
                        if (machine.ioBoardSlots[i].digitalIN[j] > 0) {
                            // snprintf(msg, msg_size, (char*)"X");
                            // strncat((char*)out_str, (char*)"X", (App.sendBufferSize - 2 ) );
                            snprintf(msg, msg_size, (char*) "<td class=\"onHttpImg\"></td>");
                        } else {
                            snprintf(msg, msg_size, (char*) "<td class=\"offHttpImg\"></td>");
                        }
                        strncat((char*) out_str, msg, (App.sendBufferSize - strlen((char*) msg)));
                    }
                    strncat((char*) out_str, (char*) "</tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));


                    ///////////////////////////////////////////
                    // Invio dati
                    //
                    // SOCKET_SEND_DATA();
                    // out_str[0] = 0;
                    // out_str_size[0] = 0;

                }




                if (machine.ioBoardSlots[i].numDigitalOUT > 0) {

                    // Intestazione
                    strncat((char*) out_str, (char*) "<tr style=\"background-color:#ECEAEA\"><td>DigOUT</td>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
                    for (j = 0; j < machine.ioBoardSlots[i].numDigitalOUT; j++) {
                        snprintf(msg, msg_size, (char*) "<td class=\"c\">%d</td>", j + 1);
                        strncat((char*) out_str, msg, (App.sendBufferSize - strlen((char*) msg)));
                    }
                    strncat((char*) out_str, (char*) "</tr><tr><td></td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));


                    for (j = 0; j < machine.ioBoardSlots[i].numDigitalOUT; j++) {
                        if (machine.ioBoardSlots[i].digitalOUT[j] > 0) {
                            // snprintf(msg, msg_size, (char*)"X");
                            // strncat((char*)out_str, (char*)"X", (App.sendBufferSize - 2 ) );
                            snprintf(msg, msg_size, (char*) "<td class=\"onHttpImg\"></td>");
                        } else {
                            snprintf(msg, msg_size, (char*) "<td class=\"offHttpImg\"></td>");
                        }
                        strncat((char*) out_str, msg, (App.sendBufferSize - strlen((char*) msg)));
                    }
                    strncat((char*) out_str, (char*) "</tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));

                    ///////////////////////////////////////////
                    // Invio dati
                    //
                    // SOCKET_SEND_DATA();
                    // out_str[0] = 0;
                    // out_str_size[0] = 0;
                }




                if (machine.ioBoardSlots[i].numAnalogIN > 0) {

                    // Intestazione
                    strncat((char*) out_str, (char*) "<tr style=\"background-color:#ECEAEA\"><td>AnalogIN</td>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
                    for (j = 0; j < machine.ioBoardSlots[i].numAnalogIN; j++) {
                        snprintf(msg, msg_size, (char*) "<td class=\"r\">%d</td>", j + 1);
                        strncat((char*) out_str, msg, (App.sendBufferSize - strlen((char*) msg)));
                    }
                    strncat((char*) out_str, (char*) "</tr><tr><td></td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));


                    for (j = 0; j < machine.ioBoardSlots[i].numAnalogIN; j++) {
                        strncat((char*) out_str, (char*) "<td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
                        snprintf(msg, msg_size, (char*) "%d", (int) machine.ioBoardSlots[i].analogIN[j]);
                        strncat((char*) out_str, msg, (App.sendBufferSize - strlen((char*) msg)));
                    }
                    strncat((char*) out_str, (char*) "</tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));

                    ///////////////////////////////////////////
                    // Invio dati
                    //
                    // SOCKET_SEND_DATA();
                    // out_str[0] = 0;
                    // out_str_size[0] = 0;
                }





                if (machine.ioBoardSlots[i].numAnalogOUT > 0) {

                    // Intestazione
                    strncat((char*) out_str, (char*) "<tr style=\"background-color:#ECEAEA\"><td>AnalogOUT</td>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
                    for (j = 0; j < machine.ioBoardSlots[i].numAnalogOUT; j++) {
                        snprintf(msg, msg_size, (char*) "<td class=\"r\">%d</td>", j + 1);
                        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
                    }
                    strncat((char*) out_str, (char*) "</tr><tr><td></td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));



                    for (j = 0; j < machine.ioBoardSlots[i].numAnalogOUT; j++) {
                        strncat((char*) out_str, (char*) "<td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
                        snprintf(msg, msg_size, (char*) "%d", (int) machine.ioBoardSlots[i].analogOUT[j]);
                        strncat((char*) out_str, msg, (App.sendBufferSize - strlen((char*) msg)));
                    }
                    strncat((char*) out_str, (char*) "</tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));

                    ///////////////////////////////////////////
                    // Invio dati
                    //
                    // SOCKET_SEND_DATA();
                    // out_str[0] = 0;
                    // out_str_size[0] = 0;

                }


                if (GLIOBoard[i].i2cNumValues) {
                    SOCKET_SEND_DATA();
                    // Intestazione
                    snprintf(msg, msg_size, (char*) "<tr style=\"background-color:#ECEAEA\"><td colspan=%d>i2c sensor(s)<br/>%d item(s)</td>", 1, GLSCRBoard[i].i2cNumValues);
                    strncat((char*) out_str, msg, (App.sendBufferSize - strlen((char*) msg)));
                    for (j = 0; j < GLIOBoard[i].i2cNumValues; j++) {
                        snprintf(msg, msg_size, (char*) "<td class=\"r\" colspan=2>%0.3f</td>", (float) GLIOBoard[i].i2cValues[j] / 1000.0f);
                        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
                    }
                    strncat((char*) out_str, (char*) "</tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
                }

            } else {
                // Non presente nella logica
                strncat((char*) out_str, (char*) "<tr style=\"background-color:#ECEAEA\"><td>MISSING in logic</td><td class=\"r\"></td></tr>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
                strncat((char*) out_str, (char*) "</tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
            }

            strncat((char*) out_str, (char*) "</table></center>", (App.sendBufferSize - 32 - strlen((char*) out_str)));



            if ((uint32_t) strlen((char*) out_str) >= App.sendBufferSize) {
                snprintf(msg, msg_size, (char*) "[!http size:%u/%u]\n", (uint32_t) strlen((char*) out_str), App.sendBufferSize);
                vDisplayMessage(msg);
                retVal = -1;
                goto end_func;
            }

        }


        ///////////////////////////////////////////
        // Invio dati
        //
        SOCKET_SEND_DATA();
        // out_str[0] = 0;
        // out_str_size[0] = 0;



        ////////////////////
        // Schede seriali
        //

        if (!machine.numSerialSlots) {
            strncat((char*) out_str, (char*) "<center><br/><b>No Serial Board detected</b><br/></center>\r\n", (App.sendBufferSize - 32 - strlen((char*) out_str)));
        } else {

            strncat((char*) out_str, (char*) "<br/><center><table width=600 >", (App.sendBufferSize - 255 - strlen((char*) out_str)));

            strncat((char*) out_str, (char*) "<tr style=\"background-color:lightGray;\"><td colspan=10>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
            snprintf(msg, msg_size, (char*) "Serial port : %d item(s)", machine.numSerialSlots);
            strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
            strncat((char*) out_str, (char*) "</td></tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));

            strncat((char*) out_str, (char*) "<tr style=\"background-color:#ECEAEA\"><td class=\"r\">Name</td><td class=\"r\">ID</td><td>Baud</td><td>Data</td><td>Parity</td><td>Stop</td><td>Actuator</td><td>Homed</td><td>Pending</td><td>Running</td><td>Done</td><td>State</td></tr>", (App.sendBufferSize - 255 - strlen((char*) out_str)));

            for (i = 0; i < machine.numSerialSlots; i++) {
                strncat((char*) out_str, (char*) "<tr><td>", (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));
                snprintf(msg, msg_size, (char*) "SER #%d", (int) i + 1);
                strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

                strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
                snprintf(msg, msg_size, (char*) "%d.%d", (int) machine.serialSlots[i].boardId, machine.serialSlots[i].stationId);
                strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

                strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
                snprintf(msg, msg_size, (char*) "%d", (int) machine.serialSlots[i].baud);
                strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

                strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
                snprintf(msg, msg_size, (char*) "%d", (int) machine.serialSlots[i].data_bit);
                strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

                strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
                snprintf(msg, msg_size, (char*) "%c", (char) machine.serialSlots[i].parity);
                strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

                strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
                snprintf(msg, msg_size, (char*) "%d", (int) machine.serialSlots[i].stop_bit);
                strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

                strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
                if (machine.serialSlots[i].pActuator) {
                    snprintf(msg, msg_size, (char*) "%s", ((LP_ACTUATOR)machine.serialSlots[i].pActuator)->name);
                } else {
                    snprintf(msg, msg_size, (char*) "<none>");
                }
                strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

                strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
                if (machine.serialSlots[i].pActuator) {
                    snprintf(msg, msg_size, (char*) "%s", ((LP_ACTUATOR)machine.serialSlots[i].pActuator)->homingDone ? "ok" : "-");
                } else {
                    snprintf(msg, msg_size, (char*) "[]");
                }
                strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));
                
                strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
                snprintf(msg, msg_size, (char*) "%d", (int) machine.serialSlots[i].pendingCommand);
                strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

                strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
                snprintf(msg, msg_size, (char*) "%d", (int) machine.serialSlots[i].runningCommand);
                strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

                strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
                snprintf(msg, msg_size, (char*) "%d", (int) machine.serialSlots[i].doneCommand);
                strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

                strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
                snprintf(msg, msg_size, (char*) "%s", (char*) get_serial_status((void*) &machine.serialSlots[i]));
                strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

                strncat((char*) out_str, (char*) "</td></tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
            }

            strncat((char*) out_str, (char*) "</table></center>", (App.sendBufferSize - 32 - strlen((char*) out_str)));

        }



        ///////////////////////////////////////////
        // Invio dati
        //
        SOCKET_SEND_DATA();
        // out_str[0] = 0;


        ////////////////////
        // Schede CANBUS
        //

        if (!machine.numSerialSlots) {
            strncat((char*) out_str, (char*) "<center><br/><b>No CANBus detected</b><br/></center>\r\n", (App.sendBufferSize - 32 - strlen((char*) out_str)));
        } else {

            strncat((char*) out_str, (char*) "<br/><center><table width=600 >", (App.sendBufferSize - 255 - strlen((char*) out_str)));

            strncat((char*) out_str, (char*) "<tr style=\"background-color:lightGray;\"><td colspan=13>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
            snprintf(msg, msg_size, (char*) "CANBUS : %d item(s)", machine.numCANSlots);
            strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
            strncat((char*) out_str, (char*) "</td></tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));

            strncat((char*) out_str, (char*) "<tr style=\"background-color:#ECEAEA\"><td class=\"r\">Name</td><td class=\"r\">ID</td><td>Kbps/Rate</td><td>Reads</td><td>Error</td><td>No.Acts</td><td>Acts<span style=\"font-size:75%\">(Homed)</span></td><td>Pending</td><td>Running</td><td>Done</td><td>State</td></tr>", (App.sendBufferSize - 255 - strlen((char*) out_str)));

            for (i = 0; i < machine.numCANSlots; i++) {
                strncat((char*) out_str, (char*) "<tr><td>", (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));
                snprintf(msg, msg_size, (char*) "CANBus #%d", (int) i + 1);
                strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

                strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
                snprintf(msg, msg_size, (char*) "%d", (int) machine.CANSlots[i].boardId);
                strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

                strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
                snprintf(msg, msg_size, (char*) "%d</br>%d", (int) machine.CANSlots[i].Kbps, GLCANLastDataPerSec);
                strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

                strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
                snprintf(msg, msg_size, (char*) "%d", (int) machine.CANSlots[i].readCount);
                strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

                strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
                snprintf(msg, msg_size, (char*) "%d", (char) machine.CANSlots[i].streamErrorCount);
                strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

                strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
                snprintf(msg, msg_size, (char*) "%d", (int) machine.CANSlots[i].nActuators);
                strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

                strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
                strncat((char*) out_str, (char*) "<table>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
                
                for (int32_t i_act = 0; i_act < machine.CANSlots[i].nActuators; i_act++) {
                    strncat((char*) out_str, (char*) "<tr><td>", (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));
                    snprintf(msg, msg_size, (char*) "%s", ((LP_ACTUATOR)machine.CANSlots[i].pActuators[i_act])->name);
                    strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

                    strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
                    switch (((LP_ACTUATOR)machine.CANSlots[i].pActuators[i_act])->homingDone) {
                        case 1:
                            snprintf(msg, msg_size, (char*) "<span style=\"color:darkGreen;\">Y</span>");
                            break;
                        case -1:
                            snprintf(msg, msg_size, (char*) "<span style=\"color:Navy;\">/</span>");
                            break;
                        case 0:
                            snprintf(msg, msg_size, (char*) "<span style=\"color:Orange;\">N</span>");
                            break;
                        default:
                            snprintf(msg, msg_size, (char*) "<span style=\"color:Red;\">(?)</span>");
                            break;
                    }
                    strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));
                    
                    strncat((char*) out_str, (char*) "</td></tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
                }
                strncat((char*) out_str, (char*) "</table>", (App.sendBufferSize - 16 - strlen((char*) out_str)));

                strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
                snprintf(msg, msg_size, (char*) "%d", (int) machine.CANSlots[i].pendingCommand);
                strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

                strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
                snprintf(msg, msg_size, (char*) "%d", (int) machine.CANSlots[i].runningCommand);
                strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

                strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
                snprintf(msg, msg_size, (char*) "%d", (int) machine.CANSlots[i].doneCommand);
                strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

                strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
                snprintf(msg, msg_size, (char*) "%s", (char*) get_canbus_status((void*) &machine.CANSlots[i]));
                strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

                strncat((char*) out_str, (char*) "</td></tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
            }

            strncat((char*) out_str, (char*) "</table></center>", (App.sendBufferSize - 32 - strlen((char*) out_str)));

        }



        ///////////////////////////////////////////
        // Invio dati
        //
        SOCKET_SEND_DATA();
        // out_str[0] = 0;



        
        
        ////////////////////
        // Schede SCR
        //

        if (!App.NumSCRBoard) {
            strncat((char*) out_str, (char*) "<center><br/><b>No SCR> Board detected</b><br/></center>\r\n", (App.sendBufferSize - 32 - strlen((char*) out_str)));
        } else {

            for (i = 0; i < App.NumSCRBoard; i++) {
                strncat((char*) out_str, (char*) "<br/><center><table width=90%% >", (App.sendBufferSize - 255 - strlen((char*) out_str)));
                strncat((char*) out_str, (char*) "<tr style=\"background-color:lightGray;\"><td colspan=16>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
                snprintf(msg, msg_size, (char*) "SCR #%d - %d.%d.%d.%d - %.2x.%.2x.%.2x.%.2x.%.2x.%.2x", i + 1,
                        GLSCRBoard[i].ip[0], GLSCRBoard[i].ip[1], GLSCRBoard[i].ip[2], GLSCRBoard[i].ip[3],
                        GLSCRBoard[i].mac[0], GLSCRBoard[i].mac[1], GLSCRBoard[i].mac[2], GLSCRBoard[i].mac[3], GLSCRBoard[i].mac[4], GLSCRBoard[i].mac[5]
                        );
                strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
                strncat((char*) out_str, (char*) "</td></tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));

                strncat((char*) out_str, (char*) "<tr style=\"background-color:lightGray;\">", (App.sendBufferSize - 64 - strlen((char*) out_str)));

                strncat((char*) out_str, (char*) "<td>Row1</td>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                strncat((char*) out_str, (char*) "<td>Row2</td>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                strncat((char*) out_str, (char*) "<td>Row3</td>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                strncat((char*) out_str, (char*) "<td>Row4</td>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                strncat((char*) out_str, (char*) "<td>Row5</td>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                strncat((char*) out_str, (char*) "<td>Row6</td>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                strncat((char*) out_str, (char*) "<td>Row7</td>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                strncat((char*) out_str, (char*) "<td>Row8</td>", (App.sendBufferSize - 64 - strlen((char*) out_str)));

                strncat((char*) out_str, (char*) "<td>AnalogV1</td>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                strncat((char*) out_str, (char*) "<td>AnalogV2</td>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                strncat((char*) out_str, (char*) "<td>AnalogV3</td>", (App.sendBufferSize - 64 - strlen((char*) out_str)));

                strncat((char*) out_str, (char*) "<td>KA ms</td>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                strncat((char*) out_str, (char*) "<td>Toll</td>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                strncat((char*) out_str, (char*) "<td>MaxErr</td>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                strncat((char*) out_str, (char*) "<td>Status Code</td>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                strncat((char*) out_str, (char*) "<td>Voltage Factor</td>", (App.sendBufferSize - 64 - strlen((char*) out_str)));

                strncat((char*) out_str, (char*) "</tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));

                strncat((char*) out_str, (char*) "<tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));

                for (j = 0; j < 8 /*GLSCRBoard[i].numRows*/; j++) {
                    snprintf(msg, msg_size, (char*) "<td class=\"r\">%d<h6>(%d)</h6></td>", GLSCRBoard[i].Rows[j], 100 - GLSCRBoard[i].Rows[j]);
                    strncat((char*) out_str, msg, (App.sendBufferSize - strlen((char*) msg)));
                }


                snprintf(msg, msg_size, (char*) "<td class=\"r\">%0.1fV<h6>(%0.3f)</h6></td>", 273.0f / 100.0f * GLSCRBoard[i].AnalogVoltage[0], GLSCRBoard[i].AnalogVoltage[0]);
                strncat((char*) out_str, msg, (App.sendBufferSize - strlen((char*) msg)));

                snprintf(msg, msg_size, (char*) "<td class=\"r\">%0.1fV<h6>(%0.3f)</h6></td>", 273.0f / 100.0f * GLSCRBoard[i].AnalogVoltage[1], GLSCRBoard[i].AnalogVoltage[1]);
                strncat((char*) out_str, msg, (App.sendBufferSize - strlen((char*) msg)));

                snprintf(msg, msg_size, (char*) "<td class=\"r\">%0.1fV<h6>(%0.3f)</h6></td>", 273.0f / 100.0f * GLSCRBoard[i].AnalogVoltage[2], GLSCRBoard[i].AnalogVoltage[2]);
                strncat((char*) out_str, msg, (App.sendBufferSize - strlen((char*) msg)));

                snprintf(msg, msg_size, (char*) "<td class=\"r\">%d</td>", GLSCRBoard[i].KALoopTimeMs);
                strncat((char*) out_str, msg, (App.sendBufferSize - strlen((char*) msg)));

                snprintf(msg, msg_size, (char*) "<td class=\"r\">%0.3f</td>", GLSCRBoard[i].TollRefChange);
                strncat((char*) out_str, msg, (App.sendBufferSize - strlen((char*) msg)));

                snprintf(msg, msg_size, (char*) "<td class=\"r\">%d</td>", GLSCRBoard[i].MaxErr);
                strncat((char*) out_str, msg, (App.sendBufferSize - strlen((char*) msg)));

                snprintf(msg, msg_size, (char*) "<td class=\"r\">%d</td>", GLSCRBoard[i].StatusCode);
                strncat((char*) out_str, msg, (App.sendBufferSize - strlen((char*) msg)));

                float voltageFact = 0.0f;
                short i2cTime = 0;
                unsigned int i2cReadCounter = 0;

                memcpy(&voltageFact, &GLSCRBoard[i].StatusData, 4);
                memcpy(&i2cTime, &GLSCRBoard[i].StatusData[4], 2);
                memcpy(&i2cReadCounter, &GLSCRBoard[i].StatusData[6], 4);

                snprintf(msg, msg_size, (char*) "<td class=\"r\" style=\"font-size:85%%\">%0.3f<br/>%dms<br/>%d<br/>(%dbytes)</td>", voltageFact, i2cTime, i2cReadCounter, GLSCRBoard[i].StatusDataSize);
                strncat((char*) out_str, msg, (App.sendBufferSize - strlen((char*) msg)));


                strncat((char*) out_str, (char*) "</tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));

                if (GLSCRBoard[i].i2cNumValues) {
                    SOCKET_SEND_DATA();
                    // Intestazione
                    snprintf(msg, msg_size, (char*) "<tr style=\"background-color:#ECEAEA\"><td colspan=%d>i2c sensor(s)<br/>%d item(s)</td>", GLSCRBoard[i].i2cNumValues, GLSCRBoard[i].i2cNumValues);
                    strncat((char*) out_str, msg, (App.sendBufferSize - strlen((char*) msg)));

                    for (j = 0; j < GLSCRBoard[i].i2cNumValues; j++) {
                        snprintf(msg, msg_size, (char*) "<td class=\"r\">%0.3f</td>", (float) GLSCRBoard[i].i2cValues[j] / 1000.0f);
                        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
                    }
                    strncat((char*) out_str, (char*) "</tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
                }
            }

            strncat((char*) out_str, (char*) "</table></center>", (App.sendBufferSize - 32 - strlen((char*) out_str)));
        }





    } else if (strnicmp((char*) ptrURL, (char*) "STAT", 4) == 0) {
        ////////////////////
        // Statistiche
        //


        // Intestazione predefinita risposta
        handle_http_default_header_response(out_str, out_str_size, PtrOrigin, NULL);


        ///////////////////////////////////////////
        // Invio dati
        //
        SOCKET_SEND_DATA();


        ////////////////////////////
        // Statistiche Real Time
        //
        strncat((char*) out_str, (char*) "<br/><center><table width=70%% >", (App.sendBufferSize - 255 - strlen((char*) out_str)));
        strncat((char*) out_str, (char*) "<tr style=\"background-color:lightGray;\"><td colspan=6>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "<h2>RealTime statistics</h2>");
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td></tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));

        strncat((char*) out_str, (char*) "<tr style=\"background-color:transparent;\">", (App.sendBufferSize - 255 - strlen((char*) out_str)));
        strncat((char*) out_str, (char*) "<td colspan=6>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        strncat((char*) out_str, App.KernelHTMLString, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        strncat((char*) out_str, (char*) "</td></tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));


        strncat((char*) out_str, (char*) "<tr><td colspan=6 style=\"border-top:1px solid lightGray; height:1px\"></td></tr>", (App.sendBufferSize - 255 - strlen((char*) out_str)));

        

        strncat((char*) out_str, (char*) "<tr style=\"background-color:transparent;\"><td>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "RealTime Errors");
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "<b>%d</b>", (int) GLLogicErr);
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "Max cycle time");
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "<b>%d</b>ms", (int) GLmaxtOut);
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        // snprintf(msg, msg_size, (char*) "");
        // strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td></tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));


        strncat((char*) out_str, (char*) "<tr style=\"background-color:transparent;\"><td>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "Logic max time");
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "<b>%0.3f</b>us", (float) App.LogicMaxTimeNanoSec / 1000.0f);
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "Usage");
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "<b>%0.1f</b>%%", (float) (App.LogicMaxTimeNanoSec / 1000.0f) / 1000.0f * 100.0f );
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td></tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));

        
        strncat((char*) out_str, (char*) "<tr style=\"background-color:transparent;\"><td>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "Logic min time");
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "<b>%0.3f</b>us", (float) App.LogicMinTimeNanoSec / 1000.0f);
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "Usage");
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "<b>%0.1f</b>%%", (float) (App.LogicMinTimeNanoSec / 1000.0f) / 1000.0f * 100.0f );
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td></tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));

        
        strncat((char*) out_str, (char*) "<tr style=\"background-color:transparent;\"><td>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "Logic time");
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "<b>%0.3f</b>us", (float) App.LogicLastTimeNanoSec / 1000.0f);
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "Wait cycle");
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "<b>%d</b>us", (int32_t) App.LogicWaitUSec);
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "Usage");
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "<b>%0.1f</b>%%", (float) (App.LogicLastTimeNanoSec / 1000.0f) / 1000.0f * 100.0f );
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td></tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));

        
        strncat((char*) out_str, (char*) "<tr><td colspan=6 style=\"border-top:1px solid lightGray; height:1px\"></td></tr>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
        

        strncat((char*) out_str, (char*) "<tr style=\"background-color:transparent;\"><td>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "UI state");
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td colspan=2>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "%s", dataExchangeGetStat((int32_t)App.UICommState));
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "HTTP state");
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td colspan=2>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "%s", dataExchangeGetStat((int32_t)App.HTTPServerState));
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td></tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));

        strncat((char*) out_str, (char*) "<tr style=\"background-color:transparent;\"><td>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "IO Keep Alive timeout");
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "<b>%d</b>ms", (int) GLIOBoard[0].tickTocIOKeepAliveTimeout);
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "IO Watchdog");
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "<b>%d</b>ms", (int) GLIOBoard[0].tickTocIOWatchDogTimeout);
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "IO Pending delay");
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "<b>%d</b>ms", (int) GLIOBoard[0].tickTocIOPendingTimeout);
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td></tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));

        strncat((char*) out_str, (char*) "<tr><td colspan=6 style=\"border-top:1px solid lightGray; height:1px\"></td></tr>", (App.sendBufferSize - 255 - strlen((char*) out_str)));


        strncat((char*) out_str, (char*) "<tr style=\"background-color:transparent;\"><td>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "IO min rate");
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "<b>%d</b>Hz", (int) GLIOMinDataPerSec != NOT_SET_HIGH_VALUE ? GLIOMinDataPerSec : 0);
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "IO rate");
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "<b>%d</b>Hz", (int) GLIOLastDataPerSec);
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "IO max rate");
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "<b>%d</b>Hz", (int) GLIOMaxDataPerSec);
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td></tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));




        strncat((char*) out_str, (char*) "<tr style=\"background-color:transparent;\"><td>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "UI min rate");
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "<b>%d</b>Hz", (int) GLUIMinDataPerSec != NOT_SET_HIGH_VALUE ? GLUIMinDataPerSec : 0);
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "UI rate");
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "<b>%d</b>Hz", (int) GLUILastDataPerSec);
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "UI max rate");
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "<b>%d</b>Hz", (int) GLUIMaxDataPerSec);
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td></tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));




        strncat((char*) out_str, (char*) "<tr style=\"background-color:transparent;\"><td>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "Serial min rate");
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "<b>%d</b>Hz", (int) GLSERMinDataPerSec != NOT_SET_HIGH_VALUE ? GLSERMinDataPerSec : 0);
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "Serial rate");
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "<b>%d</b>Hz", (int) GLSERLastDataPerSec);
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "Serial max rate");
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "<b>%d</b>Hz", (int) GLSERMaxDataPerSec);
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td></tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));



        strncat((char*) out_str, (char*) "<tr style=\"background-color:transparent;\"><td>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "Canbus min rate");
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "<b>%d</b>Hz", (int) GLCANMinDataPerSec != NOT_SET_HIGH_VALUE ? GLCANMinDataPerSec : 0);
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "Canbus rate");
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "<b>%d</b>Hz", (int) GLCANLastDataPerSec);
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "Canbus max rate");
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "<b>%d</b>Hz", (int) GLCANMaxDataPerSec);
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td></tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        

        strncat((char*) out_str, (char*) "<tr><td colspan=6 style=\"border-top:1px solid lightGray; height:1px\"></td></tr>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
        


        strncat((char*) out_str, (char*) "<tr style=\"background-color:transparent;\"><td>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "UI max cycle time");
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "<b>%d</b>ms", (int) App.HTTPMaxTime[1]);
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "IO max cycle time");
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "<b>%d</b>ms", (int) App.HTTPMaxTime[2]);
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "Serial max cycle time");
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "<b>%d</b>ms", (int) App.HTTPMaxTime[3]);
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td></tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));




        strncat((char*) out_str, (char*) "<tr style=\"background-color:transparent;\"><td>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "CAN max cycle time");
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "<b>%d</b>ms", (int) App.HTTPMaxTime[4]);
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "USB max cycle time");
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "<b>%d</b>ms", (int) App.HTTPMaxTime[5]);
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "SCR max cycle time");
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "<b>%d</b>ms", (int) App.HTTPMaxTime[6]);
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td></tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));


        strncat((char*) out_str, (char*) "<tr><td colspan=6 style=\"border-top:1px solid lightGray; height:1px\"></td></tr>", (App.sendBufferSize - 255 - strlen((char*) out_str)));

        
        strncat((char*) out_str, (char*) "<tr style=\"background-color:transparent;\"><td>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "Current alarm");
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "<b>%d</b>", (int) machine.curAlarmList);
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "No.alarms");
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "<b>%d</b>", (int) machine.numAlarmList);
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "Alarms allocated");
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "<b>%d</b>", (int) machine.numAlarmListAllocated);
        strncat((char*) out_str, msg, (App.sendBufferSize - 255 - strlen((char*) msg)));
        strncat((char*) out_str, (char*) "</td></tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));

        strncat((char*) out_str, (char*) "</table></center>", (App.sendBufferSize - 32 - strlen((char*) out_str)));


        
        
        
        ///////////////////////////////////////////
        // Invio dati
        //
        SOCKET_SEND_DATA();
        

        
        ///////////////////////////////////////////
        // Dati UI sommari
        //
        
        strncat((char*) out_str, (char*) "<center><br/><table width=600 >", (App.sendBufferSize - 255 - strlen((char*) out_str)));
        
        
        strncat((char*) out_str, (char*) "<tr style=\"background-color:#ECEAEA\"><td colspan=3><h2>Application's summary</h2></td></tr>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
        
        strncat((char*) out_str, (char*) "<tr style=\"background-color:#ECEAEA\"><td class=\"r\">Description</td><td class=\"r\">Value</td><td>Max</td></tr>", (App.sendBufferSize - 255 - strlen((char*) out_str)));

        strncat((char*) out_str, (char*) "<tr><td class=\"r\">Time</td><td class=\"r\">", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        float secs = (xTaskGetTickCount() * portTICK_RATE_MS) / 1000.0f;
        uint32_t h = (uint32_t) secs / 3600l;
        uint32_t m = (uint32_t) ((float) ((uint32_t) secs % 3600l) / 60.0f);

        snprintf(msg, msg_size, (char*) "%d:%d:%0.3f", (int) h, (int) m, secs - (float) h * 3600.0f - (float) m * 60.0f);
        strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

        strncat((char*) out_str, (char*) "</td><td class=\"r\">", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "%0.3f", secs);
        strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));
        strncat((char*) out_str, (char*) "</td></tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));



        strncat((char*) out_str, (char*) "<tr><td class=\"r\">Cache Vars", (App.sendBufferSize - 16 - strlen((char*) out_str)));

        strncat((char*) out_str, (char*) "</td><td class=\"r\">", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "%d", GLCUNumVars);
        strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

        strncat((char*) out_str, (char*) "</td><td class=\"r\">", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "%d", GLCUNumVarsAllocated);
        strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

        strncat((char*) out_str, (char*) "</td></tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));


        strncat((char*) out_str, (char*) "<tr><td class=\"r\">Alarms", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        // machine.curAlarmList        
        // machine.lastAlarmListId, machine.lastAlarmId
        
        strncat((char*) out_str, (char*) "</td><td class=\"r\">", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "%d", machine.numAlarmList);
        strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

        strncat((char*) out_str, (char*) "</td><td class=\"r\">", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "%d", machine.numAlarmListAllocated);
        strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

        strncat((char*) out_str, (char*) "</td></tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));



        strncat((char*) out_str, (char*) "<tr><td class=\"r\">Last Alarm/Id", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        
        strncat((char*) out_str, (char*) "</td><td class=\"r\">", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "%d", machine.curAlarmList);
        strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

        strncat((char*) out_str, (char*) "</td><td class=\"r\">", (App.sendBufferSize - 16 - strlen((char*) out_str)));
        snprintf(msg, msg_size, (char*) "%d", machine.lastAlarmId);
        strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

        strncat((char*) out_str, (char*) "</td></tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));


        
        strncat((char*) out_str, (char*) "</table>", (App.sendBufferSize - 32 - strlen((char*) out_str)));




        ///////////////////////////////////////////
        // Invio dati
        //
        SOCKET_SEND_DATA();

        ////////////////////
        // Attuatori
        //

        strncat((char*) out_str, (char*) "<br/><table width=600 >", (App.sendBufferSize - 255 - strlen((char*) out_str)));
        strncat((char*) out_str, (char*) "<tr style=\"background-color:#ECEAEA;\"><td colspan=10><h2>Actuators</h2></td></tr>", (App.sendBufferSize - 255 - strlen((char*) out_str)));

        for (i = 0; i < machine.num_actuator; i++) {

            strncat((char*) out_str, (char*) "<tr><td colspan=10 style=\"border-bottom:1px solid darkRed\"></td></tr>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
            strncat((char*) out_str, (char*) "<tr style=\"background-color:#ECEAEA\"><td>Name</td><td>Protocol</td><td class=\"c\">Homed</td><td class=\"c\">Pos/Tgt</td><td class=\"r\">Pos</td><td class=\"r\">Spos</td><td class=\"r\">Epos</td><td class=\"r\">Vpos</td><td class=\"r\">Acc1</td><td class=\"r\">Speed1</td><td class=\"r\">Dec1</td></tr>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
            strncat((char*) out_str, (char*) "<tr><td colspan=10 style=\"border-top:1px solid darkRed\"></td></tr>", (App.sendBufferSize - 255 - strlen((char*) out_str)));

        
            strncat((char*) out_str, (char*) "<tr><td><b>", (App.sendBufferSize - 32 - strlen((char*) out_str)));
            strncat((char*) out_str, (char*) machine.actuator[i].name, (App.sendBufferSize - strlen(machine.actuator[i].name) - strlen((char*) out_str)));
            strncat((char*) out_str, (char*) "<b>", (App.sendBufferSize - 16 - strlen((char*) out_str)));

            strncat((char*) out_str, (char*) "</td><td class=\"c\">", (App.sendBufferSize - 16 - strlen((char*) out_str)));
                switch (machine.actuator[i].protocol) {
                    case PROTOCOL_NONE:
                        snprintf(msg, msg_size, (char*) "<span style=\"color:darkGreen;\">None</span>");
                        break;
                    case VIRTUAL_AC_SERVO:
                        snprintf(msg, msg_size, (char*) "<span style=\"color:darkGray;\">Virtual</span>");
                        break;
                    case CANOPEN_AC_SERVO_DELTA:
                        snprintf(msg, msg_size, (char*) "<span style=\"color:darkGreen;\">CanOpen Delta</span>");
                        break;
                    case MODBUS_AC_SERVO_LICHUAN:
                        snprintf(msg, msg_size, (char*) "<span style=\"color:Navy;\">Modbus LI</span>");
                        break;
                    case MODBUS_AC_SERVO_DELTA:
                        snprintf(msg, msg_size, (char*) "<span style=\"color:Navy;\">Modbus Delta</span>");
                        break;
                    default:
                        snprintf(msg, msg_size, (char*) "<span style=\"color:Red;\">UNKNOWN</span>");
                        break;
                }
            strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));


            strncat((char*) out_str, (char*) "</td><td class=\"c\">", (App.sendBufferSize - 16 - strlen((char*) out_str)));
                switch (machine.actuator[i].homingDone) {
                    case 1:
                        snprintf(msg, msg_size, (char*) "<span style=\"color:darkGreen;\">Y</span>");
                        break;
                    case -1:
                        snprintf(msg, msg_size, (char*) "<span style=\"color:Navy;\">/</span>");
                        break;
                    case 0:
                        snprintf(msg, msg_size, (char*) "<span style=\"color:Orange;\">N</span>");
                        break;
                    default:
                        snprintf(msg, msg_size, (char*) "<span style=\"color:Red;\">(?)</span>");
                        break;
                }
            strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));
                   
            strncat((char*) out_str, (char*) "</td><td class=\"c\">", (App.sendBufferSize - 16 - strlen((char*) out_str)));
            if (machine.actuator[i].position == machine.actuator[i].target_position) {
                snprintf(msg, msg_size, (char*) "%s", get_actuator_pos_desc((void*)&machine.actuator[i], machine.actuator[i].position));
                strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));
            } else {
                snprintf(msg, msg_size, (char*) "<table><tr><td>%s</td><td>-></td><td>%s</td></tr></table>", get_actuator_pos_desc((void*)&machine.actuator[i], machine.actuator[i].position), get_actuator_pos_desc((void*)&machine.actuator[i], machine.actuator[i].target_position));
                strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));
            }

            strncat((char*) out_str, (char*) "</td><td class=\"r\">", (App.sendBufferSize - 16 - strlen((char*) out_str)));
            snprintf(msg, msg_size, (char*) "%0.3f", machine.actuator[i].cur_rpos);
            strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

            strncat((char*) out_str, (char*) "</td><td class=\"r\">", (App.sendBufferSize - 16 - strlen((char*) out_str)));
            snprintf(msg, msg_size, (char*) "%0.3f", machine.actuator[i].start_rpos);
            strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

            strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
            snprintf(msg, msg_size, (char*) "%0.3f", machine.actuator[i].end_rpos);
            strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

            strncat((char*) out_str, (char*) "</td><td class=\"r\">", (App.sendBufferSize - 16 - strlen((char*) out_str)));
            snprintf(msg, msg_size, (char*) "%0.3f", machine.actuator[i].cur_vpos);
            strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

            strncat((char*) out_str, (char*) "</td><td class=\"r\">", (App.sendBufferSize - 16 - strlen((char*) out_str)));
            snprintf(msg, msg_size, (char*) "%0.3f", machine.actuator[i].acc_auto1);
            strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

            strncat((char*) out_str, (char*) "</td><td class=\"r\">", (App.sendBufferSize - 16 - strlen((char*) out_str)));
            snprintf(msg, msg_size, (char*) "%0.3f", machine.actuator[i].speed_auto1);
            strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

            strncat((char*) out_str, (char*) "</td><td class=\"r\">", (App.sendBufferSize - 16 - strlen((char*) out_str)));
            snprintf(msg, msg_size, (char*) "%0.3f", machine.actuator[i].dec_auto1);
            strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));


            strncat((char*) out_str, (char*) "</td></tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
            
            strncat((char*) out_str, (char*) "<tr style=\"background-color:#ECEAEA\"><tdclass=\"r\"><td class=\"r\">Acc2</td><td class=\"r\">Speed2</td><td class=\"r\">Dec2</td><td class=\"r\">time</td><td class=\"r\">tout1</td><td class=\"r\">tout2</td><td class=\"r\">twarn1</td><td class=\"r\">twarn2</td><td class=\"c\">step</td><td class=\"r\">reads</td><td class=\"r\">err</td></tr>", (App.sendBufferSize - 255 - strlen((char*) out_str)));

            strncat((char*) out_str, (char*) "<tr style=\"background-color:#f3f4f5\">", (App.sendBufferSize - 16 - strlen((char*) out_str)));


            strncat((char*) out_str, (char*) "<td class=\"r\">", (App.sendBufferSize - 16 - strlen((char*) out_str)));
            snprintf(msg, msg_size, (char*) "%0.3f", machine.actuator[i].acc_auto2);
            strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

            strncat((char*) out_str, (char*) "</td><td class=\"r\">", (App.sendBufferSize - 16 - strlen((char*) out_str)));
            snprintf(msg, msg_size, (char*) "%0.3f", machine.actuator[i].speed_auto2);
            strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

            strncat((char*) out_str, (char*) "</td><td class=\"r\">", (App.sendBufferSize - 16 - strlen((char*) out_str)));
            snprintf(msg, msg_size, (char*) "%0.3f", machine.actuator[i].dec_auto2);
            strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

            strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
            snprintf(msg, msg_size, (char*) "<table style=\"font-size: 12px;\"><tr><td>%d</td><td>%d</td><td>%d</td><td>%d</td></tr><tr><td>%d</td><td>%d</td><td>%d</td><td>%d</td></tr></table>"
                    , machine.actuator[i].time_ms1, machine.actuator[i].time_ms11, machine.actuator[i].time_ms12, machine.actuator[i].time_ms13
                    , machine.actuator[i].time_ms2, machine.actuator[i].time_ms21, machine.actuator[i].time_ms22, machine.actuator[i].time_ms23);
            strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

            strncat((char*) out_str, (char*) "</td><td class=\"r\">", (App.sendBufferSize - 16 - strlen((char*) out_str)));
            snprintf(msg, msg_size, (char*) "%d", machine.actuator[i].timeout1_ms);
            strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

            strncat((char*) out_str, (char*) "</td><td class=\"r\">", (App.sendBufferSize - 16 - strlen((char*) out_str)));
            snprintf(msg, msg_size, (char*) "%d", machine.actuator[i].timeout2_ms);
            strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

            strncat((char*) out_str, (char*) "</td><td class=\"r\">", (App.sendBufferSize - 16 - strlen((char*) out_str)));
            snprintf(msg, msg_size, (char*) "%d", machine.actuator[i].timewarn1_ms);
            strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

            strncat((char*) out_str, (char*) "</td><td class=\"r\">", (App.sendBufferSize - 16 - strlen((char*) out_str)));
            snprintf(msg, msg_size, (char*) "%d", machine.actuator[i].timewarn2_ms);
            strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

            strncat((char*) out_str, (char*) "</td><td class=\"c\">", (App.sendBufferSize - 16 - strlen((char*) out_str)));
            snprintf(msg, msg_size, (char*) "%s", get_actuator_step((void *) &machine.actuator[i]));
            strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

            strncat((char*) out_str, (char*) "</td><td class=\"r\">", (App.sendBufferSize - 16 - strlen((char*) out_str)));
            snprintf(msg, msg_size, (char*) "%d", machine.actuator[i].readCounter);
            strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

            strncat((char*) out_str, (char*) "</td><td class=\"r\">", (App.sendBufferSize - 16 - strlen((char*) out_str)));
            snprintf(msg, msg_size, (char*) "%d", machine.actuator[i].error);
            strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));
            strncat((char*) out_str, (char*) "</td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));



            strncat((char*) out_str, (char*) "</td></tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
            
            strncat((char*) out_str, (char*) "<tr style=\"background-color:#ECEAEA\"><td colspan=10>Homing</td></tr>", (App.sendBufferSize - 255 - strlen((char*) out_str)));

            strncat((char*) out_str, (char*) "<tr style=\"background-color:#f1f1f1\">", (App.sendBufferSize - 16 - strlen((char*) out_str)));
            strncat((char*) out_str, (char*) "<td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
            strncat((char*) out_str, (char*) "</td><td  class=\"r\">", (App.sendBufferSize - 16 - strlen((char*) out_str)));
            snprintf(msg, msg_size, (char*) "Mode");
            strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

            strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
            snprintf(msg, msg_size, (char*) "%s", machine.actuator[i].homingTorqueMode ? "Torque" : "DI");
            strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

            strncat((char*) out_str, (char*) "</td><td class=\"r\">", (App.sendBufferSize - 16 - strlen((char*) out_str)));
            snprintf(msg, msg_size, (char*) "Torque(%%)");
            strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

            strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
            snprintf(msg, msg_size, (char*) "%0.2f", machine.actuator[i].homing_rated_torque);
            strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

            strncat((char*) out_str, (char*) "</td><td class=\"r\">", (App.sendBufferSize - 16 - strlen((char*) out_str)));
            snprintf(msg, msg_size, (char*) "Speed(rpm)");
            strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

            strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
            snprintf(msg, msg_size, (char*) "%0.2f", machine.actuator[i].homing_speed_rpm);
            strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

            strncat((char*) out_str, (char*) "</td><td class=\"r\">", (App.sendBufferSize - 16 - strlen((char*) out_str)));
            snprintf(msg, msg_size, (char*) "Offset(mm/deg)");
            strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

            strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
            snprintf(msg, msg_size, (char*) "%0.3f", machine.actuator[i].homing_offset_mm);
            strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

            strncat((char*) out_str, (char*) "</td><td class=\"r\">", (App.sendBufferSize - 16 - strlen((char*) out_str)));
            snprintf(msg, msg_size, (char*) "Direction");
            strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

            strncat((char*) out_str, (char*) "</td><td>", (App.sendBufferSize - 16 - strlen((char*) out_str)));
            snprintf(msg, msg_size, (char*) "%s", machine.actuator[i].homing_position == 0 ? "[0 to 1]" : "[1 to 0]");
            strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));

            strncat((char*) out_str, (char*) "</td></tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));


            
            if (i % 5 == 0) {
                ///////////////////////////////////////////
                // Invio dati
                //
                SOCKET_SEND_DATA();
            }
        }


        strncat((char*) out_str, (char*) "</table></center>", (App.sendBufferSize - 32 - strlen((char*) out_str)));


        ///////////////////////////////////////////
        // Invio dati
        //
        SOCKET_SEND_DATA();



        strncat((char*) out_str, (char*) "<center><br/><table width=600 >", (App.sendBufferSize - 255 - strlen((char*) out_str)));
        strncat((char*) out_str, (char*) "<tr style=\"background-color:#ECEAEA\"><td colspan=2><h2>WORK PARAM</h2> (machine.workSet.*)</td></tr>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
        strncat((char*) out_str, (char*) "<tr style=\"background-color:#ECEAEA\"><td class=\"r\">Name</td><td class=\"r\">Value</td></tr>", (App.sendBufferSize - 255 - strlen((char*) out_str)));


#define PRINT_PARAMETER(__name, __name_desc, __format)    \
                            snprintf(msg, msg_size, (char*)"<tr><td class=\"r\">%s</td>", __name_desc);    \
                            strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str))); \
                            strncat((char*) out_str, (char*) "<td class=\"r\"><b>", (App.sendBufferSize - 32 - strlen((char*) out_str)));   \
                            snprintf(msg, msg_size, __format, __name);    \
                            strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));  \
                            strncat((char*) out_str, (char*) "<b></td></tr>", (App.sendBufferSize - 16 - strlen((char*) out_str)));


        PRINT_PARAMETER(machine.workSet.loadCount, "loadCount", "%d");
        PRINT_PARAMETER(machine.workSet.name, "name", "%s");
        PRINT_PARAMETER(machine.workSet.description, "description", "%s");

#ifdef xBM_COMPILE
        PRINT_PARAMETER(machine.workSet.capacity, "capacity", "%0.3f")
        PRINT_PARAMETER(machine.workSet.preform_weight, "preform_weight", "%0.3f");
        PRINT_PARAMETER(machine.workSet.preform_thickness, "preform_thickness", "%0.3f");
        PRINT_PARAMETER(machine.workSet.preform_color, "preform_color", "%s");
        PRINT_PARAMETER(machine.workSet.production, "production", "%d");
        PRINT_PARAMETER(machine.workSet.primary_air_gap_mm, "primary_air_gap", "%0.3f");
        PRINT_PARAMETER(machine.workSet.secondary_air_gap_ms, "secondary_air_gap", "%d");
        PRINT_PARAMETER(machine.workSet.min_secondary_air_time_ms, "min_secondary_air_time", "%d");
        PRINT_PARAMETER(machine.workSet.discharge_air_time_ms, "discharge_air_time_ms", "%d");
        PRINT_PARAMETER(machine.workSet.recovery_air_factor, "recovery_air_factor", "%0.3f");
        PRINT_PARAMETER(machine.workSet.max_pressure_in_mold, "max_pressure_in_mold", "%0.3f");
        PRINT_PARAMETER(machine.workSet.pressure_min, "pressure_min", "%0.3f");
        PRINT_PARAMETER(machine.workSet.pressure_max, "pressure_max", "%0.3f");
        PRINT_PARAMETER(machine.workSet.pressure_check, "pressure_check", "%0.3f");
        PRINT_PARAMETER(machine.workSet.pressure_check_gap, "pressure_check_gap", "%0.3f");
        PRINT_PARAMETER(machine.workSet.pressure_check_time, "pressure_check_time", "%0.3f");
        PRINT_PARAMETER(machine.workSet.stretch_mantein_force, "stretch_mantein_force", "%0.3f");
        PRINT_PARAMETER(machine.workSet.stretch_bottom_gap_mm, "stretch_bottom_gap", "%0.3f");
        PRINT_PARAMETER(machine.workSet.ventilation_ratio, "ventilation_ratio", "%0.3f");
        PRINT_PARAMETER(machine.workSet.preform_temp1, "preform_temp1", "%0.3f");
        PRINT_PARAMETER(machine.workSet.preform_temp2, "preform_temp2", "%0.3f");
        PRINT_PARAMETER(machine.workSet.preform_temp3, "preform_temp3", "%0.3f");
        PRINT_PARAMETER(machine.workSet.preform_temp_gap1, "preform_temp_gap1", "%0.3f");
        PRINT_PARAMETER(machine.workSet.preform_temp_gap2, "preform_temp_gap2", "%0.3f");
        PRINT_PARAMETER(machine.workSet.preform_temp_gap3, "preform_temp_gap3", "%0.3f");
        PRINT_PARAMETER(machine.workSet.init_heat_ratio1, "init_heat_ratio1", "%0.3f");
        PRINT_PARAMETER(machine.workSet.init_heat_ratio2, "init_heat_ratio2", "%0.3f");
        PRINT_PARAMETER(machine.workSet.global_heat_ratio1, "global_heat_ratio1", "%0.3f");
        PRINT_PARAMETER(machine.workSet.global_heat_ratio2, "global_heat_ratio2", "%0.3f");
        PRINT_PARAMETER(machine.workSet.standby_heat_ratio1, "standby_heat_ratio1", "%0.3f");
        PRINT_PARAMETER(machine.workSet.standby_heat_ratio2, "standby_heat_ratio2", "%0.3f");

        PRINT_PARAMETER(machine.workSet.owen1_min_temp, "owen1_min_temp", "%0.3f");
        PRINT_PARAMETER(machine.workSet.owen2_min_temp, "owen2_min_temp", "%0.3f");
        PRINT_PARAMETER(machine.workSet.owen1_max_temp, "owen1_max_temp", "%0.3f");
        PRINT_PARAMETER(machine.workSet.owen2_max_temp, "owen2_max_temp", "%0.3f");

        PRINT_PARAMETER(machine.workSet.owen1_row[0], "owen1_row1", "%0.3f");
        PRINT_PARAMETER(machine.workSet.owen1_row[1], "owen1_row2", "%0.3f");
        PRINT_PARAMETER(machine.workSet.owen1_row[2], "owen1_row3", "%0.3f");
        PRINT_PARAMETER(machine.workSet.owen1_row[3], "owen1_row4", "%0.3f");
        PRINT_PARAMETER(machine.workSet.owen1_row[4], "owen1_row5", "%0.3f");
        PRINT_PARAMETER(machine.workSet.owen1_row[5], "owen1_row6", "%0.3f");
        PRINT_PARAMETER(machine.workSet.owen1_row[6], "owen1_row7", "%0.3f");
        PRINT_PARAMETER(machine.workSet.owen1_row[7], "owen1_row8", "%0.3f");
        PRINT_PARAMETER(machine.workSet.owen1_row[8], "owen1_row9", "%0.3f");
        PRINT_PARAMETER(machine.workSet.owen1_row[9], "owen1_row10", "%0.3f");
        PRINT_PARAMETER(machine.workSet.owen1_row[1], "owen1_row11", "%0.3f");
        PRINT_PARAMETER(machine.workSet.owen1_row[11], "owen1_row12", "%0.3f");
        PRINT_PARAMETER(machine.workSet.owen1_row[12], "owen1_row13", "%0.3f");
        PRINT_PARAMETER(machine.workSet.owen1_row[13], "owen1_row14", "%0.3f");
        PRINT_PARAMETER(machine.workSet.owen1_row[14], "owen1_row15", "%0.3f");
        PRINT_PARAMETER(machine.workSet.owen1_row[15], "owen1_row16", "%0.3f");

        ///////////////////////////////////////////
        // Invio dati
        //
        SOCKET_SEND_DATA();


        /*
        float owen1_power[16], owen2_power[16];
         */

        PRINT_PARAMETER(machine.workSet.pit_unlock_time_ms, "pit_unlock_time_ms", "%d");
        PRINT_PARAMETER(machine.workSet.preform_loader_up_time_ms, "preform_loader_up_time_ms", "%d");
        PRINT_PARAMETER(machine.workSet.bottle_eject_down_time_ms, "bottle_eject_down_time_ms", "%d");
        PRINT_PARAMETER(machine.workSet.empy_preform_elevator_timeout_ms, "empy_preform_elevator_timeout_ms", "%d");
        PRINT_PARAMETER(machine.workSet.empy_preform_orientator_roller_timeout_ms, "empy_preform_orientator_roller_timeout_ms", "%d");
        PRINT_PARAMETER(machine.workSet.empy_preform_orientator_pit_timeout_ms, "empy_preform_orientator_pit_timeout_ms", "%d");
        PRINT_PARAMETER(machine.workSet.fan_motor_time_msec, "fan_motor_time_msec", "%d");
        PRINT_PARAMETER(machine.workSet.roll_motor_time_msec, "roll_motor_time_msec", "%d");
        PRINT_PARAMETER(machine.workSet.aspiration_persistence_time_msec, "aspiration_persistence_time_msec", "%d");
        PRINT_PARAMETER(machine.workSet.preform_elevator_time_msec, "preform_elevator_time_msec", "%d");
        PRINT_PARAMETER(machine.workSet.unjammer_time_msec, "unjammer_time_msec", "%d");

        PRINT_PARAMETER(machine.workSet.num_bottles_to_product, "num_bottles_to_product", "%d");
        PRINT_PARAMETER(machine.workSet.online_bottles_transfer_present, "online_bottles_transfer_present", "%d");
        PRINT_PARAMETER(machine.workSet.force_bottles_discharge, "force_bottles_discharge", "%d");



        strncat((char*) out_str, (char*) "</table></center>", (App.sendBufferSize - 32 - strlen((char*) out_str)));


        ///////////////////////////////////////////
        // Invio dati
        //
        SOCKET_SEND_DATA();

        strncat((char*) out_str, (char*) "<center><br/><table width=600 >", (App.sendBufferSize - 255 - strlen((char*) out_str)));
        strncat((char*) out_str, (char*) "<tr style=\"background-color:#ECEAEA\"><td colspan=2><h2>SETTINGS</h2> (machine.settings.*)</td></tr>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
        strncat((char*) out_str, (char*) "<tr style=\"background-color:#ECEAEA\"><td class=\"r\">Name</td><td class=\"r\">Value</td></tr>", (App.sendBufferSize - 255 - strlen((char*) out_str)));


        PRINT_PARAMETER(machine.settings.startup_owens_cycles, "startup_owens_cycles", "%d");
        PRINT_PARAMETER(machine.settings.startup_owens_delay_ms, "startup_owens_delay_ms", "%d");
        PRINT_PARAMETER(machine.settings.turnoff_owens_cycles, "turnoff_owens_cycles", "%d");
        PRINT_PARAMETER(machine.settings.turnoff_owens_delay_ms, "turnoff_owens_delay_ms", "%d");
        PRINT_PARAMETER(machine.settings.owens_standby_cycles, "owens_standby_cycles", "%d");
        PRINT_PARAMETER(machine.settings.owens_standby_delay_ms, "owens_standby_delay_ms", "%d");
        PRINT_PARAMETER(machine.settings.owens_towork_cycles, "owens_towork_cycles", "%d");
        PRINT_PARAMETER(machine.settings.owens_towork_delay_ms, "owens_towork_delay_ms", "%d");
        PRINT_PARAMETER(machine.settings.initial_owens_cycles, "initial_owens_cycles", "%d");
        PRINT_PARAMETER(machine.settings.initial_owens_ratio, "initial_owens_ratio", "%0.3f");
        PRINT_PARAMETER(machine.settings.chain_stepper1_pause_ms, "chain_stepper1_pause_ms", "%d");
        PRINT_PARAMETER(machine.settings.chain_stepper2_pause_ms, "chain_stepper2_pause_ms", "%d");
        PRINT_PARAMETER(machine.settings.chain_stepper3_pause_ms, "chain_stepper3_pause_ms", "%d");
        PRINT_PARAMETER(machine.settings.trasf_x_forward_pause_ms, "trasf_x_forward_pause_ms", "%d");
        PRINT_PARAMETER(machine.settings.chain_trasf_z_down_pause_ms, "chain_trasf_z_down_pause_ms", "%d");
        PRINT_PARAMETER(machine.settings.chain_picker_open_pause_ms, "chain_picker_open_pause_ms", "%d");
        PRINT_PARAMETER(machine.settings.chain_picker_close_pause_ms, "chain_picker_close_pause_ms", "%d");
        PRINT_PARAMETER(machine.settings.pref_load_inside_pause_ms, "pref_load_inside_pause_ms", "%d");
        PRINT_PARAMETER(machine.settings.pref_load_outside_pause_ms, "pref_load_outside_pause_ms", "%d");
        PRINT_PARAMETER(machine.settings.pit_stopper_inside_pause_ms, "pit_stopper_inside_pause_ms", "%d");
        PRINT_PARAMETER(machine.settings.pit_stopper_outside_pause_ms, "pit_stopper_outside_pause_ms", "%d");
        PRINT_PARAMETER(machine.settings.aspirator_delay_ms, "aspirator_delay_ms", "%d");

#elif xCNC_COMPILE
        PRINT_PARAMETER(machine.workSet.rapid_feed, "rapid_feed", "%0.1f");
        PRINT_PARAMETER(machine.workSet.mill_feed, "mill_feed", "%0.1f");
        PRINT_PARAMETER(machine.workSet.spindle_speed, "spindle_speed", "%0.1f");

        PRINT_PARAMETER(machine.workSet.current_tool, "current_tool", "%d");
        PRINT_PARAMETER(machine.workSet.tool_diam, "tool_diam", "%0.3f");
        PRINT_PARAMETER(machine.workSet.tool_height, "tool_height", "%0.3f");
        PRINT_PARAMETER(machine.workSet.tool_time, "tool_time", "%d");

        PRINT_PARAMETER(machine.workSet.tool_name, "tool_name", "%s");
#endif
        

    
        strncat((char*) out_str, (char*) "</table></center>", (App.sendBufferSize - 32 - strlen((char*) out_str)));




    } else if (strnicmp((char*) ptrURL, (char*) "RESET_RTC", 9) == 0) {

        // Intestazione predefinita risposta
        handle_http_default_header_response(out_str, out_str_size, PtrOrigin, NULL);

        ////////////////////
        // Reset RTC
        //
        dataExchangeReset();


        ///////////////////////////////////////////
        // Invio dati
        //
        SOCKET_SEND_DATA();

    } else if (strnicmp((char*) ptrURL, (char*) "REBOOT", 6) == 0) {

        if (is_machine_stopped()) {
            
            if (has_user_permission(0)) {

                //////////////////////////
                // risposta HTTP
                //
                handle_http_default_header_response(out_str, out_str_size, PtrOrigin, NULL);

                ////////////////////
                // Reboot
                //
                xrt_reboot();

            } else {
                strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkRed; padding:20px; background-color:rgb(255,235,235); font-size:36px;\">Cannot reboot : Need permission level 0 (guest)", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                snprintf(msg, msg_size, (char*) " (err:%d)", errno);
                /// strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));
                // risposta HTTP
                handle_http_default_header_response(out_str, out_str_size, PtrOrigin, msg);
            }
        } else {
            strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkRed; padding:20px; background-color:rgb(255,235,235); font-size:36px;\">Cannot reboot reboot until machine is running", (App.sendBufferSize - 64 - strlen((char*) out_str)));
            snprintf(msg, msg_size, (char*) " (err:%d)", errno);
            /// strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));
            // risposta HTTP
            handle_http_default_header_response(out_str, out_str_size, PtrOrigin, msg);
        }        
        

        
        
        
        //////////////////////////////////////
        // Invio GCode fa fonte esterna
        //
        
#ifdef xCNC_COMPILE        
        
        
    } else if (strnicmp((char*) ptrURL, (char*) "machine.App.GCode", 13) == 0) {
        
        // N.B.: Gcode con il metodo get : legge il contenuto della macchina


        if (is_machine_stopped()) {
            
            if (has_user_permission(0)) {

                ptrURL += 13;
                
                //////////////////////////
                // risposta HTTP
                //
                snprintf(msg, msg_size, (char*)"<br/><center><span style=\"color:darkGreen; padding:20px; background-color:rgb(255,235,235); font-size:36px;\">Recived %d bytes of GCode", nReciv);
                strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));
                // risposta HTTP
                handle_http_default_header_response(out_str, out_str_size, PtrOrigin, NULL);

                
            } else {
                strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkRed; padding:20px; background-color:rgb(255,235,235); font-size:36px;\">Cannot reboot : Need permission level 0 (guest)", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                snprintf(msg, msg_size, (char*) " (err:%d)", errno);
                // strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));
                // risposta HTTP
                handle_http_default_header_response(out_str, out_str_size, PtrOrigin, msg);
            }
        } else {
            strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkRed; padding:20px; background-color:rgb(255,235,235); font-size:36px;\">Cannot reboot reboot until machine is running", (App.sendBufferSize - 64 - strlen((char*) out_str)));
            snprintf(msg, msg_size, (char*) " (err:%d)", errno);
            // strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));
            // risposta HTTP
            handle_http_default_header_response(out_str, out_str_size, PtrOrigin, msg);
        }        
        

#endif
        
        
    } else if (strnicmp((char*) ptrURL, (char*) "favicon.ico", 11) == 0) {

        handle_http_empty_header_response(out_str, out_str_size);

    } else {

        //////////////////////////////////
        // HOME
        //

        // Intestazione predefinita risposta
        handle_http_default_header_response(out_str, out_str_size, PtrOrigin, NULL);


    }




end_of_response:


    // strncat((char*) out_str, (char*) "</body>", (App.sendBufferSize - 32 - strlen((char*) out_str)));


    if ((uint32_t) strlen((char*) out_str) >= App.sendBufferSize) {
        snprintf(msg, msg_size, "[!http size:%u/%u]\n", (uint32_t) strlen((char*) out_str), App.sendBufferSize);
        vDisplayMessage(msg);
        retVal = -1;
        goto end_func;
    }


end_func:



    if (msg)
        free(msg);

    return retVal;
}



int handle_http_update_firmware_form(char *out_str, uint32_t *out_str_size) {
    uint32_t sendBufferSize = out_str_size[0];


    strncat((char*) out_str, (char*) "<script type=\"text/javascript\" language=\"javascript\">\n\
        window.addEventListener('message', receiveMessage);\n\
        function receiveMessage(evt) {\n\
            var varArray = evt.data.split('=');\n\
            var obj = document.getElementById(varArray[0]);\n\
            if (obj) {\n\
                if (obj.id == 'frmFileUrl') {\n\
                    var XACAddr = document.getElementById(\"gatewayAddress\").value;\n\
                    var XACRes = document.getElementById(\"gatewayResource\").value;\n\
                    var XACPort = document.getElementById(\"gatewayPort\").value;\n\
                    var MacAddr = document.getElementById(\"macAddr\").value;\n\
                    obj.value = 'http://' + XACAddr +':' + XACPort + '/' + XACRes + '/' + MacAddr + '/' + varArray[1];\n\
                } else {\n\
                    obj.value = varArray[1];\n\
                }\n\
            }\n\
        }\n\
        function getFirmwareList() {\n\
        var frmListFramme = document.getElementById('frmListHTTPframe');\n\
        var XACAddr = document.getElementById(\"gatewayAddress\").value;\n\
        var XACRes = document.getElementById(\"gatewayResource\").value;\n\
        var XACPort = document.getElementById(\"gatewayPort\").value;\n\
        var MacAddr = document.getElementById(\"macAddr\").value;\n\
        var objHTTPFrame = window;\n\
        var frmAddr = 'http://' + XACAddr +':' + XACPort + '/' + XACRes + '/firmwaretListContent.jsp?MacAddr=' + MacAddr + '';\n\
        if (XACAddr) {\n\
                if (MacAddr) {\n\
                        // alert(frmAddr);\n\
                        // objHTTPFrame.document.getElementById('frmFileUrl').value = XACAddr +':' + XACPort + '/' + XACRes + '/' + MacAddr + '/Firmwares/lcu.2.8'; // '/XAC/0.50.ba.cb.b2.e3/Firmwares/lcu.1.6';\n\
                        if (frmListFramme) {\n\
                            if (frmListFramme.src != frmAddr) {\n\
                                frmListFramme.src = frmAddr;\n\
                            } else {\n\
                                frmListFramme.src = ''\n\
                                frmListFramme.src = frmAddr;\n\
                                if (frmListFramme.contentWindow)\n\
                                    frmListFramme.contentWindow.location.reload(true);\n\
                                else\n\
                                    frmListFramme.location.reload(true);\n\
                            }\n\
                        }\n\
                } else {\n\
                    alert('Invalid CU Mac Addr!\\n\\nPlease connect to the CU before start remote service');\n\
                }\n\
        } else {\n\
            alert('Invalid XAC server!\\n\\nPlease define xProject Assist Center server address');\n\
        }\n\
        // window.parent.postMessage('gatewayAddress', '*');\n\
    }\n\
    \n\
    function confirmUpdateFirmware(obj) {\n\
        if (confirm('Warning : Firmware will be update...\\n\\nProceed now ?')) { \n\
            if (obj) obj.disabled  = true;\n\
            document.getElementById('updateFirmwareURL').submit();\n\
            return 0;\n\
        } else {\n\
            return 1;\n\
        }\n\
    }\n\
    </script>", (App.sendBufferSize - 512 - strlen((char*) out_str)));


    strncat((char*) out_str, (char*) "<center><form action=\"/UPDATE_FIRMWARE\" method=\"POST\" id=\"updateFirmware\" enctype=\"multipart/form-data\" >", (App.sendBufferSize - 255 - strlen((char*) out_str)));
    strncat((char*) out_str, (char*) "<table width=600 >", (App.sendBufferSize - 255 - strlen((char*) out_str)));
    strncat((char*) out_str, (char*) "<tr style=\"background-color:#ECEAEA\"><td colspan=2><div><span style=\"font-size:27px\">UPDATE FIRMWARE <b>from file</b></span></div></td></tr>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
    strncat((char*) out_str, (char*) "<tr style=\"background-color:#transparent\"><td class=\"r\">File to upload</td><td class=\"r\"><input id=\"file\" name=\"file\" type=\"file\" accept=\".\" style=\"border:0px solid red; font-size: 17px; -moz-border-radius: 5px; -webkit-border-radius: 5px; border-radius: 5px; -khtml-border-radius: 5px;\" onClick=\"document.getElementById(frmFileUrl').value='';\"/></td></tr>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
    strncat((char*) out_str, (char*) "<tr style=\"background-color:#ECEAEA\">", (App.sendBufferSize - 255 - strlen((char*) out_str)));
    strncat((char*) out_str, (char*) "<td colspan=2><table><tr><td><h2>Warning</h2>Sending wrong or corrupted file may hang the Control Unit and stop permanently the communications. Please make sure to upload the right file</td>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
    strncat((char*) out_str, (char*) "<td class=\"r\" colspan=2><button width=200 id=\"Send\" type=\"button\" onclick=\"confirmUpdateFirmware(this);\" style=\"border:1px solid navy; font-size: 26px; padding:10px; -moz-border-radius: 5px; -webkit-border-radius: 5px; border-radius: 5px; -khtml-border-radius: 5px; \">Send file</button></td></tr></table></td>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
    strncat((char*) out_str, (char*) "</tr>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
    strncat((char*) out_str, (char*) "</table></form></center>", (App.sendBufferSize - 32 - strlen((char*) out_str)));



    strncat((char*) out_str, (char*) "<center><form action=\"/UPDATE_FIRMWARE\" method=\"POST\" id=\"updateFirmwareURL\" enctype=\"application/x-www-form-urlencoded\" >", (App.sendBufferSize - 255 - strlen((char*) out_str)));
    strncat((char*) out_str, (char*) "<input id=\"gatewayAddress\" type=\"hidden\" />", (App.sendBufferSize - 255 - strlen((char*) out_str)));
    strncat((char*) out_str, (char*) "<input id=\"gatewayResource\" type=\"hidden\" />", (App.sendBufferSize - 255 - strlen((char*) out_str)));
    strncat((char*) out_str, (char*) "<input id=\"gatewayPort\" type=\"hidden\" />", (App.sendBufferSize - 255 - strlen((char*) out_str)));
    strncat((char*) out_str, (char*) "<input id=\"macAddr\" type=\"hidden\" />", (App.sendBufferSize - 255 - strlen((char*) out_str)));
    strncat((char*) out_str, (char*) "<table width=600 >", (App.sendBufferSize - 255 - strlen((char*) out_str)));
    strncat((char*) out_str, (char*) "<tr style=\"background-color:#EDEBEB\"><td colspan=3><div><span style=\"font-size:27px\">UPDATE FIRMWARE <b>from XAC</b></span></div></td></tr>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
    strncat((char*) out_str, (char*) "<tr style=\"background-color:transparent\"><td colspan=3>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
    strncat((char*) out_str, (char*) "<iframe id=\"frmListHTTPframe\" src=\"\" style=\"height: --110px; width: 98%; display:block;\
                                                                                                        padding-left: 12px; \
													margin-top:5px; margin-left:5px; margin-right:15px; margin-bottom:0px; \
													border-radius: 11px 0px 0px 11px; \
													background-color:#FAFAFA; color:#121212; \
													border:0px solid darkGray; \
													overflow:auto; \">\
													</iframe>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
    strncat((char*) out_str, (char*) "</td></tr>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
    strncat((char*) out_str, (char*) "<tr style=\"background-color:#EDEBEB\">", (App.sendBufferSize - 255 - strlen((char*) out_str)));
    strncat((char*) out_str, (char*) "<td style=\"width: 80px;\">Firmware</td><td colspan=2 class=\"r\"><input style=\"width: 100%; padding: 6px; \" id=\"frmFileUrl\" name=\"frmFileUrl\" type=\"text\" /></td>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
    strncat((char*) out_str, (char*) "</tr>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
    strncat((char*) out_str, (char*) "<tr style=\"background-color:#transparent\">", (App.sendBufferSize - 255 - strlen((char*) out_str)));
    strncat((char*) out_str, (char*) "<td colspan=3><table style=\"width: 100%; border:0px; \"><tr>", (App.sendBufferSize - 32 - strlen((char*) out_str)));    
    strncat((char*) out_str, (char*) "<td><a href=\"javascript:void(0)\" width=100 onClick=\"window.location.href='/'\" style=\"font-size: 26px; padding:10px; \">Cancel</a></td>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
    strncat((char*) out_str, (char*) "<td><button name=\"GetFirmware\" type=\"button\" style=\"padding: 15px; font-size: 17px; -moz-border-radius: 5px; -webkit-border-radius: 5px; border-radius: 5px; -khtml-border-radius: 5px;\" onclick=\"getFirmwareList()\">Get from XAC</button></td>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
    strncat((char*) out_str, (char*) "<td class=\"r\"><button width=100 id=\"Update\" type=\"button\"  onclick=\"confirmUpdateFirmware(this);\" style=\"border:1px solid red; font-size: 26px; padding:10px; -moz-border-radius: 5px; -webkit-border-radius: 5px; border-radius: 5px; -khtml-border-radius: 5px; \">Update</button></td>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
    strncat((char*) out_str, (char*) "</tr></table></td>", (App.sendBufferSize - 32 - strlen((char*) out_str)));    
    strncat((char*) out_str, (char*) "</tr>", (App.sendBufferSize - 255 - strlen((char*) out_str)));
    strncat((char*) out_str, (char*) "</table></form></center>", (App.sendBufferSize - 32 - strlen((char*) out_str)));

    return 1;
}

int handle_http_put_request(TcpSocket httpSocket, char *HTTPRequest, char *requestBody, uint32_t nReciv, char *out_str, uint32_t *out_str_size, char *PtrOrigin) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint32_t sendBufferSize = out_str_size[0];
    uint32_t nSend = 0, nSent = 0;
    int i, j;

    int retVal = 1;


    if (strcmpi((char*) HTTPRequest, (char*) "application/x-www-form-urlencoded") == 0) {

    }

    if (strnicmp((char*) HTTPRequest, (char*) "/FILE ", 6) == 0) {

    }

    return retVal;
}






int handle_http_post_request(TcpSocket httpSocket, char *HTTPRequest, char *requestBody, uint32_t nReciv, char *out_str, uint32_t *out_str_size, char *PtrOrigin, char *RequestingHost, char *ContentType, char *Boundary, char *ContentLen) {
    uint32_t sendBufferSize = out_str_size[0];
    uint32_t nSend = 0, nSent = 0;
    char msg[256];
    uint32_t msg_size = sizeof (msg);
    BOOL getFromURL = FALSE;
    char *sfrmFileUrl = NULL, *frmFileUrl = NULL;
    char *fname = (char *) "./lcu", *fBKname = (char *) "./lcu.bak", *tmpfname = (char *) "./lcu.tmp";
    char baseFolder[256], filename[256], bkfilename[256], params[256], tmpfilename[256];



    int retVal = 1;


    if (App.ExecFilePath[0]) {
        strip_file_name(App.ExecFilePath, baseFolder, sizeof (baseFolder), NULL, 0, 0 + 1);
    } else {
        get_cur_module_path(baseFolder, sizeof (baseFolder));
        getcwd(baseFolder, sizeof (baseFolder));
    }

    snprintf(filename, sizeof (filename), "%s/%s", baseFolder, fname);
    snprintf(bkfilename, sizeof (bkfilename), "%s/%s", baseFolder, fBKname);
    snprintf(tmpfilename, sizeof (tmpfilename), "%s/%s", baseFolder, tmpfname);


    
    if (strnicmp((char*) HTTPRequest, (char*) "POST /UPDATE_FIRMWARE HTTP/", 27) == 0) {

        if (is_machine_stopped()) {
        
            if (has_user_permission(2)) {

                if (requestBody != NULL) {
                    if (strlen(requestBody) < 2048) {
                        frmFileUrl = strstr(requestBody, "frmFileUrl=");
                        if (frmFileUrl) {
                            frmFileUrl += 11;
                            if (*frmFileUrl == '"') frmFileUrl++;
                            sfrmFileUrl = frmFileUrl;
                            while (*frmFileUrl && *frmFileUrl != '\r' && *frmFileUrl != '\n')
                                frmFileUrl++;
                            if (*frmFileUrl == '\r' || *frmFileUrl == '\n' || *frmFileUrl == '"')
                                *frmFileUrl = 0;


                            if (sfrmFileUrl && strlen(sfrmFileUrl) > 12) {
                                getFromURL = TRUE;
                            }
                        }
                    }
                }

                if (getFromURL) {
                    int res = 0, fileSize = 0;
                    char urlDecoded[256], Err[256], MacAddress[64];


                    snprintf(MacAddress, sizeof (MacAddress), "%x.%x.%x.%x.%x.%x", App.MacAddress[0], App.MacAddress[1], App.MacAddress[2], App.MacAddress[3], App.MacAddress[4], App.MacAddress[5]);

                    snprintf(params, sizeof (params), "MacAddr=%s", MacAddress);


                    urldecode2(sfrmFileUrl, urlDecoded, sizeof (urlDecoded));

                    res = http_request(urlDecoded, (char*) "GET", params, tmpfilename, &fileSize, Err, sizeof (Err));
                    if (res <= 0) {
                        strncat((char*) out_str, (char*) "<br/><center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                        strncat((char*) out_str, (char*) "<table style=\"color:darkRed; padding:20px;\"><tr><td colspan=2 style=\"color:darkRed; padding:20px; font-size:36px;\"> ERROR : unable to update firmware</td></tr>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                        strncat((char*) out_str, (char*) "<tr><td>URL</td><td>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                        strncat((char*) out_str, (char*) urlDecoded, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));
                        strncat((char*) out_str, (char*) "</td></tr>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                        strncat((char*) out_str, (char*) "<tr><td>Error</td><td>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                        snprintf(msg, msg_size, (char*) "%d - %s", res, Err);
                        strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));
                        strncat((char*) out_str, (char*) "</td></tr>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                        strncat((char*) out_str, (char*) "<tr><td>Mac Address</td><td>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                        strncat((char*) out_str, (char*) MacAddress, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));
                        strncat((char*) out_str, (char*) "</td></tr>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                        strncat((char*) out_str, (char*) "</table><br/></center>", (App.sendBufferSize - 64 - strlen((char*) out_str)));

                        // Generazione allarme
                        snprintf(msg, sizeof (msg), "[XP] Error Updating firmware (url:%s) (folder:%s) (err:%d)", urlDecoded, baseFolder, res);
                        if (generate_alarm((char*) msg, 9998, 0, (int) ALARM_FATAL_ERROR, 0+1) < 0) {
                        }

                    } else {

                        if (!remove(bkfilename) && access(bkfilename, F_OK) != -1) {
                            strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkRed; padding:20px; background-color:rgb(255,235,235); font-size:36px;\">ERROR : unable to remove backup file</span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                        } else {
                            if (rename(filename, bkfilename) != 0) {
                                strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkRed; padding:20px; background-color:rgb(255,235,235); font-size:36px;\">ERROR : unable to rename file</span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                            } else {
                                if (rename(tmpfilename, filename) != 0) {
                                    strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkRed; padding:20px; background-color:rgb(255,235,235); font-size:36px;\">ERROR : unable to rename file</span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                                } else {
                                    if (chmod(filename, S_IRWXO | S_IRWXG | S_IRWXU) != 0) {
                                        strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkRed; padding:20px; background-color:rgb(255,235,235); font-size:36px;\">ERROR : unable to change file permission", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                                        snprintf(msg, msg_size, (char*) " (err:%d)", errno);
                                        strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));
                                        strncat((char*) out_str, (char*) "</span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                                    } else {

                                        snprintf(msg, sizeof (msg), "[XP] Firmware updated (url:%s)", urlDecoded);
                                        if (generate_alarm((char*) msg, 9999, 0, (int) ALARM_WARNING, 0+1) < 0) {
                                        }
                                        vDisplayMessage((char*) msg);
                                        strncat((char*) out_str, (char*) "<br/><center><span style=\"color:navy; font-size:36px;\">Firmware updated...<br/><span style=\"font-size:17px;\">(", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                                        snprintf(msg, msg_size, (char*) "%s", urlDecoded);
                                        strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));
                                        snprintf(msg, msg_size, (char*) " %0.1fkb", (float) fileSize / 1024.0f);
                                        strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));
                                        strncat((char*) out_str, (char*) ")</span></span></center><br/>", (App.sendBufferSize - 32 - strlen((char*) out_str)));
                                        strncat((char*) out_str, (char*) "<br/><center><span style=\"color:navy; font-size:27px;\">In order to take effect <a href='/REBOOT'>reoot the system</a>...<span style=\"font-size:17px;\">", (App.sendBufferSize - 64 - strlen((char*) out_str)));

                                        goto handle_default_reponse;
                                    }
                                }
                            }
                        }
                    }

                } else {

                    if (strnicmp((char*) ContentType, (char*) "Multipart/form-data", 19) == 0) {

                        if (Boundary && Boundary[0]) {
                            int iContentLen = 0;

                            if (ContentLen)
                                iContentLen = atoi(ContentLen);

                            if (iContentLen > 0) {

                                // vDisplayMessage((char*) requestBody);
                                strncpy(msg, "--", sizeof (msg));
                                strncat(msg, Boundary, sizeof (msg));

                                if (requestBody) {
                                    char *startFileContent = requestBody + 4;
                                    char *endFileContent = strstr(requestBody + 4, msg);

                                    if (!endFileContent)
                                        endFileContent = strstr(requestBody + 4, Boundary);

                                    requestBody += 4;


                                    if (endFileContent) {
                                        // endFileContent[0] = 0;                        
                                    }

                                    if (startFileContent) {
                                        unsigned int fileContentSize3 = 0, fileContentSize2 = 0, fileContentSize = strlen(startFileContent);
                                        char *subRequesrBody = NULL;

                                        if (fileContentSize) {

                                            if (strnicmp((char*) requestBody, (char*) Boundary, strlen(Boundary)) == 0) {
                                                requestBody += strlen(Boundary);
                                            }

                                            subRequesrBody = strstr(requestBody, "\r\n\r\n");
                                            if (!subRequesrBody)
                                                subRequesrBody = strstr(requestBody, "\r\r\r\r");
                                            if (!subRequesrBody)
                                                subRequesrBody = strstr(requestBody, "\n\n\n\n");
                                            if (!subRequesrBody)
                                                subRequesrBody = strstr(requestBody, "\n\r\n\r");

                                            if (subRequesrBody) {
                                                // *subRequesrBody = 0;
                                                subRequesrBody += 4;
                                            }

                                            if (subRequesrBody) {
                                                unsigned int currentPos = (unsigned int) &subRequesrBody[0] - (unsigned int) &HTTPRequest[0];
                                                unsigned int currentBodyPos = (unsigned int) &subRequesrBody[0] - (unsigned int) &requestBody[0];

                                                startFileContent = subRequesrBody;
                                                fileContentSize = nReciv - currentPos;

                                                fileContentSize2 = iContentLen - currentBodyPos;
                                                fileContentSize3 = endFileContent ? ((unsigned int) &endFileContent[0] - (unsigned int) &startFileContent[0]) : fileContentSize;

                                                if (fileContentSize3 + 4 < fileContentSize2) {
                                                    // Lettura incompeta
                                                    return -9;
                                                }

                                                // vDisplayMessage((char*) startFileContent);\

                                                if (startFileContent[0] == '\x7f' && startFileContent[1] == 'E' && startFileContent[2] == 'L' && startFileContent[3] == 'F') {

                                                    if (baseFolder[0])
                                                        chdir(baseFolder);

                                                    if (!remove(bkfilename) && access(bkfilename, F_OK) != -1) {
                                                        strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkRed; padding:20px; background-color:rgb(255,235,235); font-size:36px;\">ERROR : unable to remove backup file</span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                                                    } else {
                                                        if (rename(filename, bkfilename) != 0) {
                                                            strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkRed; padding:20px; background-color:rgb(255,235,235); font-size:36px;\">ERROR : unable to rename file</span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                                                        } else {
                                                            FILE *f = fopen(filename, "w+");
                                                            if (!f || (int) f == (int) - 1) {
                                                                strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkRed; padding:20px; background-color:rgb(255,235,235); font-size:36px;\">ERROR : unable create file</span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                                                            } else {
                                                                if (fwrite(startFileContent, fileContentSize3, 1, f) <= 0) {
                                                                    strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkRed; padding:20px; background-color:rgb(255,235,235); font-size:36px;\">ERROR : unable to write to file</span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                                                                } else {
                                                                    if (fclose(f) != 0) {
                                                                        strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkRed; padding:20px; background-color:rgb(255,235,235); font-size:36px;\">ERROR : unable to close file</span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                                                                    } else {
                                                                        if (chmod(filename, S_IRWXO | S_IRWXG | S_IRWXU) != 0) {
                                                                            strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkRed; padding:20px; background-color:rgb(255,235,235); font-size:36px;\">ERROR : unable to change file permission", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                                                                            snprintf(msg, msg_size, (char*) " (err:%d)", errno);
                                                                            strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));
                                                                            strncat((char*) out_str, (char*) "</span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                                                                        } else {
                                                                            snprintf(msg, sizeof (msg), "[XP] Firmware updated (size:%d bytes) (folder:%s)", fileContentSize3, baseFolder);
                                                                            if (generate_alarm((char*) msg, 9999, 0, (int) ALARM_WARNING, 0+1) < 0) {
                                                                            }
                                                                            vDisplayMessage((char*) msg);
                                                                            
                                                                            strncat((char*) out_str, (char*) "<br/><center><span style=\"color:navy; font-size:36px;\">Firmware updated...<span style=\"font-size:17px;\">(", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                                                                            snprintf(msg, msg_size, (char*) "%0.2f kb", (float) fileContentSize3 / 1024.0f);
                                                                            strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));
                                                                            strncat((char*) out_str, (char*) ")</span></span></center><br/>", (App.sendBufferSize - 32 - strlen((char*) out_str)));
                                                                            strncat((char*) out_str, (char*) "<br/><center><span style=\"color:navy; font-size:27px;\">In order to take effect <a href='/REBOOT'>reoot the system</a>...<span style=\"font-size:17px;\">", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                                                                            
                                                                            goto handle_default_reponse;
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                } else {
                                                    strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkRed; padding:20px; background-color:rgb(255,235,235); font-size:36px;\">ERROR : wrong file type</span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                                                }
                                            } else {
                                                strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkRed; padding:20px; background-color:rgb(255,235,235); font-size:36px;\">ERROR : Invalid file content</span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                                            }
                                        } else {
                                            strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkRed; padding:20px; background-color:rgb(255,235,235); font-size:36px;\">ERROR : Empty file content</span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                                        }
                                    } else {
                                        strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkRed; font-size:36px;\">ERROR : Wrong file content</span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                                    }
                                } else {
                                    strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkRed; font-size:36px;\">ERROR : Wrong body content</span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                                }
                            } else {
                                strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkRed; font-size:36px;\">ERROR : Wrong file size</span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                            }
                        } else {
                            strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkRed; font-size:36px;\">ERROR : Wrong Boundary</span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                        }
                    } else {
                        strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkRed; font-size:36px;\">ERROR : Unknown Content-Type on Request <span style=\"font-size:17px;\">( '", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                        strncat((char*) out_str, (char*) ContentType, (App.sendBufferSize - 64 - strlen((char*) out_str)));
                        strncat((char*) out_str, (char*) "' - only multipart/form-data is supported) </span></span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                    }
                }
            } else {
                strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkRed; padding:20px; background-color:rgb(255,235,235); font-size:36px;\">Permission level 2 (Admin) required</span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                if (generate_alarm((char*) "UPDATE FIRMWARE : Permission denied", 9901, 0, (int) ALARM_WARNING, 0+1) < 0) {
                }
            }
        } else {
            strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkRed; padding:20px; background-color:rgb(255,235,235); font-size:36px;\">Machine must be in STOP state</span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
            if (generate_alarm((char*) "Cannot reboot until machine is running", 9902, 0, (int) ALARM_WARNING, 0+1) < 0) {
            }
        }
        



#ifdef xCNC_COMPILE
        
    } else if (strnicmp((char*) HTTPRequest, (char*) "POST /machine.App.GCode HTTP/", 25) == 0) {

        if (is_machine_stopped()) {
        
            if (has_user_permission(0)) {


                if (strnicmp((char*) ContentType, (char*) "Multipart/form-data", 19) == 0) {
                    
                    // Invio di un file : da implementare
                    
                } else if (strnicmp((char*) ContentType, (char*) "application/x-www-form-urlencoded", 33) == 0) {                                      

                    if (requestBody && requestBody[0]) {
                        int iContentLen = 0;

                        if (ContentLen)
                            iContentLen = atoi(ContentLen);

                        if (iContentLen > 0) {
                            uint32_t decoded_data_length = 0;
                            char *decoded_data = NULL;

                            decoded_data = (char*)NewBase64Decode((char*)requestBody, (size_t)strlen((char*)requestBody), (size_t*)&decoded_data_length);
                            if (decoded_data) {
                                decoded_data[decoded_data_length] = 0;            
                            
                                // Aggiornamento GCode
                                if (gcode_update_content ( decoded_data ) < 0) {
                                    strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkRed; font-size:36px;\">ERROR : Update GCode Failed</span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                                }

                                gcode_update_start_row ( machine.App.GCode.startRow );
                                                                    
                                free(decoded_data);
                                decoded_data = NULL;
                                
                                snprintf(msg, msg_size, (char*)"<br/><center><span style=\"color:darkGreen; padding:20px; background-color:rgb(235,255,235); font-size:36px;\">Recived %d line(s) <span style=\"font-size:80%%\">(%d bytes)</span> of GCode</span></center>", (int)machine.App.GCode.numRows, (int)decoded_data_length);
                                strncat((char*) out_str, (char*) msg, (App.sendBufferSize - strlen(msg) - strlen((char*) out_str)));                               
                            }
                            
                        } else {
                            strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkRed; font-size:36px;\">ERROR : Wrong GCode size</span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                        }
                    } else {
                        strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkRed; font-size:36px;\">ERROR : Wrong GCode body</span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                    }
                    
                } else {
                    strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkRed; font-size:36px;\">ERROR : Unknown Content-Type on Request <span style=\"font-size:17px;\">( '", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                    strncat((char*) out_str, (char*) ContentType, (App.sendBufferSize - 64 - strlen((char*) out_str)));
                    strncat((char*) out_str, (char*) "' - only multipart/form-data is supported) </span></span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                }

            } else {
                strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkRed; padding:20px; background-color:rgb(255,235,235); font-size:36px;\">Permission level 2 (Admin) required</span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
                if (generate_alarm((char*) "UPDATE FIRMWARE : Permission denied", 9901, 0, (int) ALARM_WARNING, 0+1) < 0) {
                }
            }
        } else {
            strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkRed; padding:20px; background-color:rgb(255,235,235); font-size:36px;\">Machine must be in STOP state</span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
            if (generate_alarm((char*) "Cannot reboot until machine is running", 9902, 0, (int) ALARM_WARNING, 0+1) < 0) {
            }
        }

#endif
        
    } else {
        strncat((char*) out_str, (char*) "<br/><center><span style=\"color:darkRed; font-size:36px;\">ERROR : Unknown POST Request:", (App.sendBufferSize - 64 - strlen((char*) out_str)));
        strncat((char*) out_str, (char*) HTTPRequest, (App.sendBufferSize - strlen(HTTPRequest) - strlen((char*) out_str)));
        strncat((char*) out_str, (char*) "</span></center><br/>", (App.sendBufferSize - 64 - strlen((char*) out_str)));
    }


handle_default_reponse:
                                                    
    // Intestazione predefinita risposta
    handle_http_default_header_response(out_str, out_str_size, PtrOrigin, NULL);
    

    *out_str_size = strlen(out_str);

    return retVal;
}