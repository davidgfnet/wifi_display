
#ifndef __GDE943A2_H___
#define __GDE943A2_H___

// Call this before messing around with the display, it will setup GPIOs and stuff
void einkd_init(int direction);
void einkd_deinit();

// Power ON/OFF the screen, takes a while
void einkd_PowerOn();
void einkd_PowerOff();

// Repaint the screen. Pass color buffers (should be 120000 bytes each)
void einkd_refresh(const unsigned char * buffer);
void einkd_refresh_compressed(const unsigned char * buffer);
void einkd_clear(int color);


#endif

