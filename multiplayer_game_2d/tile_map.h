#pragma once

#include <SFML/Graphics.hpp>


template<int p_rows, int p_columns>
struct tile_map
{
  const int rows = p_rows;
  const int columns = p_columns;
  const int tile_count = p_rows * p_columns;
  const float tile_size_x;
  const float tile_size_y;
  const int vertice_count = (p_rows * p_columns * 4) + 4; // (4 vertices per tile) + 4 vertices for background
  sf::Texture tiles_texture;                              // a tile sheet of tile_side_length x tile_side_length sized tiles where the first tile is the default background
  sf::Vertex vertex_buffer[(p_rows * p_columns * 4) + 4]; // (4 vertices per tile) + 4 vertices for background
  int bitmap[p_rows * p_columns] = {0};
  const int tile_side_length;                             // pixel width and height for a tile in tile sheet

  tile_map(const char* tiles_texture_file_path, const float window_size_x, const float window_size_y, const int p_tile_side_length) : tile_size_x(window_size_x / rows),tile_size_y(window_size_y / columns), tile_side_length(p_tile_side_length)
  {
    tiles_texture.loadFromFile(tiles_texture_file_path);

    // assign screen coordinates and texture coordinates for background
    vertex_buffer[0].position  = sf::Vector2f(0.0f, 0.0f);
    vertex_buffer[0].texCoords = sf::Vector2f(0.0f , 0.0f);
    vertex_buffer[1].position  = sf::Vector2f(tile_size_x * rows, 0);
    vertex_buffer[1].texCoords = sf::Vector2f((float) tile_side_length, 0.0f);
    vertex_buffer[2].position  = sf::Vector2f(tile_size_x * rows, tile_size_y * columns);
    vertex_buffer[2].texCoords = sf::Vector2f((float) tile_side_length, (float) tile_side_length);
    vertex_buffer[3].position  = sf::Vector2f(0.0f, tile_size_y * columns);
    vertex_buffer[3].texCoords = sf::Vector2f(0.0f, (float) tile_side_length);

    // assign screen coordinates for each vertex in tiles
    for(int y=0,vertex=4; y < columns; ++y)
    for(int x=0         ; x < rows   ; ++x, vertex+=4)
    {
      vertex_buffer[vertex].position   = sf::Vector2f(x * tile_size_x, y * tile_size_y);
      vertex_buffer[vertex+1].position = sf::Vector2f((x+1) * tile_size_x, y * tile_size_y);
      vertex_buffer[vertex+2].position = sf::Vector2f((x+1) * tile_size_x, (y+1) * tile_size_y);
      vertex_buffer[vertex+3].position = sf::Vector2f(x * tile_size_x, (y+1) * tile_size_y);
    }
  }

  void update_tex_coords_from_bitmap()
  {
    for(int tile=0, vertex=4,texture_offset; tile < tile_count; ++tile, vertex+=4)
    {
      texture_offset = bitmap[tile] * tile_side_length;

      vertex_buffer[vertex].texCoords   = sf::Vector2f((float) texture_offset                   , 0.0f);
      vertex_buffer[vertex+1].texCoords = sf::Vector2f((float) texture_offset + tile_side_length, 0.0f);
      vertex_buffer[vertex+2].texCoords = sf::Vector2f((float) texture_offset + tile_side_length, (float) tile_side_length);
      vertex_buffer[vertex+3].texCoords = sf::Vector2f((float) texture_offset                   , (float) tile_side_length);
    }
  }

};
