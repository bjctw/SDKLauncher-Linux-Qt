INCLUDE_DIRECTORIES(.)
ADD_LIBRARY(jsoncpp SHARED json_reader.cpp json_value.cpp json_writer.cpp)
#INSTALL(TARGETS jsoncpp LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX})
