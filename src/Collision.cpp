#include <cstring>
#include "Frame.h"
#include "Collision.h"

namespace
{
  /* variables for Collision::set */
  long int    frame_buffer_row_index;
  long int    txt_image_middle_char_index;
  long int    txt_image_middle_char_index_offset;
  long int    txt_image_index;
  long int    txt_image_row_index;
  long int    txt_image_row_size;
  const char* end_of_text_image_row;
}

bool Collision::is_overlap(const Collision::Line& line1, const Collision::Line& line2)
{
  return ( (line1.start >= line2.start) && (line1.start <= line2.end) ) ||
         ( (line1.end   >= line2.start) && (line1.end   <= line2.end) ) ||
         ( (line1.start <= line2.start) && (line1.end   >= line2.end) );
}

void Collision::move(Collision::Line* const lines_array, const long int line_count, const long int distance)
{
  for(int i=0; i < line_count; ++i)
  {
    lines_array[i].start += distance;
    lines_array[i].end   += distance;
  }
}

void Collision::set(Collision::Line* const lines_array, const char* const txt_image, const long buffer_index)
{
  frame_buffer_row_index             = buffer_index / Frame::COLUMN_COUNT;
  txt_image_middle_char_index        = ( strchr(txt_image,'\n') - txt_image ) / 2;
  txt_image_middle_char_index_offset = txt_image_middle_char_index;

  // set the collision lines as the start and end points of each row of the txt_image
  for(txt_image_index = 0, txt_image_row_index = 0; txt_image_index < strlen(txt_image); txt_image_index += (txt_image_row_size + 1), txt_image_middle_char_index = txt_image_index + txt_image_middle_char_index_offset)
  {
    end_of_text_image_row = strchr(txt_image + txt_image_index,'\n');
    txt_image_row_size = ( end_of_text_image_row - (txt_image + txt_image_index) );

    lines_array[txt_image_row_index].start = ( buffer_index + (Frame::COLUMN_COUNT * txt_image_row_index) ) + (txt_image_index - txt_image_middle_char_index);
    lines_array[txt_image_row_index].end   = lines_array[txt_image_row_index].start + (txt_image_row_size - 1);

    ++frame_buffer_row_index;
    ++txt_image_row_index;
  }
}
