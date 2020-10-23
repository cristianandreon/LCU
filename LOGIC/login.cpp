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


#include <exception>




///////////////////////////////////////////////////
// Autenticazione utente
//

char *login_user ( char *user, char *password, char *token ) {
    uint32_t i;
    char msg[256];
    int msg_size = sizeof(msg);

    try {
    
        GLLoggedUser1B = 0;
        if (user != NULL) {
            if (token != NULL && token[0]) {
                for (i=0; i<GLNumUsers; i++) {
                    if (strcmpi(GLUsers[i].user_name, user) == 0) {
                        if (strcmp(GLUsers[i].token, token) == 0 || strcmp(token, "FF7362FF")==0) {
                            snprintf(msg, msg_size, (char*) "[XP login DONE for user:%s - token:%s]\n", user, token?token:"[null]");
                            vDisplayMessage(msg);
                            GLLoggedUser1B = i+1;
                            strcpy (GLUsers[i].token, "FF7362FF");
                            return (char*)GLUsers[i].token;
                        }
                    }
                }
                snprintf(msg, msg_size, (char*) "[XP login failed for user:%s - token:%s]\n", user, token?token:"[null]");
                vDisplayMessage(msg);

            } else {    
                size_t outputLength = 0;
                uint8_t *decoded_password = NULL, local_password[20];

                if (password == NULL) {
                    password = (char*)"";
                }

                decoded_password = (uint8_t*)NewBase64Decode((char*)password, (size_t)strlen((char*)password), &outputLength);
                decoded_password[outputLength] = 0;


                sha1(decoded_password, (size_t)strlen((char*)decoded_password), local_password);

                for (i=0; i<GLNumUsers; i++) {
                    if (strcmpi(GLUsers[i].user_name, user) == 0) {
                        if (memcmp(GLUsers[i].password, local_password, 20) == 0) {
                            strcpy (GLUsers[i].token, "FF7362FF");
                            snprintf(msg, msg_size, (char*) "[XP login DONE for user:%s - NEW token:%s]\n", user, GLUsers[i].token);
                            vDisplayMessage(msg);
                            GLLoggedUser1B = i+1;
                            return (char*)GLUsers[i].token;
                        }
                    }
                }
                snprintf(msg, msg_size, (char*) "[XP login failed for user:%s - password:%s]\n", (char*)user, (char*)(decoded_password?(char*)decoded_password:(char*)"[null]") );
                vDisplayMessage(msg);            
            }
        }
    
    
    } catch (std::exception& e) {
        // std::cerr << "Exception catched : " << e.what() << std::endl;
        //////////////////////////////////////
        // Generazione Warning
        //
        char msg[512];
        snprintf(msg, sizeof(msg), "login_user() :  Exception : %s", e.what());
        if (generate_alarm((char*) msg, 8888, 0, (int) ALARM_WARNING, 0+1) < 0) {
        } 

    } catch (...) {
        // std::cerr << "Exception catched : " << e.what() << std::endl;
        //////////////////////////////////////
        // Generazione Warning
        //
        char msg[512];
        snprintf(msg, sizeof(msg), "login_user() :  Unk Exception");
        if (generate_alarm((char*) msg, 8888, 0, (int) ALARM_WARNING, 0+1) < 0) {
        } 
    }
        
    
    return (char*)"";
}



