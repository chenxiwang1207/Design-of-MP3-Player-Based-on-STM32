#ifndef _APP_DISPLAY_LRC_H
#define _APP_DISPLAY_LRC_H

#include "com_type.h"

s8 find_lrc_index(u16 current_sec);
u8 app_get_lrc(const char* mp3_filename);
u8 lrc_update_display(void);

void lrc_force_redraw(void);
u8 lrc_get_refresh_flag(void);
void lrc_clear_refresh_flag(void);
void lrc_hide(void);
void lrc_reset(void);

#endif
