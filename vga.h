#ifndef VGA__H
#define VGA__H

#include <stdint.h>



#define MAX_X 35
#define MAX_Y 16
#define CHAR_HEIGHT 15
#define CHAR_WIDTH   9
#define VIDEOPORT_TOPLINE_Y  CHAR_HEIGHT
#define VIDEOPORT_BOTLINE_Y  ((CHAR_HEIGHT * (MAX_Y - 1)) - 3)
#define VIDEOPORT_HEIGHT (VIDEOPORT_BOTLINE_Y - VIDEOPORT_TOPLINE_Y)

#ifndef NCURSES
void clear( void ); //FIXME 
#endif


uint8_t char_width( uint8_t c  );
uint8_t char_height( uint8_t c  );




#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

//we are use 5/6/5 rgb format ( RRRRRGGGGGGBBBBB); 16 bits per pixel
#define RGB565(r,g,b) ((((r) >> 3) << 11) | (((g) >> 2) << 5) | ((b) >> 3))

#define COLOR_BLACK   RGB565( 0x00, 0x00, 0x00 )
#define COLOR_RED     RGB565( 0xFF, 0x00, 0x00 )
#define COLOR_GREEN   RGB565( 0x00, 0xFF, 0x00 )
#define COLOR_BLUE    RGB565( 0x00, 0x00, 0xFF )
#define COLOR_WHITE   RGB565( 0xFF, 0xFF, 0xFF )
#define COLOR_CYAN    RGB565( 0x00, 0xFF, 0xFF )  
#define COLOR_MAGENTA RGB565( 0xFF, 0x00, 0xFF ) 
#define COLOR_YELLOW  RGB565( 0xFF, 0xFF, 0x00 )

#define COLOR_FG_DEFAULT COLOR_WHITE
#define COLOR_BG_DEFAULT COLOR_BLUE
#define COLOR_HIGHLIGHT  COLOR_YELLOW
#define COLOR_READONLY   COLOR_CYAN

//#define MAX_COLORS 8

#define VGA_DEFAULT_FONT  0
#define VGA_MEDIUM_FONT   1
#define VGA_SMALL_FONT    2


#define CHAR_ROTATE_NONE   0
#define CHAR_ROTATE_RIGHT  1
#define CHAR_ROTATE_LEFT   2
#define CHAR_ROTATE_INVERT 3



#define CHAR_SIZE   32          //in bytes

#define VGA_DEFAULT_FONT_WIDTH  9
#define VGA_DEFAULT_FONT_HEIGHT 15
#define VGA_DEFAULT_FONT_OFFSET 0
#define VGA_MEDIUM_FONT_WIDTH   6
#define VGA_MEDIUM_FONT_HEIGHT  13
#define VGA_MEDIUM_FONT_OFFSET  (17+5)
#define VGA_SMALL_FONT_WIDTH    5
#define VGA_SMALL_FONT_HEIGHT   8
#define VGA_SMALL_FONT_OFFSET   17

typedef struct {
  uint8_t width;
  uint8_t height;
} vga_font_t;


//init/deinit
int vga_init( char *dev );
int vga_deinit( void );



//control of color scheme 
void vga_set_color( uint16_t col );
void vga_set_colors( uint16_t fg, uint16_t bg );
void vga_set_bgcolor( uint16_t color );
void vga_set_fgcolor( uint16_t color );
uint16_t vga_fg_color( void );
uint16_t vga_bg_color( void );
void vga_normal( void );
void vga_set_default( void );
void vga_highlight( void );
void vga_invert( void );
void vga_readonly( void );
uint16_t vga_invert_color(uint16_t color);
uint16_t vga_color( uint8_t index );



//control of fonts 
int vga_load_font( char *font );

void vga_select_font( uint8_t index );
uint8_t vga_current_font( void );

void vga_set_transpfont( uint8_t tf );
uint8_t vga_transpfont( void );

uint8_t font_width( uint8_t index );
uint8_t font_height( uint8_t index );

uint8_t vga_char_width( void );
uint8_t vga_char_height( void );

//uint8_t char_width( uint8_t c  );
//uint8_t char_height( uint8_t c  );




//drawind dots
void vga_dot( uint16_t x, uint16_t y );
void vga_c_dot( uint16_t x, uint16_t y, uint16_t color );



//drawing lines 
void vga_set_dashmask( uint8_t mask );

void vga_line( uint16_t x1, uint16_t y1,
               uint16_t x2, uint16_t y2 );

void vga_c_line( uint16_t x1, uint16_t y1,
                 uint16_t x2, uint16_t y2, 
                 uint16_t color );

void vga_d_line( uint16_t x1, uint16_t y1,
                 uint16_t x2, uint16_t y2,
                 uint8_t mask );

void vga_cd_line( uint16_t x1, uint16_t y1,
                  uint16_t x2, uint16_t y2,
                  uint16_t color, uint8_t mask );


//drawing rect
void vga_fill_rect( uint16_t x1, uint16_t y1,
                    uint16_t x2, uint16_t y2 );

void vga_c_fill_rect( uint16_t x1, uint16_t y1,
                      uint16_t x2, uint16_t y2,
                      uint16_t color );

void vga_c_rect( uint16_t x1, uint16_t y1,
                 uint16_t x2, uint16_t y2,
                 uint16_t color );

void vga_fill_screen( uint16_t color );




//drawing chars
void vga_putchar(  uint8_t c, uint16_t x, uint16_t y );
void vga_rot_putchar( uint8_t c, uint16_t x, uint16_t y, uint8_t rotation );
void vga_g_putchar(  uint8_t c, uint8_t col, uint8_t row );


//drawing strings
void vga_putstr( const char *s, uint16_t x, uint16_t y );
void vga_c_putstr( const char *s, uint16_t x, uint16_t y,  uint16_t color );
void vga_rot_putstr( const char *s, uint16_t x, uint16_t y, uint8_t rotation );
void vga_g_putstr( const char *s, uint8_t col, uint8_t row );
void vga_gc_putstr( const char *s, uint8_t col, uint8_t row, uint16_t color );



//drawing circle
void vga_circle( uint16_t x, uint16_t y, uint16_t radius);
void vga_c_circle( uint16_t x, uint16_t y, uint16_t radius, uint16_t color);
void vga_fill_circle( uint16_t x, uint16_t y, uint16_t radius );
void vga_c_fill_circle( uint16_t x, uint16_t y, uint16_t radius, uint16_t color);

#endif
