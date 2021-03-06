PROJECT(eeereader)
CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

IF(DEFINED ENV{QMAKE_MOC})
	SET(QMAKE_MOC "$ENV{QMAKE_MOC}" CACHE FILEPATH "" FORCE)
ENDIF(DEFINED ENV{QMAKE_MOC})
IF(DEFINED ENV{QMAKE_UIC})
	SET(QMAKE_UIC "$ENV{QMAKE_UIC}" CACHE FILEPATH "" FORCE)
ENDIF(DEFINED ENV{QMAKE_UIC})
IF(DEFINED ENV{QMAKE_RCC})
	SET(QMAKE_RCC "$ENV{QMAKE_RCC}" CACHE FILEPATH "" FORCE)
ENDIF(DEFINED ENV{QMAKE_RCC})
IF(DEFINED ENV{INCLUDE_PATH})
	SET(INCLUDE_PATH "$ENV{INCLUDE_PATH}" CACHE FILEPATH "" FORCE)
ENDIF(DEFINED ENV{INCLUDE_PATH})
IF(DEFINED ENV{LIBRARY_PATH})
	SET(LIBRARY_PATH "$ENV{LIBRARY_PATH}" CACHE FILEPATH "" FORCE)
ENDIF(DEFINED ENV{LIBRARY_PATH})

IF(NOT DEFINED QMAKE_MOC OR NOT DEFINED QMAKE_UIC OR NOT DEFINED QMAKE_RCC)
	MESSAGE(FATAL_ERROR "Please define QMAKE_MOC, QMAKE_UIC, and QMAKE_RCC environment variables.")
ENDIF(NOT DEFINED QMAKE_MOC OR NOT DEFINED QMAKE_UIC OR NOT DEFINED QMAKE_RCC)

SET(QT_MOC_EXECUTABLE "${QMAKE_MOC}")
SET(QT_RCC_EXECUTABLE "${QMAKE_RCC}")
SET(QT_UIC_EXECUTABLE "${QMAKE_UIC}")
SET(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS}")
SET(CMAKE_C_FLAGS "$ENV{CFLAGS}")
IF(${CMAKE_BUILD_TYPE} MATCHES "Release")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNDEBUG")
ENDIF(${CMAKE_BUILD_TYPE} MATCHES "Release")
SET(CMAKE_EXE_LINKER_FLAGS "$ENV{LDFLAGS}")
SET(CMAKE_SKIP_BUILD_RPATH TRUE)

IF(DEFINED INCLUDE_PATH)
	STRING(REPLACE ":" ";" IPATH ${INCLUDE_PATH})
	FOREACH(DIR ${IPATH})
		SET(INCLUDE_DIR "${INCLUDE_DIR} -I${DIR}")
		IF(EXISTS ${DIR}/stdio.h)
			SET(SYS_INCLUDE_DIR ${DIR})
		ENDIF(EXISTS ${DIR}/stdio.h)
		IF(NOT FOUND_QT)
			IF(EXISTS ${DIR}/Qt)
				SET(QT_INCLUDE_DIR ${DIR})
				SET(INCLUDE_DIR "${INCLUDE_DIR} -I${DIR}/Qt -I${DIR}/QtCore -I${DIR}/QtDBus -I${DIR}/QtGui -I${DIR}/QtNetwork -I${DIR}/QtSql -I${DIR}/QtWebKit -I${DIR}/QtXml -I${DIR}/QtScript")
				SET(FOUND_QT "YES")
			ENDIF(EXISTS ${DIR}/Qt)
		ENDIF(NOT FOUND_QT)
	ENDFOREACH(DIR)

	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${INCLUDE_DIR}")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${INCLUDE_DIR}")
ENDIF(DEFINED INCLUDE_PATH)

IF(DEFINED LIBRARY_PATH)
	STRING(REPLACE ":" ";" LPATH ${LIBRARY_PATH})
	FOREACH(DIR ${LPATH})
		SET(LIBRARY_DIR "${LIBRARY_DIR} -L${DIR}")
	ENDFOREACH(DIR)

	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LIBRARY_DIR}")
ENDIF(DEFINED LIBRARY_PATH)

EXEC_PROGRAM(${CMAKE_CXX_COMPILER} ARGS "-dumpmachine" OUTPUT_VARIABLE MACHINE)
IF(${MACHINE} MATCHES "arm.*")
	SET(ARM "YES")
ENDIF(${MACHINE} MATCHES "arm.*")


IF(ARM)
	SET(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/../dist)
ELSE(ARM)
	SET(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/../dist.x86)
ENDIF(ARM)

MACRO(QT4_GET_MOC_FLAGS _moc_flags)
	SET(${_moc_flags})
	GET_DIRECTORY_PROPERTY(_inc_DIRS INCLUDE_DIRECTORIES)

	FOREACH(_current ${_inc_DIRS})
		SET(${_moc_flags} ${${_moc_flags}} "-I${_current}")
	ENDFOREACH(_current ${_inc_DIRS})

	GET_DIRECTORY_PROPERTY(_defines COMPILE_DEFINITIONS)
	FOREACH(_current ${_defines})
		SET(${_moc_flags} ${${_moc_flags}} "-D${_current}")
	ENDFOREACH(_current ${_defines})
ENDMACRO(QT4_GET_MOC_FLAGS)

MACRO(QT4_WRAP_CPP outfiles)
	QT4_GET_MOC_FLAGS(moc_flags)
	FOREACH(it ${ARGN})
		GET_FILENAME_COMPONENT(outfilename ${it} NAME_WE)
		GET_FILENAME_COMPONENT(infile ${it} ABSOLUTE)
		SET(outfile ${CMAKE_CURRENT_BINARY_DIR}/moc_${outfilename}.cpp)
		ADD_CUSTOM_COMMAND(OUTPUT ${outfile} COMMAND ${QT_MOC_EXECUTABLE} ARGS ${moc_flags} -o ${outfile} ${infile} DEPENDS ${infile})
		SET(${outfiles} ${${outfiles}} ${outfile})
	ENDFOREACH(it)
ENDMACRO(QT4_WRAP_CPP)
MACRO(QT4_WRAP_UI outfiles)
	FOREACH(it ${ARGN})
		GET_FILENAME_COMPONENT(outfilename ${it} NAME_WE)
		GET_FILENAME_COMPONENT(infile ${it} ABSOLUTE)
		SET(outfile ${CMAKE_CURRENT_BINARY_DIR}/ui_${outfilename}.h)
		ADD_CUSTOM_COMMAND(OUTPUT ${outfile} COMMAND ${QT_UIC_EXECUTABLE} ARGS -o ${outfile} ${infile} DEPENDS ${infile})
		SET(${outfiles} ${${outfiles}} ${outfile})
	ENDFOREACH(it)
ENDMACRO(QT4_WRAP_UI)
MACRO(QT4_ADD_RESOURCES outfiles)
	FOREACH(it ${ARGN})
		GET_FILENAME_COMPONENT(outfilename ${it} NAME_WE)
		GET_FILENAME_COMPONENT(infile ${it} ABSOLUTE)
		GET_FILENAME_COMPONENT(rc_path ${infile} PATH)
		SET(outfile ${CMAKE_CURRENT_BINARY_DIR}/qrc_${outfilename}.cpp)
		#  parse file for dependencies 
		#  all files are absolute paths or relative to the location of the qrc file
		FILE(READ "${infile}" _RC_FILE_CONTENTS)
		STRING(REGEX MATCHALL "<file[^<]+" _RC_FILES "${_RC_FILE_CONTENTS}")
		SET(_RC_DEPENDS)
		FOREACH(_RC_FILE ${_RC_FILES})
			STRING(REGEX REPLACE "^<file[^>]*>" "" _RC_FILE "${_RC_FILE}")
			STRING(REGEX MATCH "^/|([A-Za-z]:/)" _ABS_PATH_INDICATOR "${_RC_FILE}")
			IF(NOT _ABS_PATH_INDICATOR)
				SET(_RC_FILE "${rc_path}/${_RC_FILE}")
			ENDIF(NOT _ABS_PATH_INDICATOR)
			SET(_RC_DEPENDS ${_RC_DEPENDS} "${_RC_FILE}")
		ENDFOREACH(_RC_FILE)
		ADD_CUSTOM_COMMAND(OUTPUT ${outfile}
			COMMAND ${QT_RCC_EXECUTABLE}
			ARGS ${rcc_options} -name ${outfilename} -o ${outfile} ${infile}
			MAIN_DEPENDENCY ${infile}
			DEPENDS ${_RC_DEPENDS})
		SET(${outfiles} ${${outfiles}} ${outfile})
	ENDFOREACH(it)
ENDMACRO(QT4_ADD_RESOURCES)

#SET(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++ -nodefaultlibs ${CMAKE_CXX_FLAGS}")
SET(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++ -fpermissive -Wno-reserved-user-defined-literal ${CMAKE_CXX_FLAGS}")

#ADD_SUBDIRECTORY(../jsoncpp .)

INCLUDE_DIRECTORIES(../include/readium-sdk ../include)
LINK_DIRECTORIES(../../lib .)
QT4_WRAP_CPP(MOC_SRCS DREpubInterface.h)
ADD_EXECUTABLE(SDKLauncher-Linux-Qt SDKLauncher-Linux-Qt.cpp DREpubInterface.cpp sdkHelper.cpp ${MOC_SRCS})
#TARGET_LINK_LIBRARIES(SDKLauncher-Linux-Qt jsoncpp epub3 zip ssl c++ QtGui QtWebKit QtCore)
TARGET_LINK_LIBRARIES(SDKLauncher-Linux-Qt jsoncpp epub3 xml2 zip ssl QtNetwork QtGui QtWebKit QtCore)

MESSAGE(STATUS "CMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}")
MESSAGE(STATUS "CMAKE_C_FLAGS=${CMAKE_C_FLAGS}")
MESSAGE(STATUS "CMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}")
MESSAGE(STATUS "ARM=${ARM}")

INSTALL(DIRECTORY ../res DESTINATION ${CMAKE_INSTALL_PREFIX})
INSTALL(FILES ../lib/libepub3.so DESTINATION ${CMAKE_INSTALL_PREFIX})
INSTALL(FILES ../build/x86/libjsoncpp.so DESTINATION ${CMAKE_INSTALL_PREFIX})
INSTALL(TARGETS SDKLauncher-Linux-Qt RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
