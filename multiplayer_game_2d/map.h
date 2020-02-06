#pragma once

class sf::Vector2f;
class sf::Texture;
class sf::VertexArray;

template<int p_rows, int p_columns>
struct map
{
  const int rows = p_rows;
  const int columns = p_columns;
  const int tile_count = p_rows * p_columns;
  const sf::Vector2f tile_size;
  const sf::Texture floor_texture;
  const sf::Texture floor_aesthetic_texture;
  sf::VertexArray vertex_buffer;
  int bitmap[p_rows * p_columns];

  map(sf::Texture p_floor_texture, sf::Texture p_floor_aesthetic_texture) : floor_texture(p_floor_texture), floor_aesthetic_texture(p_floor_aesthetic_texture)
  {
  }
};
