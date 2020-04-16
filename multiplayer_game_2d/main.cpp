#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <string.h>
#include <windows.h>
#include "tile_map.h"
#include "gameplay_entities.h"


#pragma warning(disable : 26812)


#define TILE_MAP_WIDTH              16  // in pixels
#define TILE_MAP_HEIGHT             9   // in pixels
#define TILE_MAP_TEXTURE_SIDE_SIZE  64  // in pixels
#define MAX_COLLISIONS_PER_TILE     9   // potential game objects (collisions) in an single tile



namespace game_entity_ids_per_tile
{
  /* @remember: all collision rectangles at max are tile-width */

  int current_tile_index;
  int current_tile_bucket_index;
  int current_gameplay_object_id;
  int current_collision_vertex;
  int y_index;
  int x_index;
  int current_max_tile_bucket_index_limit;

  int tile_buckets[MAX_COLLISIONS_PER_TILE * TILE_MAP_WIDTH * TILE_MAP_HEIGHT]; // MAX_COLLISIONS_PER_TILE game_entity_ids per tile


  inline void update(const tile_map<TILE_MAP_WIDTH,TILE_MAP_HEIGHT>& p_tile_map, const gameplay_entities<TILE_MAP_WIDTH * TILE_MAP_HEIGHT>& p_game_entities)
  {
    memset(tile_buckets, -1, sizeof(tile_buckets));

    for(current_collision_vertex=0; current_collision_vertex < p_game_entities.vertex_count; ++current_collision_vertex) // vertex_count is same as collision_vertex_count
    {
      current_gameplay_object_id = current_collision_vertex / 4;
      if (p_game_entities.is_garbage_flags[current_gameplay_object_id]) continue;

      y_index = static_cast<int>(p_game_entities.collision_vertices[current_collision_vertex].y / p_tile_map.tile_size_y);
      x_index = static_cast<int>(p_game_entities.collision_vertices[current_collision_vertex].x / p_tile_map.tile_size_x);

      // check if vertex is visible
      if(   (y_index < 0) 
         || (y_index > (p_tile_map.height - 1)) 
         || (x_index < 0)
         || (x_index > (p_tile_map.width - 1))
        ) continue;


      current_tile_index = (y_index * p_tile_map.width) + x_index;
      current_tile_bucket_index = current_tile_index * MAX_COLLISIONS_PER_TILE;
      current_max_tile_bucket_index_limit = current_tile_bucket_index + MAX_COLLISIONS_PER_TILE;

      // find open tile_bucket for current_tile_index
      for(; ( current_tile_bucket_index < current_max_tile_bucket_index_limit )
                        && (tile_buckets[current_tile_bucket_index] != -1)
                        && (tile_buckets[current_tile_bucket_index] != current_gameplay_object_id); ++current_tile_bucket_index) {};


      tile_buckets[current_tile_bucket_index] = current_gameplay_object_id;
    }
  }

  #ifdef _DEBUG
    inline void print_tile_buckets()
    {
      std::cout << "\n";
      for(int i=0; i < MAX_COLLISIONS_PER_TILE * TILE_MAP_WIDTH * TILE_MAP_HEIGHT; i+= MAX_COLLISIONS_PER_TILE)
      {
        std::cout << "Tile index: " << i/MAX_COLLISIONS_PER_TILE  << std::endl;

        for(int collision_index=0; collision_index < MAX_COLLISIONS_PER_TILE; ++collision_index)
        {
          int entity_id = tile_buckets[i + collision_index];
          if (entity_id == -1) continue;
          std::cout << "\tid: " << entity_id << std::endl;
        }
      }
      std::cout << "\n";
    }
  #endif

} // game_entity_ids_per_tile



int main()
{
  /* create window */
  sf::VideoMode desktop_video_mode = sf::VideoMode::getDesktopMode();
  sf::RenderWindow window(desktop_video_mode, "2D Multiplayer Game", sf::Style::Fullscreen);
  //window.setVerticalSyncEnabled(true);
  window.setActive(true);
  sf::Vector2u window_size = window.getSize();

  #ifdef _DEBUG
    bool show_debug_data = true;
    sf::Font mandalore_font;
    mandalore_font.loadFromFile("Assets/Fonts/mandalore.ttf");
  #endif

  sf::SoundBuffer tingling_sound_buffer;
  tingling_sound_buffer.loadFromFile("Assets/Sounds/tingling.wav");
  sf::Sound tingling;
  tingling.setBuffer(tingling_sound_buffer);
  
  tile_map<TILE_MAP_WIDTH,TILE_MAP_HEIGHT>* test_map_background = new tile_map<TILE_MAP_WIDTH,TILE_MAP_HEIGHT>("Assets/Images/test_map_background.png", (float) window_size.x, (float) window_size.y, TILE_MAP_TEXTURE_SIDE_SIZE);
  gameplay_entities<TILE_MAP_WIDTH * TILE_MAP_HEIGHT>* game_entities = new gameplay_entities<TILE_MAP_WIDTH * TILE_MAP_HEIGHT>("Assets/Images/gameplay_entities.png", TILE_MAP_TEXTURE_SIDE_SIZE * 3); // need to be able to handle a single gameplay entity per tile

  
  game_entities->is_garbage_flags[0] = false;
  game_entities->is_garbage_flags[1] = false;
  game_entities->is_garbage_flags[2] = false;
  game_entities->is_garbage_flags[3] = false;
  game_entities->is_garbage_flags[4] = false;
  game_entities->types[0] = gameplay_entities_type_and_sprite_sheet_row_index::MARIO;
  game_entities->types[1] = gameplay_entities_type_and_sprite_sheet_row_index::BOMB;
  game_entities->types[2] = gameplay_entities_type_and_sprite_sheet_row_index::BOMB;
  game_entities->types[3] = gameplay_entities_type_and_sprite_sheet_row_index::BOMB;
  game_entities->types[4] = gameplay_entities_type_and_sprite_sheet_row_index::BOMB;
  game_entities->animation_indexes[0] = 0;
  game_entities->animation_indexes[1] = 0;
  game_entities->animation_indexes[2] = 0;
  game_entities->velocities[0] = sf::Vector2f(0.0f, 0.0f);
  game_entities->velocities[1] = sf::Vector2f(128.0f,0.0f);
  game_entities->velocities[2] = sf::Vector2f(16.0f,32.0f);
  game_entities->velocities[3] = sf::Vector2f(100.0f,83.0f);
  game_entities->velocities[4] = sf::Vector2f(-25.0f,-100.0f);

  // initialize entity positions to (0,0) origin
  for(int i=0; i < game_entities->vertex_count; i+=4)
  {
    game_entities->vertex_buffer[i].position   = sf::Vector2f(-test_map_background->tile_size_x, -test_map_background->tile_size_y * 2);
    game_entities->vertex_buffer[i+1].position = sf::Vector2f(2 *test_map_background->tile_size_x, -test_map_background->tile_size_y * 2);
    game_entities->vertex_buffer[i+2].position = sf::Vector2f(test_map_background->tile_size_x * 2, test_map_background->tile_size_y);
    game_entities->vertex_buffer[i+3].position = sf::Vector2f(-test_map_background->tile_size_x, test_map_background->tile_size_y);
  }

  // initialize default collision rectangles
  for(int i=0; i < game_entities->vertex_count; i+=4)
  {
    // the tile_size - 0.01f is to currently handle overlapping tile vertices
    game_entities->collision_vertices[i]   = sf::Vector2f(0.0f, 0.0f);
    game_entities->collision_vertices[i+1] = sf::Vector2f(test_map_background->tile_size_x - 0.01f, 0.0f);
    game_entities->collision_vertices[i+2] = sf::Vector2f(test_map_background->tile_size_x - 0.01f, test_map_background->tile_size_y - 0.01f);
    game_entities->collision_vertices[i+3] = sf::Vector2f(0.0f, test_map_background->tile_size_y - 0.01f);
  }



  /* setup and run game loop */
  sf::Event window_event;
  sf::Clock clock;
  sf::Time  elapsed_frame_time;
  sf::Int32 elapsed_frame_time_milliseconds;
  sf::Int64 elapsed_frame_time_microseconds;
  float     elapsed_frame_time_seconds;

  while (window.isOpen())
  {
    // determine framerate
    elapsed_frame_time = clock.restart();
    elapsed_frame_time_milliseconds = elapsed_frame_time.asMilliseconds();
    elapsed_frame_time_microseconds = elapsed_frame_time.asMicroseconds();
    elapsed_frame_time_seconds      = elapsed_frame_time.asSeconds();

    #ifdef _DEBUG
      if (elapsed_frame_time_milliseconds > 16)
        std::cout << "elapsed_frame_time_milliseconds: " << elapsed_frame_time_milliseconds << std::endl;
    #endif



    /* get input and events */
    while (window.pollEvent(window_event))
    {
      switch (window_event.type)
      {
        case sf::Event::Closed:
              window.close();
              break;

        case sf::Event::KeyPressed:
              if (window_event.key.code == sf::Keyboard::Left)   game_entities->velocities[0] = sf::Vector2f( (float) -2 * test_map_background->tile_size_x, 0.0f );
              if (window_event.key.code == sf::Keyboard::Right)  game_entities->velocities[0] = sf::Vector2f( (float) 2 * test_map_background->tile_size_x, 0.0f  );
              if (window_event.key.code == sf::Keyboard::Up)     game_entities->velocities[0] = sf::Vector2f( 0.0f, (float) -2 * test_map_background->tile_size_y );
              if (window_event.key.code == sf::Keyboard::Down)   game_entities->velocities[0] = sf::Vector2f( 0.0f, (float)  2 * test_map_background->tile_size_y );
              if (window_event.key.code == sf::Keyboard::T)      tingling.play();

              if (window_event.key.code == sf::Keyboard::P)      // use for testing random stuff
                                                                 game_entities->animation_indexes[1] = (game_entities->animation_indexes[1] + 1) % 3;
              break;

        case sf::Event::KeyReleased:
              if (window_event.key.code == sf::Keyboard::Left)   game_entities->velocities[0] = sf::Vector2f(0.0f, 0.0f);
              if (window_event.key.code == sf::Keyboard::Right)  game_entities->velocities[0] = sf::Vector2f(0.0f, 0.0f);
              if (window_event.key.code == sf::Keyboard::Up)     game_entities->velocities[0] = sf::Vector2f(0.0f, 0.0f);
              if (window_event.key.code == sf::Keyboard::Down)   game_entities->velocities[0] = sf::Vector2f(0.0f, 0.0f);

              #ifdef _DEBUG
                if ( window_event.key.code == sf::Keyboard::D ) show_debug_data = !show_debug_data;
              #endif

              break;

        default:
              break;
      }
    }



    /* calculate gameplay stuff */
    test_map_background->bitmap[0] = 0;
    test_map_background->bitmap[1] = 1;
    test_map_background->bitmap[2] = 2;
    test_map_background->bitmap[3] = 3;
    test_map_background->bitmap[4] = 3;
    test_map_background->bitmap[5] = 2;
    test_map_background->bitmap[6] = 1;
    test_map_background->bitmap[7] = 0;
    test_map_background->bitmap[15] = 2;
    test_map_background->bitmap[143] = 2;
    test_map_background->update_tex_coords_from_bitmap();

    game_entities->update_positions_and_tex_coords(elapsed_frame_time_seconds);

    game_entity_ids_per_tile::update(*test_map_background, *game_entities);

    /* draw */
    window.clear(sf::Color::Black);

    window.draw(test_map_background->vertex_buffer, test_map_background->vertex_count, sf::Quads, &test_map_background->tiles_texture);
    window.draw(game_entities->vertex_buffer, game_entities->vertex_count, sf::Quads, &game_entities->sprite_sheet_texture);

    #ifdef _DEBUG
      if(show_debug_data)
      {
        window.draw( *(test_map_background->generate_debug_line_vertices(sf::Color::Blue))    );
        window.draw( *(game_entities->generate_debug_collision_line_vertices(sf::Color::Red)) );
        //window.draw( *(game_entities->generate_debug_line_vertices(sf::Color::Yellow))        );

        static sf::Text tile_index_text[TILE_MAP_WIDTH * TILE_MAP_HEIGHT];
        test_map_background->generate_debug_tile_index_text(tile_index_text, mandalore_font, sf::Color::Blue);
        for(auto& text : tile_index_text) window.draw(text);

        static sf::Text game_entity_index_text[TILE_MAP_WIDTH * TILE_MAP_HEIGHT];
        game_entities->generate_debug_index_text(game_entity_index_text, mandalore_font, sf::Color::Yellow);
        for(auto& text : game_entity_index_text) window.draw(text);
      }
    #endif

    // draw HUD (if decided to have static HUD)
    // draw options if requested

    window.display();


    // handle underflow
    if (elapsed_frame_time_seconds == 0.0f) Sleep(1);
  } // end of game loop

  return 0;
}
