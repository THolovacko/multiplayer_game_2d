#pragma once

#include <string.h>
#include <SFML/Graphics.hpp>


namespace
{
  int current_tile_index;
  int current_gameplay_object_id;
}

template<int p_width, int p_height>
struct tile_map
{
  const int width = p_width;
  const int height = p_height;
  const int tile_count = p_width * p_height;
  const float tile_size_x;
  const float tile_size_y;
  const int vertice_count = (p_width * p_height * 4) + 4;   // (4 vertices per tile) + 4 vertices for background
  sf::Texture tiles_texture;                                // a tile sheet of tile_sheet_side_length x tile_sheet_side_length sized tiles where the first tile is the default background
  sf::Vertex vertex_buffer[(p_width * p_height * 4) + 4];   // (4 vertices per tile) + 4 vertices for background
  int bitmap[p_width * p_height] = {0};
  const int tile_sheet_side_length;                         // pixel width and height for a tile in tile sheet
  int gameplay_object_ids_per_tile[9 * p_width * p_height]; // 9 potential game objects (collisions) in an single tile

  tile_map(const char* tiles_texture_file_path, const float window_size_x, const float window_size_y, const int p_tile_side_length) : tile_size_x(window_size_x / width),tile_size_y(window_size_y / height), tile_sheet_side_length(p_tile_side_length)
  {
    tiles_texture.loadFromFile(tiles_texture_file_path);

    // assign screen coordinates and texture coordinates for background
    vertex_buffer[0].position  = sf::Vector2f(0.0f, 0.0f);
    vertex_buffer[0].texCoords = sf::Vector2f(0.0f , 0.0f);
    vertex_buffer[1].position  = sf::Vector2f(tile_size_x * width, 0);
    vertex_buffer[1].texCoords = sf::Vector2f((float) tile_sheet_side_length, 0.0f);
    vertex_buffer[2].position  = sf::Vector2f(tile_size_x * width, tile_size_y * height);
    vertex_buffer[2].texCoords = sf::Vector2f((float) tile_sheet_side_length, (float) tile_sheet_side_length);
    vertex_buffer[3].position  = sf::Vector2f(0.0f, tile_size_y * height);
    vertex_buffer[3].texCoords = sf::Vector2f(0.0f, (float) tile_sheet_side_length);

    // assign screen coordinates for each vertex in tiles
    for(int y=0,vertex=4; y < height; ++y)
    for(int x=0         ; x < width   ; ++x, vertex+=4)
    {
      vertex_buffer[vertex].position   = sf::Vector2f(x * tile_size_x, y * tile_size_y);
      vertex_buffer[vertex+1].position = sf::Vector2f((x+1) * tile_size_x, y * tile_size_y);
      vertex_buffer[vertex+2].position = sf::Vector2f((x+1) * tile_size_x, (y+1) * tile_size_y);
      vertex_buffer[vertex+3].position = sf::Vector2f(x * tile_size_x, (y+1) * tile_size_y);
    }

    // intialize gameplay_object_ids_per_tile
    memset(gameplay_object_ids_per_tile, -1, sizeof(gameplay_object_ids_per_tile));
  }

  void update_tex_coords_from_bitmap()
  {
    for(int tile=0, vertex=4,texture_offset; tile < tile_count; ++tile, vertex+=4)
    {
      texture_offset = bitmap[tile] * tile_sheet_side_length;

      vertex_buffer[vertex].texCoords   = sf::Vector2f((float) texture_offset                         , 0.0f);
      vertex_buffer[vertex+1].texCoords = sf::Vector2f((float) texture_offset + tile_sheet_side_length, 0.0f);
      vertex_buffer[vertex+2].texCoords = sf::Vector2f((float) texture_offset + tile_sheet_side_length, (float) tile_sheet_side_length);
      vertex_buffer[vertex+3].texCoords = sf::Vector2f((float) texture_offset                         , (float) tile_sheet_side_length);
    }
  }

  void update_gameplay_object_ids_per_tile(const sf::Vector2f* const gameplay_objects_collision_vertices_buffer, const int gameplay_objects_collision_vertices_buffer_size)
  {
    memset(gameplay_object_ids_per_tile, -1, sizeof(gameplay_object_ids_per_tile));

    for(int i=0; i < gameplay_objects_collision_vertices_buffer_size; ++i)
    {
      current_tile_index = ( (gameplay_objects_collision_vertices_buffer[i].y / tile_size_y) * width ) + (gameplay_objects_collision_vertices_buffer[i].x / tile_size_x);

      // find open slot in gameplay_object_ids_per_tile
      for(int i=0; (i < 9) && !(gameplay_object_ids_per_tile[current_tile_index] == -1); ++i)
      {
        ++current_tile_index;
      }

      current_gameplay_object_id = i / 4;

      gameplay_object_ids_per_tile[current_tile_index] = current_gameplay_object_id;
    }
  }

};
