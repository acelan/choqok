
include_directories(
${CHOQOK_INCLUDES}
${CMAKE_CURRENT_BINARY_DIR}
)

set( choqok_plurk_SRCS 
plurkaccount.cpp
plurkmicroblog.cpp
plurkpostwidget.cpp
plurksearch.cpp
plurkeditaccount.cpp
plurklistdialog.cpp
plurktimelinewidget.cpp
plurkapidmessagedialog.cpp
plurkapimicroblog.cpp
plurkapimicroblogwidget.cpp
plurkapisearchdialog.cpp
plurkapisearchtimelinewidget.cpp
plurkapiwhoiswidget.cpp
plurkapishowthread.cpp
plurkapitextedit.cpp
plurkapicomposerwidget.cpp
plurkapitimelinewidget.cpp
plurkapisearch.cpp
)

kde4_add_ui_files(choqok_plurk_SRCS
plurkeditaccount_base.ui
plurklistdialog_base.ui
)

kde4_add_plugin(choqok_plurk ${choqok_plurk_SRCS})

target_link_libraries(choqok_plurk ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${QJSON_LIBRARY} ${QTOAUTH_LIBRARY} choqok plurkapihelper)

# Install

install(TARGETS choqok_plurk DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES choqok_plurk.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

add_subdirectory(icons)
