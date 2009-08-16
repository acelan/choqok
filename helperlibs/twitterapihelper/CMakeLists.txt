
include_directories(
${CHOQOK_INCLUDES}
)

set(twitterhelper_LIB_SRCS
twitterapiaccount.cpp
twitterapimicroblog.cpp
twitterapipostwidget.cpp
)
# kde4_add_kcfg_files(twitterhelper_LIB_SRCS
# choqokbehaviorsettings.kcfgc
# choqokappearancesettings.kcfgc
# )
# kde4_add_ui_files(choqok_LIB_SRCS ui/timelinewidget_base.ui )

kde4_add_library(twitterapihelper SHARED ${twitterhelper_LIB_SRCS})

target_link_libraries(twitterapihelper ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KUTILS_LIBS} choqok)

set_target_properties(twitterapihelper PROPERTIES
VERSION 0.1
SOVERSION 0
)
if(MSVC)
    set_target_properties(twitterapihelper PROPERTIES
    OUTPUT_NAME libtwitterapihelper
    )
endif(MSVC)

# install( FILES choqokbehaviorsettings.kcfg choqokappearancesettings.kcfg
# DESTINATION ${KCFG_INSTALL_DIR})

install(TARGETS twitterapihelper ${INSTALL_TARGETS_DEFAULT_ARGS})
# install( FILES choqokplugin.desktop choqokshortenerplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})