SET(SOURCES screens/ui_indexpage.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    images/ui_img_1717730324.c
    fonts/ui_font_SiYuan32.c
    fonts/ui_font_SiYuan16.c
    fonts/ui_font_iconfont32.c)

add_library(ui ${SOURCES})
