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


#define TILE_MAP_WIDTH              17
#define TILE_MAP_HEIGHT             11
#define TILE_MAP_COUNT              (TILE_MAP_WIDTH * TILE_MAP_HEIGHT)
#define MAX_GAMEPLAY_ENTITIES       TILE_MAP_COUNT
#define TILE_MAP_TEXTURE_SIDE_SIZE  64                                  // in pixels
#define MAX_COLLISIONS_PER_TILE     5                                   // potential game objects (collisions) in an single tile
#define MAX_CHAIN_COLLISIONS        (2 * TILE_MAP_WIDTH)


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
  all_gameplay_entities->is_garbage_flags[5] = false;
  all_gameplay_entities->is_garbage_flags[6] = false;
  all_gameplay_entities->is_garbage_flags[7] = false;
  all_gameplay_entities->is_garbage_flags[8] = false;
  all_gameplay_entities->is_garbage_flags[9] = false;
  all_gameplay_entities->is_garbage_flags[10] = false;
  all_gameplay_entities->is_garbage_flags[11] = false;
  all_gameplay_entities->is_garbage_flags[12] = false;
  all_gameplay_entities->is_garbage_flags[13] = false;
  all_gameplay_entities->is_garbage_flags[14] = false;



  all_gameplay_entities->types[0] = gameplay_entity_type::MARIO;
  all_gameplay_entities->types[1] = gameplay_entity_type::BOMB;
  all_gameplay_entities->types[2] = gameplay_entity_type::BOMB;
  all_gameplay_entities->types[3] = gameplay_entity_type::BOMB;
  all_gameplay_entities->types[4] = gameplay_entity_type::BOMB;
  all_gameplay_entities->types[5] = gameplay_entity_type::BOMB;
  all_gameplay_entities->types[6] = gameplay_entity_type::BOMB;
  all_gameplay_entities->types[7] = gameplay_entity_type::BOMB;
  all_gameplay_entities->types[8] = gameplay_entity_type::BOMB;
  all_gameplay_entities->types[9] = gameplay_entity_type::BOMB;
  all_gameplay_entities->types[10] = gameplay_entity_type::BOMB;
  all_gameplay_entities->types[11] = gameplay_entity_type::BOMB;
  all_gameplay_entities->types[12] = gameplay_entity_type::BOMB;
  all_gameplay_entities->types[13] = gameplay_entity_type::BOMB;
  all_gameplay_entities->types[14] = gameplay_entity_type::BOMB;
 
  all_gameplay_entities->animation_indexes[0] = 0;
  all_gameplay_entities->animation_indexes[1] = 0;
  all_gameplay_entities->animation_indexes[2] = 0;
  all_gameplay_entities->velocities[0] = sf::Vector2f(0.0f ,0.0f );
  all_gameplay_entities->velocities[1] = sf::Vector2f(50.0f, 0.0f);
  all_gameplay_entities->velocities[2] = sf::Vector2f(0.0f, 0.0f);
  all_gameplay_entities->velocities[3] = sf::Vector2f(0.0f, 0.0f);
  all_gameplay_entities->velocities[4] = sf::Vector2f(25.0f ,0.0f);
  all_gameplay_entities->velocities[5] = sf::Vector2f(-100.0f ,0.0f );
  all_gameplay_entities->velocities[6] = sf::Vector2f(0.0f, -10.0f);
  all_gameplay_entities->velocities[7] = sf::Vector2f(0.0f, 0.0f);
  all_gameplay_entities->velocities[8] = sf::Vector2f(-25.0f, 0.0f);
  all_gameplay_entities->velocities[9] = sf::Vector2f(-25.0f ,0.0f);
  all_gameplay_entities->velocities[10] = sf::Vector2f(-100.0f ,0.0f );
  all_gameplay_entities->velocities[11] = sf::Vector2f(0.0f, 100.0f);
  all_gameplay_entities->velocities[12] = sf::Vector2f(0.0f, 100.0f);
  all_gameplay_entities->velocities[13] = sf::Vector2f(-50.0f, 0.0f);
  all_gameplay_entities->velocities[14] = sf::Vector2f(25.0f ,0.0f);



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
    all_gameplay_entities->collision_vertices[i]   = sf::Vector2f(0.05f, 0.0f);
    all_gameplay_entities->collision_vertices[i+1] = sf::Vector2f(test_tile_map->tile_size_x - 0.05f, 0.0f);
    all_gameplay_entities->collision_vertices[i+2] = sf::Vector2f(test_tile_map->tile_size_x - 0.05f, test_tile_map->tile_size_y - 0.05f);
    all_gameplay_entities->collision_vertices[i+3] = sf::Vector2f(0.05f, test_tile_map->tile_size_y - 0.05f);
  }

  all_gameplay_entities->update_position_by_offset( 0, sf::Vector2f(test_tile_map->tile_size_x * 2, 3 * test_tile_map->tile_size_y) );
  all_gameplay_entities->update_position_by_offset( 1, sf::Vector2f(test_tile_map->tile_size_x * 4, 5 * test_tile_map->tile_size_y) );
  all_gameplay_entities->update_position_by_offset( 2, sf::Vector2f(test_tile_map->tile_size_x * 6, 7 * test_tile_map->tile_size_y) );
  all_gameplay_entities->update_position_by_offset( 3, sf::Vector2f(test_tile_map->tile_size_x * 8, 3 * test_tile_map->tile_size_y) );
  all_gameplay_entities->update_position_by_offset( 4, sf::Vector2f(test_tile_map->tile_size_x * 3, 7 * test_tile_map->tile_size_y) );
  all_gameplay_entities->update_position_by_offset( 5, sf::Vector2f(test_tile_map->tile_size_x * -10002, 2* test_tile_map->tile_size_y) );
  all_gameplay_entities->update_position_by_offset( 6, sf::Vector2f(test_tile_map->tile_size_x * 7, 8 * test_tile_map->tile_size_y) );
  all_gameplay_entities->update_position_by_offset( 7, sf::Vector2f(test_tile_map->tile_size_x * 6, 9 * test_tile_map->tile_size_y) );
  all_gameplay_entities->update_position_by_offset( 8, sf::Vector2f(test_tile_map->tile_size_x * 8, 6 * test_tile_map->tile_size_y) );
  all_gameplay_entities->update_position_by_offset( 9, sf::Vector2f(test_tile_map->tile_size_x * 9, 4 * test_tile_map->tile_size_y) );
  all_gameplay_entities->update_position_by_offset( 10, sf::Vector2f(test_tile_map->tile_size_x * 2, 9 * test_tile_map->tile_size_y) );
  all_gameplay_entities->update_position_by_offset( 11, sf::Vector2f(test_tile_map->tile_size_x * 4, 4 * test_tile_map->tile_size_y) );
  all_gameplay_entities->update_position_by_offset( 12, sf::Vector2f(test_tile_map->tile_size_x * 13, 8 * test_tile_map->tile_size_y) );
  all_gameplay_entities->update_position_by_offset( 13, sf::Vector2f(test_tile_map->tile_size_x * 8, 5 * test_tile_map->tile_size_y) );
  all_gameplay_entities->update_position_by_offset( 14, sf::Vector2f(test_tile_map->tile_size_x * 4, 9 * test_tile_map->tile_size_y) );



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


    // generate walls
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

    //for(int tile_index = 0; tile_index < test_tile_map->tile_count; tile_index += (test_tile_map->width * 2))
    for(int tile_index = (test_tile_map->width * 2); tile_index < test_tile_map->tile_count; tile_index += (test_tile_map->width * 2))
    {
      for(int tile_index_offset=1; tile_index_offset < test_tile_map->width; ++tile_index_offset)
      {
        if (tile_index_offset % 2 == 0)
          test_tile_map->bitmap[tile_index + tile_index_offset] = static_cast<int>(test_tile_map_bitmap_type::WALL);
      }
    }

    test_tile_map->update_tex_coords_from_bitmap();

    /* collision update loop */

    all_gameplay_entities->update_positions_by_velocity(elapsed_frame_time_seconds);
    gameplay_entity_ids_per_tile::update(*test_tile_map, *all_gameplay_entities, gameplay_entity_id_to_tile_bucket_index_of_first_vertex);

    std::bitset<MAX_GAMEPLAY_ENTITIES> is_wall_corrected;
    is_wall_corrected.reset();

    std::bitset<MAX_GAMEPLAY_ENTITIES> is_right_of_way_corrected;
    is_right_of_way_corrected.reset();

    sf::Vector2f velocity_cache[MAX_GAMEPLAY_ENTITIES];
    for(int i=0; i < MAX_GAMEPLAY_ENTITIES; ++i)
      velocity_cache[i] = all_gameplay_entities->velocities[i];

    for (int chain_collision_index=0; chain_collision_index < MAX_CHAIN_COLLISIONS; ++chain_collision_index)
    {
      // handle gameplay entities that are off the map (for now just delete them. decided correcting probably isn't good idea)
      if(gameplay_entity_ids_per_tile::off_map_bitfield.any())
      {
        for(int gameplay_entity_id=0; gameplay_entity_id < MAX_GAMEPLAY_ENTITIES; ++gameplay_entity_id)
        {
          if(gameplay_entity_ids_per_tile::off_map_bitfield[gameplay_entity_id] == false)
          {
            continue;
          }
          else
          {
            all_gameplay_entities->is_garbage_flags[gameplay_entity_id] = true;
          };
        }

        gameplay_entity_ids_per_tile::update(*test_tile_map, *all_gameplay_entities, gameplay_entity_id_to_tile_bucket_index_of_first_vertex);
      }

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

              // look at velocity signs and sizes to determine or maybe undo velocity move to decide

              // !!! arbitrary values for when entities are on top of each other (game broke?)
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

              // look at velocity signs and sizes to determine or maybe undo velocity moves to decide

              // !!! arbitrary values for when entities are on top of each other (game broke?)
              most_left_gameplay_entity_id  = current_gameplay_entity_id;
              most_right_gameplay_entity_id = next_gameplay_entity_id;
            }

            //  if yes for both then there is an overlap
            if(is_y_overlap && is_x_overlap)
            {
              // calculate how long it took to intersect and apply original velocity for that time, then apply new velocity for remaining time
              //    use equations of motion: gameplay_entity_position(time) = gameplay_entity_position(0) + (velocity * time)
              //    need to solve for time when gameplay_entity_positions are equal to each other
              // if entities have different x/y velocities then only move the entity that has the right of way; the other is treated as running into a wall


              // undo velocity moves
              all_gameplay_entities->update_position_by_offset( current_gameplay_entity_id, -1.0f * all_gameplay_entities->velocities[current_gameplay_entity_id] * elapsed_frame_time_seconds * static_cast<float>(!is_wall_corrected[current_gameplay_entity_id]) * static_cast<float>(!is_right_of_way_corrected[current_gameplay_entity_id]) );
              all_gameplay_entities->update_position_by_offset( next_gameplay_entity_id,    -1.0f * all_gameplay_entities->velocities[next_gameplay_entity_id]    * elapsed_frame_time_seconds * static_cast<float>(!is_wall_corrected[next_gameplay_entity_id])    * static_cast<float>(!is_right_of_way_corrected[next_gameplay_entity_id])    );

              bool is_current_gameplay_entity_stationary = (all_gameplay_entities->velocities[current_gameplay_entity_id].x == 0.0f) && (all_gameplay_entities->velocities[current_gameplay_entity_id].y == 0.0f);
              bool is_next_gameplay_entity_stationary    = (all_gameplay_entities->velocities[next_gameplay_entity_id].x    == 0.0f) && (all_gameplay_entities->velocities[next_gameplay_entity_id].y    == 0.0f);
              bool is_x_axis_velocity_collision          = std::abs(all_gameplay_entities->velocities[current_gameplay_entity_id].x) + std::abs(all_gameplay_entities->velocities[next_gameplay_entity_id].x) > 0.0f;
              bool is_y_axis_velocity_collision          = std::abs(all_gameplay_entities->velocities[current_gameplay_entity_id].y) + std::abs(all_gameplay_entities->velocities[next_gameplay_entity_id].y) > 0.0f;


              sf::Vector2f collision_velocity;
              float intersect_time;
              float post_intersect_time;
              float velocity_midpoint;
              int   right_of_way_entity_id;
              int   non_right_of_way_entity_id;
   
              if (is_x_axis_velocity_collision && !is_y_axis_velocity_collision)
              {
                //if(all_gameplay_entities->velocities[most_right_gameplay_entity_id].x == all_gameplay_entities->velocities[most_left_gameplay_entity_id].x) continue;
                intersect_time     = ( all_gameplay_entities->collision_vertices[(most_left_gameplay_entity_id * 4) + 1].x - all_gameplay_entities->collision_vertices[most_right_gameplay_entity_id * 4].x) / ( all_gameplay_entities->velocities[most_right_gameplay_entity_id].x - all_gameplay_entities->velocities[most_left_gameplay_entity_id].x );
                velocity_midpoint  = (all_gameplay_entities->velocities[current_gameplay_entity_id].x + all_gameplay_entities->velocities[next_gameplay_entity_id].x) / 2.0f;
                collision_velocity = sf::Vector2f(velocity_midpoint, 0.0f);
              }
              else if (is_y_axis_velocity_collision && !is_x_axis_velocity_collision)
              {
                //if (all_gameplay_entities->velocities[most_down_gameplay_entity_id].y == all_gameplay_entities->velocities[most_up_gameplay_entity_id].y) continue;
                intersect_time     = ( all_gameplay_entities->collision_vertices[(most_up_gameplay_entity_id * 4) + 2].y - all_gameplay_entities->collision_vertices[most_down_gameplay_entity_id * 4].y ) / ( all_gameplay_entities->velocities[most_down_gameplay_entity_id].y - all_gameplay_entities->velocities[most_up_gameplay_entity_id].y );
                velocity_midpoint  = (all_gameplay_entities->velocities[current_gameplay_entity_id].y + all_gameplay_entities->velocities[next_gameplay_entity_id].y) / 2.0f;
                collision_velocity = sf::Vector2f(0.0f, velocity_midpoint);
              }
              else if (is_x_axis_velocity_collision && is_y_axis_velocity_collision)
              {
                int x_velocity_gameplay_entity_id;
                int y_velocity_gameplay_entity_id;

                if( all_gameplay_entities->velocities[current_gameplay_entity_id].x )
                {
                  x_velocity_gameplay_entity_id = current_gameplay_entity_id;
                  y_velocity_gameplay_entity_id = next_gameplay_entity_id;
                }
                else
                {
                  x_velocity_gameplay_entity_id = next_gameplay_entity_id;
                  y_velocity_gameplay_entity_id = current_gameplay_entity_id;
                }

                //if( (all_gameplay_entities->velocities[most_down_gameplay_entity_id].y == all_gameplay_entities->velocities[most_up_gameplay_entity_id].y) && (all_gameplay_entities->velocities[most_right_gameplay_entity_id].x == all_gameplay_entities->velocities[most_left_gameplay_entity_id].x) ) continue;

                // decide which direction took longer to intersect
                float x_intersect_time = ( all_gameplay_entities->collision_vertices[(most_left_gameplay_entity_id * 4) + 1].x - all_gameplay_entities->collision_vertices[most_right_gameplay_entity_id * 4].x) / ( all_gameplay_entities->velocities[most_right_gameplay_entity_id].x - all_gameplay_entities->velocities[most_left_gameplay_entity_id].x );
                float y_intersect_time = ( all_gameplay_entities->collision_vertices[(most_up_gameplay_entity_id * 4) + 2].y - all_gameplay_entities->collision_vertices[most_down_gameplay_entity_id * 4].y ) / ( all_gameplay_entities->velocities[most_down_gameplay_entity_id].y - all_gameplay_entities->velocities[most_up_gameplay_entity_id].y );

                // check if axis was already overlapping
                if (x_intersect_time > elapsed_frame_time_seconds) x_intersect_time *= -1.0f;
                if (y_intersect_time > elapsed_frame_time_seconds) y_intersect_time *= -1.0f;

                if (x_intersect_time >= y_intersect_time)
                {
                  right_of_way_entity_id     = y_velocity_gameplay_entity_id;
                  non_right_of_way_entity_id = x_velocity_gameplay_entity_id;
                  intersect_time = x_intersect_time;
                }
                else
                {
                  right_of_way_entity_id     = x_velocity_gameplay_entity_id;
                  non_right_of_way_entity_id = y_velocity_gameplay_entity_id;
                  intersect_time = y_intersect_time;
                }

                collision_velocity = -0.01f * ( all_gameplay_entities->velocities[non_right_of_way_entity_id] / (std::abs(all_gameplay_entities->velocities[non_right_of_way_entity_id].x) + std::abs(all_gameplay_entities->velocities[non_right_of_way_entity_id].y)) );
              }
              else // if (is_current_gameplay_entity_stationary && is_next_gameplay_entity_stationary)
              {
                // this case handles when both are stationary
                intersect_time = 0.0f;
                collision_velocity = sf::Vector2f(0.0f,0.0f);
              }


              // !!! should check for weird underflow post_interesect time or when post_intersect_time is negative?
              post_intersect_time = elapsed_frame_time_seconds - intersect_time;

              // !!! change these for asserts
              //if( (post_intersect_time > elapsed_frame_time_seconds) || (post_intersect_time < 0.0f) )
              //    std::cout << "bad post_intersect_time: " << post_intersect_time << std::endl;
              
              if ( !(is_x_axis_velocity_collision && is_y_axis_velocity_collision) )
              {
                all_gameplay_entities->update_position_by_offset( current_gameplay_entity_id, all_gameplay_entities->velocities[current_gameplay_entity_id] * intersect_time);
                all_gameplay_entities->update_position_by_offset( next_gameplay_entity_id, all_gameplay_entities->velocities[next_gameplay_entity_id]    * intersect_time);

                all_gameplay_entities->update_position_by_offset( current_gameplay_entity_id, collision_velocity * post_intersect_time );
                all_gameplay_entities->update_position_by_offset( next_gameplay_entity_id,    collision_velocity * post_intersect_time );

                // make entities slightly not touch
                if ( (collision_velocity.x + collision_velocity.y) != 0.0f)
                {
                  sf::Vector2f separate_offset = 0.01f * ( collision_velocity / (collision_velocity.x + collision_velocity.y) );

                  if (separate_offset.x)
                  {
                    if (separate_offset.x > 0.0f)
                    {
                      all_gameplay_entities->update_position_by_offset( most_right_gameplay_entity_id, separate_offset );
                    }
                    else
                    {
                      all_gameplay_entities->update_position_by_offset( most_left_gameplay_entity_id, separate_offset );
                    }
                  }
                  else if (separate_offset.y)
                  {
                    if (separate_offset.y > 0.0f)
                    {
                      all_gameplay_entities->update_position_by_offset( most_down_gameplay_entity_id, separate_offset );
                    }
                    else
                    {
                      all_gameplay_entities->update_position_by_offset( most_up_gameplay_entity_id, separate_offset );
                    }
                  }
                  all_gameplay_entities->velocities[current_gameplay_entity_id] = collision_velocity;
                  all_gameplay_entities->velocities[next_gameplay_entity_id] = collision_velocity;
                }
                else // velocity midpoint is 0
                {
                  sf::Vector2f current_separate_offset = -0.01f * ( all_gameplay_entities->velocities[current_gameplay_entity_id] / std::abs(all_gameplay_entities->velocities[current_gameplay_entity_id].x + all_gameplay_entities->velocities[current_gameplay_entity_id].y) );
                  all_gameplay_entities->update_position_by_offset( current_gameplay_entity_id, current_separate_offset );

                  sf::Vector2f next_separate_offset = -0.01f * ( all_gameplay_entities->velocities[next_gameplay_entity_id] / std::abs(all_gameplay_entities->velocities[next_gameplay_entity_id].x + all_gameplay_entities->velocities[next_gameplay_entity_id].y) );
                  all_gameplay_entities->update_position_by_offset( next_gameplay_entity_id, next_separate_offset );

                  //all_gameplay_entities->velocities[current_gameplay_entity_id] = (-1.0f * all_gameplay_entities->velocities[current_gameplay_entity_id]) + current_separate_offset;
                  //all_gameplay_entities->velocities[next_gameplay_entity_id]    = (-1.0f * all_gameplay_entities->velocities[next_gameplay_entity_id]) + next_separate_offset;
                  //is_right_of_way_corrected[current_gameplay_entity_id] = true;
                  //is_right_of_way_corrected[next_gameplay_entity_id]    = true;
                }
              }
              else
              {
                all_gameplay_entities->update_position_by_offset( right_of_way_entity_id, all_gameplay_entities->velocities[right_of_way_entity_id] * elapsed_frame_time_seconds);

                all_gameplay_entities->velocities[non_right_of_way_entity_id] = (-2.0f * all_gameplay_entities->velocities[non_right_of_way_entity_id]);

                all_gameplay_entities->update_position_by_offset( non_right_of_way_entity_id, all_gameplay_entities->velocities[non_right_of_way_entity_id] * elapsed_frame_time_seconds );                
                is_right_of_way_corrected[non_right_of_way_entity_id] = true;

                is_wall_corrected[right_of_way_entity_id] = true;
              }

              //  commit overlap gameplay event?
            }
          }
        }
      }

      gameplay_entity_ids_per_tile::update(*test_tile_map, *all_gameplay_entities, gameplay_entity_id_to_tile_bucket_index_of_first_vertex);

      // correct gameplay entities that are overlapping walls
      {
        int tile_bucket_index_limit;
        int gameplay_entity_id;
        float offset_x;
        float offset_y;

        for (int tile_index=0; tile_index < test_tile_map->tile_count; ++tile_index)
        {
          if(static_cast<test_tile_map_bitmap_type>(test_tile_map->bitmap[tile_index]) != test_tile_map_bitmap_type::WALL) continue;

          tile_bucket_index_limit = (tile_index + 1) * MAX_COLLISIONS_PER_TILE;
          
          for(int tile_bucket_index = (tile_index * MAX_COLLISIONS_PER_TILE); tile_bucket_index < tile_bucket_index_limit; ++tile_bucket_index)
          {
            offset_x = 0.0f;
            offset_y = 0.0f;
            gameplay_entity_id = gameplay_entity_ids_per_tile::tile_buckets[tile_bucket_index];
            
            if (gameplay_entity_id == -1) break;
            if (is_wall_corrected[gameplay_entity_id]) continue;

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
                offset_x += 0.01f;
              }
            }
            else if(all_gameplay_entities->velocities[gameplay_entity_id].y)
            {
              if(all_gameplay_entities->velocities[gameplay_entity_id].y > 0.0f)
              {
                offset_y = test_tile_map->vertex_buffer[( (tile_index+1) * 4)].position.y - all_gameplay_entities->collision_vertices[(gameplay_entity_id * 4) + 2].y;
                offset_y += -0.01f;
              }
              else
              {
                offset_y = test_tile_map->vertex_buffer[( (tile_index+1) * 4) + 2].position.y - all_gameplay_entities->collision_vertices[(gameplay_entity_id * 4)].y;
                offset_y += 0.01f;
              }
            }
            else
            {
              // !!! assert error gameplay entity in wall has no x or y velocity because even if pushed into a wall it must have had velocity change?
            }

            all_gameplay_entities->update_position_by_offset( gameplay_entity_id, sf::Vector2f(offset_x, offset_y) );
            all_gameplay_entities->velocities[gameplay_entity_id] = (-1.0f * all_gameplay_entities->velocities[gameplay_entity_id]) + sf::Vector2f(offset_x, offset_y);
            is_wall_corrected[gameplay_entity_id] = true;
          }
        }
      }

      gameplay_entity_ids_per_tile::update(*test_tile_map, *all_gameplay_entities, gameplay_entity_id_to_tile_bucket_index_of_first_vertex);
    } // end of chain collision loop

    for(int i=0; i < MAX_GAMEPLAY_ENTITIES; ++i)
      all_gameplay_entities->velocities[i] = velocity_cache[i];

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
