/*
 * lcdcontrol.c - Application to change parameters of LCD2USB display.
              Modified sources from:
 *            http://www.harbaum.org/till/lcd2usb
 */

#include "lcdcontrol.h"

void print_libusb_error(int errorcode) {
  fprintf(stderr, "USB request failed! %s\n",
    usb_strerror());
}

int lcd_send(int request, int value, int index) {
  int err = usb_control_msg(handle, USB_TYPE_VENDOR, request,
          value, index, NULL, 0, 1000);
  if(err < 0) {
      print_libusb_error(err);
    return -1;
  }
  return 0;
}

void lcd_flush(void) {
  int request, value, index;
  
  /* anything to flush? ignore request if not */
  if (buffer_current_type == -1)
    return;
  
  /* build request byte */
  request = buffer_current_type | (buffer_current_fill - 1);
  
  /* fill value and index with buffer contents. endianess should IMHO not */
  /* be a problem, since usb_control_msg() will handle this. */
  value = buffer[0] | (buffer[1] << 8);
  index = buffer[2] | (buffer[3] << 8);
  
  /* send current buffer contents */
  lcd_send(request, value, index);
  
  /* buffer is now free again */
  buffer_current_type = -1;
  buffer_current_fill = 0;
}

void lcd_enqueue(int command_type, int value) {
  if ((buffer_current_type >= 0) && (buffer_current_type != command_type))
    lcd_flush();
  
  /* add new item to buffer */
  buffer_current_type = command_type;
  buffer[buffer_current_fill++] = value;
  
  /* flush buffer if it's full */
  if (buffer_current_fill == BUFFER_MAX_CMD)
    lcd_flush();
}

void lcd_command(const unsigned char ctrl, const unsigned char cmd) {
  lcd_enqueue(LCD_CMD | ctrl, cmd);
}

void lcd_clear(void) {
  lcd_command(LCD_BOTH, 0x01);    /* clear display */
  lcd_command(LCD_BOTH, 0x03);    /* return home */
}

void lcd_home(void) {
  lcd_command(LCD_BOTH, 0x03);    /* return home */
}

void lcd_write(const char *data) {
  int ctrl = LCD_CTRL_0;
  
  while(*data) 
    lcd_enqueue(LCD_DATA | ctrl, *data++);
  
  lcd_flush();
}

void lcd_echo(void) {
  int i, nBytes, errors=0;
  unsigned short val, ret;

  for(i=0;i<ECHO_NUM;i++) {
    val = rand() & 0xffff;
    
    nBytes = usb_control_msg(handle, 
     USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
     LCD_ECHO, val, 0, (char*)&ret, sizeof(ret), 1000);

    if(nBytes < 0) {
      print_libusb_error(nBytes);
      return;
    }

    if(val != ret)
      errors++;
  }

  if(errors) 
    fprintf(stderr, "ERROR: %d out of %d echo transfers failed!\n", 
      errors, ECHO_NUM);
  else 
    if(verbose_mode)
      printf("Echo test successful!\n");
}

int lcd_get(unsigned char cmd) {
  unsigned char       buffer[2];
  int                 nBytes;

  /* send control request and accept return value */
  nBytes = usb_control_msg(handle, 
     USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
     cmd, 0, 0, (char *)buffer, sizeof(buffer), 1000);

  if(nBytes < 0) {
    print_libusb_error(nBytes);
    return -1;
  }

  return buffer[0] + 256*buffer[1];
}

void lcd_get_version(void) {
  int ver = lcd_get(LCD_GET_FWVER);

  if(ver != -1 && verbose_mode) 
    printf("Firmware version %d.%d\n", ver&0xff, ver>>8);
}

void lcd_get_controller(void) {
  int ctrl = lcd_get(LCD_GET_CTRL);

  if(ctrl != -1 && verbose_mode) {
    if(ctrl)
      printf("Installed controllers: %s%s\n", 
       (ctrl&1)?"CTRL0":"",
       (ctrl&2)?" CTRL1":"");
    else
      fprintf(stderr, "No controllers installed!\n");
  }
}

void lcd_get_keys(void) {
  int keymask = lcd_get(LCD_GET_KEYS);

  if(keymask != -1 && verbose_mode) 
    printf("Keys: 0:%s 1:%s\n",
     (keymask&1)?"on":"off",
     (keymask&2)?"on":"off");
}

void lcd_set(unsigned char cmd, int value) {
  int errors = usb_control_msg(handle, 
       USB_TYPE_VENDOR, cmd, value, 0, 
       NULL, 0, 1000);
  if(errors < 0) {
    print_libusb_error(errors);
  }
}

void lcd_fade_out(int time) {
  int i;
  for(i=255;i>=0;i--) {
    lcd_set(LCD_SET_BRIGHTNESS, i);
    MSLEEP(time);
  }
}

void lcd_fade_in(int time) {
  int i;
  for(i=0;i<=255;i++) {
    lcd_set(LCD_SET_BRIGHTNESS, i);
    MSLEEP(time);
  }
}

void parseArguments(struct arguments *arguments) {
  if(arguments->contrast!=-1) {
    lcd_set(LCD_SET_CONTRAST, arguments->contrast);
  }
  if(arguments->fade_out!=-1) {
    lcd_fade_out(arguments->fade_out);
  }
  if(arguments->fade_in!=-1) {
    lcd_fade_in(arguments->fade_in);
  }
  if(arguments->brightness!=-1) {
    lcd_set(LCD_SET_BRIGHTNESS, arguments->brightness);
  }
}

static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  struct arguments *arguments = state->input;
  int val = 0;
  if(arg)
    sscanf(arg,"%d",&val);
  switch (key)
    {
    case 'v':
      verbose_mode=1;
      break;
    case 'c':
      arguments->contrast=val;    
      break;
    case 'b':
      arguments->brightness=val;
      break;
    case 'u':
      arguments->fade_in=val;
      break;
    case 'd':
      arguments->fade_out=val;
      break;
    case ARGP_KEY_ARG:
      break;
    case ARGP_KEY_END:
      break;
    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

int main(int argc, char *argv[]) {
  struct usb_bus      *bus;
  struct usb_device   *dev;
  struct arguments arguments;

  /* Default values. */
  arguments.fade_in = -1;
  arguments.fade_out = -1;
  arguments.contrast = -1;
  arguments.brightness = -1;
  argp_parse (&argp, argc, argv, 0, 0, &arguments);

  usb_init();
  
  usb_find_busses();
  usb_find_devices();
  
  for(bus = usb_get_busses(); bus; bus = bus->next) {
    for(dev = bus->devices; dev; dev = dev->next) {
      if((dev->descriptor.idVendor == LCD2USB_VID) && 
   (dev->descriptor.idProduct == LCD2USB_PID)) {
  
  if(verbose_mode)
   printf("Found LCD2USB device on bus %s device %s.\n", 
         bus->dirname, dev->filename);
  
  /* open device */
  if(!(handle = usb_open(dev))) 
    fprintf(stderr, "Error: Cannot open USB device: %s\n", 
      usb_strerror());
  break;
      }
    }
  }
  
  if(!handle) {
    fprintf(stderr, "Error: Could not find LCD2USB device\n");

#ifdef WIN
    printf("Press return to quit\n");
    getchar();
#endif
    exit(-1);
  }

  /* make lcd interface return some bytes to */
  /* test transfer reliability */
  lcd_echo();
  /* read some values from adaptor */
  lcd_get_version();
  lcd_get_controller();
  lcd_get_keys();
  parseArguments(&arguments);
  usb_close(handle);

#ifdef WIN
  printf("Press return to quit\n");
  getchar();
#endif

  return 0;
}
