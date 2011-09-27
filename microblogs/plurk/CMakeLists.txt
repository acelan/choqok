include_directories(
${CHOQOK_INCLUDES}
${CMAKE_CURRENT_BINARY_DIR}
)

set( choqok_plurk_SRCS 
plurkaccount.cpp
plurkmicroblog.cpp
plurkeditaccountwidget.cpp
)

kde4_add_plugin(choqok_plurk ${choqok_plurk_SRCS})

target_link_libraries(choqok_plurk ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${QJSON_LIBRARY} ${KDEPIMLIBS_KABC_LIBS} ${KDEPIMLIBS_KPIMUTILS_LIBS} ${KDEPIMLIBS_KMIME_LIBS} ${PLURK_LIBRARIES} choqok )

# Install

install(TARGETS choqok_plurk DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES choqok_plurk.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

add_subdirectory(icons)
