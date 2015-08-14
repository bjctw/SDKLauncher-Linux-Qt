#!/bin/bash
#BUILD_TYPE="Release"
BUILD_TYPE="Debug"

SDK="/usr"
#QTDIR="/home/ben/qt-embedded-linux-opensource-4.5.3"
#QTDIR="/home/ben/qt/qt-everywhere-opensource-4.7.4"
QTDIR="/home/ben/qt/qt-everywhere-opensource-4.8.6-debug"
export CC="$SDK/bin/clang-3.5"
export CXX="$SDK/bin/clang++-3.5"
#export CC="$SDK/bin/gcc-4.9"
#export CXX="$SDK/bin/g++-4.9"
export QMAKE_MOC="$QTDIR/bin/moc"
export QMAKE_RCC="$QTDIR/bin/rcc"
export QMAKE_UIC="$QTDIR/bin/uic"
export INCLUDE_PATH="$QTDIR/include:$SDK/include"
export LIBRARY_PATH="$QTDIR/lib"
#export CXXFLAGS="-fpermissive"

DEFINE+=" -D CMAKE_BUILD_TYPE:STRING=$BUILD_TYPE"
DEFINE+=" -D CMAKE_VERBOSE_MAKEFILE:BOOL=TRUE"
cmake -Wno-dev ${DEFINE} ../../src
