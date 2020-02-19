#pragma once

#include <SFML/Graphics.hpp>


template<int p_max_size>
struct gameplay_entities
{
  sf::Vertex vertex_buffer[p_max_size * 4]; // 4 vertices per entity
  sf::Texture sprite_sheet_texture;         // a sprite sheet where each row is a separate entity and each column is a different frame for an animation
  const int sprite_sheet_side_length;       // the pixel length and width of each sprite
  const int max_size = p_max_size;

  bool is_garbage[p_max_size];      // memory at this index is overwritable
  int animation_index[p_max_size];  // current frame for animation
  sf::Vector2f velocity[p_max_size];
  // type enum

  gameplay_entities() {};
};
