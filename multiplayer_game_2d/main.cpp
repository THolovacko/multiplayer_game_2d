#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <string.h>
#include <windows.h>
#include <bitset>
#include <cmath>
#include "tile_map.h"
#include "gameplay_entities.h"


#pragma warning(disable : 26812)


#define TILE_MAP_WIDTH              18
#define TILE_MAP_HEIGHT             11
#define TILE_MAP_COUNT              (TILE_MAP_WIDTH * TILE_MAP_HEIGHT)
#define MAX_GAMEPLAY_ENTITIES       TILE_MAP_COUNT
#define TILE_MAP_TEXTURE_SIDE_SIZE  64                                  // in pixels
#define MAX_COLLISIONS_PER_TILE     9                                   // potential game objects (collisions) in an single tile


namespace gameplay_entity_ids_per_tile
{
  // @remember: the second vertex in tile is not considered overlapping the first vertex in the next tile; the same goes for the 3rd vertex of a tile not sharing position of the 4th vertex of the next tile
  // @remember:     ex) if the position of a gameplay vertex is same position as top-left vertex in tile, it is considered in that tile and not also in the previous tile

  int current_tile_index;
  int current_tile_bucket_index;
  int current_gameplay_entity_id;
  int current_collision_vertex;
  int current_y_index;
  int current_x_index;
  int current_max_tile_bucket_index_limit;
  int current_potential_bucket_indexes[4];
  int current_gameplay_entity_id_bucket_index_limit;

  int tile_buckets[MAX_COLLISIONS_PER_TILE * MAX_GAMEPLAY_ENTITIES];
  std::bitset<MAX_GAMEPLAY_ENTITIES> off_map_bitfield;  // gameplay entity is partially or fully off the tile map


  inline void update(const tile_map<TILE_MAP_WIDTH,TILE_MAP_HEIGHT>& p_tile_map, const gameplay_entities<MAX_GAMEPLAY_ENTITIES>& p_game_entities, int (&p_gameplay_entity_id_to_tile_bucket_index_of_first_vertex)[MAX_GAMEPLAY_ENTITIES])
  {
    memset(tile_buckets, -1, sizeof(tile_buckets));
    memset(p_gameplay_entity_id_to_tile_bucket_index_of_first_vertex, -1, sizeof(p_gameplay_entity_id_to_tile_bucket_index_of_first_vertex));
    off_map_bitfield.reset();

    for(current_collision_vertex=0; current_collision_vertex < p_game_entities.vertex_count; ++current_collision_vertex) // vertex_count is same as collision_vertex_count
    {
      current_gameplay_entity_id = current_collision_vertex / 4;
      if (p_game_entities.is_garbage_flags[current_gameplay_entity_id]) continue;

      // check if vertex is not visible
      if(   (p_game_entities.collision_vertices[current_collision_vertex].y < 0) 
         || (p_game_entities.collision_vertices[current_collision_vertex].y > ( (p_tile_map.height * p_tile_map.tile_size_y) - 1 ))
         || (p_game_entities.collision_vertices[current_collision_vertex].x < 0)
         || (p_game_entities.collision_vertices[current_collision_vertex].x > ( (p_tile_map.width  * p_tile_map.tile_size_x) - 1 ))
        )
      {
        off_map_bitfield[current_gameplay_entity_id] = true;
        continue;
      }

      current_y_index = static_cast<int>(p_game_entities.collision_vertices[current_collision_vertex].y / p_tile_map.tile_size_y);
      current_x_index = static_cast<int>(p_game_entities.collision_vertices[current_collision_vertex].x / p_tile_map.tile_size_x);

      current_tile_index = (current_y_index * p_tile_map.width) + current_x_index;
      current_tile_bucket_index = current_tile_index * MAX_COLLISIONS_PER_TILE;
      current_max_tile_bucket_index_limit = current_tile_bucket_index + MAX_COLLISIONS_PER_TILE;

      if( (current_collision_vertex % 4) == 0 ) // checking if top-left vertex
      {
        p_gameplay_entity_id_to_tile_bucket_index_of_first_vertex[current_gameplay_entity_id] = current_tile_index;
      }

      // find open tile_bucket for current_tile_index
      for(; ( current_tile_bucket_index < current_max_tile_bucket_index_limit )
                        && (tile_buckets[current_tile_bucket_index] != -1)
                        && (tile_buckets[current_tile_bucket_index] != current_gameplay_entity_id); ++current_tile_bucket_index) {};

      tile_buckets[current_tile_bucket_index] = current_gameplay_entity_id;
    }

  }

  inline void update(const tile_map<TILE_MAP_WIDTH,TILE_MAP_HEIGHT>& p_tile_map, const gameplay_entities<MAX_GAMEPLAY_ENTITIES>& p_game_entities, int (&p_gameplay_entity_id_to_tile_bucket_index_of_first_vertex)[MAX_GAMEPLAY_ENTITIES], const int target_gameplay_entity_id, const int first_vertex_bucket_index)
  {
    current_potential_bucket_indexes[0] = first_vertex_bucket_index;
    current_potential_bucket_indexes[1] = first_vertex_bucket_index + MAX_COLLISIONS_PER_TILE;
    current_potential_bucket_indexes[2] = current_potential_bucket_indexes[1] + (MAX_COLLISIONS_PER_TILE * TILE_MAP_WIDTH);
    current_potential_bucket_indexes[3] = current_potential_bucket_indexes[2] - MAX_COLLISIONS_PER_TILE;

    for(auto& potential_vertex_bucket_index : current_potential_bucket_indexes)
    {
      // remove and re-sort target target_gameplay_entity_id in bucket index and then the other potential buckets
      if (potential_vertex_bucket_index <= ( (MAX_COLLISIONS_PER_TILE * p_tile_map.tile_count) - MAX_COLLISIONS_PER_TILE ) )
      {
        current_gameplay_entity_id_bucket_index_limit = potential_vertex_bucket_index + MAX_COLLISIONS_PER_TILE;

        // find bucket with target target_gameplay_entity_id then swap until end of bucket
        for(current_tile_bucket_index=potential_vertex_bucket_index; current_tile_bucket_index < current_gameplay_entity_id_bucket_index_limit; ++current_tile_bucket_index)
        {
          if(tile_buckets[current_tile_bucket_index] == -1) break; // the rest of the bucket is empty
     
          if(tile_buckets[current_tile_bucket_index] == target_gameplay_entity_id)
          {
            // swap until next bucket index is the current_gameplay_entity_id_bucket_index_limit then set last value in bucket as -1
            for(; (current_tile_bucket_index+1) < current_gameplay_entity_id_bucket_index_limit; ++current_tile_bucket_index)
            {
              tile_buckets[current_tile_bucket_index] = tile_buckets[current_tile_bucket_index+1];
            }
            tile_buckets[current_gameplay_entity_id_bucket_index_limit - 1] = -1; // this is always true because removing an element implies a non full bucket
            break;
          }
        }
      }
    }

    // update collision hash with new positions for target_gameplay_entity
    p_gameplay_entity_id_to_tile_bucket_index_of_first_vertex[target_gameplay_entity_id] = -1;
    off_map_bitfield[target_gameplay_entity_id] = false;

    int collision_vertex_limit = (target_gameplay_entity_id * 4) + 4;
    for(current_collision_vertex=(target_gameplay_entity_id * 4); current_collision_vertex < collision_vertex_limit; ++current_collision_vertex)
    {
      current_gameplay_entity_id = current_collision_vertex / 4;
      if (p_game_entities.is_garbage_flags[current_gameplay_entity_id]) continue;

      // check if vertex is not visible
      if(   (p_game_entities.collision_vertices[current_collision_vertex].y < 0) 
         || (p_game_entities.collision_vertices[current_collision_vertex].y > ( (p_tile_map.height * p_tile_map.tile_size_y) - 1 ))
         || (p_game_entities.collision_vertices[current_collision_vertex].x < 0)
         || (p_game_entities.collision_vertices[current_collision_vertex].x > ( (p_tile_map.width  * p_tile_map.tile_size_x) - 1 ))
        )
      {
        off_map_bitfield[current_gameplay_entity_id] = true;
        continue;
      }

      current_y_index = static_cast<int>(p_game_entities.collision_vertices[current_collision_vertex].y / p_tile_map.tile_size_y);
      current_x_index = static_cast<int>(p_game_entities.collision_vertices[current_collision_vertex].x / p_tile_map.tile_size_x);

      current_tile_index = (current_y_index * p_tile_map.width) + current_x_index;
      current_tile_bucket_index = current_tile_index * MAX_COLLISIONS_PER_TILE;
      current_max_tile_bucket_index_limit = current_tile_bucket_index + MAX_COLLISIONS_PER_TILE;

      if( (current_collision_vertex % 4) == 0 ) // checking if top-left vertex
      {
        p_gameplay_entity_id_to_tile_bucket_index_of_first_vertex[current_gameplay_entity_id] = current_tile_index;
      }

      // find open tile_bucket for current_tile_index
      for(; ( current_tile_bucket_index < current_max_tile_bucket_index_limit )
                        && (tile_buckets[current_tile_bucket_index] != -1)
                        && (tile_buckets[current_tile_bucket_index] != current_gameplay_entity_id); ++current_tile_bucket_index) {};

      tile_buckets[current_tile_bucket_index] = current_gameplay_entity_id;
    }
  } // update with gameplay_id

  #ifdef _DEBUG
    inline void print_tile_buckets()
    {
      std::cout << "\n";
      for(int i=0; i < MAX_COLLISIONS_PER_TILE * MAX_GAMEPLAY_ENTITIES; i+= MAX_COLLISIONS_PER_TILE)
      {
        std::cout << "Tile index: " << i/MAX_COLLISIONS_PER_TILE  << std::endl;

        for(int collision_index=0; collision_index < MAX_COLLISIONS_PER_TILE; ++collision_index)
        {
          int entity_id = tile_buckets[i + collision_index];
          //if (entity_id == -1) continue;
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

  sf::SoundBuffer tingling_sound_buffer;
  tingling_sound_buffer.loadFromFile("Assets/Sounds/tingling.wav");
  sf::Sound tingling;
  tingling.setBuffer(tingling_sound_buffer);
  
  tile_map<TILE_MAP_WIDTH,TILE_MAP_HEIGHT>* test_tile_map = new tile_map<TILE_MAP_WIDTH,TILE_MAP_HEIGHT>("Assets/Images/test_tile_map.png", (float) window_size.x, (float) window_size.y, TILE_MAP_TEXTURE_SIDE_SIZE);
  gameplay_entities<MAX_GAMEPLAY_ENTITIES>* all_gameplay_entities = new gameplay_entities<MAX_GAMEPLAY_ENTITIES>("Assets/Images/gameplay_entities.png", TILE_MAP_TEXTURE_SIDE_SIZE * 3); // need to be able to handle a single gameplay entity per tile
  int gameplay_entity_id_to_tile_bucket_index_of_first_vertex[MAX_GAMEPLAY_ENTITIES];
  gameplay_entity_ids_per_tile::update(*test_tile_map, *all_gameplay_entities, gameplay_entity_id_to_tile_bucket_index_of_first_vertex); // initialize

  #ifdef _DEBUG
    bool show_debug_data = true;
    sf::Font mandalore_font;
    mandalore_font.loadFromFile("Assets/Fonts/mandalore.ttf");

    static sf::Text tile_index_text[TILE_MAP_WIDTH * TILE_MAP_HEIGHT];
    test_tile_map->generate_debug_tile_index_text(tile_index_text, mandalore_font, sf::Color::Blue);
    static sf::Text game_entity_index_text[MAX_GAMEPLAY_ENTITIES];
  #endif
  
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
  all_gameplay_entities->velocities[0] = sf::Vector2f(0.0f ,0.0f );
  all_gameplay_entities->velocities[1] = sf::Vector2f(0.0f, 10.0f);
  all_gameplay_entities->velocities[2] = sf::Vector2f(0.0f, 0.0f);
  all_gameplay_entities->velocities[3] = sf::Vector2f(10.4f, 0.0f);
  all_gameplay_entities->velocities[4] = sf::Vector2f(25.0f ,0.0f);

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
    all_gameplay_entities->collision_vertices[i]   = sf::Vector2f(25.1f, 0.0f);
    all_gameplay_entities->collision_vertices[i+1] = sf::Vector2f(test_tile_map->tile_size_x - 25.1f, 0.0f);
    all_gameplay_entities->collision_vertices[i+2] = sf::Vector2f(test_tile_map->tile_size_x - 25.1f, test_tile_map->tile_size_y - 25.1f);
    all_gameplay_entities->collision_vertices[i+3] = sf::Vector2f(25.1f, test_tile_map->tile_size_y - 25.1f);
  }

  all_gameplay_entities->update_position_by_offset( 0, sf::Vector2f(test_tile_map->tile_size_x * 2, 3 * test_tile_map->tile_size_y) );
  all_gameplay_entities->update_position_by_offset( 1, sf::Vector2f(test_tile_map->tile_size_x * 4, 5 * test_tile_map->tile_size_y) );
  all_gameplay_entities->update_position_by_offset( 2, sf::Vector2f(test_tile_map->tile_size_x * 6, 7 * test_tile_map->tile_size_y) );
  all_gameplay_entities->update_position_by_offset( 3, sf::Vector2f(test_tile_map->tile_size_x * 8, 3 * test_tile_map->tile_size_y) );
  all_gameplay_entities->update_position_by_offset( 4, sf::Vector2f(test_tile_map->tile_size_x * 3, 7 * test_tile_map->tile_size_y) );

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
      if ( (elapsed_frame_time_milliseconds > 16) && show_debug_data)
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
                //if ( window_event.key.code == sf::Keyboard::P ) gameplay_entity_ids_per_tile::print_tile_buckets();
              #endif

              break;

        default:
              break;
      }
    }



    /* calculate gameplay stuff */


    // generate boundary walls
    for(int i=0; i < test_tile_map->width; ++i)
    {
      test_tile_map->bitmap[i] = static_cast<int>(test_tile_map_bitmap_type::WALL);
    }

    for(int i=0; i < test_tile_map->height; ++i)
    {
      test_tile_map->bitmap[i * test_tile_map->width] = static_cast<int>(test_tile_map_bitmap_type::WALL);
    }

    for(int i=(test_tile_map->tile_count - test_tile_map->width); i < test_tile_map->tile_count; ++i)
    {
      test_tile_map->bitmap[i] = static_cast<int>(test_tile_map_bitmap_type::WALL);
    }

    for(int i=0; i < test_tile_map->height; ++i)
    {
      test_tile_map->bitmap[(i * test_tile_map->width) + (test_tile_map->width - 1)] = static_cast<int>(test_tile_map_bitmap_type::WALL);
    }

    for(int tile_index = (test_tile_map->width * 2); tile_index < test_tile_map->tile_count; tile_index += (test_tile_map->width * 2))
    {
      for(int tile_index_offset=0; tile_index_offset < test_tile_map->width; ++tile_index_offset)
      {
        if (tile_index_offset % 2 != 0)
          test_tile_map->bitmap[tile_index + tile_index_offset] = static_cast<int>(test_tile_map_bitmap_type::WALL);
      }
    }

    test_tile_map->update_tex_coords_from_bitmap();


    all_gameplay_entities->update_positions_by_velocity(elapsed_frame_time_seconds);
    gameplay_entity_ids_per_tile::update(*test_tile_map, *all_gameplay_entities, gameplay_entity_id_to_tile_bucket_index_of_first_vertex);

    // correct gameplay entities that are off the map
    if(gameplay_entity_ids_per_tile::off_map_bitfield.any())
    {
      float offset_x;
      float offset_y;
      bool  need_to_correct_x;
      bool  need_to_correct_y;
      int   vertex_index;
      int   y_index;
      int   x_index;
      int   first_vertex_tile_map_index;

      for(int gameplay_entity_id=0; gameplay_entity_id < MAX_GAMEPLAY_ENTITIES; ++gameplay_entity_id)
      {
        if(gameplay_entity_ids_per_tile::off_map_bitfield[gameplay_entity_id] == false) continue;

        vertex_index = gameplay_entity_id * 4;

        y_index = static_cast<int>(all_gameplay_entities->collision_vertices[vertex_index].y / test_tile_map->tile_size_y);
        x_index = static_cast<int>(all_gameplay_entities->collision_vertices[vertex_index].x / test_tile_map->tile_size_x);
        first_vertex_tile_map_index = (y_index * test_tile_map->width) + x_index;

        // does top left vertice have negative x or y?
        offset_x = all_gameplay_entities->collision_vertices[vertex_index].x;
        offset_y = all_gameplay_entities->collision_vertices[vertex_index].y;
        need_to_correct_x = offset_x < 0.0f;
        need_to_correct_y = offset_y < 0.0f;

        all_gameplay_entities->update_position_by_offset(gameplay_entity_id, sf::Vector2f( -1.0f * offset_x * static_cast<float>(need_to_correct_x) , -1.0f * offset_y * static_cast<float>(need_to_correct_y) ));

        // does bottom right vertice have too big x or y?
        offset_x = all_gameplay_entities->collision_vertices[vertex_index + 2].x - ( (test_tile_map->width  * test_tile_map->tile_size_x) - 1 );
        offset_y = all_gameplay_entities->collision_vertices[vertex_index + 2].y - ( (test_tile_map->height * test_tile_map->tile_size_y) - 1 );
        need_to_correct_x = offset_x > 0.0f;
        need_to_correct_y = offset_y > 0.0f;

        all_gameplay_entities->update_position_by_offset(gameplay_entity_id, sf::Vector2f( -1.0f * offset_x * static_cast<float>(need_to_correct_x) , -1.0f * offset_y * static_cast<float>(need_to_correct_y) ));
      }
    }


    // correct gameplay entities that are overlapping walls
    {
      int tile_bucket_index_limit;
      int gameplay_entity_id;
      float offset_x;
      float offset_y;

      for (int tile_index=0; tile_index < test_tile_map->tile_count; ++tile_index)
      {
        if(static_cast<test_tile_map_bitmap_type>(test_tile_map->bitmap[tile_index]) != test_tile_map_bitmap_type::WALL) continue;

        offset_x = 0.0f;
        offset_y = 0.0f;
        tile_bucket_index_limit = (tile_index + 1) * MAX_COLLISIONS_PER_TILE;
        
        for(int tile_bucket_index = (tile_index * MAX_COLLISIONS_PER_TILE); tile_bucket_index < tile_bucket_index_limit; ++tile_bucket_index)
        {
          gameplay_entity_id = gameplay_entity_ids_per_tile::tile_buckets[tile_bucket_index];
          
          if(gameplay_entity_id == -1) break;

          if(all_gameplay_entities->velocities[gameplay_entity_id].x)
          {
            if(all_gameplay_entities->velocities[gameplay_entity_id].x > 0.0f)
            {
              offset_x = test_tile_map->vertex_buffer[( (tile_index+1) * 4)].position.x - all_gameplay_entities->collision_vertices[(gameplay_entity_id * 4) + 1].x;
              offset_x += -0.01f;
            }
            else
            {
              offset_x = test_tile_map->vertex_buffer[( (tile_index+1) * 4) + 1].position.x - all_gameplay_entities->collision_vertices[(gameplay_entity_id * 4)].x;
            }
          }

          if(all_gameplay_entities->velocities[gameplay_entity_id].y)
          {
            if(all_gameplay_entities->velocities[gameplay_entity_id].y > 0.0f)
            {
              offset_y = test_tile_map->vertex_buffer[( (tile_index+1) * 4)].position.y - all_gameplay_entities->collision_vertices[(gameplay_entity_id * 4) + 2].y;
              offset_y += -0.01f;
            }
            else
            {
              offset_y = test_tile_map->vertex_buffer[( (tile_index+1) * 4) + 2].position.y - all_gameplay_entities->collision_vertices[(gameplay_entity_id * 4)].y;
            }
          }

          all_gameplay_entities->update_position_by_offset( gameplay_entity_id, sf::Vector2f(offset_x, offset_y) );
        }
      }
    }

    // maybe only re-collision-sort if someone at least 1 was off map or overlapping tile?
    gameplay_entity_ids_per_tile::update(*test_tile_map, *all_gameplay_entities, gameplay_entity_id_to_tile_bucket_index_of_first_vertex);

    //  then handle gameplay_entity overlaps: set velocities, correct overlapping, and commit overlap gameplay_events (game_entities)
        // find the midpoint between the 2 entities for each x and y and reset both velocties for each entity to the new calculated values
    //  then commit tile_map trigger events ex) powerups, hearts, etc...

    // iterate through all tiles checking for overlaps
    for(int tile_bucket_index=0; tile_bucket_index < (MAX_COLLISIONS_PER_TILE * TILE_MAP_COUNT); tile_bucket_index += MAX_COLLISIONS_PER_TILE)
    {
      // compare all entities in current tile (the max collisoins - 1 is because no one left to compare it to)
      for(int offset=0; offset < (MAX_COLLISIONS_PER_TILE - 1); ++offset)
      {
        int current_gameplay_entity_id = gameplay_entity_ids_per_tile::tile_buckets[tile_bucket_index + offset];
        if (current_gameplay_entity_id == -1) break;  // nothing left in bucket

        for(int other_bucket_index = tile_bucket_index + offset + 1; other_bucket_index < (tile_bucket_index + MAX_COLLISIONS_PER_TILE); ++other_bucket_index)
        {
          int next_gameplay_entity_id = gameplay_entity_ids_per_tile::tile_buckets[other_bucket_index];
          if (next_gameplay_entity_id == -1) break; // nothing left in bucket

          // check if overlap already handled this frame?

          sf::Vector2f current_top_left_vertex = all_gameplay_entities->collision_vertices[current_gameplay_entity_id * 4];
          sf::Vector2f next_top_left_vertex    = all_gameplay_entities->collision_vertices[next_gameplay_entity_id * 4];
          int most_up_gameplay_entity_id;
          int most_down_gameplay_entity_id;
          int most_left_gameplay_entity_id;
          int most_right_gameplay_entity_id;
          bool is_y_overlap;
          bool is_x_overlap;
          
          if (current_top_left_vertex.y != next_top_left_vertex.y) // if both are equal first condition passes
          {
            most_up_gameplay_entity_id = ( current_gameplay_entity_id * static_cast<int>(current_top_left_vertex.y < next_top_left_vertex.y)    )
                                       + ( next_gameplay_entity_id    * static_cast<int>(next_top_left_vertex.y    < current_top_left_vertex.y) );

            most_down_gameplay_entity_id = ( current_gameplay_entity_id * static_cast<int>(most_up_gameplay_entity_id != current_gameplay_entity_id) )
                                         + ( next_gameplay_entity_id    * static_cast<int>(most_up_gameplay_entity_id != next_gameplay_entity_id)    );

            is_y_overlap = all_gameplay_entities->collision_vertices[(most_up_gameplay_entity_id * 4) + 2].y >= all_gameplay_entities->collision_vertices[most_down_gameplay_entity_id * 4].y;
          }
          else
          {
            is_y_overlap = true;

            // arbitrary values for when entities are on top of each other (game broke?)
            most_up_gameplay_entity_id   = current_gameplay_entity_id;
            most_down_gameplay_entity_id = next_gameplay_entity_id;
          }

          if (current_top_left_vertex.x != next_top_left_vertex.x) // if both are equal first condition passes
          {
            most_left_gameplay_entity_id = ( current_gameplay_entity_id * static_cast<int>(current_top_left_vertex.x < next_top_left_vertex.x)    )
                                         + ( next_gameplay_entity_id    * static_cast<int>(next_top_left_vertex.x    < current_top_left_vertex.x) );

            most_right_gameplay_entity_id = ( current_gameplay_entity_id * static_cast<int>(most_left_gameplay_entity_id != current_gameplay_entity_id) )
                                          + ( next_gameplay_entity_id    * static_cast<int>(most_left_gameplay_entity_id != next_gameplay_entity_id)    );

            is_x_overlap = all_gameplay_entities->collision_vertices[(most_left_gameplay_entity_id * 4) + 1].x >= all_gameplay_entities->collision_vertices[most_right_gameplay_entity_id * 4].x;
          }
          else
          {
            is_x_overlap = true;

            // arbitrary values for when entities are on top of each other (game broke?)
            most_left_gameplay_entity_id  = current_gameplay_entity_id;
            most_right_gameplay_entity_id = next_gameplay_entity_id;
          }

          //  if yes for both then there is an overlap
          if(is_y_overlap && is_x_overlap)
          {
            // @remember: gameplay entities by design won't be stationary and located in 2 tiles

            // undo velocity moves
            all_gameplay_entities->update_position_by_offset(current_gameplay_entity_id, -1.0f * all_gameplay_entities->velocities[current_gameplay_entity_id] * elapsed_frame_time_seconds);
            all_gameplay_entities->update_position_by_offset(next_gameplay_entity_id,    -1.0f * all_gameplay_entities->velocities[next_gameplay_entity_id]    * elapsed_frame_time_seconds);

            // calculate how long it took to intersect and then apply original velocity for that time
            //    use equations of motion: gameplay_entity_position(time) = gameplay_entity_position(0) + (velocity * time)
            //    need to solve for time when gameplay_entity_positions are equal to each other


            // if entities have different x/y velocities then only move the entity that has the right of way; the other is treated as running into a wall
            //    decide right of way = check if far vertice on velocity direction side have no overlap

            bool is_current_gameplay_entity_stationary;
            bool is_next_gameplay_entity_stationary;
            bool is_current_gameplay_entity_x_velocity;
            bool is_next_gameplay_entity_x_velocity;

            if (all_gameplay_entities->velocities[current_gameplay_entity_id].x)
            {
              is_current_gameplay_entity_stationary = false;
              is_current_gameplay_entity_x_velocity = true;
            }
            else
            {
              is_current_gameplay_entity_x_velocity = false;
              if (all_gameplay_entities->velocities[current_gameplay_entity_id].y == 0.0f)
              {
                is_current_gameplay_entity_stationary = true;
              }
              else
              {
                is_current_gameplay_entity_stationary = false;
              }
            }

            if (all_gameplay_entities->velocities[next_gameplay_entity_id].x)
            {
              is_next_gameplay_entity_stationary = false;
              is_next_gameplay_entity_x_velocity = true;
            }
            else
            {
              is_next_gameplay_entity_x_velocity = false;
              if (all_gameplay_entities->velocities[next_gameplay_entity_id].y == 0.0f)
              {
                is_next_gameplay_entity_stationary = true;
              }
              else
              {
                is_next_gameplay_entity_stationary = false;
              }
            }


            float intersect_time;
            sf::Vector2f new_velocity;
            bool is_different_axis_velocities = false;

            if ( (is_current_gameplay_entity_x_velocity && is_next_gameplay_entity_x_velocity) || (is_current_gameplay_entity_x_velocity && is_next_gameplay_entity_stationary) || (is_next_gameplay_entity_x_velocity && is_current_gameplay_entity_stationary) )
            {
              // handle x velocity only
              intersect_time = std::abs(( all_gameplay_entities->collision_vertices[(most_left_gameplay_entity_id * 4) + 1].x - all_gameplay_entities->collision_vertices[most_right_gameplay_entity_id * 4].x) / ( all_gameplay_entities->velocities[most_right_gameplay_entity_id].x - all_gameplay_entities->velocities[most_left_gameplay_entity_id].x ));
              float x_midpoint = (all_gameplay_entities->velocities[current_gameplay_entity_id].x + all_gameplay_entities->velocities[next_gameplay_entity_id].x) / 2.0f;
              new_velocity = sf::Vector2f(x_midpoint, 0.0f);
            }
            else if ( (!is_current_gameplay_entity_x_velocity && !is_next_gameplay_entity_x_velocity) || (!is_current_gameplay_entity_x_velocity && is_next_gameplay_entity_stationary) || (!is_next_gameplay_entity_x_velocity && is_current_gameplay_entity_stationary) )
            {
              // handle y velocity only
              intersect_time = std::abs(( all_gameplay_entities->collision_vertices[(most_up_gameplay_entity_id * 4)   + 2].y - all_gameplay_entities->collision_vertices[most_down_gameplay_entity_id * 4].y ) / ( all_gameplay_entities->velocities[most_down_gameplay_entity_id].y  - all_gameplay_entities->velocities[most_up_gameplay_entity_id].y   ));
              float y_midpoint = (all_gameplay_entities->velocities[current_gameplay_entity_id].y + all_gameplay_entities->velocities[next_gameplay_entity_id].y) / 2.0f;
              new_velocity = sf::Vector2f(0.0f, y_midpoint);
            }
            else
            {
              // ! should probably assert if game broke because of collision for 2 stationary gameplay_entities
              is_different_axis_velocities = true;
            }

            if (!is_different_axis_velocities)
            {
              all_gameplay_entities->update_position_by_offset( current_gameplay_entity_id, all_gameplay_entities->velocities[current_gameplay_entity_id] * intersect_time);
              all_gameplay_entities->update_position_by_offset( current_gameplay_entity_id, all_gameplay_entities->velocities[next_gameplay_entity_id]    * intersect_time);

              all_gameplay_entities->update_position_by_offset( current_gameplay_entity_id,  new_velocity * (elapsed_frame_time_seconds - intersect_time) );
              all_gameplay_entities->update_position_by_offset( next_gameplay_entity_id,     new_velocity * (elapsed_frame_time_seconds - intersect_time) );
            }
            else
            {
              // teleport entity that doesn't have right away
              // do normal velocity move for entity that does have right of way
            }

            //    set new velocities?
            //    commit overlap gameplay event
          }
        }
      }
    }

    // collision sort again?

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
        window.draw( *(test_tile_map->generate_debug_line_vertices(sf::Color::Blue))                  );
        window.draw( *(all_gameplay_entities->generate_debug_collision_line_vertices(sf::Color::Red)) );
        //window.draw( *(all_gameplay_entities->generate_debug_line_vertices(sf::Color::Yellow))        );

        for(auto& text : tile_index_text) window.draw(text);

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
