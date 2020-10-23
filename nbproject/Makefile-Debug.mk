#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/bef1c5b7/HTTPRequest.o \
	${OBJECTDIR}/_ext/bef1c5b7/HTTPServer.o \
	${OBJECTDIR}/_ext/bef1c5b7/LinuxARP.o \
	${OBJECTDIR}/_ext/bef1c5b7/actuatorCommand.o \
	${OBJECTDIR}/_ext/bef1c5b7/base64.o \
	${OBJECTDIR}/_ext/bef1c5b7/canbusCommands.o \
	${OBJECTDIR}/_ext/bef1c5b7/dataExchange.o \
	${OBJECTDIR}/_ext/bef1c5b7/dataExchangeCAN.o \
	${OBJECTDIR}/_ext/bef1c5b7/dataExchangeIO.o \
	${OBJECTDIR}/_ext/bef1c5b7/dataExchangeSCR.o \
	${OBJECTDIR}/_ext/bef1c5b7/dataExchangeSER.o \
	${OBJECTDIR}/_ext/bef1c5b7/dataExchangeUSB-workinprogress.o \
	${OBJECTDIR}/_ext/bef1c5b7/modbusCommands.o \
	${OBJECTDIR}/_ext/bef1c5b7/sha1.o \
	${OBJECTDIR}/_ext/bef1c5b7/webSocket.o \
	${OBJECTDIR}/_ext/bef1c5b7/xProjectCommand.o \
	${OBJECTDIR}/_ext/bef1c5b7/xrt_modbus.o \
	${OBJECTDIR}/_ext/1fc5adf3/alarms.o \
	${OBJECTDIR}/_ext/1fc5adf3/login.o \
	${OBJECTDIR}/_ext/1fc5adf3/main.o \
	${OBJECTDIR}/_ext/1fc5adf3/simulate.o \
	${OBJECTDIR}/_ext/1fc5adf3/track.o \
	${OBJECTDIR}/_ext/8b554d47/ControlUnit.o \
	${OBJECTDIR}/_ext/8b554d47/MemoryManager.o \
	${OBJECTDIR}/_ext/8b554d47/RTLinux.o \
	${OBJECTDIR}/_ext/89687c84/utility.o \
	${OBJECTDIR}/_ext/2888cb1d/grammar.o \
	${OBJECTDIR}/_ext/2888cb1d/libconfig.o \
	${OBJECTDIR}/_ext/2888cb1d/persistManager.o \
	${OBJECTDIR}/_ext/2888cb1d/scanctx.o \
	${OBJECTDIR}/_ext/2888cb1d/scanner.o \
	${OBJECTDIR}/_ext/2888cb1d/strbuf.o \
	${OBJECTDIR}/_ext/13972e7c/modbus-data.o \
	${OBJECTDIR}/_ext/13972e7c/modbus-rtu.o \
	${OBJECTDIR}/_ext/13972e7c/modbus-tcp.o \
	${OBJECTDIR}/_ext/13972e7c/modbus.o \
	${OBJECTDIR}/_ext/8c2bd3e9/automatic.o \
	${OBJECTDIR}/_ext/8c2bd3e9/checks.o \
	${OBJECTDIR}/_ext/8c2bd3e9/emergency.o \
	${OBJECTDIR}/_ext/8c2bd3e9/init.o \
	${OBJECTDIR}/_ext/8c2bd3e9/logic_precomp.o \
	${OBJECTDIR}/_ext/8c2bd3e9/manual.o \
	${OBJECTDIR}/_ext/8c2bd3e9/preform_registry.o \
	${OBJECTDIR}/_ext/8c2bd3e9/recovering.o \
	${OBJECTDIR}/_ext/8c2bd3e9/safety.o \
	${OBJECTDIR}/_ext/8c2bd3e9/ui-io.o \
	${OBJECTDIR}/_ext/4a6d875c/automatic.o \
	${OBJECTDIR}/_ext/4a6d875c/emergency.o \
	${OBJECTDIR}/_ext/4a6d875c/gcode.o \
	${OBJECTDIR}/_ext/4a6d875c/init.o \
	${OBJECTDIR}/_ext/4a6d875c/logic_precomp.o \
	${OBJECTDIR}/_ext/4a6d875c/manual.o \
	${OBJECTDIR}/_ext/4a6d875c/mill_utility.o \
	${OBJECTDIR}/_ext/4a6d875c/recovering.o \
	${OBJECTDIR}/_ext/4a6d875c/safety.o \
	${OBJECTDIR}/_ext/4a6d875c/ui-io.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lpthread

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/lcu

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/lcu: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/lcu ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/_ext/bef1c5b7/HTTPRequest.o: /media/Data/COMM/HTTPRequest.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/bef1c5b7
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bef1c5b7/HTTPRequest.o /media/Data/COMM/HTTPRequest.cpp

${OBJECTDIR}/_ext/bef1c5b7/HTTPServer.o: /media/Data/COMM/HTTPServer.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/bef1c5b7
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bef1c5b7/HTTPServer.o /media/Data/COMM/HTTPServer.cpp

${OBJECTDIR}/_ext/bef1c5b7/LinuxARP.o: /media/Data/COMM/LinuxARP.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bef1c5b7
	${RM} "$@.d"
	$(COMPILE.c) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bef1c5b7/LinuxARP.o /media/Data/COMM/LinuxARP.c

${OBJECTDIR}/_ext/bef1c5b7/actuatorCommand.o: /media/Data/COMM/actuatorCommand.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/bef1c5b7
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bef1c5b7/actuatorCommand.o /media/Data/COMM/actuatorCommand.cpp

${OBJECTDIR}/_ext/bef1c5b7/base64.o: /media/Data/COMM/base64.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/bef1c5b7
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bef1c5b7/base64.o /media/Data/COMM/base64.cpp

${OBJECTDIR}/_ext/bef1c5b7/canbusCommands.o: /media/Data/COMM/canbusCommands.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/bef1c5b7
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bef1c5b7/canbusCommands.o /media/Data/COMM/canbusCommands.cpp

${OBJECTDIR}/_ext/bef1c5b7/dataExchange.o: /media/Data/COMM/dataExchange.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/bef1c5b7
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bef1c5b7/dataExchange.o /media/Data/COMM/dataExchange.cpp

${OBJECTDIR}/_ext/bef1c5b7/dataExchangeCAN.o: /media/Data/COMM/dataExchangeCAN.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/bef1c5b7
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bef1c5b7/dataExchangeCAN.o /media/Data/COMM/dataExchangeCAN.cpp

${OBJECTDIR}/_ext/bef1c5b7/dataExchangeIO.o: /media/Data/COMM/dataExchangeIO.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/bef1c5b7
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bef1c5b7/dataExchangeIO.o /media/Data/COMM/dataExchangeIO.cpp

${OBJECTDIR}/_ext/bef1c5b7/dataExchangeSCR.o: /media/Data/COMM/dataExchangeSCR.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/bef1c5b7
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bef1c5b7/dataExchangeSCR.o /media/Data/COMM/dataExchangeSCR.cpp

${OBJECTDIR}/_ext/bef1c5b7/dataExchangeSER.o: /media/Data/COMM/dataExchangeSER.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/bef1c5b7
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bef1c5b7/dataExchangeSER.o /media/Data/COMM/dataExchangeSER.cpp

${OBJECTDIR}/_ext/bef1c5b7/dataExchangeUSB-workinprogress.o: /media/Data/COMM/dataExchangeUSB-workinprogress.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/bef1c5b7
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bef1c5b7/dataExchangeUSB-workinprogress.o /media/Data/COMM/dataExchangeUSB-workinprogress.cpp

${OBJECTDIR}/_ext/bef1c5b7/modbusCommands.o: /media/Data/COMM/modbusCommands.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/bef1c5b7
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bef1c5b7/modbusCommands.o /media/Data/COMM/modbusCommands.cpp

${OBJECTDIR}/_ext/bef1c5b7/sha1.o: /media/Data/COMM/sha1.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/bef1c5b7
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bef1c5b7/sha1.o /media/Data/COMM/sha1.cpp

${OBJECTDIR}/_ext/bef1c5b7/webSocket.o: /media/Data/COMM/webSocket.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/bef1c5b7
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bef1c5b7/webSocket.o /media/Data/COMM/webSocket.cpp

${OBJECTDIR}/_ext/bef1c5b7/xProjectCommand.o: /media/Data/COMM/xProjectCommand.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/bef1c5b7
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bef1c5b7/xProjectCommand.o /media/Data/COMM/xProjectCommand.cpp

${OBJECTDIR}/_ext/bef1c5b7/xrt_modbus.o: /media/Data/COMM/xrt_modbus.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bef1c5b7
	${RM} "$@.d"
	$(COMPILE.c) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bef1c5b7/xrt_modbus.o /media/Data/COMM/xrt_modbus.c

${OBJECTDIR}/_ext/1fc5adf3/alarms.o: /media/Data/LOGIC/alarms.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/1fc5adf3
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1fc5adf3/alarms.o /media/Data/LOGIC/alarms.cpp

${OBJECTDIR}/_ext/1fc5adf3/login.o: /media/Data/LOGIC/login.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/1fc5adf3
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1fc5adf3/login.o /media/Data/LOGIC/login.cpp

${OBJECTDIR}/_ext/1fc5adf3/main.o: /media/Data/LOGIC/main.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/1fc5adf3
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1fc5adf3/main.o /media/Data/LOGIC/main.cpp

${OBJECTDIR}/_ext/1fc5adf3/simulate.o: /media/Data/LOGIC/simulate.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/1fc5adf3
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1fc5adf3/simulate.o /media/Data/LOGIC/simulate.cpp

${OBJECTDIR}/_ext/1fc5adf3/track.o: /media/Data/LOGIC/track.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/1fc5adf3
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1fc5adf3/track.o /media/Data/LOGIC/track.cpp

${OBJECTDIR}/_ext/8b554d47/ControlUnit.o: /media/Data/RTLinux/ControlUnit.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8b554d47
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8b554d47/ControlUnit.o /media/Data/RTLinux/ControlUnit.cpp

${OBJECTDIR}/_ext/8b554d47/MemoryManager.o: /media/Data/RTLinux/MemoryManager.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8b554d47
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8b554d47/MemoryManager.o /media/Data/RTLinux/MemoryManager.cpp

${OBJECTDIR}/_ext/8b554d47/RTLinux.o: /media/Data/RTLinux/RTLinux.c
	${MKDIR} -p ${OBJECTDIR}/_ext/8b554d47
	${RM} "$@.d"
	$(COMPILE.c) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8b554d47/RTLinux.o /media/Data/RTLinux/RTLinux.c

${OBJECTDIR}/_ext/89687c84/utility.o: /media/Data/RTLinux/Utility/utility.c
	${MKDIR} -p ${OBJECTDIR}/_ext/89687c84
	${RM} "$@.d"
	$(COMPILE.c) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/89687c84/utility.o /media/Data/RTLinux/Utility/utility.c

${OBJECTDIR}/_ext/2888cb1d/grammar.o: /media/Data/libconfig-1.5/lib/grammar.c
	${MKDIR} -p ${OBJECTDIR}/_ext/2888cb1d
	${RM} "$@.d"
	$(COMPILE.c) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/2888cb1d/grammar.o /media/Data/libconfig-1.5/lib/grammar.c

${OBJECTDIR}/_ext/2888cb1d/libconfig.o: /media/Data/libconfig-1.5/lib/libconfig.c
	${MKDIR} -p ${OBJECTDIR}/_ext/2888cb1d
	${RM} "$@.d"
	$(COMPILE.c) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/2888cb1d/libconfig.o /media/Data/libconfig-1.5/lib/libconfig.c

${OBJECTDIR}/_ext/2888cb1d/persistManager.o: /media/Data/libconfig-1.5/lib/persistManager.c
	${MKDIR} -p ${OBJECTDIR}/_ext/2888cb1d
	${RM} "$@.d"
	$(COMPILE.c) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/2888cb1d/persistManager.o /media/Data/libconfig-1.5/lib/persistManager.c

${OBJECTDIR}/_ext/2888cb1d/scanctx.o: /media/Data/libconfig-1.5/lib/scanctx.c
	${MKDIR} -p ${OBJECTDIR}/_ext/2888cb1d
	${RM} "$@.d"
	$(COMPILE.c) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/2888cb1d/scanctx.o /media/Data/libconfig-1.5/lib/scanctx.c

${OBJECTDIR}/_ext/2888cb1d/scanner.o: /media/Data/libconfig-1.5/lib/scanner.c
	${MKDIR} -p ${OBJECTDIR}/_ext/2888cb1d
	${RM} "$@.d"
	$(COMPILE.c) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/2888cb1d/scanner.o /media/Data/libconfig-1.5/lib/scanner.c

${OBJECTDIR}/_ext/2888cb1d/strbuf.o: /media/Data/libconfig-1.5/lib/strbuf.c
	${MKDIR} -p ${OBJECTDIR}/_ext/2888cb1d
	${RM} "$@.d"
	$(COMPILE.c) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/2888cb1d/strbuf.o /media/Data/libconfig-1.5/lib/strbuf.c

${OBJECTDIR}/_ext/13972e7c/modbus-data.o: /media/Data/libmodbus-3.1.4/src/modbus-data.c
	${MKDIR} -p ${OBJECTDIR}/_ext/13972e7c
	${RM} "$@.d"
	$(COMPILE.c) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/13972e7c/modbus-data.o /media/Data/libmodbus-3.1.4/src/modbus-data.c

${OBJECTDIR}/_ext/13972e7c/modbus-rtu.o: /media/Data/libmodbus-3.1.4/src/modbus-rtu.c
	${MKDIR} -p ${OBJECTDIR}/_ext/13972e7c
	${RM} "$@.d"
	$(COMPILE.c) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/13972e7c/modbus-rtu.o /media/Data/libmodbus-3.1.4/src/modbus-rtu.c

${OBJECTDIR}/_ext/13972e7c/modbus-tcp.o: /media/Data/libmodbus-3.1.4/src/modbus-tcp.c
	${MKDIR} -p ${OBJECTDIR}/_ext/13972e7c
	${RM} "$@.d"
	$(COMPILE.c) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/13972e7c/modbus-tcp.o /media/Data/libmodbus-3.1.4/src/modbus-tcp.c

${OBJECTDIR}/_ext/13972e7c/modbus.o: /media/Data/libmodbus-3.1.4/src/modbus.c
	${MKDIR} -p ${OBJECTDIR}/_ext/13972e7c
	${RM} "$@.d"
	$(COMPILE.c) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/13972e7c/modbus.o /media/Data/libmodbus-3.1.4/src/modbus.c

${OBJECTDIR}/_ext/8c2bd3e9/automatic.o: /media/Data/xBM-Logic/automatic.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8c2bd3e9
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8c2bd3e9/automatic.o /media/Data/xBM-Logic/automatic.cpp

${OBJECTDIR}/_ext/8c2bd3e9/checks.o: /media/Data/xBM-Logic/checks.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8c2bd3e9
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8c2bd3e9/checks.o /media/Data/xBM-Logic/checks.cpp

${OBJECTDIR}/_ext/8c2bd3e9/emergency.o: /media/Data/xBM-Logic/emergency.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8c2bd3e9
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8c2bd3e9/emergency.o /media/Data/xBM-Logic/emergency.cpp

${OBJECTDIR}/_ext/8c2bd3e9/init.o: /media/Data/xBM-Logic/init.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8c2bd3e9
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8c2bd3e9/init.o /media/Data/xBM-Logic/init.cpp

${OBJECTDIR}/_ext/8c2bd3e9/logic_precomp.o: /media/Data/xBM-Logic/logic_precomp.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8c2bd3e9
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8c2bd3e9/logic_precomp.o /media/Data/xBM-Logic/logic_precomp.cpp

${OBJECTDIR}/_ext/8c2bd3e9/manual.o: /media/Data/xBM-Logic/manual.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8c2bd3e9
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8c2bd3e9/manual.o /media/Data/xBM-Logic/manual.cpp

${OBJECTDIR}/_ext/8c2bd3e9/preform_registry.o: /media/Data/xBM-Logic/preform_registry.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8c2bd3e9
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8c2bd3e9/preform_registry.o /media/Data/xBM-Logic/preform_registry.cpp

${OBJECTDIR}/_ext/8c2bd3e9/recovering.o: /media/Data/xBM-Logic/recovering.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8c2bd3e9
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8c2bd3e9/recovering.o /media/Data/xBM-Logic/recovering.cpp

${OBJECTDIR}/_ext/8c2bd3e9/safety.o: /media/Data/xBM-Logic/safety.c
	${MKDIR} -p ${OBJECTDIR}/_ext/8c2bd3e9
	${RM} "$@.d"
	$(COMPILE.c) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8c2bd3e9/safety.o /media/Data/xBM-Logic/safety.c

${OBJECTDIR}/_ext/8c2bd3e9/ui-io.o: /media/Data/xBM-Logic/ui-io.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8c2bd3e9
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8c2bd3e9/ui-io.o /media/Data/xBM-Logic/ui-io.cpp

${OBJECTDIR}/_ext/4a6d875c/automatic.o: /media/Data/xCNC-Logic/automatic.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/4a6d875c
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/4a6d875c/automatic.o /media/Data/xCNC-Logic/automatic.cpp

${OBJECTDIR}/_ext/4a6d875c/emergency.o: /media/Data/xCNC-Logic/emergency.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/4a6d875c
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/4a6d875c/emergency.o /media/Data/xCNC-Logic/emergency.cpp

${OBJECTDIR}/_ext/4a6d875c/gcode.o: /media/Data/xCNC-Logic/gcode.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/4a6d875c
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/4a6d875c/gcode.o /media/Data/xCNC-Logic/gcode.cpp

${OBJECTDIR}/_ext/4a6d875c/init.o: /media/Data/xCNC-Logic/init.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/4a6d875c
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/4a6d875c/init.o /media/Data/xCNC-Logic/init.cpp

${OBJECTDIR}/_ext/4a6d875c/logic_precomp.o: /media/Data/xCNC-Logic/logic_precomp.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/4a6d875c
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/4a6d875c/logic_precomp.o /media/Data/xCNC-Logic/logic_precomp.cpp

${OBJECTDIR}/_ext/4a6d875c/manual.o: /media/Data/xCNC-Logic/manual.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/4a6d875c
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/4a6d875c/manual.o /media/Data/xCNC-Logic/manual.cpp

${OBJECTDIR}/_ext/4a6d875c/mill_utility.o: /media/Data/xCNC-Logic/mill_utility.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/4a6d875c
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/4a6d875c/mill_utility.o /media/Data/xCNC-Logic/mill_utility.cpp

${OBJECTDIR}/_ext/4a6d875c/recovering.o: /media/Data/xCNC-Logic/recovering.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/4a6d875c
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/4a6d875c/recovering.o /media/Data/xCNC-Logic/recovering.cpp

${OBJECTDIR}/_ext/4a6d875c/safety.o: /media/Data/xCNC-Logic/safety.c
	${MKDIR} -p ${OBJECTDIR}/_ext/4a6d875c
	${RM} "$@.d"
	$(COMPILE.c) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/4a6d875c/safety.o /media/Data/xCNC-Logic/safety.c

${OBJECTDIR}/_ext/4a6d875c/ui-io.o: /media/Data/xCNC-Logic/ui-io.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/4a6d875c
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/4a6d875c/ui-io.o /media/Data/xCNC-Logic/ui-io.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
