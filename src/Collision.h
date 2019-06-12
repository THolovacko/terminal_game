#pragma once

namespace Collision
{
  /* Collision::Lines are 2 points on the same Frame::buffer row */
  struct Line
  {
    long int start;
    long int end;
  };
  
  bool is_overlap(const Collision::Line& line1, const Collision::Line& line2);
  void move(Collision::Line* const lines_array, const long int line_count, const long int distance);
  void set(Collision::Line* const lines_array, const char* const txt_image, const long buffer_index);
}
