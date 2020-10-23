#ifdef WATCOM
#include "FreeRTOS.h"
#else
#include "./../RTLinux/RTLinux.h"
#endif




#define EXTERN

#include "./logic_precomp.h"




int emergency_stop ( void ) {

emergency();

return 1;
}


