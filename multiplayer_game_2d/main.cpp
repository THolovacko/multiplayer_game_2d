#include <SFML/Graphics.hpp>
#include <iostream>
#include <windows.h>

int main()
{
  // create window
  sf::VideoMode desktop_video_mode = sf::VideoMode::getDesktopMode();
  sf::RenderWindow window(desktop_video_mode, "2D Multiplayer Game", sf::Style::Fullscreen);
  window.setVerticalSyncEnabled(true);
  window.setActive(true);
  sf::Vector2u window_size = window.getSize();



  sf::Texture background_texture;
  sf::Sprite  background;
  background_texture.loadFromFile("Assets/Images/bomberman_map.png");
  sf::Vector2u background_texture_size = background_texture.getSize();
  float background_scale_x = (float) window_size.x / background_texture_size.x;
  float background_scale_y = (float) window_size.y / background_texture_size.y;
  background.setTexture(background_texture);
  background.setScale(background_scale_x, background_scale_y);

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



  // setup and run game loop
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



    // get input and events
    while (window.pollEvent(window_event))
    {
      if (window_event.type == sf::Event::Closed) window.close();
    }



    // calculate gameplay stuff
    bomb.move(0.0f,100.0f * elapsed_frame_time_seconds);



    // draw
    window.clear(sf::Color::Black);
    window.draw(background);
    window.draw(local_player);
    window.draw(bomb);
    window.display();


    // handle underflow
    if (elapsed_frame_time_seconds == 0.0f) Sleep(1);
  } // end of game loop

  return 0;
}
