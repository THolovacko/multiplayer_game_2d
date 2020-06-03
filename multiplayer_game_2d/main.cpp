#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <string.h>
#include <windows.h>
#include <bitset>
#include <cmath>
#include "gameplay.h"


#pragma warning(disable : 26812)


// @remember: should decrease MAX_ENTITIES_PER_TILE when game is finished or when related design decisions are final
#define TILE_MAP_WIDTH              17
#define TILE_MAP_HEIGHT             11
#define TILE_MAP_COUNT              (TILE_MAP_WIDTH * TILE_MAP_HEIGHT)
#define MAX_GAMEPLAY_ENTITIES       TILE_MAP_COUNT
#define TILE_MAP_TEXTURE_SIDE_SIZE  64                                  // in pixels
#define MAX_ENTITIES_PER_TILE       10                                  // potential game objects (collisions) in an single tile
#define MAX_CHAIN_COLLISIONS        (2 * TILE_MAP_WIDTH)



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
  gameplay_entity_ids_per_tile<TILE_MAP_WIDTH,TILE_MAP_HEIGHT,MAX_GAMEPLAY_ENTITIES,MAX_ENTITIES_PER_TILE>* tile_to_gameplay_entities = new gameplay_entity_ids_per_tile<TILE_MAP_WIDTH,TILE_MAP_HEIGHT,MAX_GAMEPLAY_ENTITIES,MAX_ENTITIES_PER_TILE>();

  #ifdef _DEBUG
    bool show_debug_data = true;
    sf::Font mandalore_font;
    mandalore_font.loadFromFile("Assets/Fonts/mandalore.ttf");

    static sf::Text tile_index_text[TILE_MAP_WIDTH * TILE_MAP_HEIGHT];
    test_tile_map->generate_debug_tile_index_text(tile_index_text, mandalore_font, sf::Color::Blue);
    static sf::Text game_entity_index_text[MAX_GAMEPLAY_ENTITIES];
  #endif
  
  all_gameplay_entities->is_garbage_flags[0]  = false;
  all_gameplay_entities->is_garbage_flags[1]  = false;
  all_gameplay_entities->is_garbage_flags[2]  = true;
  all_gameplay_entities->is_garbage_flags[3]  = false;
  all_gameplay_entities->is_garbage_flags[4]  = true;
  all_gameplay_entities->is_garbage_flags[5]  = true;
  all_gameplay_entities->is_garbage_flags[6]  = true;
  all_gameplay_entities->is_garbage_flags[7]  = true;
  all_gameplay_entities->is_garbage_flags[8]  = true;
  all_gameplay_entities->is_garbage_flags[9]  = true;
  all_gameplay_entities->is_garbage_flags[10] = true;
  all_gameplay_entities->is_garbage_flags[11] = true;
  all_gameplay_entities->is_garbage_flags[12] = true;
  all_gameplay_entities->is_garbage_flags[13] = true;
  all_gameplay_entities->is_garbage_flags[14] = true;



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
  /*
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
  */


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
    all_gameplay_entities->collision_vertices[i]   = sf::Vector2f(25.0f, 0.0f);
    all_gameplay_entities->collision_vertices[i+1] = sf::Vector2f(test_tile_map->tile_size_x - 25.0f, 0.0f);
    all_gameplay_entities->collision_vertices[i+2] = sf::Vector2f(test_tile_map->tile_size_x - 25.0f, test_tile_map->tile_size_y - 25.0f);
    all_gameplay_entities->collision_vertices[i+3] = sf::Vector2f(25.0f, test_tile_map->tile_size_y - 25.0f);
  }

  all_gameplay_entities->update_position_by_offset( 0, sf::Vector2f(test_tile_map->tile_size_x * 2, 3 * test_tile_map->tile_size_y) );
  all_gameplay_entities->update_position_by_offset( 1, sf::Vector2f(test_tile_map->tile_size_x * 4, 5 * test_tile_map->tile_size_y) );
  all_gameplay_entities->update_position_by_offset( 2, sf::Vector2f(test_tile_map->tile_size_x * 6, 7 * test_tile_map->tile_size_y) );
  all_gameplay_entities->update_position_by_offset( 3, sf::Vector2f(test_tile_map->tile_size_x * 8, 3 * test_tile_map->tile_size_y) );
  all_gameplay_entities->update_position_by_offset( 4, sf::Vector2f(test_tile_map->tile_size_x * 3, 7 * test_tile_map->tile_size_y) );
  all_gameplay_entities->update_position_by_offset( 5, sf::Vector2f(test_tile_map->tile_size_x * 4, 7 * test_tile_map->tile_size_y) );
  all_gameplay_entities->update_position_by_offset( 6, sf::Vector2f(test_tile_map->tile_size_x * 7, 8 * test_tile_map->tile_size_y) );
  all_gameplay_entities->update_position_by_offset( 7, sf::Vector2f(test_tile_map->tile_size_x * 6, 9 * test_tile_map->tile_size_y) );
  all_gameplay_entities->update_position_by_offset( 8, sf::Vector2f(test_tile_map->tile_size_x * 11, 6 * test_tile_map->tile_size_y) );
  all_gameplay_entities->update_position_by_offset( 9, sf::Vector2f(test_tile_map->tile_size_x * 9, 4 * test_tile_map->tile_size_y) );
  all_gameplay_entities->update_position_by_offset( 10, sf::Vector2f(test_tile_map->tile_size_x * 2, 9 * test_tile_map->tile_size_y) );
  all_gameplay_entities->update_position_by_offset( 11, sf::Vector2f(test_tile_map->tile_size_x * 5, 4 * test_tile_map->tile_size_y) );
  all_gameplay_entities->update_position_by_offset( 12, sf::Vector2f(test_tile_map->tile_size_x * 13, 8 * test_tile_map->tile_size_y) );
  all_gameplay_entities->update_position_by_offset( 13, sf::Vector2f(test_tile_map->tile_size_x * 8, 5 * test_tile_map->tile_size_y) );
  all_gameplay_entities->update_position_by_offset( 14, sf::Vector2f(test_tile_map->tile_size_x * 4, 9 * test_tile_map->tile_size_y) );


  float timestep;
  float intersection_time;
  entity_collision_input* const all_entity_collision_data = new entity_collision_input[MAX_GAMEPLAY_ENTITIES];
  entity_collision* const all_entity_collisions = new entity_collision[TILE_MAP_COUNT * MAX_ENTITIES_PER_TILE];



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
                //if ( window_event.key.code == sf::Keyboard::P ) tile_to_gameplay_entities->print_tile_buckets();
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
      test_tile_map->bitmap[i] = static_cast<int>(tile_map_bitmap_type::WALL);
    }

    for(int i=0; i < test_tile_map->height; ++i)
    {
      test_tile_map->bitmap[i * test_tile_map->width] = static_cast<int>(tile_map_bitmap_type::WALL);
    }

    for(int i=(test_tile_map->tile_count - test_tile_map->width); i < test_tile_map->tile_count; ++i)
    {
      test_tile_map->bitmap[i] = static_cast<int>(tile_map_bitmap_type::WALL);
    }

    for(int i=0; i < test_tile_map->height; ++i)
    {
      test_tile_map->bitmap[(i * test_tile_map->width) + (test_tile_map->width - 1)] = static_cast<int>(tile_map_bitmap_type::WALL);
    }

    for(int tile_index = (test_tile_map->width * 2); tile_index < test_tile_map->tile_count; tile_index += (test_tile_map->width * 2))
    {
      for(int tile_index_offset=1; tile_index_offset < test_tile_map->width; ++tile_index_offset)
      {
        if (tile_index_offset % 2 == 0)
          test_tile_map->bitmap[tile_index + tile_index_offset] = static_cast<int>(tile_map_bitmap_type::WALL);
      }
    }

    test_tile_map->update_tex_coords_from_bitmap();



    /* collision update loop */

    tile_to_gameplay_entities->update(*test_tile_map, *all_gameplay_entities);
    sf::Vector2f velocity_cache[MAX_GAMEPLAY_ENTITIES];
    memcpy(&velocity_cache, all_gameplay_entities->velocities, sizeof(all_gameplay_entities->velocities));
    timestep = elapsed_frame_time_seconds;
    int chain_collision_count = 0;
    int collision_count;
    std::bitset<MAX_GAMEPLAY_ENTITIES> entity_is_wall;


    while ( (timestep > 0.0f) && (chain_collision_count < MAX_CHAIN_COLLISIONS) )
    {
      entity_is_wall.reset();

      // query for collision data
      for(int i=0; i < MAX_GAMEPLAY_ENTITIES; ++i)
      {
        all_entity_collision_data[i].velocity = all_gameplay_entities->velocities[i];

        all_entity_collision_data[i].collision_vertices[0] = all_gameplay_entities->collision_vertices[(i*4) + 0];
        all_entity_collision_data[i].collision_vertices[1] = all_gameplay_entities->collision_vertices[(i*4) + 1];
        all_entity_collision_data[i].collision_vertices[2] = all_gameplay_entities->collision_vertices[(i*4) + 2];
        all_entity_collision_data[i].collision_vertices[3] = all_gameplay_entities->collision_vertices[(i*4) + 3];
      }

      collision_count = calculate_collisions<MAX_GAMEPLAY_ENTITIES,MAX_ENTITIES_PER_TILE,TILE_MAP_WIDTH,TILE_MAP_HEIGHT>(all_entity_collision_data,all_entity_collisions,timestep,*test_tile_map,tile_to_gameplay_entities,all_gameplay_entities->is_garbage_flags);

      intersection_time = timestep;

      // find smallest collision time
      for(int i=0; i < collision_count; ++i)
      {
        if ( std::abs(all_entity_collisions[i].intersection_time) < intersection_time) intersection_time = all_entity_collisions[i].intersection_time;
      }

      all_gameplay_entities->update_positions_by_velocity(intersection_time);

      // resolve collisions
      for(int i=0; i < collision_count; ++i)
      {

        if (all_entity_collisions[i].intersection_time == intersection_time)
        {
          // set new velocites
          if( (all_entity_collisions[i].entity_ids[1] == -1) || (entity_is_wall[ all_entity_collisions[i].entity_ids[1] ]) ) // wall collision check
          {
            all_gameplay_entities->velocities[all_entity_collisions[i].entity_ids[0]] = sf::Vector2f(0.0f,0.0f);
            entity_is_wall[all_entity_collisions[i].entity_ids[0]] = true;
          }
          else if(all_entity_collisions[i].right_of_way_id >= 0)
          {
            int non_right_of_way_id = (all_entity_collisions[i].entity_ids[0] != all_entity_collisions[i].right_of_way_id) ? all_entity_collisions[i].entity_ids[0] : all_entity_collisions[i].entity_ids[1];

            all_gameplay_entities->velocities[non_right_of_way_id] = sf::Vector2f(0.0f,0.0f);
            entity_is_wall[non_right_of_way_id] = true;
          }
          else
          {
            int first_id  = all_entity_collisions[i].entity_ids[0];
            int second_id = all_entity_collisions[i].entity_ids[1];

            sf::Vector2f collision_velocity( (all_entity_collision_data[first_id].velocity.x + all_entity_collision_data[second_id].velocity.x) / 2.0f,
                                             (all_entity_collision_data[first_id].velocity.y + all_entity_collision_data[second_id].velocity.y) / 2.0f);

            all_gameplay_entities->velocities[first_id]  = collision_velocity;
            all_gameplay_entities->velocities[second_id] = collision_velocity;
          }

          // commit gameplay event?
        }
      }

      chain_collision_count += (collision_count + static_cast<int>(collision_count == 0)); // make sure at least increment by 1 !!! should probably change this later
      timestep -= intersection_time;
      tile_to_gameplay_entities->update(*test_tile_map, *all_gameplay_entities);
    }

    memcpy(all_gameplay_entities->velocities, &velocity_cache, sizeof(all_gameplay_entities->velocities));



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
