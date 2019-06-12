#include <fstream>
#include <cstring>
#include "TxtImage.h"

namespace
{
  /* varables for calculate_row_count */
  long int    row_count;
  const char* end_of_txt_image_row_address;
}

extern const char* const TxtImage::PLAYER_CHARACTER_FILE_PATH = "./txt_images/player_character.txt";
extern const char* const TxtImage::STAR_FILE_PATH             = "./txt_images/star.txt";
extern const char* const TxtImage::BALL2_FILE_PATH            = "./txt_images/ball_2.txt";
extern const char* const TxtImage::TNT_FILE_PATH              = "./txt_images/tnt.txt";
extern const char* const TxtImage::TNT_EXPLOSION_FILE_PATH    = "./txt_images/tnt_explosion.txt";

const char* TxtImage::load(const char* const file_path)
{
  std::ifstream file_stream(file_path);
  std::string tmp_file_str((std::istreambuf_iterator<char>(file_stream)), std::istreambuf_iterator<char>());
  file_stream.close();

  char* txt_image = new char[tmp_file_str.length() + 1];
  std::strcpy(txt_image, tmp_file_str.c_str());

  return txt_image;
}

long int TxtImage::calculate_row_count(const char* const txt_image)
{
  row_count = 0;
  end_of_txt_image_row_address = txt_image;

  while ( end_of_txt_image_row_address = strchr(end_of_txt_image_row_address + 1,'\n') ) { ++row_count; }
  return row_count;
}
