#ifndef __COLORS_H__
#define __COLORS_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_system.h"
#include "utils.h"

/* hsv --> rgb */
#define hue_t   uint16_t
#define HUE_MAX   360
#define SAT_MAX   100
#define VAL_MAX   100

#define SAT_MIN 1
#define VAL_MIN 1

typedef struct color_rgb_t {
   uint8_t      r;
   uint8_t      g;
   uint8_t      b;
} color_rgb_t;

typedef struct color_hsv_t {
   hue_t      h;
   uint8_t      s;
   uint8_t      v;
} color_hsv_t;

static const uint16_t hsv_colors_7[7] = { 0,     	// red	
                                          30,		// orange
                                          60,		// yellow
                                          120,		// green
                                          180,		// cyan
                                          240,		// blue
                                          300		// purple
                                        };

// ***** predefined hsv colors


// ***** predefined rgb colors
/* Colors                                R   G   B*/
static const color_rgb_t COLOR_BLACK         = {  0,  0,  0};
// main 7 colors   
static const color_rgb_t COLOR_RED           = {255,   0,   0};
#define HEX_COLOR_RED   "FF0000"

static const color_rgb_t COLOR_ORANGE        = {255, 128,   0};  
#define HEX_COLOR_ORANGE "FF8000"

static const color_rgb_t COLOR_YELLOW        = {255, 255,   0};
#define HEX_COLOR_YELLOW "FFFF00"

static const color_rgb_t COLOR_GREEN         = {  0, 255,   0};
#define HEX_COLOR_GREEN "00FF00"

static const color_rgb_t COLOR_CYAN          = {  0, 255, 255};
#define HEX_COLOR_CYAN "00FFFF"

static const color_rgb_t COLOR_BLUE          = {  0,   0, 255};
#define HEX_COLOR_BLUE "0000FF"

static const color_rgb_t COLOR_PURPLE        = {255,   0, 255};
#define HEX_COLOR_PURPLE "FF00FF"

static const color_rgb_t COLOR_WHITE         = {255, 255, 255};
// light colors
static const color_rgb_t COLOR_LIMEGREEN     = {128, 255,   0}; 
#define HEX_COLOR_LIMEGREEN "80FF00"

static const color_rgb_t COLOR_LIGHTBLUE     = {  0, 128, 255}; 
#define HEX_COLOR_LIGHTBLUE "0080FF"

static const color_rgb_t COLOR_VIOLET        = {128,   0, 255}; 
#define HEX_COLOR_VIOLET "8000FF"

static const color_rgb_t COLOR_LIGHTGREEN    = {  0, 255, 128}; 
#define HEX_COLOR_LIGHTGREEN "00FF80"

static const color_rgb_t COLOR_PINK          = {255,   0, 128}; 
#define HEX_COLOR_PINK "FF0080"

static const color_rgb_t COLOR_DARKPURPLE    = {128,   0, 128};
#define HEX_COLOR_DARKPURPLE "800080"

static const color_rgb_t COLOR_TEAL          = {  0, 128, 128};
#define HEX_COLOR_TEAL "008080"

static const color_rgb_t COLOR_OLIVE         = {128, 128,   0};
#define HEX_COLOR_OLIVE "808000"

color_hsv_t hsv;
color_rgb_t *rgb;


#define HUE_QUADRANT   ((360 + 5)/ 6)

void  int_to_rgb(uint32_t color32, volatile color_rgb_t *rgb);
void  rgb_to_int(const color_rgb_t *rgb, uint32_t *color32);
void  rgbi_to_int(uint8_t r, uint8_t g, uint8_t b, uint32_t *color32);
void  hex_to_rgb(const char *color, volatile color_rgb_t *rgb);
void  hsv_to_rgb(volatile color_rgb_t *rgb, const color_hsv_t hsv);
void  rgb_to_hsv(const color_rgb_t *rgb, color_hsv_t *hsv);



#endif