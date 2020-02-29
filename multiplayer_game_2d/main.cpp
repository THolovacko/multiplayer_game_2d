#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <windows.h>
#include "tile_map.h"
#include "gameplay_entities.h"


int main()
{
  /* create window */
  sf::VideoMode desktop_video_mode = sf::VideoMode::getDesktopMode();
  sf::RenderWindow window(desktop_video_mode, "2D Multiplayer Game", sf::Style::Fullscreen);
  window.setVerticalSyncEnabled(true);
  window.setActive(true);
  sf::Vector2u window_size = window.getSize();

  sf::SoundBuffer tingling_sound_buffer;
  tingling_sound_buffer.loadFromFile("Assets/Sounds/tingling.wav");
  sf::Sound tingling;
  tingling.setBuffer(tingling_sound_buffer);
  
  tile_map<16,9> test_map_background("Assets/Images/test_map_background.png", (float) window_size.x, (float) window_size.y, 64);
  gameplay_entities<5> entities("Assets/Images/gameplay_entities.png", 192); // (192 = 64 * 3)

  entities.is_garbage_flags[0] = false;
  entities.is_garbage_flags[1] = false;
  entities.is_garbage_flags[2] = false;
  entities.is_garbage_flags[3] = false;
  entities.is_garbage_flags[4] = false;
  entities.types[0] = gameplay_entities_type_and_sprite_sheet_row_index::MARIO;
  entities.types[1] = gameplay_entities_type_and_sprite_sheet_row_index::BOMB;
  entities.types[2] = gameplay_entities_type_and_sprite_sheet_row_index::BOMB;
  entities.types[3] = gameplay_entities_type_and_sprite_sheet_row_index::BOMB;
  entities.types[4] = gameplay_entities_type_and_sprite_sheet_row_index::BOMB;
  entities.animation_indexes[0] = 2;
  entities.animation_indexes[1] = 1;
  entities.animation_indexes[2] = 2;
  entities.velocities[0] = sf::Vector2f(0.0f, 0.0f);
  entities.velocities[1] = sf::Vector2f(64.0f,0.0f);
  entities.velocities[2] = sf::Vector2f(16.0f,32.0f);
  entities.velocities[3] = sf::Vector2f(32.0f,100.0f);
  entities.velocities[4] = sf::Vector2f(0.0f,16.0f);

  // initialize entity positions to (0,0) origin
  for(int i=0; i < entities.vertice_count; i+=4)
  {
    entities.vertex_buffer[i].position   = sf::Vector2f(-test_map_background.tile_size_x, -test_map_background.tile_size_y * 2);
    entities.vertex_buffer[i+1].position = sf::Vector2f(2 *test_map_background.tile_size_x, -test_map_background.tile_size_y * 2);
    entities.vertex_buffer[i+2].position = sf::Vector2f(test_map_background.tile_size_x * 2, test_map_background.tile_size_y);
    entities.vertex_buffer[i+3].position = sf::Vector2f(-test_map_background.tile_size_x, test_map_background.tile_size_y);
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
    std::cout << "elapsed_frame_time_milliseconds: " << elapsed_frame_time_milliseconds << std::endl;



    /* get input and events */
    while (window.pollEvent(window_event))
    {
      switch (window_event.type)
      {
        case sf::Event::Closed:
              window.close();
              break;

        case sf::Event::KeyPressed:
              if (window_event.key.code == sf::Keyboard::Left)   entities.velocities[0] = sf::Vector2f( (float) -2 * test_map_background.tile_size_x, 0.0f );
              if (window_event.key.code == sf::Keyboard::Right)  entities.velocities[0] = sf::Vector2f( (float) 2 * test_map_background.tile_size_x, 0.0f  );
              if (window_event.key.code == sf::Keyboard::Up)     entities.velocities[0] = sf::Vector2f( 0.0f, (float) -2 * test_map_background.tile_size_y );
              if (window_event.key.code == sf::Keyboard::Down)   entities.velocities[0] = sf::Vector2f( 0.0f, (float)  2 * test_map_background.tile_size_y );
              if (window_event.key.code == sf::Keyboard::T)      tingling.play();
              break;

        case sf::Event::KeyReleased:
              break;

        default:
              break;
      }
    }



    /* calculate gameplay stuff */
    test_map_background.bitmap[0] = 0;
    test_map_background.bitmap[1] = 1;
    test_map_background.bitmap[2] = 2;
    test_map_background.bitmap[3] = 3;
    test_map_background.bitmap[4] = 3;
    test_map_background.bitmap[5] = 2;
    test_map_background.bitmap[6] = 1;
    test_map_background.bitmap[7] = 0;
    test_map_background.bitmap[15] = 2;
    test_map_background.bitmap[143] = 2;
    test_map_background.update_tex_coords_from_bitmap();

    entities.update_positions_and_tex_coords(elapsed_frame_time_seconds);

    /* draw */
    window.clear(sf::Color::Black);

    // draw map
    window.draw(test_map_background.vertex_buffer, test_map_background.vertice_count, sf::Quads, &test_map_background.tiles_texture);
    // draw entities
    window.draw(entities.vertex_buffer, entities.vertice_count, sf::Quads, &entities.sprite_sheet_texture);
    // draw HUD (if decided to have static HUD)
    // draw options if requested

    window.display();



    // handle underflow
    if (elapsed_frame_time_seconds == 0.0f) Sleep(1);
  } // end of game loop

  return 0;
}
