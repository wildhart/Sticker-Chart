#pragma once

void menu_cell_draw_job(GContext* ctx, const Layer *cell_layer, const uint8_t index);
void menu_cell_draw_other(GContext* ctx, const Layer *cell_layer, const char *title, const char *sub_title, GBitmap ** icon);
void main_menu_show(void);
void main_menu_hide(void);
void main_menu_update(void);
void main_menu_highlight_job(const int index);