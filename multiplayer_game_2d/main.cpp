#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <string.h>
#include <windows.h>
#include <bitset>
#include <cmath>
#include <cstdlib>
#include <time.h>
#include "gameplay.h"


#pragma warning(disable : 26812)


// @optimize: should decrease MAX_ENTITIES_PER_TILE when game is finished or when related design decisions are final
#define TILE_MAP_WIDTH              17
#define TILE_MAP_HEIGHT             11
#define TILE_MAP_COUNT              (TILE_MAP_WIDTH * TILE_MAP_HEIGHT)
#define MAX_GAMEPLAY_ENTITIES       TILE_MAP_COUNT
#define TILE_MAP_TEXTURE_SIDE_SIZE  64                                  // in pixels
#define MAX_ENTITIES_PER_TILE       10                                  // potential game objects in an single tile
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

  for(int i=1; i < test_tile_map->height; ++i)
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
  all_gameplay_entities->is_garbage_flags[2]  = false;
  all_gameplay_entities->is_garbage_flags[3]  = false;
  all_gameplay_entities->is_garbage_flags[4]  = false;
  all_gameplay_entities->is_garbage_flags[5]  = false;
  all_gameplay_entities->is_garbage_flags[6]  = false;
  all_gameplay_entities->is_garbage_flags[7]  = false;
  all_gameplay_entities->is_garbage_flags[8]  = false;
  all_gameplay_entities->is_garbage_flags[9]  = false;
  all_gameplay_entities->is_garbage_flags[10] = false;
  all_gameplay_entities->is_garbage_flags[11] = false;
  all_gameplay_entities->is_garbage_flags[12] = false;
  all_gameplay_entities->is_garbage_flags[13] = false;
  all_gameplay_entities->is_garbage_flags[14] = false;
  all_gameplay_entities->is_garbage_flags[15] = false;
  all_gameplay_entities->is_garbage_flags[16] = false;
  all_gameplay_entities->is_garbage_flags[17] = false;
  all_gameplay_entities->is_garbage_flags[18] = false;
  all_gameplay_entities->is_garbage_flags[19] = false;


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
  all_gameplay_entities->types[14] = gameplay_entity_type::BOMB;
  all_gameplay_entities->types[15] = gameplay_entity_type::BOMB;
  all_gameplay_entities->types[16] = gameplay_entity_type::BOMB;
  all_gameplay_entities->types[17] = gameplay_entity_type::BOMB;
  all_gameplay_entities->types[18] = gameplay_entity_type::BOMB;
  all_gameplay_entities->types[19] = gameplay_entity_type::BOMB;
 
  all_gameplay_entities->animation_indexes[0] = 0;
  all_gameplay_entities->animation_indexes[1] = 0;
  all_gameplay_entities->animation_indexes[2] = 0;

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
    all_gameplay_entities->collision_vertices[i]   = sf::Vector2f(1.0f, 1.0f);
    all_gameplay_entities->collision_vertices[i+1] = sf::Vector2f(test_tile_map->tile_size_x - 1.0f, 1.0f);
    all_gameplay_entities->collision_vertices[i+2] = sf::Vector2f(test_tile_map->tile_size_x - 1.0f, test_tile_map->tile_size_y - 1.0f);
    all_gameplay_entities->collision_vertices[i+3] = sf::Vector2f(1.0f, test_tile_map->tile_size_y - 1.0f);
  }

  // set spawn positions
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
  all_gameplay_entities->update_position_by_offset( 15, sf::Vector2f(test_tile_map->tile_size_x * 7, 3 * test_tile_map->tile_size_y) );
  all_gameplay_entities->update_position_by_offset( 16, sf::Vector2f(test_tile_map->tile_size_x * 6, 5 * test_tile_map->tile_size_y) );
  all_gameplay_entities->update_position_by_offset( 17, sf::Vector2f(test_tile_map->tile_size_x * 5, 9 * test_tile_map->tile_size_y) );
  all_gameplay_entities->update_position_by_offset( 18, sf::Vector2f(test_tile_map->tile_size_x * 5, 7 * test_tile_map->tile_size_y) );
  all_gameplay_entities->update_position_by_offset( 19, sf::Vector2f(test_tile_map->tile_size_x * 5, 1 * test_tile_map->tile_size_y) );


  // initialize gameplay_entity moves
  gameplay_entity_moves<MAX_GAMEPLAY_ENTITIES,TILE_MAP_WIDTH,TILE_MAP_HEIGHT>* all_entity_moves = new gameplay_entity_moves<MAX_GAMEPLAY_ENTITIES,TILE_MAP_WIDTH,TILE_MAP_HEIGHT>( all_gameplay_entities->all_collision_vertices_origin_positions(), all_gameplay_entities->is_garbage_flags, *test_tile_map);
  gameplay_entity_move_request* all_move_requests = new gameplay_entity_move_request[MAX_GAMEPLAY_ENTITIES];

  gameplay_entity_move_request player_move_request;



  /* setup and run game loop */
  sf::Event window_event;
  sf::Clock clock;
  sf::Time  elapsed_frame_time;
  sf::Int32 elapsed_frame_time_milliseconds;
  sf::Int64 elapsed_frame_time_microseconds;
  float     elapsed_frame_time_seconds;

  srand(static_cast<unsigned int>(time(NULL))); // @optimize: randomn values should probably be pre-generated or at least only generated once


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

    // reset all move requests
    for (int i = 0; i < MAX_GAMEPLAY_ENTITIES; ++i) all_move_requests[i].velocity = sf::Vector2f(0.0f,0.0f);



    /* get input and events */
    while (window.pollEvent(window_event))
    {
      switch (window_event.type)
      {
        case sf::Event::Closed:
              window.close();
              break;

        case sf::Event::KeyPressed:
              if (window_event.key.code == sf::Keyboard::P)      // use for testing random stuff
                                                                 all_gameplay_entities->animation_indexes[1] = (all_gameplay_entities->animation_indexes[1] + 1) % 3;
              if (window_event.key.code == sf::Keyboard::T)      tingling.play();
              break;

        case sf::Event::KeyReleased:
              #ifdef _DEBUG
                if ( window_event.key.code == sf::Keyboard::D ) show_debug_data = !show_debug_data;
                //if ( window_event.key.code == sf::Keyboard::P ) tile_to_gameplay_entities->print_tile_buckets();
              #endif

              break;

        default:
              break;
      }
    }

    if      (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))   player_move_request.velocity = sf::Vector2f( (float) -3 * test_tile_map->tile_size_x, 0.0f );
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))  player_move_request.velocity = sf::Vector2f( (float) 3 * test_tile_map->tile_size_x, 0.0f  );
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))     player_move_request.velocity = sf::Vector2f( 0.0f, (float) -3 * test_tile_map->tile_size_y );
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))   player_move_request.velocity = sf::Vector2f( 0.0f, (float)  3 * test_tile_map->tile_size_y );



    /* calculate gameplay stuff */
    player_move_request.current_origin_position = all_gameplay_entities->collision_vertices[0];

    // if already moving and passed distance threshold then chamber move else if already moving do nothing else if stationary then move player
    if ( (all_entity_moves->velocities[0].x && (std::abs(all_entity_moves->destination_origin_positions[0].x - all_entity_moves->current_origin_positions[0].x) >= (test_tile_map->tile_size_x / 3.0f)) ) ||
         (all_entity_moves->velocities[0].y && (std::abs(all_entity_moves->destination_origin_positions[0].y - all_entity_moves->current_origin_positions[0].y) >= (test_tile_map->tile_size_y / 3.0f)) ) )
    {
      if (player_move_request.velocity.x > 0) player_move_request.destination_origin_position = player_move_request.current_origin_position + sf::Vector2f(test_tile_map->tile_size_x,0.0f);
      if (player_move_request.velocity.x < 0) player_move_request.destination_origin_position = player_move_request.current_origin_position + sf::Vector2f(-1.0f * test_tile_map->tile_size_x, 0.0f);
      if (player_move_request.velocity.y > 0) player_move_request.destination_origin_position = player_move_request.current_origin_position + sf::Vector2f(0.0f,test_tile_map->tile_size_y);
      if (player_move_request.velocity.y < 0) player_move_request.destination_origin_position = player_move_request.current_origin_position + sf::Vector2f(0.0f,-1.0f * test_tile_map->tile_size_y);

      if (player_move_request.velocity.x || player_move_request.velocity.y)
      {
        all_move_requests[0] = player_move_request;
      }
    }
    else if ( !(all_entity_moves->velocities[0].x) && !(all_entity_moves->velocities[0].y))
    {
      if (player_move_request.velocity.x > 0) player_move_request.destination_origin_position = player_move_request.current_origin_position + sf::Vector2f(test_tile_map->tile_size_x,0.0f);
      if (player_move_request.velocity.x < 0) player_move_request.destination_origin_position = player_move_request.current_origin_position + sf::Vector2f(-1.0f * test_tile_map->tile_size_x, 0.0f);
      if (player_move_request.velocity.y > 0) player_move_request.destination_origin_position = player_move_request.current_origin_position + sf::Vector2f(0.0f,test_tile_map->tile_size_y);
      if (player_move_request.velocity.y < 0) player_move_request.destination_origin_position = player_move_request.current_origin_position + sf::Vector2f(0.0f,-1.0f * test_tile_map->tile_size_y);

      if (player_move_request.velocity.x || player_move_request.velocity.y)
      {
        all_move_requests[0] = player_move_request;
      }
    }
    else player_move_request.velocity = sf::Vector2f(0.0f, 0.0f); // reset chamber
 

    // generate test movement requests
    generate_move_request_input stress_test_move_requests[MAX_GAMEPLAY_ENTITIES];
    for(int i=0; i < 14; ++i)
    {
      int random_number = (rand() % 10 + 1);
      stress_test_move_requests[i].gameplay_entity_id = (i+1);

      float x = 0.0f;
      float y = 0.0f;
      // decide axis
      (random_number > 5) ? x = 1.0f : y = 1.0f;

      // decide sign
      random_number = (rand() % 10 + 1);
      if(random_number > 5) { x *= -1.0f; y *= -1.0f; }

      // decide magnitude
      random_number = 10;//(rand() % 10 + 1);
      x *= ( static_cast<float>(random_number) * 25.0f);
      y *= ( static_cast<float>(random_number) * 25.0f);

      stress_test_move_requests[i].velocity = sf::Vector2f(x,y);
    }
    all_gameplay_entities->generate_move_requests(stress_test_move_requests,all_move_requests, 14,test_tile_map->tile_size_x,test_tile_map->tile_size_y);


    // update movement
    all_entity_moves->submit_all_moves(all_move_requests, *test_tile_map, all_gameplay_entities->is_garbage_flags);
    all_entity_moves->update_by_velocities(elapsed_frame_time_seconds, *test_tile_map);
    all_gameplay_entities->set_all_positions(all_entity_moves->current_origin_positions);


    tile_to_gameplay_entities->update(*test_tile_map, *all_gameplay_entities);


    // handle tile_map triggers



    /* draw */
    test_tile_map->update_tex_coords_from_bitmap();
    all_gameplay_entities->update_tex_coords(elapsed_frame_time_seconds);

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
