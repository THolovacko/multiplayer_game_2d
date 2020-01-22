#include <SFML/Window.hpp>
#include <iostream>
#include <windows.h>

int main()
{
	// create window
	sf::VideoMode highest_resolution_video_mode;
	{
		std::vector<sf::VideoMode> full_screen_modes = sf::VideoMode::getFullscreenModes();
		if (full_screen_modes.size() == 0)
		{
			std::cout << "no full screen modes returned from sf::VideoMode::getFullscreenModes()";
			return 0;
		}
		highest_resolution_video_mode = full_screen_modes[0];
	}
	sf::Window window(highest_resolution_video_mode, "2D Multiplayer Game", sf::Style::Fullscreen);
	window.setVerticalSyncEnabled(true);


  // setup and run game loop
	sf::Event window_event;
	sf::Clock clock;
	sf::Time  elapsed_frame_time;
	sf::Int32	elapsed_frame_time_milliseconds;
	sf::Int64	elapsed_frame_time_microseconds;
  float     elapsed_frame_time_seconds;

  while (window.isOpen())
  {
		elapsed_frame_time = clock.restart();
		elapsed_frame_time_milliseconds = elapsed_frame_time.asMilliseconds();
		elapsed_frame_time_microseconds = elapsed_frame_time.asMicroseconds();
    elapsed_frame_time_seconds      = elapsed_frame_time.asSeconds();
		std::cout << elapsed_frame_time_seconds << std::endl;

    while (window.pollEvent(window_event))
    {
      if (window_event.type == sf::Event::Closed) window.close();
    }

    if (elapsed_frame_time_seconds <= 0.0f) Sleep(1);
  } // end of game loop

  return 0;
}
