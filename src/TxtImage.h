#pragma once

namespace TxtImage
{
  /* A TxtImage is just an array of chars that are loaded from a file of ASCII art */

  extern const char* const PLAYER_CHARACTER_FILE_PATH;
  extern const char* const STAR_FILE_PATH;
  extern const char* const BALL2_FILE_PATH;
  extern const char* const TNT_FILE_PATH;
  extern const char* const TNT_EXPLOSION_FILE_PATH;

  const char* load(const char* const file_path);
  long int calculate_row_count(const char* const txt_image);
}
