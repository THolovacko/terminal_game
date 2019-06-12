#pragma once

namespace Collision { struct Line; }

namespace Frame
{
  extern const long int COLUMN_COUNT;                   // 2 of the columns will be used for \n\r
  extern const long int ROW_COUNT;                       
  extern const long int CHAR_COUNT;                      
  extern const long int NEW_LINE_CARRIAGE_RETURN_SIZE;  // number of chars needed for \n\r
  extern const long int RATE_TARGET_MICROSECONDS;
  extern char*          buffer;

  struct Entity
  {
    long int         buffer_index;
    double           buffer_index_remainder;  // used to handle frame-rate independence (used to handle moving fractions of a char)
    double           velocity;                // chars moved per unit of time (most likely seconds)
    const char*      txt_image;
    Collision::Line* collision_lines;
  };

  void draw(const char* const txt_image, const long buffer_index);
  struct winsize get_terminal_sizes();
}
