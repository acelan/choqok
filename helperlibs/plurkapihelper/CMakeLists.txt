
include_directories(
${CHOQOK_INCLUDES}
)

set(plurkhelper_LIB_SRCS
plurkapialerts.cpp
plurkapiblocks.cpp
plurkapicliques.cpp
plurkapiemoticons.cpp
plurkapifriendsandfans.cpp
plurkapiplurksearch.cpp
plurkapiplurktop.cpp
plurkapiprofile.cpp
plurkapirealtime.cpp
plurkapiresponses.cpp
plurkapitimeline.cpp
plurkapiusers.cpp
)

# kde4_add_kcfg_files(plurkhelper_LIB_SRCS
# plurkapisettings.kcfgc
# )

kde4_add_library(plurkapihelper SHARED ${plurkhelper_LIB_SRCS})

target_link_libraries( plurkapihelper
                       ${QT_QTNETWORK_LIBRARY}
                       ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KUTILS_LIBS}
                       ${QTOAUTH_LIBRARY} ${QJSON_LIBRARY}
                       choqok)

set_target_properties(plurkapihelper PROPERTIES
VERSION 1.0.1
SOVERSION 1
)

install(TARGETS plurkapihelper ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
plurkapi.h
plurkapialerts.h
plurkapiblocks.h
plurkapicliques.h
plurkapiemoticons.h
plurkapifriendsandfans.h
plurkapiplurksearch.h
plurkapiplurktop.h
plurkapiprofile.h
plurkapirealtime.h
plurkapiresponses.h
plurkapitimeline.h
plurkapiusers.h
DESTINATION ${INCLUDE_INSTALL_DIR}/choqok/plurkapihelper COMPONENT Devel)
