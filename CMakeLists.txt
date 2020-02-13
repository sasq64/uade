cmake_minimum_required(VERSION 3.5)
project(uade)

option(USE_UADECORE "Build and use uadecore forked playback" OFF)

# OSX does not have these by default
include_directories(SYSTEM /usr/local/include)
link_directories(/usr/local/lib)

# Configure ossupport

try_compile(HAVE_STRLREP ${CMAKE_BINARY_DIR} SOURCES ${CMAKE_SOURCE_DIR}/compat/strltest.c)
try_compile(HAVE_CANON ${CMAKE_BINARY_DIR} SOURCES ${CMAKE_SOURCE_DIR}/compat/canontest.c)
try_compile(HAVE_MEMMEM ${CMAKE_BINARY_DIR} SOURCES ${CMAKE_SOURCE_DIR}/compat/memmemtest.c)
try_compile(HAVE_UNLIKELY ${CMAKE_BINARY_DIR} SOURCES ${CMAKE_SOURCE_DIR}/compat/unlikelytest.c)

set(SUPPORT_INCLUDE "#include <uade/unixsupport.h>")
set(SUPPORT_SOURCES ${SUPPORT_SOURCES} src/frontends/common/unixsupport.c)

if(NOT ${HAVE_STRLREP})
    set(SUPPORT_SOURCES ${SUPPORT_SOURCES} compat/strlrep.c)
    file(READ compat/strlrep.h STRLREP_H_CONTENTS)
endif()

if(NOT ${HAVE_CANON})
    set(SUPPORT_SOURCES ${SUPPORT_SOURCES} compat/canonrep.c)
    file(READ compat/canonrep.h CANON_H_CONTENTS)
endif()

if(NOT ${HAVE_MEMMEM})
    set(SUPPORT_SOURCES ${SUPPORT_SOURCES} compat/memmemrep.c)
    file(READ compat/memmemrep.h MEMMEM_H_CONTENTS)
endif()

configure_file(ossupport.h.in uade/ossupport.h)

# Configure options

file(READ version UADE_VERSION)
string(STRIP ${UADE_VERSION} UADE_VERSION)
set(CONFIG_BASE_DIR "/usr/local/share/uade")
set(CONFIG_UADE_CORE "/usr/local/lib/uade/uadecore")
configure_file(options.h.in uade/options.h)

# Configure others

configure_file(compilersupport.h.in uade/compilersupport.h)
configure_file(sysincludes.h.in uade/sysincludes.h)
configure_file(src/sd-sound-generic.h sd-sound.h)

set(UADE_SOURCES
    src/frontends/common/unixatomic.c
    src/frontends/common/uadeipc.c
    src/frontends/common/amifilemagic.c
    src/frontends/common/eagleplayer.c
    src/frontends/common/effects.c
    src/frontends/common/uadecontrol.c
    src/frontends/common/uadeconf.c
    src/frontends/common/uadestate.c
    src/frontends/common/uadeutils.c
    src/frontends/common/md5.c
    src/frontends/common/rmc.c
    src/frontends/common/songdb.c
    src/frontends/common/songinfo.c
    src/frontends/common/vparray.c
    src/frontends/common/support.c
    src/frontends/common/fifo.c
    ${SUPPORT_SOURCES}
    bencode/bencode.c)

set(UADECORE_SOURCES
    src/newcpu.c
    src/memory.c
    src/custom.c
    src/cia.c
    src/audio.c
    src/compiler.c
    src/missing.c
    src/sd-sound-generic.c
    src/md-support.c
    src/cfgfile.c
    src/fpp.c
    src/debug.c
    src/readcpu.c
    src/uade.c
    src/uademain.c
    src/sinctable.c
    src/text_scope.c
    src/frontends/common/uadeutils.c
    src/frontends/common/uadeipc.c
    src/frontends/common/unixatomic.c
    ${SUPPORT_SOURCES})

add_executable(build68k src/build68k.c)
target_include_directories(build68k PRIVATE src src/include)
add_executable(gencpu src/gencpu.c src/readcpu.c ${CMAKE_CURRENT_BINARY_DIR}/cpudefs.c src/missing.c)
target_include_directories(gencpu PRIVATE src src/include)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/cpudefs.c
    COMMAND build68k < ${CMAKE_CURRENT_SOURCE_DIR}/src/table68k > ${CMAKE_CURRENT_BINARY_DIR}/cpudefs.c
    DEPENDS $<TARGET_FILE:build68k>
    COMMENT "Running build68k")

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/cpuemu.c
        ${CMAKE_CURRENT_BINARY_DIR}/cputbl.h
        ${CMAKE_CURRENT_BINARY_DIR}/cpustbl.c
    COMMAND gencpu
    DEPENDS $<TARGET_FILE:gencpu>
    COMMENT "Running gencpu")

add_library(corelib STATIC
    ${UADECORE_SOURCES}
    ${CMAKE_CURRENT_BINARY_DIR}/cpuemu.c
    ${CMAKE_CURRENT_BINARY_DIR}/cpudefs.c
    ${CMAKE_CURRENT_BINARY_DIR}/cpustbl.c)
target_link_libraries(corelib PRIVATE m)
target_compile_options(corelib PRIVATE -Wno-format -Wno-implicit-function-declaration)
target_include_directories(corelib PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    src
    src/include
    src/frontends/include
)

add_library(uade STATIC ${UADE_SOURCES} compat/socketpair.c)
target_compile_options(uade PRIVATE -Wno-format -Wno-implicit-function-declaration)
target_link_libraries(uade PUBLIC m pthread)
target_include_directories(uade PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
        bencode/include
        src/frontends/include
        src/frontends/common)

if(USE_UADECORE)
    add_executable(uadecore src/main.c)
    target_link_libraries(uadecore PRIVATE corelib)
else()
    target_link_libraries(uade PUBLIC corelib pthread)
    target_compile_definitions(uade PRIVATE NO_UADECORE)
endif()

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    add_executable(uade123
        src/frontends/uade123/audio.c
        src/frontends/uade123/playloop.c
        src/frontends/uade123/chrarray.c
        src/frontends/uade123/terminal.c
        src/frontends/uade123/playlist.c
        src/frontends/uade123/uade123.c
        src/frontends/common/unixwalkdir.c
    )
    target_link_libraries(uade123 PRIVATE uade ao)

    add_executable(uadesimple
        src/frontends/uadesimple/audio.c
        src/frontends/uadesimple/uadesimple.c
    )
    target_link_libraries(uadesimple PRIVATE uade ao)
endif()
