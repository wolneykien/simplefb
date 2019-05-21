#include <X11/Xlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include <linux/input.h>

#define WIDTH           320 
#define HEIGHT          240

#define ALL_PIXELS      HEIGHT * WIDTH
#define ARRAY_SIZE(x)   (sizeof(x) / sizeof((x)[0]))

const char fb_init_arr[ 2*ALL_PIXELS ];

#define FB_DEFAULT_PATH "/tmp/fb0"
#define KBD_DEFAULT_PATH "/tmp/kbd0"

uint8_t red5(uint16_t value) {                                        
    return (value >> 11) & 0x1F;                                        
}                                                                     
                                                                      
uint8_t green6(uint16_t value) {                                      
    return (value >> 5) & 0x3F;                                         
}                                                                     
                                                                      
uint8_t blue5(uint16_t value) {                                       
    return value & 0x1F;                                                
}                                                                     
                                                                      
uint8_t red(uint16_t value) {                                         
    return red5(value) * 255 / 31;                                      
}                                                                     
                                                                      
uint8_t green(uint16_t value) {                                       
    return green6(value) * 255 / 63;                                    
}                                                                     
                                                                      
uint8_t blue(uint16_t value) {                                        
    return blue5(value) * 255 / 31;                                     
}           

int draw( Display *display, Window *window, GC *gc, Colormap *colormap,
          const char *fb_path ) 
{
  static XColor clr;
  static uint16_t px;

  FILE* file = fopen( fb_path, "r" );

  int i;
  int c;
  for( i = 0; i < ALL_PIXELS; i++ ) 
  {
    if( (c = fread( &px, sizeof(uint16_t), 1, file )) < 0 )
      break;

    clr.red =   red(px);
    clr.green = green(px);
    clr.blue =  blue(px);

    clr.pixel = ( ( (unsigned long)clr.red)   << 16 ) + 
                ( ( (unsigned long)clr.green)  << 8 ) + 
                ( ( (unsigned long)clr.blue)        );

    XSetForeground( display, *gc, clr.pixel );
    XDrawPoint( display, *window, *gc, i % WIDTH, i / WIDTH );
  }

  fclose( file );

  return 0; 
}

uint16_t get_input_event_code( unsigned int xkeycode )
{
  return xkeycode-8;
}

int main( int argc, char **argv )
{
  struct sigaction act;
  int lasterr;

  sigemptyset( &act.sa_mask );
  act.sa_handler = SIG_IGN;
  act.sa_flags   = 0;

  if ( sigaction( SIGPIPE, &act, NULL ) ) {
    lasterr = errno;
    fprintf( stderr, "Error: sigaction error: %s (%i)\n",
             strerror( lasterr), lasterr );
    return lasterr;
  }

  char* fb_path = getenv( "ETU_FB_PATH" );

  if( fb_path == NULL ) {
    fb_path = FB_DEFAULT_PATH;
  }

  int fb_fd = creat( fb_path, 0644 );
  if( fb_fd < 0 ) {
    perror( "creat" );
    return -1;
  }

  int ret = write( fb_fd, fb_init_arr, ARRAY_SIZE( fb_init_arr ) );

  if (ret < 0) {
    fprintf( stderr, "Write error!\n" );
    exit(ret);
  }

  close( fb_fd );

  char* kbd_path = getenv( "ETU_KBD_PATH" );

  if( kbd_path == NULL ) {
    kbd_path = KBD_DEFAULT_PATH;
  }

  unlink( kbd_path );
  if( mkfifo( kbd_path, 0644 ) ) {
    perror( "mkfifo" );
    return -1;
  }
 
  char* shtd_path = getenv( "ETU_SHTD_PATH" );

  if( shtd_path ) {
    unlink( shtd_path );
    if( mkfifo( shtd_path, 0644 ) ) {
      perror( "mkfifo" );
      return -1;
    }
  }

  Display *dsp = XOpenDisplay( NULL );

  if( !dsp ){
    fprintf( stderr, "No X11 display\n" );
    exit(1);
  }

  int screen = DefaultScreen( dsp );
  unsigned int white = WhitePixel(dsp,screen);
  unsigned int black = BlackPixel(dsp,screen);

  Window win = XCreateSimpleWindow(dsp,
                            DefaultRootWindow( dsp ),
                            0, 0,
                            WIDTH, HEIGHT,
                            0, black,
                            white);

  GC gc = XCreateGC( dsp, win, 0, NULL);
  Colormap colormap = DefaultColormap( dsp, screen );
  XSetForeground( dsp, gc, black );
  
  long eventMask = StructureNotifyMask | KeyPressMask | ButtonPressMask;
  XSelectInput( dsp, win, eventMask );
  XMapWindow( dsp, win );

  XEvent evt;

  do { 
    XNextEvent(dsp, &evt); 
  } while( evt.type == MapNotify );

  struct et_input_event {
          uint64_t time;
          uint16_t type;
          uint16_t code;
          int32_t value;
  } kbd_event;

  sleep(1);

  FILE* file_kbd = fopen( kbd_path, "w" );

  FILE* file_shtd = NULL;
  if( shtd_path )
    file_shtd = fopen( shtd_path, "w" );

  short loop = 1;
  while( loop ) {
    if( XCheckTypedEvent( dsp, KeyPress, &evt ) ) {
      kbd_event.type = EV_KEY;
      kbd_event.code = get_input_event_code( evt.xkey.keycode );
      kbd_event.value = 1;

      if( ( kbd_event.code == KEY_F5 ) && file_shtd ) {
        kbd_event.code = KEY_POWER;
        fwrite( &kbd_event, sizeof(kbd_event), 1, file_shtd );
        fflush( file_shtd );
      } else {
        fwrite( &kbd_event, sizeof(kbd_event), 1, file_kbd );
        fflush( file_kbd );
      }
    }
    if( XCheckTypedEvent( dsp, ButtonPress, &evt ) ) {
      break;
    }
    draw( dsp, &win, &gc, &colormap, fb_path );
    usleep( 10000 );
  }

  fclose( file_kbd );

  unlink( kbd_path );
  XDestroyWindow( dsp, win );
  XCloseDisplay( dsp );
  return 0;
}
