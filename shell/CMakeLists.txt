project(shell)

set(CMAKE_INCLUDE_CURRENT_DIR TRUE)
set(CMAKE_AUTOMOC TRUE)

set(shell_SOURCES
	tblive_cmd.cpp)

add_executable(tblivecmd WIN32
	${shell_SOURCES})

install_obs_core(tblivecmd)
