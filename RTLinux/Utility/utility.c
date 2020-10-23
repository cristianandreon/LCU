/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/////////////////////////
// RT Kernel includes
//
#ifdef WATCOM
#include "FreeRTOS.h"
#include "task.h"
#else
#include "./../RTLinux.h"
#endif

int get_cur_module_path(char *pBuf, int pBufSize) {
    char szTmp[256];
    int len = 0;
    int bytes = 0;



    // snprintf(szTmp, sizeof(szTmp), "/proc/%d/exe", getpid());

    snprintf(szTmp, sizeof (szTmp), "/proc/self/exe");
    len = strlen(szTmp);
    bytes = MIN(readlink(szTmp, pBuf, len), len - 1);
    if (bytes >= 0)
        pBuf[bytes] = '\0';
    return bytes;
}



// Mode & 1	->	Considera anche / come terminatore di directory

void strip_file_name(char *source, char *dir, uint32_t dir_size, char *file, uint32_t file_size, int Mode) {
    int x, len, pos;

    if (file) {
        file[0] = 0;
    }

    if (dir) {
        dir[0] = 0;
    }

    if (source) {
        len = strlen(source);
        pos = -1;

        if (Mode & 1) {
            for (x = 0; x < len; x++) {
                if (source[x] == '\\' || source[x] == '/') {
                    pos = x;
                }
            }
        } else {
            for (x = 0; x < len; x++) {
                if (source[x] == '\\') {
                    pos = x;
                }
            }
        }


        if (pos >= 0) {
            if (file) {
                strncpy(file, (MDB_CHAR *) & source[pos + 1], file_size);
            }
            if (dir) {
                strncpy(dir, source, dir_size);
                dir[pos] = 0;
            }
        } else {
            if (file) {
                strncpy(file, source, file_size);
            }
            if (dir) {
                dir[0] = 0;
            }
        }
    }

    return;
}

void urldecode2(const char *src, char *dst, int dst_size) {
    char a, b;
    while (*src) {
        if ((*src == '%') &&
                ((a = src[1]) && (b = src[2])) &&
                (isxdigit(a) && isxdigit(b))) {
            if (a >= 'a')
                a -= 'a' - 'A';
            if (a >= 'A')
                a -= ('A' - 10);
            else
                a -= '0';
            if (b >= 'a')
                b -= 'a' - 'A';
            if (b >= 'A')
                b -= ('A' - 10);
            else
                b -= '0';
            *dst++ = 16 * a + b;
            src += 3;

        } else if (*src == '+') {
            *dst++ = ' ';
            src++;

        } else {
            *dst++ = *src++;
        }
    }

    *dst++ = '\0';
}



#define HANDLE_HTTP_REQUEST_ERROR( __err, __exit_code) \
        vDisplayMessage(__err);\
        if(outErr) strncpy(outErr, __err, outErrSize);\
        xrt_shutdown(&socket_desc);\
        if(file) fclose(file);\
        return __exit_code;

// www.axmag.com/download/pdfurl-guide.pdf

int http_request(char *url, char *method, char *post, char *filename, int *outSize, char *outErr, int outErrSize) {
    int socket_desc = 0;

    char message[512], hostName[256], responseHeader[1024], *server_addr = NULL, *end_server_addr = NULL;
    char server_reply[512], urlDecoded[256], resource[256], *dataToWrite = NULL, *contentStr= NULL;
    int total_len = 0;
    int file_len = 0;
    int len = 0;
    int retVal = 0;
    int port = 80;
    int prepareFile = 0;
    int received_len = 0;
    int headerFound = 0;
    int statusCode = 0;
    

    FILE *file = NULL;
    struct sockaddr_in server = {0};
    struct hostent *host = NULL;




    if (outSize) *outSize = 0;
        if(outErr) *outErr = 0;

        if (!url)
        return -1;
    
    
    urldecode2(url, urlDecoded, sizeof (urlDecoded));

    //Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        snprintf(message, sizeof(message), "http_request() : Could not create socket\n");
        HANDLE_HTTP_REQUEST_ERROR(message, -2);
    }

    server_addr = strstr(urlDecoded, "https://");
    if (server_addr) {
        server_addr += 8;
    } else {
        server_addr = strstr(urlDecoded, "http://");
        if (server_addr) {
            server_addr += 7;
        }
    }

    if (!server_addr)
        server_addr = urlDecoded;

                
    resource[0] = 0;
    end_server_addr = strstr(server_addr, ":");
    if (!end_server_addr) {
        end_server_addr = strstr(server_addr, "/");
        if (end_server_addr) {
            strncpy(resource, (end_server_addr + 0), sizeof (resource));
        }
    } else {
        char *sResource = strstr(server_addr, "/");
        if (sResource) {
            char lastchar = *sResource; 
            port = atoi(end_server_addr + 1);
            *sResource = lastchar;
            strncpy(resource, (sResource + 0), sizeof (resource));
        }
    }

    
    if (!resource[0]) {
        snprintf(message, sizeof(message), "http_request() : no such host\n");
        HANDLE_HTTP_REQUEST_ERROR(message, -30);
    }
    
    if (end_server_addr) {
        char last_c = end_server_addr[0];
        end_server_addr[0] = 0;

        strncpy(hostName, server_addr, sizeof (hostName));
        
        host = gethostbyname(server_addr);
        if (host == NULL) {
            snprintf(message, sizeof(message), "http_request() : no such host\n");
            HANDLE_HTTP_REQUEST_ERROR(message, -31);
        }

        end_server_addr[0] = last_c;

        if (host) {
            // server.sin_addr.s_addr = inet_addr("198.11.181.184");
            memcpy((void*) &server.sin_addr.s_addr, (void *) host->h_addr_list[0], host->h_length);
            server.sin_family = AF_INET;
            server.sin_port = htons(port);
        } else {
            snprintf(message, sizeof(message), "http_request() : unresolved server\n");
            HANDLE_HTTP_REQUEST_ERROR(message, -40);
        }

    } else {
        snprintf(message, sizeof(message), "http_request() : undetected server name\n");
        HANDLE_HTTP_REQUEST_ERROR(message, -41);
    }


    //Connect to remote server
    // if (connect(socket_desc, (struct sockaddr *) &server, sizeof (server)) < 0) {
    if (xrt_connect_ex(socket_desc, (struct sockaddr *) &server, sizeof (server), 10) <= 0) {
        snprintf(message, sizeof(message), "http_request() : connect error, url : %s\n", urlDecoded);
        HANDLE_HTTP_REQUEST_ERROR(message, -50);
    }

    // puts("Connected\n");

    // Send request
    // Content-Type: application/octet-stream\r\nContent-Transfer-Encoding: binary\r\n
    snprintf(message, sizeof (message), "%s %s HTTP/1.1\r\nHost: %s\r\nAccept:*/*\r\nConnection: close\r\n\r\n%s", (method ? method : (post ? "POST" : "GET")), resource, hostName, post ? post : "");
    
    // vDisplayMessage(message);
    

    if (send(socket_desc, message, strlen(message), 0) < 0) {
        snprintf(message, sizeof(message), "http_request() : Send failed\n");
        HANDLE_HTTP_REQUEST_ERROR(message, -60);
    }

    // puts("Data Send\n"); 

    responseHeader[0] = 0;

    remove(filename);
    file = fopen(filename, "w+");
    if (file == NULL) {
        snprintf(message, sizeof(message), "http_request() : File could not opened\n");
        HANDLE_HTTP_REQUEST_ERROR(message, -70);
    }
    
    while (1) {
        
        server_reply[0] = 0;
        
        received_len = recv(socket_desc, server_reply, sizeof(server_reply), 0);
        
        if (received_len < 0) {
            snprintf(message, sizeof(message), "http_request() : recv failed\n");
            HANDLE_HTTP_REQUEST_ERROR(message, -80);
            
        } else if (received_len == 0) {
            if (file_len > 0) {
                if (total_len >= file_len) {
                    retVal = 1;
                    break;
                }
            } else {
                if (received_len == 0) {
                    retVal = total_len > 0 ? 1 : 0;
                    break;
                }
            }
        }

        dataToWrite = server_reply;
        
        // vDisplayMessage(server_reply);

        if (!headerFound) {
            len = strlen(responseHeader);
            strncat(responseHeader, server_reply, sizeof(responseHeader) - len - received_len );            
            contentStr = strstr(responseHeader, "\r\n\r\n");
            if (contentStr) {
                int contentPos = contentStr - responseHeader + 4;
                int offset = contentPos - len;
                
                // vDisplayMessage((char*)&responseHeader[contentPos]);
    
                dataToWrite = server_reply + offset;
                received_len -= offset;
                headerFound = 1;
                
                {   
                    char *startusCodeStr = NULL, *firstLine = strstr(responseHeader, "\r\n");
                    if (firstLine) {
                        *firstLine = 0;
                        startusCodeStr = responseHeader;
                        while (*startusCodeStr && *startusCodeStr != ' ')
                            startusCodeStr++;
                        if (*startusCodeStr == ' ')
                            startusCodeStr++;
                        statusCode = atoi(startusCodeStr);
                    }
                }
            }
        }
                
        
        if (headerFound) {
            
            if (statusCode < 200 || statusCode >= 300) {
                snprintf(message, sizeof(message), "HTTP Error : %d\n", statusCode);
                HANDLE_HTTP_REQUEST_ERROR(message, -90);
            }
                    
            total_len += received_len;
            

            // puts(server_reply);   
            if(file) {
                if (received_len > 0) {
                    if (fwrite(dataToWrite, received_len, 1, file) <= 0) {
                        snprintf(message, sizeof(message), "http_request() : File could not opened\n");
                        HANDLE_HTTP_REQUEST_ERROR(message, -81);
                    }
                }
            }
        }
        
        
        // printf("\nReceived byte size = %d\nTotal lenght = %d", received_len, total_len);
    }

    // puts("Reply received\n");

    if(file)
        fclose(file);

    xrt_shutdown(&socket_desc);

    return retVal;
}



float cal_arc_length(float StartAngleRad, float EndAngleRad, float Radius, int Direction) {
    float lenght = 0.0f, SweepAngleRad = 0.0f;
    
    if (fabs(StartAngleRad - EndAngleRad) < EPSILON) {
        lenght = 2.0f * PIGRECO * Radius;
    } else {
        if (Direction == 0) {
            // CW/Oraio
            if (StartAngleRad > EndAngleRad) {
                SweepAngleRad = StartAngleRad - EndAngleRad;
          } else {
                SweepAngleRad = StartAngleRad + 2.0f * PIGRECO - EndAngleRad;
          }

        } else if (Direction == 1) {
            // CCW/Antiorario
            if (StartAngleRad > EndAngleRad) {
                SweepAngleRad = 2.0 * PIGRECO - StartAngleRad + EndAngleRad;
            } else {
                SweepAngleRad = StartAngleRad - EndAngleRad;
            }
        }
        lenght = SweepAngleRad * Radius;
    }
    
    return lenght;
}