#include <algorithm>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <curses.h>
#include <stdlib.h>
#include <time.h>
#include <chrono>
#include <string.h>
#include "Frame.h"
#include "TxtImage.h"
#include "Collision.h"


namespace
{
  int stars_index;
  int tnts_index;
}


int main()
{
  if (Frame::COLUMN_COUNT < 25 || Frame::ROW_COUNT < 25)
  {
    puts("terminal window size must be larger");
    return 0;
  }


  srand(time(NULL));


  /* setup ncurses for input and terminal settings */
  initscr();            // start ncurses
  cbreak();             // don't require carriage returns for reading input
  noecho();             // hide user input
  nodelay(stdscr,TRUE); // don't wait for user input
  keypad(stdscr, TRUE); // make arrow keys readable
  curs_set(0);          // hide the cursor


  const int PLAYER_COUNT           = 1;
  const int TNT_COUNT              = Frame::COLUMN_COUNT / 5;
  const int STAR_COUNT             = TNT_COUNT / 4;
  const int FALLING_ENTITIES_COUNT = TNT_COUNT + STAR_COUNT;

  Frame::Entity  all_frame_entities[PLAYER_COUNT + TNT_COUNT + STAR_COUNT];
  Frame::Entity& player                 = all_frame_entities[0];
  Frame::Entity* const falling_entities = all_frame_entities + PLAYER_COUNT;
  Frame::Entity* const tnts             = all_frame_entities + PLAYER_COUNT;
  Frame::Entity* const stars            = all_frame_entities + PLAYER_COUNT + TNT_COUNT;


  player.txt_image                            = TxtImage::load(TxtImage::PLAYER_CHARACTER_FILE_PATH);
  const long int PLAYER_TXT_IMAGE_ROW_COUNT   = TxtImage::calculate_row_count(player.txt_image);
  player.buffer_index                         = (Frame::COLUMN_COUNT / 2) + ( Frame::COLUMN_COUNT * (Frame::ROW_COUNT - 10) );  // player starting position is in the middle of the last 10 rows of the frame
  const long int player_starting_buffer_index = player.buffer_index;
  const long int PLAYER_CHAR_MOVEMENT_SIZE    = 6;  // number of chars the player moves on key press


  const char* const TNT_TXT_IMAGE         = TxtImage::load(TxtImage::TNT_FILE_PATH);
  const long int TNT_TXT_IMAGE_ROW_COUNT  = TxtImage::calculate_row_count(TNT_TXT_IMAGE);
  const char* TNT_EXPLOSION_TXT_IMAGE     = TxtImage::load(TxtImage::TNT_EXPLOSION_FILE_PATH);
  const char* const STAR_TXT_IMAGE        = TxtImage::load(TxtImage::STAR_FILE_PATH);
  const long int STAR_TXT_IMAGE_ROW_COUNT = TxtImage::calculate_row_count(STAR_TXT_IMAGE);

  for(int i=0; i < TNT_COUNT; ++i)  tnts[i].txt_image  = TNT_TXT_IMAGE;
  for(int i=0; i < STAR_COUNT; ++i) stars[i].txt_image = STAR_TXT_IMAGE;


  const int MAX_UPWARD_FALLING_ENTITY_SPAWN_OFFSET = Frame::ROW_COUNT * 5; // the max number of characters to move falling entites left (up the frame) when they spawn and reset positions

  for (int i=0; i < FALLING_ENTITIES_COUNT; ++i)
  {
    falling_entities[i].buffer_index           = (rand() % Frame::COLUMN_COUNT) - ( (rand() % MAX_UPWARD_FALLING_ENTITY_SPAWN_OFFSET) * Frame::COLUMN_COUNT );  // choose random index in first row then move back MAX_UPWARD_FALLING_ENTITY_SPAWN_OFFSET chars
    falling_entities[i].buffer_index_remainder = 0.0;
    falling_entities[i].velocity               = (rand() % 30) + 10;
  }


  const int TOTAL_PLAYER_COLLISION_LINE_COUNT = PLAYER_COUNT * PLAYER_TXT_IMAGE_ROW_COUNT;
  const int TOTAL_TNT_COLLISION_LINE_COUNT    = TNT_COUNT * TNT_TXT_IMAGE_ROW_COUNT;
  const int TOTAL_STAR_COLLISION_LINE_COUNT   = STAR_COUNT * STAR_TXT_IMAGE_ROW_COUNT;

  Collision::Line  all_collision_lines[TOTAL_PLAYER_COLLISION_LINE_COUNT + TOTAL_TNT_COLLISION_LINE_COUNT + TOTAL_STAR_COLLISION_LINE_COUNT];
  Collision::Line* const player_collision_lines           = all_collision_lines;
  Collision::Line* const tnt_collision_lines              = all_collision_lines + TOTAL_PLAYER_COLLISION_LINE_COUNT;
  Collision::Line* const falling_entities_collision_lines = all_collision_lines + TOTAL_PLAYER_COLLISION_LINE_COUNT;
  Collision::Line* const star_collision_lines             = all_collision_lines + TOTAL_PLAYER_COLLISION_LINE_COUNT + TOTAL_TNT_COLLISION_LINE_COUNT;

  Collision::set(player_collision_lines, player.txt_image, player.buffer_index);
  player.collision_lines = player_collision_lines;

  for (int i=0; i < TNT_COUNT; ++i)
  {
    Collision::set(tnt_collision_lines + (i * TNT_TXT_IMAGE_ROW_COUNT), TNT_TXT_IMAGE, tnts[i].buffer_index);
    tnts[i].collision_lines = tnt_collision_lines + (i * TNT_TXT_IMAGE_ROW_COUNT);
  }

  for (int i=0; i < STAR_COUNT; ++i) 
  {
    Collision::set(star_collision_lines + (i * STAR_TXT_IMAGE_ROW_COUNT), STAR_TXT_IMAGE, stars[i].buffer_index);
    stars[i].collision_lines = ( star_collision_lines + (i * STAR_TXT_IMAGE_ROW_COUNT) );
  }


  // set player_width as length of largest collision line
  int player_width = 0;
  for(int i=0, collision_line_size = 0; i < PLAYER_TXT_IMAGE_ROW_COUNT; ++i)
  {
    collision_line_size = player_collision_lines[i].end - player_collision_lines[i].start;
    if (collision_line_size > player_width) player_width = collision_line_size;
  }

  const long int PLAYER_LEFT_MOVEMENT_BOUNDARY  = player.buffer_index - (Frame::COLUMN_COUNT / 2) + (player_width / 2);
  const long int PLAYER_RIGHT_MOVEMENT_BOUNDARY = player.buffer_index + (Frame::COLUMN_COUNT / 2) - 4 - (player_width / 2);


  int current_score = 0;
  int high_score;
  std::ifstream input_saved_data_stream("saved_data/high_score", std::ios::binary);
  input_saved_data_stream.read(reinterpret_cast<char*>(&high_score), sizeof(int));
  input_saved_data_stream.close();

  const long int CURRENT_SCORE_TXT_FRAME_BUFFER_INDEX = (Frame::COLUMN_COUNT * 5) + Frame::COLUMN_COUNT - 20;
  const long int HIGH_SCORE_TXT_FRAME_BUFFER_INDEX    = (Frame::COLUMN_COUNT * 6) + Frame::COLUMN_COUNT - 25;
  char current_score_to_str_buffer[5];
  char high_score_to_str_buffer[4];


  auto game_loop_start_time                  = std::chrono::high_resolution_clock::now();
  auto game_loop_end_time                    = std::chrono::high_resolution_clock::now();
  long int game_loop_time_delta_microseconds = 0;
  double game_loop_time_delta_seconds        = 0.0;
  long int game_loop_delay;


  /* game loop */
  while(true)
  { 
    game_loop_start_time = std::chrono::high_resolution_clock::now();

    
    switch ( getch() )
    {
      case KEY_LEFT  : if (player.buffer_index <= PLAYER_LEFT_MOVEMENT_BOUNDARY) { player.buffer_index = PLAYER_LEFT_MOVEMENT_BOUNDARY; break; }
                       player.buffer_index -= PLAYER_CHAR_MOVEMENT_SIZE;
                       Collision::move(player_collision_lines, PLAYER_TXT_IMAGE_ROW_COUNT, -1 * PLAYER_CHAR_MOVEMENT_SIZE);
                       break;

      case KEY_RIGHT : if (player.buffer_index >= PLAYER_RIGHT_MOVEMENT_BOUNDARY) { player.buffer_index = PLAYER_RIGHT_MOVEMENT_BOUNDARY; break; }
                       player.buffer_index += PLAYER_CHAR_MOVEMENT_SIZE;
                       Collision::move(player_collision_lines, PLAYER_TXT_IMAGE_ROW_COUNT, PLAYER_CHAR_MOVEMENT_SIZE);
                       break;

      case 'q'       : goto exit_game;
    }


    for(int i=0; i < FALLING_ENTITIES_COUNT; ++i)
    {
      falling_entities[i].buffer_index += Frame::COLUMN_COUNT * (int) ( falling_entities[i].buffer_index_remainder += falling_entities[i].velocity * game_loop_time_delta_seconds );  // when the remainder is 0.x and casted to an int it will become zero and when finally reaches 1 or more the falling entity will move down a row
      if (falling_entities[i].buffer_index_remainder > 1.0) falling_entities[i].buffer_index_remainder = 0.0;

      if (falling_entities[i].buffer_index > Frame::CHAR_COUNT)
      {
        falling_entities[i].buffer_index = (rand() % Frame::COLUMN_COUNT) - ( (rand() % MAX_UPWARD_FALLING_ENTITY_SPAWN_OFFSET) * Frame::COLUMN_COUNT );  // choose random index in first row then move back MAX_UPWARD_FALLING_ENTITY_SPAWN_OFFSET chars
        falling_entities[i].velocity     = (rand() % 30) + 10;
        if (i < TNT_COUNT) falling_entities[i].txt_image = TNT_TXT_IMAGE;
      }

      Collision::set(falling_entities[i].collision_lines, falling_entities[i].txt_image, falling_entities[i].buffer_index); 
    }

    for(int i=0; i < TOTAL_TNT_COLLISION_LINE_COUNT; ++i)
    for(int j=0; j < PLAYER_TXT_IMAGE_ROW_COUNT; ++j)
    {
      if ( Collision::is_overlap(player_collision_lines[j], tnt_collision_lines[i]) )
      {
        player.buffer_index = player_starting_buffer_index;
        Collision::set(player_collision_lines, player.txt_image, player.buffer_index);
        current_score = 0;

        tnts_index = i / TNT_TXT_IMAGE_ROW_COUNT;
        tnts[tnts_index].txt_image = TNT_EXPLOSION_TXT_IMAGE;
      }
    }

    for(int i=0; i < TOTAL_STAR_COLLISION_LINE_COUNT; ++i)
    for(int j=0; j < PLAYER_TXT_IMAGE_ROW_COUNT; ++j)
    {
      if ( Collision::is_overlap(player.collision_lines[j], star_collision_lines[i]) )
      {
        ++current_score;
        if(current_score > high_score) high_score = current_score;
        stars_index = i / STAR_TXT_IMAGE_ROW_COUNT;
        stars[stars_index].buffer_index = (rand() % Frame::COLUMN_COUNT) - ( (rand() % MAX_UPWARD_FALLING_ENTITY_SPAWN_OFFSET) * Frame::COLUMN_COUNT ); // choose random index in first row then move back MAX_UPWARD_FALLING_ENTITY_SPAWN_OFFSET chars
        Collision::set(stars[stars_index].collision_lines, stars[stars_index].txt_image, stars[stars_index].buffer_index);
      }
    }


    memset(Frame::buffer, ' ', Frame::CHAR_COUNT);

    for(auto& frame_entity : all_frame_entities) Frame::draw(frame_entity.txt_image, frame_entity.buffer_index);

    memcpy(Frame::buffer + CURRENT_SCORE_TXT_FRAME_BUFFER_INDEX, "SCORE: ", 7);
    sprintf(current_score_to_str_buffer,"%d", current_score);
    memcpy(Frame::buffer + CURRENT_SCORE_TXT_FRAME_BUFFER_INDEX + 7, current_score_to_str_buffer, strlen(current_score_to_str_buffer));

    memcpy(Frame::buffer + HIGH_SCORE_TXT_FRAME_BUFFER_INDEX, "HIGH SCORE: ", 12);
    sprintf(high_score_to_str_buffer,"%d", high_score);
    memcpy(Frame::buffer + HIGH_SCORE_TXT_FRAME_BUFFER_INDEX + 12, high_score_to_str_buffer, strlen(high_score_to_str_buffer));

    for(int i = Frame::COLUMN_COUNT - Frame::NEW_LINE_CARRIAGE_RETURN_SIZE; i < Frame::CHAR_COUNT; i += Frame::COLUMN_COUNT)
    {
      Frame::buffer[i]   = '\n';
      Frame::buffer[i+1] = '\r';
    }

    Frame::buffer[Frame::CHAR_COUNT] = '\0';

    std::cout << Frame::buffer;

    /* game_loop delay */
    game_loop_end_time                = std::chrono::high_resolution_clock::now();
    game_loop_time_delta_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(game_loop_end_time - game_loop_start_time).count();
    game_loop_delay                   = Frame::RATE_TARGET_MICROSECONDS - game_loop_time_delta_microseconds;

    if (game_loop_delay > 0) usleep(Frame::RATE_TARGET_MICROSECONDS - game_loop_time_delta_microseconds);

    game_loop_end_time                = std::chrono::high_resolution_clock::now();
    game_loop_time_delta_microseconds = std::chrono::duration_cast<std::chrono::microseconds>(game_loop_end_time - game_loop_start_time).count();
    game_loop_time_delta_seconds      = game_loop_time_delta_microseconds / 1000000.0f;
  };

  exit_game: ;

  std::ofstream output_saved_data_stream("saved_data/high_score", std::ios::binary);
  output_saved_data_stream.write(reinterpret_cast<const char*>(&high_score), sizeof(int));
  output_saved_data_stream.close();

  endwin(); // reset terminal settings

  return 0;
}
