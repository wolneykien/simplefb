#include "vga.h"

#define OFFSET 16
#define HEIGHT ((240 - OFFSET*2) / 8)

int main( void )
{
  char dev[] = "fb0";
  char fnt[] = "koi8r.fnt";
  char* text[8] = { "black", "blue", "green", "cyan", 
                    "red", "magenta", "yellow", "white" };

  uint16_t colors[8] = {
    COLOR_BLACK,
    COLOR_BLUE,
    COLOR_GREEN,
    COLOR_CYAN,
    COLOR_RED,
    COLOR_MAGENTA,
    COLOR_YELLOW,
    COLOR_WHITE,
  };

  vga_init( dev );
  vga_load_font( fnt );   
  
  vga_set_transpfont( 1 );
  vga_select_font(VGA_DEFAULT_FONT);

  int i;
  for ( i = 0; i < 8; i ++ ) {
    uint16_t c = colors[i];
    vga_c_fill_rect( 0, HEIGHT*i + OFFSET, 320, (i+1)*HEIGHT + OFFSET, c );
    vga_set_colors( ~c, c );
    vga_putstr( text[i], 150, HEIGHT*i + OFFSET + (HEIGHT / 4) );
  }

  vga_select_font(VGA_MEDIUM_FONT);
  vga_rot_putstr( "test rotate left", 20, 200, CHAR_ROTATE_LEFT  );
  
  vga_select_font(VGA_SMALL_FONT);
  vga_rot_putstr( "test rotate right", 40, 20, CHAR_ROTATE_RIGHT  );
  vga_rot_putstr( "invert", 300, 200, CHAR_ROTATE_INVERT  );

  vga_set_dashmask( 0xF0 );
  vga_line( 0,0,319,239);

  vga_deinit();
  return 0;  
}
