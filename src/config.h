#ifndef CONFIG_H
#define CONFIG_H

//This file contains the default startup values

//write colors as hex values, hex to rgb
#define HEXC(c) \
	(c >> (2*8))&0xFF, \
	(c >> (1*8))&0xFF, \
	(c >> (0*8))&0xFF, 255

#define RBLACK  (Color){HEXC(0x181818)}
#define RWHITE  (Color){HEXC(0xFFFFFF)}
#define RRED    (Color){HEXC(0xF43841)}
#define RYELLOW (Color){HEXC(0xFFDD33)}
#define RORANGE (Color){HEXC(0xCC8C3C)}
#define RGREEN  (Color){HEXC(0x73C936)}
#define RAQUA   (Color){HEXC(0x95A99F)}
#define RPURPLE (Color){HEXC(0x303540)}
#define RBLUE   (Color){HEXC(0x565F73)}
#define RGRAY   (Color){HEXC(0x52494E)}

#define COLORS_ENABLE true
#define FONT_SIZE 26
//NOTE: only mono spaced fonts works correctly
#define FONT_PATH "assets/fonts/IosevkaTerm/IosevkaTermNerdFontMono-Regular.ttf"
//"assets/fonts/BigBlueTerminal/BigBlueTermPlusNerdFontMono-Regular.ttf"
#define SHADER_ENABLE false
#define SHADER_PATH "assets/shader/crt.glsl"

#define WINDOW_W 800
#define WINDOW_H 600

#define MODKEY KEY_LEFT_CONTROL //default mod key (see ../raylib/include/raylib.h)

#endif // !CONFIG_H
