#pragma once

#include <SFML/Graphics.hpp>

namespace
{
  sf::Vector2f current_velocity;
  float current_sprite_sheet_y_position;
  float current_sprite_sheet_x_position;
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
  sf::Vertex vertex_buffer[p_max_size * 4];           // 4 vertices per entity
  sf::Texture sprite_sheet_texture;                   // a sprite sheet where each row is a separate entity and each column is a different frame for an animation (the first row is transparent)
  sf::Vector2f collision_vertices[p_max_size * 4];    // 4 vertices per entity [top-left, top-right, bottom-right, bottom-left]
  const int sprite_sheet_side_length;                 // the pixel length and width of each entity animation frame
  const int max_size = p_max_size;
  const int vertice_count = p_max_size * 4;           // 4 vertices per entity

  gameplay_entities_type_and_sprite_sheet_row_index types[p_max_size] = {gameplay_entities_type_and_sprite_sheet_row_index::NONE}; // type of gameplay entity that's also used to specify row in sprite_sheet
  int animation_indexes[p_max_size] = {0};                                                                                         // current frame for animation
  sf::Vector2f velocities[p_max_size];                                                                                             // how many pixels per second to move in x and y directions
  bool is_garbage_flags[p_max_size];                                                                                               // memory at this index is overwritable when true


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
      vertex.texCoords = sf::Vector2f(0.0f, 0.0f);
    }

    for(auto& default_velocity: velocities)
    {
      default_velocity = sf::Vector2f(0.0f, 0.0f);
    }

    for(auto& collision_vertice : collision_vertices)
    {
      collision_vertice = sf::Vector2f(0.0f, 0.0f);
    }
  }

  void update_positions_and_tex_coords(const float elapsed_frame_time_seconds)
  {
    // update positions based on velocity
    for(int entity_index=0,vertex=0; entity_index < max_size; ++entity_index,vertex += 4)
    {
      current_velocity = velocities[entity_index] * (elapsed_frame_time_seconds * !is_garbage_flags[entity_index]);

      vertex_buffer[vertex].position   += current_velocity;
      vertex_buffer[vertex+1].position += current_velocity;
      vertex_buffer[vertex+2].position += current_velocity;
      vertex_buffer[vertex+3].position += current_velocity;

      collision_vertices[vertex]   += current_velocity;
      collision_vertices[vertex+1] += current_velocity;
      collision_vertices[vertex+2] += current_velocity;
      collision_vertices[vertex+3] += current_velocity;
    }

    // update tex coords based on type and animation index
    for(int entity_index=0,vertex=0; entity_index < max_size; ++entity_index,vertex += 4)
    {
      current_sprite_sheet_y_position = (float) (sprite_sheet_texture.getSize().y - sprite_sheet_side_length) - static_cast<int>(types[entity_index]) * sprite_sheet_side_length * !is_garbage_flags[entity_index];
      current_sprite_sheet_x_position = (float) animation_indexes[entity_index] * sprite_sheet_side_length * !is_garbage_flags[entity_index];

      vertex_buffer[vertex].texCoords   = sf::Vector2f(current_sprite_sheet_x_position, current_sprite_sheet_y_position);
      vertex_buffer[vertex+1].texCoords = sf::Vector2f(current_sprite_sheet_x_position + sprite_sheet_side_length, current_sprite_sheet_y_position);
      vertex_buffer[vertex+2].texCoords = sf::Vector2f(current_sprite_sheet_x_position + sprite_sheet_side_length, current_sprite_sheet_y_position + sprite_sheet_side_length);
      vertex_buffer[vertex+3].texCoords = sf::Vector2f(current_sprite_sheet_x_position, current_sprite_sheet_y_position + sprite_sheet_side_length);
    }
  }
};
