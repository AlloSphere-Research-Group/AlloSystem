# set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -I/usr/local/include -I/usr/local/include/mysql" )
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/usr/local/lib -lmysqlpp")

include_directories(/usr/local/include /usr/local/Cellar/mysql/5.7.17/include/mysql)
link_directories(/usr/local/lib)
target_link_libraries(${APP_NAME} mysqlpp)
