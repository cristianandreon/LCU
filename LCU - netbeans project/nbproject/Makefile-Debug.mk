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
CND_PLATFORM=Cygwin-Windows
CND_DLIB_EXT=dll
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/d29389fb/HTTPRequest.o \
	${OBJECTDIR}/_ext/d29389fb/HTTPServer.o \
	${OBJECTDIR}/_ext/d29389fb/LinuxARP.o \
	${OBJECTDIR}/_ext/d29389fb/actuatorCommand.o \
	${OBJECTDIR}/_ext/d29389fb/base64.o \
	${OBJECTDIR}/_ext/d29389fb/canbusCommands.o \
	${OBJECTDIR}/_ext/d29389fb/dataExchange.o \
	${OBJECTDIR}/_ext/d29389fb/dataExchangeCAN.o \
	${OBJECTDIR}/_ext/d29389fb/dataExchangeIO.o \
	${OBJECTDIR}/_ext/d29389fb/dataExchangeSCR.o \
	${OBJECTDIR}/_ext/d29389fb/dataExchangeSER.o \
	${OBJECTDIR}/_ext/d29389fb/dataExchangeUSB-workinprogress.o \
	${OBJECTDIR}/_ext/d29389fb/modbusCommands.o \
	${OBJECTDIR}/_ext/d29389fb/sha1.o \
	${OBJECTDIR}/_ext/d29389fb/webSocket.o \
	${OBJECTDIR}/_ext/d29389fb/xProjectCommand.o \
	${OBJECTDIR}/_ext/d29389fb/xrt_modbus.o \
	${OBJECTDIR}/_ext/805c722f/alarms.o \
	${OBJECTDIR}/_ext/805c722f/login.o \
	${OBJECTDIR}/_ext/805c722f/main.o \
	${OBJECTDIR}/_ext/805c722f/simulate.o \
	${OBJECTDIR}/_ext/805c722f/track.o \
	${OBJECTDIR}/_ext/214bf283/ControlUnit.o \
	${OBJECTDIR}/_ext/214bf283/MemoryManager.o \
	${OBJECTDIR}/_ext/214bf283/RTLinux.o


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
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/lcu_-_netbeans_project_backup.exe

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/lcu_-_netbeans_project_backup.exe: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/lcu_-_netbeans_project_backup ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/_ext/d29389fb/HTTPRequest.o: ../COMM/HTTPRequest.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/d29389fb
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d29389fb/HTTPRequest.o ../COMM/HTTPRequest.cpp

${OBJECTDIR}/_ext/d29389fb/HTTPServer.o: ../COMM/HTTPServer.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/d29389fb
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d29389fb/HTTPServer.o ../COMM/HTTPServer.cpp

${OBJECTDIR}/_ext/d29389fb/LinuxARP.o: ../COMM/LinuxARP.c
	${MKDIR} -p ${OBJECTDIR}/_ext/d29389fb
	${RM} "$@.d"
	$(COMPILE.c) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d29389fb/LinuxARP.o ../COMM/LinuxARP.c

${OBJECTDIR}/_ext/d29389fb/actuatorCommand.o: ../COMM/actuatorCommand.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/d29389fb
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d29389fb/actuatorCommand.o ../COMM/actuatorCommand.cpp

${OBJECTDIR}/_ext/d29389fb/base64.o: ../COMM/base64.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/d29389fb
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d29389fb/base64.o ../COMM/base64.cpp

${OBJECTDIR}/_ext/d29389fb/canbusCommands.o: ../COMM/canbusCommands.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/d29389fb
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d29389fb/canbusCommands.o ../COMM/canbusCommands.cpp

${OBJECTDIR}/_ext/d29389fb/dataExchange.o: ../COMM/dataExchange.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/d29389fb
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d29389fb/dataExchange.o ../COMM/dataExchange.cpp

${OBJECTDIR}/_ext/d29389fb/dataExchangeCAN.o: ../COMM/dataExchangeCAN.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/d29389fb
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d29389fb/dataExchangeCAN.o ../COMM/dataExchangeCAN.cpp

${OBJECTDIR}/_ext/d29389fb/dataExchangeIO.o: ../COMM/dataExchangeIO.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/d29389fb
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d29389fb/dataExchangeIO.o ../COMM/dataExchangeIO.cpp

${OBJECTDIR}/_ext/d29389fb/dataExchangeSCR.o: ../COMM/dataExchangeSCR.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/d29389fb
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d29389fb/dataExchangeSCR.o ../COMM/dataExchangeSCR.cpp

${OBJECTDIR}/_ext/d29389fb/dataExchangeSER.o: ../COMM/dataExchangeSER.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/d29389fb
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d29389fb/dataExchangeSER.o ../COMM/dataExchangeSER.cpp

${OBJECTDIR}/_ext/d29389fb/dataExchangeUSB-workinprogress.o: ../COMM/dataExchangeUSB-workinprogress.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/d29389fb
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d29389fb/dataExchangeUSB-workinprogress.o ../COMM/dataExchangeUSB-workinprogress.cpp

${OBJECTDIR}/_ext/d29389fb/modbusCommands.o: ../COMM/modbusCommands.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/d29389fb
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d29389fb/modbusCommands.o ../COMM/modbusCommands.cpp

${OBJECTDIR}/_ext/d29389fb/sha1.o: ../COMM/sha1.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/d29389fb
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d29389fb/sha1.o ../COMM/sha1.cpp

${OBJECTDIR}/_ext/d29389fb/webSocket.o: ../COMM/webSocket.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/d29389fb
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d29389fb/webSocket.o ../COMM/webSocket.cpp

${OBJECTDIR}/_ext/d29389fb/xProjectCommand.o: ../COMM/xProjectCommand.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/d29389fb
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d29389fb/xProjectCommand.o ../COMM/xProjectCommand.cpp

${OBJECTDIR}/_ext/d29389fb/xrt_modbus.o: ../COMM/xrt_modbus.c
	${MKDIR} -p ${OBJECTDIR}/_ext/d29389fb
	${RM} "$@.d"
	$(COMPILE.c) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/d29389fb/xrt_modbus.o ../COMM/xrt_modbus.c

${OBJECTDIR}/_ext/805c722f/alarms.o: ../LOGIC/alarms.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/805c722f
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/805c722f/alarms.o ../LOGIC/alarms.cpp

${OBJECTDIR}/_ext/805c722f/login.o: ../LOGIC/login.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/805c722f
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/805c722f/login.o ../LOGIC/login.cpp

${OBJECTDIR}/_ext/805c722f/main.o: ../LOGIC/main.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/805c722f
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/805c722f/main.o ../LOGIC/main.cpp

${OBJECTDIR}/_ext/805c722f/simulate.o: ../LOGIC/simulate.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/805c722f
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/805c722f/simulate.o ../LOGIC/simulate.cpp

${OBJECTDIR}/_ext/805c722f/track.o: ../LOGIC/track.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/805c722f
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/805c722f/track.o ../LOGIC/track.cpp

${OBJECTDIR}/_ext/214bf283/ControlUnit.o: ../RTLinux/ControlUnit.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/214bf283
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/214bf283/ControlUnit.o ../RTLinux/ControlUnit.cpp

${OBJECTDIR}/_ext/214bf283/MemoryManager.o: ../RTLinux/MemoryManager.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/214bf283
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/214bf283/MemoryManager.o ../RTLinux/MemoryManager.cpp

${OBJECTDIR}/_ext/214bf283/RTLinux.o: ../RTLinux/RTLinux.c
	${MKDIR} -p ${OBJECTDIR}/_ext/214bf283
	${RM} "$@.d"
	$(COMPILE.c) -g -DDEBUG -I/mount/Data/RTLinux -I/mount/Data/Logic -I/mount/Data/mTCP -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/214bf283/RTLinux.o ../RTLinux/RTLinux.c

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
