### [Build Instructions]  

(Change /home/ben to your home directory below.)

  1. Qt  
Tested with qt-everywhere-opensource-4.8.6.  
If you need to debug with Web Inspector, a patch is required in
qt-everywhere-opensource-src-4.8.6/src/3rdparty/webkit/Source/WebKit/qt/WebCoreSupport/InspectorClientQt.cpp  
Please refer to https://bugs.webkit.org/show_bug.cgi?id=71271  
Qt build configuration, FYR:  
>  ./configure \  
  -prefix /home/ben/qt/qt-everywhere-opensource-4.8.6 -embedded -v -dbus -shared -release -stl \  
  -no-fast -no-largefile -no-exceptions -no-accessibility \  
  -no-sql-ibase -no-sql-mysql -no-sql-odbc -no-sql-psql -no-sql-sqlite -no-sql-sqlite2 \  
  -no-qt3support -no-xmlpatterns -no-phonon -svg -webkit -no-nis -no-cups -iconv -pch \  
  -little-endian -depths 1,4,8,16,32 -plugin-gfx-qvfb -plugin-gfx-transformed -plugin-gfx-vnc -plugin-gfx-linuxfb \  
  -no-gfx-multiscreen -qt-kbd-tty -no-glib -openssl -opensource

  2. Readium-SDK  
Get a modified version at:  
https://github.com/bjctw/readium-sdk  
Refer to Platform/Linux/README for building.  
libepub3.so is generated at Platform/Linux if the building process is successful.  
Copy libepub3.so to SDKLauncher-Linux-Qt/lib.  

  3.  
Modify QTDIR in SDKLauncher-Linux-Qt/build/x86/cmake.sh to your Qt installation location.  
>	./cmake.sh  
>	make  

  If everything is okay, you will get SDKLauncher-Linux-Qt/build/x86/SDKLauncher-Linux-Qt.  

### [Run]  
>	cd SDKLauncher-Linux-Qt/dist.x86  
>	LD_LIBRARY_PATH=. ./SDKLauncher-Linux-Qt

(You might need to add Qt library path in LD_LIBRARY_PATH.  
qvfb is needed when using libraries from Qt everywhere built with -embedded)  

### [Known Issues]  
There is error when display vertical-written ePubs, should be the bug of Qt webkit.  
(Refer to https://bugs.webkit.org/show_bug.cgi?id=51584)  

mailto:akira.1642@gmail.com if you have any problem/suggestion.  

**This project uses jsoncpp:**  
// Copyright 2007-2010 Baptiste Lepilleur  
// Distributed under MIT license, or public domain if desired and  
// recognized in your jurisdiction.  
// See http://jsoncpp.sourceforge.net/LICENSE  
