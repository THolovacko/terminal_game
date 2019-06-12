#include <unistd.h>
#include <sys/ioctl.h>
#include <cstring>
#include "Frame.h"

namespace
{
  /* variables used for draw */
  long int buffer_row_index;
  long int buffer_mapped_index;
  long int txt_image_index;
  long int txt_image_row_index;
  long int txt_image_middle_char_index;
  long int txt_image_middle_char_index_offset;
}

extern const long int Frame::COLUMN_COUNT                  = get_terminal_sizes().ws_col;
extern const long int Frame::ROW_COUNT                     = get_terminal_sizes().ws_row;
extern const long int Frame::CHAR_COUNT                    = ROW_COUNT * COLUMN_COUNT;
extern const long int Frame::NEW_LINE_CARRIAGE_RETURN_SIZE = 2;
extern const long int Frame::RATE_TARGET_MICROSECONDS      = 36000; //12000;
char*                 Frame::buffer                        = new char[Frame::CHAR_COUNT];

winsize Frame::get_terminal_sizes()
{
  winsize terminal_sizes;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &terminal_sizes);
  return terminal_sizes;
}

// map each char of the txt_image to where it needs to be in the Frame::buffer
void Frame::draw(const char* const txt_image, const long buffer_index)
{
  buffer_row_index                   = buffer_index / Frame::COLUMN_COUNT;
  txt_image_middle_char_index        = (strchr(txt_image,'\n') - txt_image) / 2;
  txt_image_middle_char_index_offset = txt_image_middle_char_index;

  for(txt_image_index = 0, txt_image_row_index = 0; txt_image_index < strlen(txt_image); ++txt_image_index)
  {
    // currently at end of current txt_image row
    if (txt_image[txt_image_index] == '\n')
    {
      txt_image_middle_char_index = (txt_image_index + 1) + txt_image_middle_char_index_offset;
      ++buffer_row_index;
      ++txt_image_row_index;
      continue;
    }

    /* map current txt_image char */
    buffer_mapped_index = ( buffer_index + (Frame::COLUMN_COUNT * txt_image_row_index) ) + (txt_image_index - txt_image_middle_char_index);

    if ( (buffer_mapped_index > 0) && (buffer_mapped_index < Frame::CHAR_COUNT) )
    {
      Frame::buffer[buffer_mapped_index] = txt_image[txt_image_index];
    }
  }
}


