#ifndef UI_CONFIG_H
#define UI_CONFIG_H

#include "ui_fonts.h"


// Switch brick design on/off.
#define SICK_BRICK_DESIGN                 true

// Number of supported languages.
#define UI_SUPPORTED_LANGUAGES_NUM        1

// Total number of menupoints
#define UI_NUMBER_OF_MAINPAGE_MPS         3
#define UI_NUMBER_OF_SHOWALL_MPS          3
#define UI_NUMBER_OF_OPTIONS_MPS          6

// Max char length of a menupoint.
#define UI_MAX_MP_LENGTH                  15

//Number of gameentries (GEs) fitting the display.
#define UI_LIST_LENGTH                    ((int)((LCD_DISPLAY_SIZE_Y - UI_SHOWALL_MP_HEIGHT) / UI_SHOWALL_GE_HEIGHT))

// Spacing between the Showall page menupoint and the list displaying the gameentries.
#define UI_UPPER_LIST_PADDING             (UI_SHOWALL_GE_LIST_Y - UI_SHOWALL_MP_HEIGHT)

// Page Backgrounds.
#define UI_MAINPAGE_BG_COLOR              0xFFC0
#define UI_OPTIONS_BG_COLOR               0xFFC0

// Scrollbar specifications.
#define UI_SCROLLBAR_WIDTH                15     //px
#define UI_SCROLLBAR_BG_COLOR             0xBBEB
#define UI_SCROLLBAR_FG_COLOR             0x0000

// Brickwall specifications.
#define UI_WALL_1_WIDTH                   66     //px
#define UI_WALL_2_WIDTH                   33     //px
#define UI_WALL_1_HEIGHT                  UI_SHOWALL_MP_HEIGHT
#define UI_WALL_2_HEIGHT                  (LCD_DISPLAY_SIZE_Y - UI_WALL_1_HEIGHT)
#define UI_BRICK_COLOR                    0xFFFF
#define UI_BRICK_LENGTH                   10     //px
#define UI_BRICK_HEIGHT                   4      //px
#define UI_BRICK_BORDER_COLOR             0x0000
#define UI_BRICK_BORDER_WIDTH             1      //px

// General measurements.
#define UI_MAINPAGE_MP_LENGTH             265    //px
#define UI_MAINPAGE_MP_HEIGHT             35     //px
#define UI_SHOWALL_MP_LENGTH              (LCD_DISPLAY_SIZE_X - 2 * UI_WALL_1_WIDTH)
#define UI_SHOWALL_MP_HEIGHT              34     //px
#define UI_SHOWALL_GE_LENGTH              (LCD_DISPLAY_SIZE_X - 2 * UI_WALL_2_WIDTH - UI_SCROLLBAR_WIDTH)
#define UI_SHOWALL_GE_HEIGHT              34     //px
#define UI_OPTIONS_MP_LENGTH_LONG         260    //px
#define UI_OPTIONS_MP_LENGTH_MEDIUM       200    //px
#define UI_OPTIONS_MP_LENGTH_SHORT        130    //px
#define UI_OPTIONS_MP_HEIGHT              35     //px

// Mainpage menupoint texts.
#define UI_MAINPAGE_MP_1_STRING           "BOOT CARTRIDGE"
#define UI_MAINPAGE_MP_2_STRING           "SHOW ALL GAMES"
#define UI_MAINPAGE_MP_3_STRING           "OPTIONS"

// ShowAllpage menupoint texts.
#define UI_SHOWALL_MP_1_STRING            "|ALL GAMES|"
#define UI_SHOWALL_MP_2_STRING            "|FAVORITES|"
#define UI_SHOWALL_MP_3_STRING            "|LAST PLAYED|"

// Options menupoint texts.
#define UI_OPTIONS_MP_1_STRING            "LANGUAGE"
#define UI_OPTIONS_MP_2_STRING            "DESIGNS"
#define UI_OPTIONS_MP_3_STRING            "BRIGHTNESS"
#define UI_OPTIONS_MP_4_STRING            "SCALING"
#define UI_OPTIONS_MP_5_STRING            "DIRECT BOOT"
#define UI_OPTIONS_MP_6_STRING            "SAVE CARTRIDGE"
#define UI_OPTIONS_ALTERNATE_MP_6_STRING  "END GAME"

// Mainpage menupoint coordinates.
#define UI_MAINPAGE_MP_SPACING            15     //px
#define UI_MAINPAGE_MPS_X                 29     //px
#define UI_MAINPAGE_MP_1_Y                53     //px
#define UI_MAINPAGE_MP_2_Y                (UI_MAINPAGE_MP_1_Y + UI_MAINPAGE_MP_HEIGHT + UI_MAINPAGE_MP_SPACING)
#define UI_MAINPAGE_MP_3_Y                (UI_MAINPAGE_MP_2_Y + UI_MAINPAGE_MP_HEIGHT + UI_MAINPAGE_MP_SPACING)

// ShowAllPage menupoint coordinates.
#define UI_SHOWALL_MPS_X                  UI_WALL_1_WIDTH
#define UI_SHOWALL_MPS_Y                  0      //px

// ShowAllpage List coordinates.
#define UI_SHOWALL_GE_LIST_X              UI_WALL_2_WIDTH
#define UI_SHOWALL_GE_LIST_Y              (LCD_DISPLAY_SIZE_Y - UI_LIST_LENGTH * UI_SHOWALL_GE_HEIGHT)

// Options menupoint coordinates.
#define UI_OPTIONS_MP_SPACING            10      //px
#define UI_OPTIONS_MP_1_X                20      //px
#define UI_OPTIONS_MP_2_X                (UI_OPTIONS_MP_1_X + UI_OPTIONS_MP_LENGTH_SHORT + 2*UI_OPTIONS_MP_SPACING)
#define UI_OPTIONS_MP_3_X                60      //px
#define UI_OPTIONS_MP_4_X                UI_OPTIONS_MP_3_X
#define UI_OPTIONS_MP_5_X                UI_OPTIONS_MP_3_X
#define UI_OPTIONS_MP_6_X                30
#define UI_OPTIONS_ALTERNATE_MP_6_X      95
#define UI_OPTIONS_MP_1_Y                UI_OPTIONS_MP_SPACING
#define UI_OPTIONS_MP_2_Y                UI_OPTIONS_MP_1_Y
#define UI_OPTIONS_MP_3_Y                (UI_OPTIONS_MP_1_Y + UI_OPTIONS_MP_HEIGHT + UI_OPTIONS_MP_SPACING)
#define UI_OPTIONS_MP_4_Y                (UI_OPTIONS_MP_3_Y + UI_OPTIONS_MP_HEIGHT + UI_OPTIONS_MP_SPACING)
#define UI_OPTIONS_MP_5_Y                (UI_OPTIONS_MP_4_Y + UI_OPTIONS_MP_HEIGHT + UI_OPTIONS_MP_SPACING)
#define UI_OPTIONS_MP_6_Y                (UI_OPTIONS_MP_5_Y + UI_OPTIONS_MP_HEIGHT + UI_OPTIONS_MP_SPACING)

#define UI_MP_4_CHECKMARK_X              (UI_OPTIONS_MP_4_X + UI_OPTIONS_MP_LENGTH_MEDIUM - 21)
#define UI_MP_5_CHECKMARK_X              (UI_OPTIONS_MP_5_X + UI_OPTIONS_MP_LENGTH_MEDIUM - 21)
#define UI_MP_4_CHECKMARK_Y              (UI_OPTIONS_MP_4_Y + 15)
#define UI_MP_5_CHECKMARK_Y              (UI_OPTIONS_MP_5_Y + 15)

#define UI_MP_1_ARROW_LEFT_X             (UI_OPTIONS_MP_1_X - Fonts_ArrowLeftToRight_16x31.SymbolWidth - 3)
#define UI_MP_1_ARROW_LEFT_Y             (UI_OPTIONS_MP_1_Y - 1)
#define UI_MP_1_ARROW_RIGHT_X            (UI_OPTIONS_MP_1_X + UI_OPTIONS_MP_LENGTH_SHORT + 3)
#define UI_MP_1_ARROW_RIGHT_Y            (UI_OPTIONS_MP_1_Y - 1)

#define UI_MP_3_ARROW_LEFT_X             (UI_OPTIONS_MP_3_X - Fonts_ArrowLeftToRight_16x31.SymbolWidth - 3)
#define UI_MP_3_ARROW_LEFT_Y             (UI_OPTIONS_MP_3_Y - 1)
#define UI_MP_3_ARROW_RIGHT_X            (UI_OPTIONS_MP_3_X + UI_OPTIONS_MP_LENGTH_MEDIUM + 3)
#define UI_MP_3_ARROW_RIGHT_Y            (UI_OPTIONS_MP_3_Y - 1)

// General menupoint specifications.
#define UI_MP_SPACING          		      0      //px
#define UI_MP_BORDER_WIDTH     		      0      //px
#define UI_MP_BG_COLOR         		      0xFFFF
#define UI_MP_TEXT_COLOR       		      0x0000
#define UI_MP_BORDER_COLOR     		      0x0000
#define UI_DISABLED_MP_ARROW_COLOR        0xC618
#define UI_HIGHLIGHTED_MP_ARROW_COLOR     0x0000
#define UI_MP_CHECKMARK_COLOR             0x0000
#define UI_HIGHLIGHTED_MP_CHECKMARK_COLOR 0xFFFF

#define UI_DISABLED_MP_BG_COLOR           0xC618
#define UI_DISABLED_MP_TEXT_COLOR         0x7BD0
#define UI_HIGHLIGHTED_MP_BG_COLOR        0x0000
#define UI_HIGHLIGHTED_MP_TEXT_COLOR      0xFFFF
#define UI_HIGHLIGHTED_MP_BORDER_COLOR    0xFFFF

// Game-entry specifications.
#define UI_GE_SPACING       			  0      //px
#define UI_GE_BORDER_WIDTH  			  0      //px
#define UI_GE_BG_COLOR      			  0xFFFF
#define UI_GE_TEXT_COLOR    			  0x0000
#define UI_GE_BORDER_COLOR  			  0x0000

#define UI_GE_STAR_X                      (LCD_DISPLAY_SIZE_X - UI_WALL_2_WIDTH - UI_SCROLLBAR_WIDTH - 35)
#define UI_GE_STAR_OFFSET_Y               4      //px

#define UI_GE_STAR_COLOR                  0x0000
#define UI_DISABLED_GE_BG_COLOR           0xC618
#define UI_DISABLED_GE_TEXT_COLOR         0x7BD0
#define UI_HIGHLIGHTED_GE_BG_COLOR        0x0000
#define UI_HIGHLIGHTED_GE_STAR_COLOR      0xFFFF
#define UI_HIGHLIGHTED_GE_TEXT_COLOR      0xFFFF
#define UI_HIGHLIGHTED_GE_BORDER_COLOR    0xFFFF

// Fonts.
#define UI_MP_FONT Fonts_STMFont_16x24
#define UI_GE_FONT Fonts_STMFont_16x24

#endif //#define UI_CONFIG_H
