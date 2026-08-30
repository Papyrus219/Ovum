cmake_minimum_required(VERSION 3.17)
project(ovum)

option(ERUPTOR_LOCAL_DEV "Use local code to faster test while development." OFF)
set(ERUPTOR_LOCAL_DEV_PATH "" CACHE PATH "Path to local code of eruptor engine. Use while engine development and testing.")

include(FetchContent)

if(ERUPTOR_LOCAL_DEV)
    FetchContent_Declare(
        engine
        SOURCE_DIR ${ERUPTOR_LOCAL_DEV_PATH})
else()
    FetchContent_Declare(
        engine
        GIT_REPOSITORY git@github.com:Papyrus219/Eruptor-engine.git
        GIT_TAG main)
endif()

FetchContent_MakeAvailable(engine)

set(APP_SRC ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
            ${CMAKE_CURRENT_LIST_DIR}/src/app.cpp
            ${CMAKE_CURRENT_LIST_DIR}/src/gp_communicator.cpp
            ${CMAKE_CURRENT_LIST_DIR}/src/simulation_scene.cpp
            ${CMAKE_CURRENT_LIST_DIR}/src/simulation_parser.cpp
            ${CMAKE_CURRENT_LIST_DIR}/src/simulation_saver.cpp
            ${CMAKE_CURRENT_LIST_DIR}/src/app_state.cpp
            ${CMAKE_CURRENT_LIST_DIR}/src/editor_state.cpp
            ${CMAKE_CURRENT_LIST_DIR}/src/simulation_state.cpp
)

add_executable(app ${APP_SRC})
target_link_libraries(app PUBLIC eruptor_engine eruptor_event eruptor_scene eruptor_physic)
target_include_directories(app PRIVATE include/)
target_compile_features(app PRIVATE cxx_std_23)

install(TARGETS app RUNTIME DESTINATION bin)
