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
CND_CONF=Release
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
	${OBJECTDIR}/_ext/bef1c5b7/dataExchange.o \
	${OBJECTDIR}/_ext/bef1c5b7/sha1.o \
	${OBJECTDIR}/_ext/bef1c5b7/webSocket.o \
	${OBJECTDIR}/_ext/bef1c5b7/xProjectCommand.o \
	${OBJECTDIR}/_ext/bef1c5b7/xrt_modbus.o \
	${OBJECTDIR}/_ext/1fd4b5f3/alarms.o \
	${OBJECTDIR}/_ext/1fd4b5f3/automatic.o \
	${OBJECTDIR}/_ext/1fd4b5f3/checks.o \
	${OBJECTDIR}/_ext/1fd4b5f3/emergency.o \
	${OBJECTDIR}/_ext/1fd4b5f3/init.o \
	${OBJECTDIR}/_ext/1fd4b5f3/io.o \
	${OBJECTDIR}/_ext/1fd4b5f3/logic_precomp.o \
	${OBJECTDIR}/_ext/1fd4b5f3/main.o \
	${OBJECTDIR}/_ext/1fd4b5f3/manual.o \
	${OBJECTDIR}/_ext/1fd4b5f3/preform_registry.o \
	${OBJECTDIR}/_ext/1fd4b5f3/recovering.o \
	${OBJECTDIR}/_ext/1fd4b5f3/safety.o \
	${OBJECTDIR}/_ext/1fd4b5f3/simulate.o \
	${OBJECTDIR}/_ext/1fd4b5f3/track.o \
	${OBJECTDIR}/_ext/8b554d47/ControlUnit.o \
	${OBJECTDIR}/_ext/8b554d47/MemoryManager.o \
	${OBJECTDIR}/_ext/8b554d47/RTLinux.o \
	${OBJECTDIR}/_ext/135ed078/modbus-data.o \
	${OBJECTDIR}/_ext/135ed078/modbus-rtu.o \
	${OBJECTDIR}/_ext/135ed078/modbus-tcp.o \
	${OBJECTDIR}/_ext/135ed078/modbus.o


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
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/lcu

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/lcu: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	g++ -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/lcu ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/_ext/bef1c5b7/HTTPRequest.o: /media/Data/COMM/HTTPRequest.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/bef1c5b7
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bef1c5b7/HTTPRequest.o /media/Data/COMM/HTTPRequest.cpp

${OBJECTDIR}/_ext/bef1c5b7/HTTPServer.o: /media/Data/COMM/HTTPServer.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/bef1c5b7
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bef1c5b7/HTTPServer.o /media/Data/COMM/HTTPServer.cpp

${OBJECTDIR}/_ext/bef1c5b7/LinuxARP.o: /media/Data/COMM/LinuxARP.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bef1c5b7
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bef1c5b7/LinuxARP.o /media/Data/COMM/LinuxARP.c

${OBJECTDIR}/_ext/bef1c5b7/actuatorCommand.o: /media/Data/COMM/actuatorCommand.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/bef1c5b7
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bef1c5b7/actuatorCommand.o /media/Data/COMM/actuatorCommand.cpp

${OBJECTDIR}/_ext/bef1c5b7/base64.o: /media/Data/COMM/base64.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/bef1c5b7
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bef1c5b7/base64.o /media/Data/COMM/base64.cpp

${OBJECTDIR}/_ext/bef1c5b7/dataExchange.o: /media/Data/COMM/dataExchange.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/bef1c5b7
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bef1c5b7/dataExchange.o /media/Data/COMM/dataExchange.cpp

${OBJECTDIR}/_ext/bef1c5b7/sha1.o: /media/Data/COMM/sha1.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/bef1c5b7
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bef1c5b7/sha1.o /media/Data/COMM/sha1.cpp

${OBJECTDIR}/_ext/bef1c5b7/webSocket.o: /media/Data/COMM/webSocket.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/bef1c5b7
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bef1c5b7/webSocket.o /media/Data/COMM/webSocket.cpp

${OBJECTDIR}/_ext/bef1c5b7/xProjectCommand.o: /media/Data/COMM/xProjectCommand.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/bef1c5b7
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bef1c5b7/xProjectCommand.o /media/Data/COMM/xProjectCommand.cpp

${OBJECTDIR}/_ext/bef1c5b7/xrt_modbus.o: /media/Data/COMM/xrt_modbus.c
	${MKDIR} -p ${OBJECTDIR}/_ext/bef1c5b7
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/bef1c5b7/xrt_modbus.o /media/Data/COMM/xrt_modbus.c

${OBJECTDIR}/_ext/1fd4b5f3/alarms.o: /media/Data/Logic/alarms.c
	${MKDIR} -p ${OBJECTDIR}/_ext/1fd4b5f3
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1fd4b5f3/alarms.o /media/Data/Logic/alarms.c

${OBJECTDIR}/_ext/1fd4b5f3/automatic.o: /media/Data/Logic/automatic.c
	${MKDIR} -p ${OBJECTDIR}/_ext/1fd4b5f3
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1fd4b5f3/automatic.o /media/Data/Logic/automatic.c

${OBJECTDIR}/_ext/1fd4b5f3/checks.o: /media/Data/Logic/checks.c
	${MKDIR} -p ${OBJECTDIR}/_ext/1fd4b5f3
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1fd4b5f3/checks.o /media/Data/Logic/checks.c

${OBJECTDIR}/_ext/1fd4b5f3/emergency.o: /media/Data/Logic/emergency.c
	${MKDIR} -p ${OBJECTDIR}/_ext/1fd4b5f3
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1fd4b5f3/emergency.o /media/Data/Logic/emergency.c

${OBJECTDIR}/_ext/1fd4b5f3/init.o: /media/Data/Logic/init.c
	${MKDIR} -p ${OBJECTDIR}/_ext/1fd4b5f3
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1fd4b5f3/init.o /media/Data/Logic/init.c

${OBJECTDIR}/_ext/1fd4b5f3/io.o: /media/Data/Logic/io.c
	${MKDIR} -p ${OBJECTDIR}/_ext/1fd4b5f3
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1fd4b5f3/io.o /media/Data/Logic/io.c

${OBJECTDIR}/_ext/1fd4b5f3/logic_precomp.o: /media/Data/Logic/logic_precomp.c
	${MKDIR} -p ${OBJECTDIR}/_ext/1fd4b5f3
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1fd4b5f3/logic_precomp.o /media/Data/Logic/logic_precomp.c

${OBJECTDIR}/_ext/1fd4b5f3/main.o: /media/Data/Logic/main.c
	${MKDIR} -p ${OBJECTDIR}/_ext/1fd4b5f3
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1fd4b5f3/main.o /media/Data/Logic/main.c

${OBJECTDIR}/_ext/1fd4b5f3/manual.o: /media/Data/Logic/manual.c
	${MKDIR} -p ${OBJECTDIR}/_ext/1fd4b5f3
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1fd4b5f3/manual.o /media/Data/Logic/manual.c

${OBJECTDIR}/_ext/1fd4b5f3/preform_registry.o: /media/Data/Logic/preform_registry.c
	${MKDIR} -p ${OBJECTDIR}/_ext/1fd4b5f3
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1fd4b5f3/preform_registry.o /media/Data/Logic/preform_registry.c

${OBJECTDIR}/_ext/1fd4b5f3/recovering.o: /media/Data/Logic/recovering.c
	${MKDIR} -p ${OBJECTDIR}/_ext/1fd4b5f3
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1fd4b5f3/recovering.o /media/Data/Logic/recovering.c

${OBJECTDIR}/_ext/1fd4b5f3/safety.o: /media/Data/Logic/safety.c
	${MKDIR} -p ${OBJECTDIR}/_ext/1fd4b5f3
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1fd4b5f3/safety.o /media/Data/Logic/safety.c

${OBJECTDIR}/_ext/1fd4b5f3/simulate.o: /media/Data/Logic/simulate.c
	${MKDIR} -p ${OBJECTDIR}/_ext/1fd4b5f3
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1fd4b5f3/simulate.o /media/Data/Logic/simulate.c

${OBJECTDIR}/_ext/1fd4b5f3/track.o: /media/Data/Logic/track.c
	${MKDIR} -p ${OBJECTDIR}/_ext/1fd4b5f3
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1fd4b5f3/track.o /media/Data/Logic/track.c

${OBJECTDIR}/_ext/8b554d47/ControlUnit.o: /media/Data/RTLinux/ControlUnit.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8b554d47
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8b554d47/ControlUnit.o /media/Data/RTLinux/ControlUnit.cpp

${OBJECTDIR}/_ext/8b554d47/MemoryManager.o: /media/Data/RTLinux/MemoryManager.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/8b554d47
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8b554d47/MemoryManager.o /media/Data/RTLinux/MemoryManager.cpp

${OBJECTDIR}/_ext/8b554d47/RTLinux.o: /media/Data/RTLinux/RTLinux.c
	${MKDIR} -p ${OBJECTDIR}/_ext/8b554d47
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/8b554d47/RTLinux.o /media/Data/RTLinux/RTLinux.c

${OBJECTDIR}/_ext/135ed078/modbus-data.o: /media/Data/libmodbus-3.1.0/src/modbus-data.c
	${MKDIR} -p ${OBJECTDIR}/_ext/135ed078
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/135ed078/modbus-data.o /media/Data/libmodbus-3.1.0/src/modbus-data.c

${OBJECTDIR}/_ext/135ed078/modbus-rtu.o: /media/Data/libmodbus-3.1.0/src/modbus-rtu.c
	${MKDIR} -p ${OBJECTDIR}/_ext/135ed078
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/135ed078/modbus-rtu.o /media/Data/libmodbus-3.1.0/src/modbus-rtu.c

${OBJECTDIR}/_ext/135ed078/modbus-tcp.o: /media/Data/libmodbus-3.1.0/src/modbus-tcp.c
	${MKDIR} -p ${OBJECTDIR}/_ext/135ed078
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/135ed078/modbus-tcp.o /media/Data/libmodbus-3.1.0/src/modbus-tcp.c

${OBJECTDIR}/_ext/135ed078/modbus.o: /media/Data/libmodbus-3.1.0/src/modbus.c
	${MKDIR} -p ${OBJECTDIR}/_ext/135ed078
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/135ed078/modbus.o /media/Data/libmodbus-3.1.0/src/modbus.c

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
