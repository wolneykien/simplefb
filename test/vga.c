#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <linux/fb.h>
#include <linux/kd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include "vga.h"

static uint8_t transpfont = 0;   ///font transparation

static uint8_t dash_mask = 0xFF; 
static uint8_t dash_bit  = 0x80; 

static uint8_t current_font_index;
static vga_font_t *current_font;

static char *bitmap;

static char *fbp = 0;


static struct fb_var_screeninfo vinfo;
static struct fb_fix_screeninfo finfo;
static int screensize;
static int fb;

static uint16_t fg_color;
static uint16_t bg_color;

static vga_font_t vga_font[] = {
    [ VGA_DEFAULT_FONT ] = {
      .width = VGA_DEFAULT_FONT_WIDTH,
      .height = VGA_DEFAULT_FONT_HEIGHT,
    },
    [ VGA_MEDIUM_FONT ] = {
      .width = VGA_MEDIUM_FONT_WIDTH,
      .height = VGA_MEDIUM_FONT_HEIGHT,
    },
    [ VGA_SMALL_FONT ] = {
      .width = VGA_SMALL_FONT_WIDTH,
      .height = VGA_SMALL_FONT_HEIGHT,
    },
};

/*
struct FB {
   unsigned short *bits;
   unsigned size;
   int fd;
   struct fb_fix_screeninfo fi;
   struct fb_var_screeninfo vi;
};

static void vga_update(struct FB *fb)
{
    fb->vi.yoffset = 1;
    ioctl(fb->fd, FBIOPUT_VSCREENINFO, &fb->vi);
    fb->vi.yoffset = 0;
    ioctl(fb->fd, FBIOPUT_VSCREENINFO, &fb->vi);
}
*/

//int vga_set_graphics_mode( char *dev )
//{
//  int ret = 0;
//  
//  int tty = open( dev, O_RDWR | O_SYNC  );
//  if( !tty ) {
//    fprintf( stderr, "Error: can't open %s - %s\r\n", dev, strerror( errno ) );
//    return errno;
//  }
//
///* if (ioctl(tty, KDSETMODE, KD_GRAPHICS) != 0) {
//    fprintf( stderr, "Error: can't set graphic mode - %s\r\n", strerror( errno ) );
//    ret = errno;
//    goto close_tty;
//  }
//*/
//  const char termctl[] = "\033[9;0]\033[?33l\033[?25l\033[?1c";
//  ret = write(tty, termctl, sizeof(termctl));
//  if( ret < 0 ) {
//    fprintf( stderr, "Error: can't write - %s\r\n", strerror( errno ) );
//    ret = errno;
//    goto close_tty;
//  }
//
//  if( ret != sizeof(termctl ) ) {
//    fprintf( stderr, "Error: can't write\r\n" );
//    ret = EINVAL;
//    goto close_tty;
//  }
//
//  ret = 0;
//
//close_tty:  
//  close( tty );
//  return ret;
//}

/**Initialize video subsystem
 * \param dev framebuffer device ( "/dev/fb0" for example )
 * \return Zero on succes, nozero otherwise.
 * 
 */
int vga_init( char *dev )
{
#ifndef NCURSES  
  int ret = 0;

//  vga_set_graphics_mode( "/dev/tty0" );
   
  fb = open( dev, O_RDWR );
  if (!fb) {
    fprintf( stderr, "Error: can't open %s - %s\r\n", 
	     dev, strerror( errno ) );
    return errno;
  }

/* if (ioctl(fb, FBIOGET_FSCREENINFO, &finfo)) {
    fprintf( stderr, "Error: reading fixed information - %s\r\n",
    	     strerror( errno ) );
    ret = errno;
    goto close_fb;
  }

  if (ioctl(fb, FBIOGET_VSCREENINFO, &vinfo)) {
    fprintf( stderr, "Error: reading variable information - %s\r\n",
	     strerror( errno ) );
    ret = errno;
    goto close_fb;
  }
*/

  vinfo.xres = 320;
  vinfo.yres = 240;
  vinfo.bits_per_pixel = 16;

  finfo.line_length = vinfo.xres * ( vinfo.bits_per_pixel / 8 );

  screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
  fbp = mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
  if( fbp == MAP_FAILED ) {
    fprintf( stderr, "Error: failed to map framebuffer device to memory - %s.\r\n",
             strerror( errno ) );
    ret = errno;
    goto close_fb;
  }

  vinfo.blue.offset     = 0;
  vinfo.blue.length     = 5;
  vinfo.blue.msb_right  = 0;
  vinfo.red.offset      = 11;
  vinfo.red.length      = 5;
  vinfo.red.msb_right   = 0;
  vinfo.green.offset    = 5;
  vinfo.green.length    = 6;
  vinfo.green.msb_right = 0;
  vinfo.bits_per_pixel  = 16;

/*  if (ioctl(fb, FBIOPUT_VSCREENINFO, &vinfo)) {
    fprintf( stderr, "Error: setting variable information - %s\r\n",
             strerror( errno ) );
    ret = errno;
    goto unmap_fbp;
  }

  if( ioctl( fb, FBIOGET_FSCREENINFO, &finfo ) ) {
    fprintf( stderr, "Error: reading fixed information - %s\r\n", 
	     strerror( errno ) );
    ret = errno;
    goto unmap_fbp;
  }
*/

  vga_fill_screen( COLOR_BG_DEFAULT );
  vga_set_color( COLOR_FG_DEFAULT );
  vga_set_bgcolor( COLOR_BG_DEFAULT );
  transpfont = 0;
  vga_set_dashmask( 0xFF );
  vga_select_font( VGA_DEFAULT_FONT );
  return 0;

/*unmap_fbp:
  munmap(fbp, screensize);
*/
close_fb:  
  close(fb);

  return ret;
#else
  initscr();
  return 0;
#endif

}

int vga_deinit( void )
{
#ifndef NCURSES  
  munmap(fbp, screensize);
  return close(fb);
#else
  endwin();
  return 0;
#endif  
}


#ifndef NCURSES  
/*! \brief Clear screen, without fkeys and status bar area */
void clear( void )
{
  vga_c_fill_rect( 0, VIDEOPORT_TOPLINE_Y + 1, 
                   SCREEN_WIDTH-1, VIDEOPORT_BOTLINE_Y - 1 ,
                   COLOR_BG_DEFAULT );
}
#endif


/**Return default settings for fonts, colors and masks*/
void vga_set_default( void )
{
  vga_normal();
  transpfont = 0;
  vga_select_font( VGA_DEFAULT_FONT );
  vga_set_dashmask( 0xFF );
}


/** Loads font.
 * \return 0 if successful, nozero otherwise  
 */
int vga_load_font( char *font )
{
#ifndef NCURSES  
  int f = open( font, O_RDONLY );

  if( !f ) {
    fprintf( stderr, "ERROR: Can't load %s - %s\r\n", font, strerror( errno ) );
    return errno;    
  }

  if( bitmap == NULL ) {
    bitmap = malloc( CHAR_SIZE*256 );
    if( bitmap == NULL ) {
      return ENOMEM;      
    }    
  }

  int ret = read( f, bitmap, CHAR_SIZE*256 );
  if( ret < 0 ) {
    fprintf( stderr, "ERROR: Can't read %s - %s\r\n", font, strerror( errno ) );
    return errno;    
  }

  close( f );
 
  if( ret != CHAR_SIZE*256 ) {
    fprintf( stderr, "ERROR: Invalid file size %s\r\n", font );
    return EINVAL;
  }
#endif  
  return 0;
}


uint8_t font_width( uint8_t index )
{
  return vga_font[ index ].width;
}

uint8_t font_height( uint8_t index )
{
  return vga_font[ index ].height;
}

uint8_t char_width( uint8_t c  )
{
  return vga_char_width( );
}

uint8_t char_height( uint8_t c  )
{
  return vga_char_height( );
}


uint8_t vga_char_width( void )
{
  return current_font->width;
}

uint8_t vga_char_height( void )
{
  return current_font->height;
}



/**Select a font in memory space
 * \param index the index of font to select
 * Use the VGA_*_FONT macros from vga.h to switch standard fonts.
 * */
void vga_select_font( uint8_t index )
{
  current_font = &vga_font[ index ];
  current_font_index = index;
}

uint8_t vga_current_font( void )
{
  return current_font_index;
}

uint8_t vga_transpfont( void )
{
  return transpfont;
}

/** \brief Set transparent font
 *  \param tf Transparent flag
 */ 
void vga_set_transpfont( uint8_t tf )
{
  transpfont = tf ? 1 : 0; 
}




/**
 * Set global foreground color
 * \param color The color to set *
 * */
void vga_set_color( uint16_t color )
{
  //set foreground color
  fg_color = color;
}

/**
 * Set global foreground color
 * \param color The color to set *
 * */
void vga_set_fgcolor( uint16_t color )
{
  //set foreground color
  fg_color = color;
}


/**
 * Set global background color
 * \param color The color to set *
 * */
void vga_set_bgcolor( uint16_t color )
{
  //set background color
  bg_color = color;
}


/**
 * Set both  global color variables
 * \param fg The foreground color to set 
 * \param bg The background color to set
 * */
void vga_set_colors( uint16_t fg, uint16_t bg )
{
  fg_color = fg;
  bg_color = bg;
}

uint16_t vga_fg_color( void )
{
  return fg_color;
}


uint16_t vga_bg_color( void )
{
  return bg_color;
}

/**Set default color scheme*/
void vga_normal( void )
{
  vga_set_colors( COLOR_FG_DEFAULT, COLOR_BG_DEFAULT );
}

/**Set highlight colorscheme*/
void vga_highlight( void )
{
  vga_set_colors( COLOR_HIGHLIGHT, COLOR_BG_DEFAULT );
}

/**Set inverted colorscheme*/
void vga_invert( void  )
{
  vga_set_colors( COLOR_BG_DEFAULT, COLOR_FG_DEFAULT );
}


/**Set colorscheme for RO items*/
void vga_readonly( void  )
{
  vga_set_colors( COLOR_READONLY, COLOR_BG_DEFAULT );
}




/** Draw single dot 
 * \param x X coordinate
 * \param y Y coordinate
 * \param color Color of dot
 * */
void vga_c_dot( uint16_t x, uint16_t y,  uint16_t color )
{                              
  uint32_t offset;

  /* Figure out where in memory to put the pixel */
  offset = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
           (y+vinfo.yoffset) * finfo.line_length;

  *(fbp + offset + 0) = color & 0xFF; 
  *(fbp + offset + 1) = color >> 8;
}


/** Draw single dot 
 * \param x X coordinate
 * \param y Y coordinate
 * \param color Color of dot
 * */
void vga_dot( uint16_t x, uint16_t y )
{       
  vga_c_dot( x, y, fg_color );  
}



/**
 * Set global dashmask variable for lines
 * \param mask The mask to set, every 1 means pixel to be drawn, 0 to skip
 * */
void vga_set_dashmask( uint8_t mask )
{
  dash_mask = mask;
  dash_bit  = 0x80;
}


static void vga_dash( uint16_t x, uint16_t y )
{
  if( dash_mask & dash_bit ) {
     vga_dot( x, y );
  }

  dash_bit = dash_bit >> 1;
  if ( dash_bit == 0 ) {
    dash_bit = 0x80;
  }
}


/**Render a line with current mask and color
 *  \param x1 Start coordinate
 *  \param y1 Start coordinate
 *  \param x2 End coordinate
 *  \param y2 End coordinate
 *
 *  Dashmask is always used from MSB, starting in start point.
 * */
void vga_line( uint16_t x1, uint16_t y1,
               uint16_t x2, uint16_t y2 )
{
  int16_t dx,dy,fraction;
  int8_t step_x,step_y;

  dy = y2 - y1;
  dx = x2 - x1;

  if( dy < 0 ) {
    dy = -dy;
    step_y = -1;
  }
  else {
    step_y = 1;
  }

  if( dx < 0 )  {
    dx = -dx;
    step_x = -1;
  }
  else {
    step_x = 1;
  }
  
  vga_dash( x1, y1 );

  if( dx > dy ) {
    dy <<= 1;
    fraction = dy - dx;
    dx <<= 1;
    while( x1 != x2 ) {
      if( fraction >= 0 ) {
	y1 += step_y;
	fraction -= dx;
      }
      x1 += step_x;
      fraction += dy;

      vga_dash( x1, y1 );
    }
  }
  else {
    dx <<= 1;
    fraction = dx - dy;
    dy <<= 1;
    while( y1 != y2 ) {
      if( fraction >= 0 ) {
	x1 += step_x;
	fraction -= dy;
      }
      y1 += step_y;
      fraction += dx;
      vga_dash( x1, y1 );
    }
  }
}


/**Render a line with current mask and  specific color
 *  \param x1 Start coordinate
 *  \param y1 Start coordinate
 *  \param x2 End coordinate
 *  \param y2 End coordinate
 *  \param color Explicitly set color
 *
 * This function calls vga_line.
 * */
 void
vga_c_line( uint16_t x1, uint16_t y1,
            uint16_t x2, uint16_t y2, uint16_t color )
{
#ifndef NCURSES  
  vga_set_color( color );
  vga_line( x1, y1, x2, y2 );
#endif  
}

/**Render a line with specific mask and  current color
 *  \param x1 Start coordinate
 *  \param y1 Start coordinate
 *  \param x2 End coordinate
 *  \param y2 End coordinate
 *  \param mask Explicitly set color
 *
 * This function calls vga_line.
 * */
void vga_d_line( uint16_t x1, uint16_t y1,
                 uint16_t x2, uint16_t y2, uint8_t mask )
{
  vga_set_dashmask( mask );
  vga_line( x1, y1, x2, y2 );
}

/**Render a line with specific mask and  specific color
 *  \param x1 Start coordinate
 *  \param y1 Start coordinate
 *  \param x2 End coordinate
 *  \param y2 End coordinate
 *  \param color Explicitly set color
 *  \param mask Dash mask to use 
 * This function calls vga_line.
 * */
void vga_cd_line( uint16_t x1, uint16_t y1,
             uint16_t x2, uint16_t y2,
             uint16_t color, uint8_t mask )
{
  vga_set_dashmask( mask );
  vga_set_color( color );
  vga_line( x1, y1, x2, y2 );
}


/**Render unfilled rectangle using specific color
 *  \param x1 Topleft coordinate
 *  \param y1 Topleft coordinate
 *  \param x2 Bottomright coordinate
 *  \param y2 Bottomright coordinate
 *  \param color The color to use
 * */
void
vga_c_rect( uint16_t x1, uint16_t y1,
           uint16_t x2, uint16_t y2, uint16_t color )
{
  vga_set_color( color );
  vga_line( x1, y1, x2, y1 );   //top
  vga_line( x1, y1, x1, y2 );   //left
  vga_line( x1, y2, x2, y2 );   //bot
  vga_line( x2, y1, x2, y2 );   //right
}

/**
 *  Render filled rectangle with current color.
 *  This function is always successful, but execution time depends on size.
 *  \param x1 x coordinate of first point
 *  \param y1 y coordinate of first point
 *  \param x2 x coordinate of first point
 *  \param y2 y coordinate of first point
 * */
void vga_fill_rect( uint16_t x1, uint16_t y1,
                    uint16_t x2, uint16_t y2 )
{
  uint32_t offset = 0;
  offset = (x1+vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
 	   (y1+vinfo.yoffset) * finfo.line_length;

  uint32_t x,y;
  for( y = y1; y <= y2; y++ ) {
     for( x = x1; x <= x2; x++ ) {
        *(fbp + offset + 0) = fg_color & 0xFF; 
        *(fbp + offset + 1) = fg_color >> 8;
	offset += vinfo.bits_per_pixel/8;
     }
  }
}

/**
 * Sets color and calls vga_fill_rect
 *  \param x1 x coordinate of first point
 *  \param y1 y coordinate of first point
 *  \param x2 x coordinate of first point
 *  \param y2 y coordinate of first point
 *  \param color The color to be set before drawing 
 * */
void vga_c_fill_rect( uint16_t x1, uint16_t y1,
                      uint16_t x2, uint16_t y2,
                      uint16_t color )
{
  vga_set_color( color );
  vga_fill_rect( x1, y1, x2, y2 );
}

/** Fill whole screen with specified color using vga_fill_rect
 *\param color The color to use
 * */
void vga_fill_screen( uint16_t color )
{
  vga_c_fill_rect( 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, color );
}


static uint16_t vga_symbol_addr( uint8_t c )
{
  uint16_t symbol_addr = 0;
  symbol_addr = c * CHAR_SIZE;
  switch( current_font_index ) {
    case VGA_DEFAULT_FONT:
      symbol_addr += VGA_DEFAULT_FONT_OFFSET;
      break;
    case VGA_MEDIUM_FONT:
      symbol_addr += VGA_MEDIUM_FONT_OFFSET;
      break;
    case VGA_SMALL_FONT:
      symbol_addr += VGA_SMALL_FONT_OFFSET;
      break;
  }
  return symbol_addr;  
}




/** Draw a symbol at any position on the screen and rotate it
 * \param c Symbol code
 * \param x X coordinate
 * \param y Y coordinate
 * \param rotation Direction of rotation. Rotate right if 1 and left if 2. 
 * \No rotation if 0    
 */
void vga_rot_putchar( uint8_t c, uint16_t x, uint16_t y, uint8_t rotation )
{
  uint16_t dot_x, dot_y;
  uint8_t m = 0;
  uint8_t mask = 0;
  uint8_t i,j;
  uint8_t width = char_width( c ) - 1;
  uint8_t height = char_height( c ) - 1;
  uint16_t addr =  vga_symbol_addr( c );
  
  for( j = 0; j <= height; j++ ) {
    
    for( i = 0; i <= width; i++  ) {
      if( mask == 0 ) {
	mask = 0x80;
	m = bitmap[addr];
	addr++;
      }

      switch( rotation ) {
	case CHAR_ROTATE_NONE:
	  dot_x = x + i;
	  dot_y = y + j;
	  break;
	case CHAR_ROTATE_RIGHT:
	  dot_x = x + height - j;
	  dot_y = y + i;
	  break;
	case CHAR_ROTATE_LEFT:
	  dot_x = x + j;
	  dot_y = y + width - i;
	  break;
	case CHAR_ROTATE_INVERT:
	  dot_x = x + width - i;
	  dot_y = y + height - j;
	  break;
      }

      if( m & mask ) {
	vga_c_dot( dot_x, dot_y, fg_color );     
      }
      else if ( !transpfont ) {  
        vga_c_dot( dot_x, dot_y, bg_color );     
      }

      mask = mask >> 1;
    }
  }   
}




/** Draw a symbol at any position on the screen
 * \param c Symbol code
 * \param x X coordinate
 * \param y Y coordinate
 */
void  vga_putchar( uint8_t c, uint16_t x, uint16_t y )
{
  vga_rot_putchar( c, x, y, CHAR_ROTATE_NONE);
}

/** Draw a symbol at some position on the text grid
 * \param c Symbol 
 * \param col Column 
 * \param row Row 
 */
void  vga_g_putchar(  uint8_t c, uint8_t col, uint8_t row )
{
  uint16_t x = ( uint16_t ) col * current_font->width;
  uint16_t y = row * current_font->height;
  vga_putchar( c, x, y );
}


/** Draw a null-terminated string at any position on the screen, rotating it properly.
 * \param s Pointer to first byte
 * \param x X coordinate
 * \param y Y coordinate
 * \param rotation - Rotation flag according to defines above.
 * This function will render strings cyclically over screen if x or y goes out of screen area.
 */
void vga_rot_putstr( const char *s, uint16_t x, uint16_t y, uint8_t rotation )
{
  uint8_t width;
  while( *s != '\0' ) {
    width = char_width( *s );
    vga_rot_putchar( *s++, x, y, rotation );
    switch (rotation) {
      case CHAR_ROTATE_NONE:
	x += width;
	break;
      case CHAR_ROTATE_RIGHT:
	y += width;
	break;
      case CHAR_ROTATE_LEFT:
	y -= width;
	break;
      case CHAR_ROTATE_INVERT:
	x -= width;
	break;
    }
  }
}


/** Draw a null-terminated string at any position on the screen
 * \param s Pointer to first byte
 * \param x X coordinate
 * \param y Y coordinate
 *
 * This function will render strings cyclically over screen if x becomes out of screen area.
 */
void vga_putstr( const char *s, uint16_t x, uint16_t y )
{
 vga_rot_putstr( s, x, y, CHAR_ROTATE_NONE  );
}

/** Draw a null-terminated string at any position on the screen using specific color
 * \param s Pointer to first byte
 * \param x X coordinate
 * \param y Y coordinate
 * \param color Color to set
 *
 * This function will render strings cyclically over screen if x becomes out of screen area.
 */
void vga_c_putstr( const char *s, uint16_t x, uint16_t y,
                   uint16_t color )
{
  vga_set_color( color );
  vga_putstr( s, x, y );
}

/** Draw a null-terminated string at some position on the text grid
 * \param s Pointer to first byte
 * \param col Column of first symbol
 * \param row Row of first symbol
 *
 * This function will render strings cyclically over screen if x becomes out of screen area.
 */
void vga_g_putstr( const char *s, uint8_t col, uint8_t row )
  //put string `s' starting at column `col' and row `row'
{
  uint16_t x = ( uint16_t ) col * current_font->width;
  uint16_t y = row * current_font->height;
  vga_putstr( s, x, y );
}

/** Draw a null-terminated string at some position on the text grid using specified color
 * \param s Pointer to first byte
 * \param col Column of first symbol
 * \param row Row of first symbol
 * \param color The color of letters
 * This function will render strings cyclically over screen if x becomes out of screen area.
 */
void vga_gc_putstr( const char *s, uint8_t col,
                    uint8_t row, uint16_t color )
{
  vga_set_color( color );
  vga_g_putstr( s, col, row );
}


/**Draw a full circle
 * \param x X coordinate of a center of circle
 * \param y Y coordinate of a center of circle
 * \param radius The radius of circle
 *
 * */
void vga_circle( uint16_t x, uint16_t y, uint16_t r )
{   
  uint16_t x1,y1,yk = 0;
  int16_t sigma,delta,f;
  x1 = 0;
  y1 = r;
  delta = 2*(1-r);
  do {
    vga_dot(x + x1, y + y1);
    vga_dot(x + x1, y - y1);
    vga_dot(x - x1, y + y1);
    vga_dot(x - x1, y - y1);

    f = 0;
    if (y1 < yk) {
      break;
    }

    if (delta < 0)  {
      sigma = 2*(delta+y1)-1;
      if (sigma <= 0) {
	x1++;
	delta += 2*x1+1;
	f = 1;
      }
    }
    else if (delta > 0 ) {
      sigma = 2*(delta-x1)-1;
      if (sigma > 0) {
	y1--;
	delta += 1-2*y1;
	f = 1;
      }
    }

    if( !f ) {
      x1++;
      y1--;
      delta += 2*(x1-y1-1);
    }

  } while( 1 );
}


/**Draw a filled circle
 * \param x X coordinate of a center of circle
 * \param y Y coordinate of a center of circle
 * \param radius The radius of circle
*/
void vga_fill_circle( uint16_t x, uint16_t y,
                      uint16_t radius )
{
  uint16_t crad;
  for( crad = radius; crad > 0; crad-- ) {
    vga_circle( x, y, crad );
  }
  vga_dot( x, y );
}

/**Draw an unfilled circle using specific color
 * \param x X coordinate of a center of circle
 * \param y Y coordinate of a center of circle
 * \param radius The radius of circle
 * \param color The color to use
*/
void vga_c_circle( uint16_t x, uint16_t y,
                   uint16_t radius, uint16_t color )
{
  vga_set_color( color );
  vga_circle( x, y, radius );
}

/**Draw a filled circle using specific color
 * \param x X coordinate of a center of circle
 * \param y Y coordinate of a center of circle
 * \param radius The radius of circle
 * \param color The color to use
*/
void vga_c_fill_circle( uint16_t x, uint16_t y,
                        uint16_t radius, uint16_t color )
{
  vga_set_color( color );
  vga_fill_circle( x, y, radius );
}



uint16_t vga_colors[] = 
{
 COLOR_BLACK,
 COLOR_RED,
 COLOR_GREEN,
 COLOR_YELLOW,
 COLOR_BLUE,
 COLOR_MAGENTA,
 COLOR_CYAN,
 COLOR_WHITE,
};


uint16_t vga_color( uint8_t index )
{
  return vga_colors[index];
}


