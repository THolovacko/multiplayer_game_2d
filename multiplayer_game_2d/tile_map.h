#pragma once

#include <SFML/Graphics.hpp>


template<int p_width, int p_height>
struct tile_map
{
  const int width = p_width;
  const int height = p_height;
  const int tile_count = p_width * p_height;
  const float tile_size_x;
  const float tile_size_y;
  const int vertex_count = (p_width * p_height * 4) + 4;    // (4 vertices per tile) + 4 vertices for background
  sf::Texture tiles_texture;                                // a tile sheet of tile_sheet_side_length x tile_sheet_side_length sized tiles where the first tile is the default background
  sf::Vertex vertex_buffer[(p_width * p_height * 4) + 4];   // (4 vertices per tile) + 4 vertices for background
  int bitmap[p_width * p_height] = {0};
  const int tile_sheet_side_length;                         // pixel width and height for a tile in tile sheet

  tile_map(const char* tiles_texture_file_path, const float window_size_x, const float window_size_y, const int p_tile_side_length) : tile_size_x(window_size_x / width),tile_size_y(window_size_y / height), tile_sheet_side_length(p_tile_side_length)
  {
    tiles_texture.loadFromFile(tiles_texture_file_path);

    // assign screen coordinates and texture coordinates for background
    this->vertex_buffer[0].position  = sf::Vector2f(0.0f, 0.0f);
    this->vertex_buffer[0].texCoords = sf::Vector2f(0.0f , 0.0f);
    this->vertex_buffer[1].position  = sf::Vector2f(tile_size_x * width, 0);
    this->vertex_buffer[1].texCoords = sf::Vector2f((float) tile_sheet_side_length, 0.0f);
    this->vertex_buffer[2].position  = sf::Vector2f(tile_size_x * width, tile_size_y * height);
    this->vertex_buffer[2].texCoords = sf::Vector2f((float) tile_sheet_side_length, (float) tile_sheet_side_length);
    this->vertex_buffer[3].position  = sf::Vector2f(0.0f, tile_size_y * height);
    this->vertex_buffer[3].texCoords = sf::Vector2f(0.0f, (float) tile_sheet_side_length);

    // assign screen coordinates for each vertex in tiles
    for(int y=0,vertex=4; y < height; ++y)
    for(int x=0         ; x < width   ; ++x, vertex+=4)
    {
      this->vertex_buffer[vertex].position   = sf::Vector2f(x * tile_size_x, y * tile_size_y);
      this->vertex_buffer[vertex+1].position = sf::Vector2f((x+1) * tile_size_x, y * tile_size_y);
      this->vertex_buffer[vertex+2].position = sf::Vector2f((x+1) * tile_size_x, (y+1) * tile_size_y);
      this->vertex_buffer[vertex+3].position = sf::Vector2f(x * tile_size_x, (y+1) * tile_size_y);
    }
  }

  void update_tex_coords_from_bitmap()
  {
    for(int tile=0, vertex=4,texture_offset; tile < tile_count; ++tile, vertex+=4)
    {
      texture_offset = bitmap[tile] * tile_sheet_side_length;

      this->vertex_buffer[vertex].texCoords   = sf::Vector2f((float) texture_offset                         , 0.0f);
      this->vertex_buffer[vertex+1].texCoords = sf::Vector2f((float) texture_offset + tile_sheet_side_length, 0.0f);
      this->vertex_buffer[vertex+2].texCoords = sf::Vector2f((float) texture_offset + tile_sheet_side_length, (float) tile_sheet_side_length);
      this->vertex_buffer[vertex+3].texCoords = sf::Vector2f((float) texture_offset                         , (float) tile_sheet_side_length);
    }
  }

  #ifdef _DEBUG
    #include <string.h>

    sf::VertexArray generate_debug_line_vertices(const sf::Color color) const
    {
      int debug_lines_vertex_count = (tile_count + 1) * 8; // 4 lines per tile and 2 vertices per line so 8 vertices per tile, the additional 1 is for background tile
      sf::VertexArray debug_line_vertices(sf::Lines, debug_lines_vertex_count);

      for(size_t i=0,tile_map_vertex=0; i < debug_lines_vertex_count; i+=8,tile_map_vertex+=4)
      {
        debug_line_vertices[i].position   = this->vertex_buffer[tile_map_vertex].position;
        debug_line_vertices[i+1].position = this->vertex_buffer[tile_map_vertex+1].position;
        debug_line_vertices[i].color      = color;
        debug_line_vertices[i+1].color    = color;

        debug_line_vertices[i+2].position = this->vertex_buffer[tile_map_vertex+1].position;
        debug_line_vertices[i+3].position = this->vertex_buffer[tile_map_vertex+2].position;
        debug_line_vertices[i+2].color    = color;
        debug_line_vertices[i+3].color    = color;

        debug_line_vertices[i+4].position = this->vertex_buffer[tile_map_vertex+2].position;
        debug_line_vertices[i+5].position = this->vertex_buffer[tile_map_vertex+3].position;
        debug_line_vertices[i+4].color    = color;
        debug_line_vertices[i+5].color    = color;

        debug_line_vertices[i+6].position = this->vertex_buffer[tile_map_vertex+3].position;
        debug_line_vertices[i+7].position = this->vertex_buffer[tile_map_vertex].position;
        debug_line_vertices[i+6].color    = color;
        debug_line_vertices[i+7].color    = color;
      }

      return debug_line_vertices;
    }

    void generate_debug_tile_index_text(sf::Text (&debug_tile_index_text)[p_width * p_height], const sf::Font& font, const sf::Color color) const
    {
      int character_size = static_cast<int>(tile_size_x) / 4;

      for(int i=0, tile_index=1; i < tile_count; ++tile_index, ++i)
      {
        debug_tile_index_text[i].setFont(font);
        debug_tile_index_text[i].setString(std::to_string(i));
        debug_tile_index_text[i].setFillColor(color);
        debug_tile_index_text[i].setStyle(sf::Text::Bold);
        debug_tile_index_text[i].setCharacterSize(character_size);
        debug_tile_index_text[i].setPosition( this->vertex_buffer[tile_index * 4].position );
      }
    }
  #endif

};
