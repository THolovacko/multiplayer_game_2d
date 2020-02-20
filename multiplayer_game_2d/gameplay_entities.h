#pragma once

#include <SFML/Graphics.hpp>


template<int p_max_size>
struct gameplay_entities
{
  enum class type_row_index : int
  {
    NONE  = 0,
    MARIO = 1,
    BOMB  = 2
  };

  sf::Vertex vertex_buffer[p_max_size * 4]; // 4 vertices per entity
  sf::Texture sprite_sheet_texture;         // a sprite sheet where each row is a separate entity and each column is a different frame for an animation (the first row is transparent)
  const int sprite_sheet_side_length;       // the pixel length and width of each sprite
  const int max_size = p_max_size;

  type_row_index type[p_max_size] = {type_row_index::NONE}; // type of gameplay entity and used to specify row in sprite_sheet
  int animation_index[p_max_size] = {0};                    // current frame for animation
  sf::Vector2f velocity[p_max_size];
  bool is_garbage[p_max_size];                              // memory at this index is overwritable when true

  gameplay_entities(const char* sprite_sheet_texture_file_path, const int p_sprite_sheet_side_length) : sprite_sheet_side_length(p_sprite_sheet_side_length)
  {
    sprite_sheet_texture.loadFromFile(sprite_sheet_texture_file_path);

    for(auto& garbage_flag : is_garbage)
    {
      garbage_flag = true;
    }

    for(auto& default_velocity: velocity)
    {
      default_velocity = sf::Vector2f(0.0f,0.0f);
    }
  };

  void update_position_and_tex_coords()
  {
    // update positions * is_garbage (buffer overflow precaution?)
    // update animations * is_garbage (multiplying type by is_garbage will make transparent)
  };
};
