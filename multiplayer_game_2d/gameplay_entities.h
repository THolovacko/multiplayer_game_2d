#pragma once

#include <SFML/Graphics.hpp>

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
  const int vertex_count = p_max_size * 4;            // 4 vertices per entity

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

  #ifdef _DEBUG
    #include <string.h>

    sf::VertexArray* generate_debug_collision_line_vertices(const sf::Color color) const
    {
      static int debug_collision_line_vertex_count = p_max_size * 8; // 4 lines per entity and 2 vertices per line so 8 vertices per entity
      static sf::VertexArray debug_collision_line_vertices(sf::Lines, debug_collision_line_vertex_count);

      debug_collision_line_vertices.clear();
      debug_collision_line_vertices.resize(debug_collision_line_vertex_count);

      for(size_t i=0,entity_vertex=0; i < debug_collision_line_vertex_count; i+=8,entity_vertex+=4)
      {
        size_t current_entity_id = entity_vertex / 4;
        if(is_garbage_flags[current_entity_id] == true) continue;

        debug_collision_line_vertices[i].position   = this->collision_vertices[entity_vertex];
        debug_collision_line_vertices[i+1].position = this->collision_vertices[entity_vertex+1];
        debug_collision_line_vertices[i].color      = color;
        debug_collision_line_vertices[i+1].color    = color;

        debug_collision_line_vertices[i+2].position = this->collision_vertices[entity_vertex+1];
        debug_collision_line_vertices[i+3].position = this->collision_vertices[entity_vertex+2];
        debug_collision_line_vertices[i+2].color    = color;
        debug_collision_line_vertices[i+3].color    = color;

        debug_collision_line_vertices[i+4].position = this->collision_vertices[entity_vertex+2];
        debug_collision_line_vertices[i+5].position = this->collision_vertices[entity_vertex+3];
        debug_collision_line_vertices[i+4].color    = color;
        debug_collision_line_vertices[i+5].color    = color;

        debug_collision_line_vertices[i+6].position = this->collision_vertices[entity_vertex+3];
        debug_collision_line_vertices[i+7].position = this->collision_vertices[entity_vertex];
        debug_collision_line_vertices[i+6].color    = color;
        debug_collision_line_vertices[i+7].color    = color;
      }

      return &debug_collision_line_vertices;
    }

    sf::VertexArray* generate_debug_line_vertices(const sf::Color color) const
    {
      static int debug_line_vertex_count = max_size * 8; // 4 lines per entity and 2 vertices per line so 8 vertices per entity
      static sf::VertexArray debug_line_vertices(sf::Lines, debug_line_vertex_count);

      debug_line_vertices.clear();
      debug_line_vertices.resize(debug_line_vertex_count);

      for(size_t i=0,entity_vertex=0; i < debug_line_vertex_count; i+=8,entity_vertex+=4)
      {
        size_t current_entity_id = entity_vertex / 4;
        if(is_garbage_flags[current_entity_id] == true) continue;

        debug_line_vertices[i].position   = this->vertex_buffer[entity_vertex].position;
        debug_line_vertices[i+1].position = this->vertex_buffer[entity_vertex+1].position;
        debug_line_vertices[i].color      = color;
        debug_line_vertices[i+1].color    = color;

        debug_line_vertices[i+2].position = this->vertex_buffer[entity_vertex+1].position;
        debug_line_vertices[i+3].position = this->vertex_buffer[entity_vertex+2].position;
        debug_line_vertices[i+2].color    = color;
        debug_line_vertices[i+3].color    = color;

        debug_line_vertices[i+4].position = this->vertex_buffer[entity_vertex+2].position;
        debug_line_vertices[i+5].position = this->vertex_buffer[entity_vertex+3].position;
        debug_line_vertices[i+4].color    = color;
        debug_line_vertices[i+5].color    = color;

        debug_line_vertices[i+6].position = this->vertex_buffer[entity_vertex+3].position;
        debug_line_vertices[i+7].position = this->vertex_buffer[entity_vertex].position;
        debug_line_vertices[i+6].color    = color;
        debug_line_vertices[i+7].color    = color;
      }

      return &debug_line_vertices;
    }

    void generate_debug_index_text(sf::Text (&debug_entity_index_text)[p_max_size], const sf::Font& font, const sf::Color color) const
    {
      for(auto& text : debug_entity_index_text) text.setString("");

      for(int entity_index=0; entity_index < p_max_size; ++entity_index)
      {
        if ( this->is_garbage_flags[entity_index] ) continue;

        int character_size = static_cast<int>( this->collision_vertices[(entity_index * 4) + 1].x - this->collision_vertices[entity_index * 4].x ) / 4;

        debug_entity_index_text[entity_index].setFont(font);
        debug_entity_index_text[entity_index].setString(std::to_string(entity_index));
        debug_entity_index_text[entity_index].setFillColor(color);
        debug_entity_index_text[entity_index].setStyle(sf::Text::Bold);
        debug_entity_index_text[entity_index].setCharacterSize(character_size);
        debug_entity_index_text[entity_index].setPosition( this->collision_vertices[entity_index * 4] );
      }
    }
  #endif

    private:
      sf::Vector2f current_velocity;
      float current_sprite_sheet_y_position;
      float current_sprite_sheet_x_position;

};
