#ifndef UI_PAGES_H
#define UI_PAGES_H

#include "lcd_config.h"


extern const UI_MenuPoint_t UI_MainPage_MenuPoints[UI_NUMBER_OF_MAINPAGE_MPS];
extern       UI_MenuPoint_t UI_Options_MenuPoints[UI_NUMBER_OF_OPTIONS_MPS];
extern const UI_MenuPoint_t UI_Options_alternate_MP_6;
extern const UI_MenuPoint_t UI_Options_regular_MP_6;



void UI_DrawMainPage(int selectedMP, bool isCartridgeInserted);
void UI_DrawOptionsPage(int selectedMP, bool isCartridgeInserted);
void UI_DrawShowAllPage(UI_ShowAllTabs_t design);

#endif //UI_PAGES_H
