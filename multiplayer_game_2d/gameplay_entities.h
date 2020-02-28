#pragma once

#include <SFML/Graphics.hpp>

namespace
{
  sf::Vector2f velocity;
  float sprite_sheet_y_position;
  float sprite_sheet_x_position;
}

enum class gameplay_entities_type_and_sprite_sheet_row_index : int
{
  NONE  = 0,
  MARIO = 1,
  BOMB  = 2
};

template<int p_max_size>
struct gameplay_entities
{
  sf::Vertex vertex_buffer[p_max_size * 4]; // 4 vertices per entity
  sf::Texture sprite_sheet_texture;         // a sprite sheet where each row is a separate entity and each column is a different frame for an animation (the first row is transparent)
  const int sprite_sheet_side_length;       // the pixel length and width of each sprite
  const int max_size = p_max_size;
  const int vertice_count = p_max_size * 4; // 4 vertices per entity

  gameplay_entities_type_and_sprite_sheet_row_index types[p_max_size] = {gameplay_entities_type_and_sprite_sheet_row_index::NONE}; // type of gameplay entity that's also used to specify row in sprite_sheet
  int animation_indexes[p_max_size] = {0};                                                                       // current frame for animation
  sf::Vector2f velocities[p_max_size];                                                                           // how many pixels per second to move in x and y directions
  bool is_garbage_flags[p_max_size];                                                                             // memory at this index is overwritable when true

  gameplay_entities(const char* sprite_sheet_texture_file_path, const int p_sprite_sheet_side_length) : sprite_sheet_side_length(p_sprite_sheet_side_length)
  {
    sprite_sheet_texture.loadFromFile(sprite_sheet_texture_file_path);

    for(auto& garbage_flag : is_garbage_flags)
    {
      garbage_flag = true;
    }

    // need guarantee default values in case some memory never filled
    for(auto& vertex : vertex_buffer)
    {
      vertex.position  = sf::Vector2f(0.0f, 0.0f);
      vertex.texCoords = sf::Vector2f(0.0f,0.0f);
    }

    for(auto& default_velocity: velocities)
    {
      default_velocity = sf::Vector2f(0.0f,0.0f);
    }
  }

  void update_positions_and_tex_coords(const float elapsed_frame_time_seconds)
  {
    // update positions based on velocity
    for(int i=0,vertex=0; i < max_size; ++i,vertex += 4)
    {
      velocity = velocities[i] * (elapsed_frame_time_seconds * !is_garbage_flags[i]);

      vertex_buffer[vertex].position   += velocity;
      vertex_buffer[vertex+1].position += velocity;
      vertex_buffer[vertex+2].position += velocity;
      vertex_buffer[vertex+3].position += velocity;
    }

    // update tex coords based on type and animation index
    for(int i=0,vertex=0; i < max_size; ++i,vertex += 4)
    {
      sprite_sheet_y_position = (float) static_cast<int>(types[i]) * (sprite_sheet_side_length * !is_garbage_flags[i]);
      sprite_sheet_x_position = (float) animation_indexes[i] * sprite_sheet_side_length * !is_garbage_flags[i];

      vertex_buffer[vertex].texCoords   = sf::Vector2f(sprite_sheet_x_position, sprite_sheet_y_position);
      vertex_buffer[vertex+1].texCoords = sf::Vector2f(sprite_sheet_x_position + sprite_sheet_side_length, sprite_sheet_y_position);
      vertex_buffer[vertex+2].texCoords = sf::Vector2f(sprite_sheet_x_position + sprite_sheet_side_length, sprite_sheet_y_position + sprite_sheet_side_length);
      vertex_buffer[vertex+3].texCoords = sf::Vector2f(sprite_sheet_x_position, sprite_sheet_y_position + sprite_sheet_side_length);
    }
  }
};
