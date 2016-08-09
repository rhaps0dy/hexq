cmake_minimum_required(VERSION 2.8)

project(HEXQ)

set(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11") 
set(ROM_DIR "/home/adria/Dropbox/TFG" CACHE FILEPATH "ROM directory")
add_definitions(-DROM_DIR="${ROM_DIR}")
set(HAVE_EMOJI OFF CACHE BOOL "Whether to use emoji when drawing the taxi domain")
if(HAVE_EMOJI)
	add_definitions(-DHAVE_EMOJI)
endif(HAVE_EMOJI)
set(USE_OPTIONS OFF CACHE BOOL "Whether to use options in playing MR")
set(Q_EACH_FRAME OFF CACHE BOOL "Whether to update the Q table every frame instead of every frame_skip. Has no effect if using options")
if(Q_EACH_FRAME AND NOT USE_OPTIONS)
	add_definitions(-DUSE_OPTIONS)
	add_definitions(-DOPTIONS_ARE_PRIMITIVE_FRAMESKIP)
elseif(USE_OPTIONS)
	add_definitions(-DUSE_OPTIONS)
endif()

set(ALE_DIR "../ALE-montezuma-modified" CACHE FILEPATH "Repository root for the Arcade Learning Environment")

include_directories(${ALE_DIR}/src)
link_directories(${ALE_DIR})


set(OTHERS directed_graph.cpp hexq_level.cpp taxi_domain_mdp.cpp montezuma_mdp.cpp montezuma_options_mdp.cpp)
add_executable(hexq main.cpp ${OTHERS})
add_executable(plain_sarsa plain_sarsa.cpp ${OTHERS})
add_executable(visualize_q visualize_q.cpp ${OTHERS})
target_link_libraries(hexq ale)
target_link_libraries(plain_sarsa ale)
target_link_libraries(visualize_q ale SDL)

if(LINUX)
set_target_properties(hexq PROPERTIES LINK_FLAGS -Wl,-rpath,${ALE_DIR},--stack,0x8000000)
endif(LINUX)
if(APPLE)
set_target_properties(hexq PROPERTIES LINK_FLAGS -Wl,-stack_size,0x8000000)
endif(APPLE)



install(TARGETS hexq DESTINATION .)
