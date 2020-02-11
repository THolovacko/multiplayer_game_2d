#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <windows.h>
#include "tile_map.h"

int main()
{
  /* create window */
  sf::VideoMode desktop_video_mode = sf::VideoMode::getDesktopMode();
  sf::RenderWindow window(desktop_video_mode, "2D Multiplayer Game", sf::Style::Fullscreen);
  window.setVerticalSyncEnabled(true);
  window.setActive(true);
  sf::Vector2u window_size = window.getSize();

  sf::Texture local_player_texture;
  sf::Sprite  local_player;
  local_player_texture.loadFromFile("Assets/Images/mario.png");
  sf::Vector2u local_player_texture_size = local_player_texture.getSize();
  float local_player_scale_x = (float) (window_size.x / 16.0f) / local_player_texture_size.x;
  float local_player_scale_y = (float) (window_size.y / 16.0f) / local_player_texture_size.y;
  local_player.setTexture(local_player_texture);
  local_player.setScale(local_player_scale_x, local_player_scale_y);

  sf::Texture bomb_texture;
  sf::Sprite  bomb;
  bomb_texture.loadFromFile("Assets/Images/bomb.png");
  sf::Vector2u bomb_texture_size = bomb_texture.getSize();
  float bomb_scale_x = (float) (window_size.x / 16.0f) / bomb_texture_size.x;
  float bomb_scale_y = (float) (window_size.y / 16.0f) / bomb_texture_size.y;
  bomb.setTexture(bomb_texture);
  bomb.setScale(bomb_scale_x, bomb_scale_y);

  sf::SoundBuffer tingling_sound_buffer;
  tingling_sound_buffer.loadFromFile("Assets/Sounds/tingling.wav");
  sf::Sound tingling;
  tingling.setBuffer(tingling_sound_buffer);
  
  tile_map<16,9> test("Assets/Images/floor.png", "Assets/Images/floor_aesthetic.png", (float) window_size.x, (float) window_size.y, 64);

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
              if (window_event.key.code == sf::Keyboard::Left)   local_player.move((float) -1 * test.tile_size_x, 0.0f  );
              if (window_event.key.code == sf::Keyboard::Right)  local_player.move((float) test.tile_size_x, 0.0f  );
              if (window_event.key.code == sf::Keyboard::Up)     local_player.move(0.0f, (float) -1 * test.tile_size_y);
              if (window_event.key.code == sf::Keyboard::Down)   local_player.move(0.0f, (float) test.tile_size_y);
              if (window_event.key.code == sf::Keyboard::T)      tingling.play();
              break;

        case sf::Event::KeyReleased:
              break;

        default:
              break;
      }
    }



    /* calculate gameplay stuff */

    test.bitmap[0] = 0;
    test.bitmap[1] = 1;
    test.bitmap[2] = 2;
    test.bitmap[3] = 3;
    test.bitmap[4] = 3;
    test.bitmap[5] = 2;
    test.bitmap[6] = 1;
    test.bitmap[7] = 0;
    test.bitmap[15] = 2;
    test.bitmap[143] = 2;


    test.update_tex_coords_from_bitmap();

    bomb.move(0.0f, 100.0f * elapsed_frame_time_seconds);


    /* draw */
    window.clear(sf::Color::Black);

    // draw map

    window.draw(test.vertex_buffer, 4, sf::Quads, &test.background_texture);
    window.draw(test.vertex_buffer, test.vertice_count, sf::Quads, &test.tiles_texture);
    // draw entities
    // draw HUD (if decided to have static HUD)
    // draw options if requested

    window.draw(local_player);
    window.draw(bomb);

    window.display();


    // handle underflow
    if (elapsed_frame_time_seconds == 0.0f) Sleep(1);
  } // end of game loop

  return 0;
}
