
include_directories(
${CHOQOK_INCLUDES}
)

set(plurkhelper_LIB_SRCS
plurkapiaccount.cpp
plurkapimicroblog.cpp
plurkapipostwidget.cpp
plurkapidmessagedialog.cpp
plurkapisearch.cpp
plurkapisearchdialog.cpp
plurkapimicroblogwidget.cpp
plurkapisearchtimelinewidget.cpp
plurkapiwhoiswidget.cpp
plurkapishowthread.cpp
plurkapitextedit.cpp
plurkapicomposerwidget.cpp
plurkapitimelinewidget.cpp
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
    plurkapiaccount.h
    plurkapidmessagedialog.h
    plurkapimicroblog.h
    plurkapimicroblogwidget.h
    plurkapipostwidget.h
    plurkapisearch.h
    plurkapisearchdialog.h
    plurkapisearchtimelinewidget.h
    plurkapiwhoiswidget.h
    plurkapishowthread.h
    plurkapitextedit.h
    plurkapicomposerwidget.h
    plurkapitimelinewidget.h
DESTINATION ${INCLUDE_INSTALL_DIR}/choqok/plurkapihelper COMPONENT Devel)
