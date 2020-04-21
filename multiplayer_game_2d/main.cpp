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



namespace gameplay_entity_ids_per_tile
{
  /* @remember: all hitboxes at max are tile-width */

  int current_tile_index;
  int current_tile_bucket_index;
  int current_gameplay_entity_id;
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
      current_gameplay_entity_id = current_collision_vertex / 4;
      if (p_game_entities.is_garbage_flags[current_gameplay_entity_id]) continue;

      y_index = static_cast<int>(p_game_entities.collision_vertices[current_collision_vertex].y / p_tile_map.tile_size_y);
      x_index = static_cast<int>(p_game_entities.collision_vertices[current_collision_vertex].x / p_tile_map.tile_size_x);

      // check if vertex is visible
      if(   (y_index < 0) 
         || (y_index > (p_tile_map.height - 1)) 
         || (x_index < 0)
         || (x_index > (p_tile_map.width  - 1))
        ) continue;

      current_tile_index = (y_index * p_tile_map.width) + x_index;
      current_tile_bucket_index = current_tile_index * MAX_COLLISIONS_PER_TILE;
      current_max_tile_bucket_index_limit = current_tile_bucket_index + MAX_COLLISIONS_PER_TILE;

      // find open tile_bucket for current_tile_index
      for(; ( current_tile_bucket_index < current_max_tile_bucket_index_limit )
                        && (tile_buckets[current_tile_bucket_index] != -1)
                        && (tile_buckets[current_tile_bucket_index] != current_gameplay_entity_id); ++current_tile_bucket_index) {};

      tile_buckets[current_tile_bucket_index] = current_gameplay_entity_id;
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

} // gameplay_entity_ids_per_tile



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
  
  tile_map<TILE_MAP_WIDTH,TILE_MAP_HEIGHT>* test_tile_map = new tile_map<TILE_MAP_WIDTH,TILE_MAP_HEIGHT>("Assets/Images/test_tile_map.png", (float) window_size.x, (float) window_size.y, TILE_MAP_TEXTURE_SIDE_SIZE);
  gameplay_entities<TILE_MAP_WIDTH * TILE_MAP_HEIGHT>* all_gameplay_entities = new gameplay_entities<TILE_MAP_WIDTH * TILE_MAP_HEIGHT>("Assets/Images/gameplay_entities.png", TILE_MAP_TEXTURE_SIDE_SIZE * 3); // need to be able to handle a single gameplay entity per tile
  gameplay_entity_ids_per_tile::update(*test_tile_map, *all_gameplay_entities); // initialize

  
  all_gameplay_entities->is_garbage_flags[0] = false;
  all_gameplay_entities->is_garbage_flags[1] = false;
  all_gameplay_entities->is_garbage_flags[2] = false;
  all_gameplay_entities->is_garbage_flags[3] = false;
  all_gameplay_entities->is_garbage_flags[4] = false;
  all_gameplay_entities->types[0] = gameplay_entity_type::MARIO;
  all_gameplay_entities->types[1] = gameplay_entity_type::BOMB;
  all_gameplay_entities->types[2] = gameplay_entity_type::BOMB;
  all_gameplay_entities->types[3] = gameplay_entity_type::BOMB;
  all_gameplay_entities->types[4] = gameplay_entity_type::BOMB;
  all_gameplay_entities->animation_indexes[0] = 0;
  all_gameplay_entities->animation_indexes[1] = 0;
  all_gameplay_entities->animation_indexes[2] = 0;
  all_gameplay_entities->velocities[0] = sf::Vector2f(0.0f  ,0.0f );
  all_gameplay_entities->velocities[1] = sf::Vector2f(128.0f,0.0f  );
  all_gameplay_entities->velocities[2] = sf::Vector2f(16.0f ,32.0f );
  all_gameplay_entities->velocities[3] = sf::Vector2f(100.0f,83.0f );
  all_gameplay_entities->velocities[4] = sf::Vector2f(25.0f ,100.0f);

  // initialize entity positions to (0,0) origin
  for(int i=0; i < all_gameplay_entities->vertex_count; i+=4)
  {
    all_gameplay_entities->vertex_buffer[i].position   = sf::Vector2f(-test_tile_map->tile_size_x, -test_tile_map->tile_size_y * 2);
    all_gameplay_entities->vertex_buffer[i+1].position = sf::Vector2f(2 * test_tile_map->tile_size_x, -test_tile_map->tile_size_y * 2);
    all_gameplay_entities->vertex_buffer[i+2].position = sf::Vector2f(test_tile_map->tile_size_x * 2, test_tile_map->tile_size_y);
    all_gameplay_entities->vertex_buffer[i+3].position = sf::Vector2f(-test_tile_map->tile_size_x, test_tile_map->tile_size_y);
  }

  // initialize default collision rectangles
  for(int i=0; i < all_gameplay_entities->vertex_count; i+=4)
  {
    // the tile_size - 0.01f is to currently handle overlapping tile vertices
    all_gameplay_entities->collision_vertices[i]   = sf::Vector2f(0.0f, 0.0f);
    all_gameplay_entities->collision_vertices[i+1] = sf::Vector2f(test_tile_map->tile_size_x - 0.01f, 0.0f);
    all_gameplay_entities->collision_vertices[i+2] = sf::Vector2f(test_tile_map->tile_size_x - 0.01f, test_tile_map->tile_size_y - 0.01f);
    all_gameplay_entities->collision_vertices[i+3] = sf::Vector2f(0.0f, test_tile_map->tile_size_y - 0.01f);
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
              if (window_event.key.code == sf::Keyboard::Left)   all_gameplay_entities->velocities[0] = sf::Vector2f( (float) -2 * test_tile_map->tile_size_x, 0.0f );
              if (window_event.key.code == sf::Keyboard::Right)  all_gameplay_entities->velocities[0] = sf::Vector2f( (float) 2 * test_tile_map->tile_size_x, 0.0f  );
              if (window_event.key.code == sf::Keyboard::Up)     all_gameplay_entities->velocities[0] = sf::Vector2f( 0.0f, (float) -2 * test_tile_map->tile_size_y );
              if (window_event.key.code == sf::Keyboard::Down)   all_gameplay_entities->velocities[0] = sf::Vector2f( 0.0f, (float)  2 * test_tile_map->tile_size_y );
              if (window_event.key.code == sf::Keyboard::T)      tingling.play();

              if (window_event.key.code == sf::Keyboard::P)      // use for testing random stuff
                                                                 all_gameplay_entities->animation_indexes[1] = (all_gameplay_entities->animation_indexes[1] + 1) % 3;
              break;

        case sf::Event::KeyReleased:
              if (window_event.key.code == sf::Keyboard::Left)   all_gameplay_entities->velocities[0] = sf::Vector2f(0.0f, 0.0f);
              if (window_event.key.code == sf::Keyboard::Right)  all_gameplay_entities->velocities[0] = sf::Vector2f(0.0f, 0.0f);
              if (window_event.key.code == sf::Keyboard::Up)     all_gameplay_entities->velocities[0] = sf::Vector2f(0.0f, 0.0f);
              if (window_event.key.code == sf::Keyboard::Down)   all_gameplay_entities->velocities[0] = sf::Vector2f(0.0f, 0.0f);

              #ifdef _DEBUG
                if ( window_event.key.code == sf::Keyboard::D ) show_debug_data = !show_debug_data;
              #endif

              break;

        default:
              break;
      }
    }



    /* calculate gameplay stuff */
    test_tile_map->bitmap[0] = static_cast<int>(test_tile_map_bitmap_type::NONE);
    test_tile_map->bitmap[1] = static_cast<int>(test_tile_map_bitmap_type::BLAH);
    test_tile_map->bitmap[2] = static_cast<int>(test_tile_map_bitmap_type::TEST);
    test_tile_map->bitmap[3] = static_cast<int>(test_tile_map_bitmap_type::TEMP);
    test_tile_map->bitmap[4] = static_cast<int>(test_tile_map_bitmap_type::TEMP);
    test_tile_map->bitmap[5] = static_cast<int>(test_tile_map_bitmap_type::TEST);
    test_tile_map->bitmap[6] = static_cast<int>(test_tile_map_bitmap_type::BLAH);
    test_tile_map->bitmap[7] = static_cast<int>(test_tile_map_bitmap_type::NONE);
    test_tile_map->bitmap[15] = static_cast<int>(test_tile_map_bitmap_type::BLAH);
    test_tile_map->bitmap[26] = static_cast<int>(test_tile_map_bitmap_type::WALL);
    test_tile_map->bitmap[45] = static_cast<int>(test_tile_map_bitmap_type::WALL);
    test_tile_map->bitmap[58] = static_cast<int>(test_tile_map_bitmap_type::WALL);
    test_tile_map->bitmap[83] = static_cast<int>(test_tile_map_bitmap_type::WALL);
    test_tile_map->bitmap[143] = static_cast<int>(test_tile_map_bitmap_type::BLAH);
    test_tile_map->update_tex_coords_from_bitmap();


    all_gameplay_entities->update_positions_by_velocity(elapsed_frame_time_seconds);
    gameplay_entity_ids_per_tile::update(*test_tile_map, *all_gameplay_entities);
      //  first, handle map boundaries and walls using tile_map bitmap by teleporting

      //  second, do collision sorting again (maybe only update buckets as needed) remember: probably still overlapping same tiles so data has barely changed in tile hash
      //  then handle gameplay_entity overlaps: set velocities, correct overlapping, and commit overlap gameplay_events (game_entities)
          //find the midpoint between the 2 entities for each x and y and reset both velocties for each entity to the new calculated values
      //  then commit tile_map trigger events ex) powerups, hearts, etc...

    // commit other gameplay events? (examples: timed bomb detonating, Q-ability activated)
    // process gameplay events?
    // update bitmap?
    // update textCoords?
    all_gameplay_entities->update_tex_coords(elapsed_frame_time_seconds);

    /* draw */
    window.clear(sf::Color::Black);

    window.draw(test_tile_map->vertex_buffer, test_tile_map->vertex_count, sf::Quads, &test_tile_map->tiles_texture);
    window.draw(all_gameplay_entities->vertex_buffer, all_gameplay_entities->vertex_count, sf::Quads, &all_gameplay_entities->sprite_sheet_texture);

    #ifdef _DEBUG
      if(show_debug_data)
      {
        window.draw( *(test_tile_map->generate_debug_line_vertices(sf::Color::Blue))    );
        window.draw( *(all_gameplay_entities->generate_debug_collision_line_vertices(sf::Color::Red)) );
        //window.draw( *(all_gameplay_entities->generate_debug_line_vertices(sf::Color::Yellow))        );

        static sf::Text tile_index_text[TILE_MAP_WIDTH * TILE_MAP_HEIGHT];
        test_tile_map->generate_debug_tile_index_text(tile_index_text, mandalore_font, sf::Color::Blue);
        for(auto& text : tile_index_text) window.draw(text);

        static sf::Text game_entity_index_text[TILE_MAP_WIDTH * TILE_MAP_HEIGHT];
        all_gameplay_entities->generate_debug_index_text(game_entity_index_text, mandalore_font, sf::Color::Yellow);
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
