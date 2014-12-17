/*
 * lcdcontrol.h - Application to change parameters of LCD2USB display.
              Modified sources from:
 *            http://www.harbaum.org/till/lcd2usb
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <usb.h>
#include <argp.h>

/* vendor and product id */
#define LCD2USB_VID  0x0403
#define LCD2USB_PID  0xc630

/* target is a bit map for CMD/DATA */
#define LCD_CTRL_0         (1<<3)
#define LCD_CTRL_1         (1<<4)
#define LCD_BOTH           (LCD_CTRL_0 | LCD_CTRL_1)

#define LCD_ECHO           (0<<5)
#define LCD_CMD            (1<<5)
#define LCD_DATA           (2<<5)
#define LCD_SET            (3<<5)
#define LCD_GET            (4<<5)

/* target is value to set */
#define LCD_SET_CONTRAST   (LCD_SET | (0<<3))
#define LCD_SET_BRIGHTNESS (LCD_SET | (1<<3))
#define LCD_SET_RESERVED0  (LCD_SET | (2<<3))
#define LCD_SET_RESERVED1  (LCD_SET | (3<<3))

/* target is value to get */
#define LCD_GET_FWVER      (LCD_GET | (0<<3))
#define LCD_GET_KEYS       (LCD_GET | (1<<3))
#define LCD_GET_CTRL       (LCD_GET | (2<<3))
#define LCD_GET_RESERVED1  (LCD_GET | (3<<3))

#ifdef WIN
#include <windows.h>
#include <winbase.h>
#define MSLEEP(a) Sleep(a)
#else
#define MSLEEP(a) usleep(a*1000)
#endif
#define ECHO_NUM 100

usb_dev_handle      *handle = NULL;

/* to increase performance, a little buffer is being used to */
/* collect command bytes of the same type before transmitting them */
#define BUFFER_MAX_CMD 4        /* current protocol supports up to 4 bytes */
int buffer_current_type = -1;   /* nothing in buffer yet */
int buffer_current_fill = 0;    /* -"- */
unsigned char buffer[BUFFER_MAX_CMD];
/* command format:
 * 7 6 5 4 3 2 1 0
 * C C C T T R L L
 *
 * TT = target bit map
 * R = reserved for future use, set to 0
 * LL = number of bytes in transfer - 1
 */
/* ArgParse related stuff... */
const char *argp_program_version =
  "--      LCD2USB control application      --\n\
  --      (c) 2014 by Michal Obrembski     --\n\
  --  Modified sources from Till Harbaum   --\n\
  -- http://www.harbaum.org/till/lcd2usb   --\n";
const char *argp_program_bug_address =
  "<byku@byku.com.pl>";

/* Program documentation. */
static char doc[] =
  "lcdcontrol -- program to control display connected via LCD2USB.";
/* ArgParse handler */
static error_t
parse_opt (int key, char *arg, struct argp_state *state);
/* A description of the arguments we accept. */
static struct argp_option options[] = {
    {"verbose",'v',0,0,"Produce verbose output." },
    {"contrast",'c',"VALUE",0,"Set static contrast." },
    {"brightness",'b',"VALUE",0,"Set static brightness." },
    {"fade-in",'u',"TIME",0,"Fade-in brightness with given time." },
    {"fade-out",'d',"TIME",0,"Fade-out brightness with given time." },
    { 0 }
};
struct arguments
{
  int contrast;
  int brightness;
  int fade_in;
  int fade_out;
};
static struct argp argp = { options, parse_opt, 0, doc };
static int verbose_mode = 0;
/* flush command queue due to buffer overflow / content */
/* change or due to explicit request */
void lcd_flush(void);
/* enqueue a command into the buffer */
void lcd_enqueue(int command_type, int value);
/* see HD44780 datasheet for a command description */
void lcd_command(const unsigned char ctrl, const unsigned char cmd);
/* clear display */
void lcd_clear(void);
/* return cursor to home */
void lcd_home(void);
/* write a data string to the first display */
void lcd_write(const char *data);
/* send a number of 16 bit words to the lcd2usb interface */
/* and verify that they are correctly returned by the echo */
/* command. This may be used to check the reliability of */
/* the usb interfacing */
void lcd_echo(void);
/* get a value from the lcd2usb interface */
int lcd_get(unsigned char cmd);
/* get lcd2usb interface firmware version */
void lcd_get_version(void);
/* get the bit mask of installed LCD controllers (0 = no */
/* lcd found, 1 = single controller display, 3 = dual */
/* controller display */
void lcd_get_controller(void);
/* get state of the two optional buttons */
void lcd_get_keys(void);
/* set a value in the LCD interface */
void lcd_set(unsigned char cmd, int value);
/* fade out LCD brightness with TIME steps */
void lcd_fade_out(int time);
/* fade in LCD brightness with TIME steps */
void lcd_fade_in(int time);
/* Parses arguments received from user */
void parseArguments(struct arguments *arguments);
void print_libusb_error(int errorcode);
