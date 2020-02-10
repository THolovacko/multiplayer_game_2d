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
  sf::Vector2f tile_size((float) window_size.x / 16.0f,(float) window_size.y / 9.0f); 



  sf::Texture floor_texture;
  sf::Sprite  floor;
  floor_texture.loadFromFile("Assets/Images/floor.png");
  sf::Vector2u floor_texture_size = floor_texture.getSize();
  float floor_scale_x = (float) window_size.x / floor_texture_size.x;
  float floor_scale_y = (float) window_size.y / floor_texture_size.y;
  floor.setTexture(floor_texture);
  floor.setScale(floor_scale_x, floor_scale_y);

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

  sf::Texture floor_aesthetic_texture;
  floor_aesthetic_texture.loadFromFile("Assets/Images/floor_aesthetic.png");
  sf::VertexArray floor_aesthetic_layer(sf::Quads, 576); // 144 *4 = 576

  tile_map<9,16> test("Assets/Images/floor.png", "Assets/Images/floor_aesthetic.png", (float) window_size.x, (float) window_size.y);

  int floor_aesthetic_bitmap[144] = {0};
  floor_aesthetic_bitmap[0] = 0;
  floor_aesthetic_bitmap[1] = 1;
  floor_aesthetic_bitmap[2] = 2;
  floor_aesthetic_bitmap[3] = 3;
  floor_aesthetic_bitmap[4] = 3;
  floor_aesthetic_bitmap[5] = 2;
  floor_aesthetic_bitmap[6] = 1;
  floor_aesthetic_bitmap[7] = 0;
  floor_aesthetic_bitmap[15] = 2;
  floor_aesthetic_bitmap[143] = 2;

  // for each vertex in vertex array, assign screen coordinates
  for(int y=0,vertex=0; y < 9 ; ++y)
  for(int x=0         ; x < 16; ++x, vertex+=4)
  {
    floor_aesthetic_layer[vertex].position   = sf::Vector2f(x * tile_size.x, y * tile_size.y);
    floor_aesthetic_layer[vertex+1].position = sf::Vector2f((x+1) * tile_size.x, y * tile_size.y);
    floor_aesthetic_layer[vertex+2].position = sf::Vector2f((x+1) * tile_size.x, (y+1) * tile_size.y);
    floor_aesthetic_layer[vertex+3].position = sf::Vector2f(x * tile_size.x, (y+1) * tile_size.y);
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
              if (window_event.key.code == sf::Keyboard::Left)   local_player.move((float) -1 * tile_size.x, 0.0f  );
              if (window_event.key.code == sf::Keyboard::Right)  local_player.move((float) tile_size.x, 0.0f  );
              if (window_event.key.code == sf::Keyboard::Up)     local_player.move(0.0f, (float) -1 * tile_size.x);
              if (window_event.key.code == sf::Keyboard::Down)   local_player.move(0.0f, (float) tile_size.x);
              if (window_event.key.code == sf::Keyboard::T)      tingling.play();
              break;

        case sf::Event::KeyReleased:
              break;

        default:
              break;
      }
    }



    /* calculate gameplay stuff */

    // build floor aesthetic layer
    // for each value in bitmap: assign texture coordinates (texture coordinates depend on bitmap) (probably only texture coordinates ever change in future games)
    for(int tile=0, vertex=0,texture_offset=0; tile < 144; ++tile, vertex+=4)
    {
      texture_offset = floor_aesthetic_bitmap[tile] * 64;

      floor_aesthetic_layer[vertex].texCoords   = sf::Vector2f((float) texture_offset        , 0.0f);
      floor_aesthetic_layer[vertex+1].texCoords = sf::Vector2f((float) texture_offset + 64.0f, 0.0f);
      floor_aesthetic_layer[vertex+2].texCoords = sf::Vector2f((float) texture_offset + 64.0f, 64.0f);
      floor_aesthetic_layer[vertex+3].texCoords = sf::Vector2f((float) texture_offset        , 64.0f);
    }

    bomb.move(0.0f, 100.0f * elapsed_frame_time_seconds);



    /* draw */
    window.clear(sf::Color::Black);

    // draw map
    window.draw(floor);
    window.draw(floor_aesthetic_layer, &floor_aesthetic_texture);
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
