#pragma once

#include <SFML/Graphics.hpp>
#include <bitset>
#include <cmath>
#include <assert.h>
#include <limits>



/* forward declarations */
enum class tile_map_bitmap_type : int;

template<int p_width, int p_height>
struct tile_map;



/* data declarations */
struct generate_move_request_input
{
  int gameplay_entity_id = -1;
  sf::Vector2f velocity;

  sf::Vector2f direction() const
  {
    return sf::Vector2f(velocity.x / std::abs(velocity.x + velocity.y), velocity.y / std::abs(velocity.x + velocity.y));
  }
};

struct gameplay_entity_move_request
{
  // implicit entity id
  sf::Vector2f current_origin_position;
  sf::Vector2f destination_origin_position;
  sf::Vector2f velocity;

  sf::Vector2f direction() const
  {
    return sf::Vector2f(velocity.x / std::abs(velocity.x + velocity.y), velocity.y / std::abs(velocity.x + velocity.y));
  }
};



/* gameplay_entity stuff */
enum class gameplay_entity_type : int  // these are also the sprite sheet indexes
{
  NONE  = 0,
  MARIO = 1,
  BOMB  = 2
};

template<int p_max_size>
struct gameplay_entities
{
  /* @remember: origin is top-left vertex */

  sf::Vertex vertex_buffer[p_max_size * 4];           // 4 vertices per entity
  sf::Texture sprite_sheet_texture;                   // a sprite sheet where each row is a separate entity and each column is a different frame for an animation (the first row is transparent)
  sf::Vector2f collision_vertices[p_max_size * 4];    // 4 vertices per entity [top-left, top-right, bottom-right, bottom-left]
  const int sprite_sheet_side_length;                 // the pixel length and width of each entity animation frame
  const int max_size = p_max_size;
  const int vertex_count = p_max_size * 4;            // 4 vertices per entity

  gameplay_entity_type types[p_max_size] = {gameplay_entity_type::NONE}; // type of gameplay entity that's also used to specify row in sprite_sheet
  int animation_indexes[p_max_size] = {0};                               // current frame for animation
  std::bitset<p_max_size> is_garbage_flags;


  gameplay_entities(const char* sprite_sheet_texture_file_path, const int p_sprite_sheet_side_length) : sprite_sheet_side_length(p_sprite_sheet_side_length)
  {
    static_assert(p_max_size <= std::numeric_limits<int>::max(), "Max gameplay entity count is too big to be represented by int");

    sprite_sheet_texture.loadFromFile(sprite_sheet_texture_file_path);

    for(int i=0; i < p_max_size; ++i)
    {
      is_garbage_flags[i] = true;
    }

    // need guarantee default values in case some memory never filled
    for(auto& vertex : vertex_buffer)
    {
      vertex.position  = sf::Vector2f(0.0f, 0.0f);
      vertex.texCoords = sf::Vector2f(0.0f, 0.0f);
    }

    for(auto& collision_vertice : collision_vertices)
    {
      collision_vertice = sf::Vector2f(0.0f, 0.0f);
    }
  }

  void set_all_positions(const sf::Vector2f* const all_origin_positions)
  {
    sf::Vector2f current_position_offset;

    for(int entity_index=0,vertex=0; entity_index < max_size; ++entity_index,vertex += 4)
    {
      current_position_offset = all_origin_positions[entity_index] - collision_vertices[vertex];

      vertex_buffer[vertex].position   += current_position_offset;
      vertex_buffer[vertex+1].position += current_position_offset;
      vertex_buffer[vertex+2].position += current_position_offset;
      vertex_buffer[vertex+3].position += current_position_offset;

      collision_vertices[vertex]   += current_position_offset;
      collision_vertices[vertex+1] += current_position_offset;
      collision_vertices[vertex+2] += current_position_offset;
      collision_vertices[vertex+3] += current_position_offset;
    }
  }

  const sf::Vector2f* all_collision_vertices_origin_positions()
  {
    for (int id=0; id < p_max_size; ++id)
    {
      collision_vertices_origin_positions[id] = collision_vertices[id * 4];
    }

    return collision_vertices_origin_positions;
  }

  void update_tex_coords(const float elapsed_frame_time_seconds)  // update tex coords based on type and animation index
  {
    for(int entity_index=0,vertex=0; entity_index < max_size; ++entity_index,vertex += 4)
    {
      float current_sprite_sheet_y_position = (float) (sprite_sheet_texture.getSize().y - sprite_sheet_side_length) - static_cast<int>(types[entity_index]) * sprite_sheet_side_length * !is_garbage_flags[entity_index];
      float current_sprite_sheet_x_position = (float) animation_indexes[entity_index] * sprite_sheet_side_length * !is_garbage_flags[entity_index];

      vertex_buffer[vertex].texCoords   = sf::Vector2f(current_sprite_sheet_x_position, current_sprite_sheet_y_position);
      vertex_buffer[vertex+1].texCoords = sf::Vector2f(current_sprite_sheet_x_position + sprite_sheet_side_length, current_sprite_sheet_y_position);
      vertex_buffer[vertex+2].texCoords = sf::Vector2f(current_sprite_sheet_x_position + sprite_sheet_side_length, current_sprite_sheet_y_position + sprite_sheet_side_length);
      vertex_buffer[vertex+3].texCoords = sf::Vector2f(current_sprite_sheet_x_position, current_sprite_sheet_y_position + sprite_sheet_side_length);
    }
  }

  void update_position_by_offset(const int gameplay_entity_id, const sf::Vector2f& offset)
  {
    int vertex = gameplay_entity_id * 4;

    vertex_buffer[vertex].position   += offset;
    vertex_buffer[vertex+1].position += offset;
    vertex_buffer[vertex+2].position += offset;
    vertex_buffer[vertex+3].position += offset;

    collision_vertices[vertex]   += offset;
    collision_vertices[vertex+1] += offset;
    collision_vertices[vertex+2] += offset;
    collision_vertices[vertex+3] += offset;
  }

  void generate_move_requests(const generate_move_request_input* const input, gameplay_entity_move_request* const all_move_requests, const int input_count, const float tile_size_x, const float tile_size_y) const
  {
    int input_id;
    sf::Vector2f position_offset;

    for (int i=0; i < input_count; ++i)
    {
      input_id = input[i].gameplay_entity_id;
      position_offset = sf::Vector2f( tile_size_x * static_cast<float>(input[i].direction().x), tile_size_y * static_cast<float>(input[i].direction().y) );

      all_move_requests[input_id].velocity = input[i].velocity;
      all_move_requests[input_id].current_origin_position = collision_vertices[input_id * 4];
      all_move_requests[input_id].destination_origin_position = all_move_requests[input_id].current_origin_position + position_offset;
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
      sf::Vector2f collision_vertices_origin_positions[p_max_size];
};



/* tile_map stuff */
enum class tile_map_bitmap_type : int // these are also the sprite sheet indexes
{
  NONE = 0,
  BLAH = 1,
  TEST = 2,
  TEMP = 3,
  WALL = 4
};

template<int p_width, int p_height>
struct tile_map
{
  // @remember: first 4 vertices in tile_map vertex buffer are for background tile

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
    static_assert( (p_width * p_height) <= std::numeric_limits<int>::max(), "Max tile count is too big to be represented by int" );

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
    for(int x=0         ; x < width ; ++x, vertex+=4)
    {
      this->vertex_buffer[vertex].position   = sf::Vector2f(x * tile_size_x    , y * tile_size_y);
      this->vertex_buffer[vertex+1].position = sf::Vector2f((x+1) * tile_size_x, y * tile_size_y);
      this->vertex_buffer[vertex+2].position = sf::Vector2f((x+1) * tile_size_x, (y+1) * tile_size_y);
      this->vertex_buffer[vertex+3].position = sf::Vector2f(x * tile_size_x    , (y+1) * tile_size_y);
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

  int calculate_tile_map_index(const sf::Vector2f collision_vertex) const
  {
    int y_index = static_cast<int>(collision_vertex.y / tile_size_y);
    int x_index = static_cast<int>(collision_vertex.x / tile_size_x);
    return (y_index * p_width) + x_index;
  }

  #ifdef _DEBUG
    #include <string.h>

    sf::VertexArray* generate_debug_line_vertices(const sf::Color color) const
    {
      static int debug_lines_vertex_count = ((p_width * p_height) + 1) * 8; // 4 lines per tile and 2 vertices per line so 8 vertices per tile, the additional 1 is for background tile
      static sf::VertexArray debug_line_vertices(sf::Lines, debug_lines_vertex_count);

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

      return &debug_line_vertices;
    }

    void generate_debug_tile_index_text(sf::Text(&debug_tile_index_text)[p_width * p_height], const sf::Font& font, const sf::Color color) const
    {
      static int character_size = static_cast<int>(tile_size_x) / 4;

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



/* collision stuff */
template<int p_tile_map_width, int p_tile_map_height, int p_max_gameplay_entities, int p_max_entities_per_tile>
struct gameplay_entity_ids_per_tile
{
  /*
     @remember: the second vertex in tile is not considered overlapping the first vertex in the next tile; the same goes for the 3rd vertex of a tile not sharing position of the 4th vertex of the next tile
     @remember:     ex) if the position of a gameplay vertex is same position as top-left vertex in tile, it is considered in that tile and not also in the previous tile
  */

  int tile_buckets[p_max_entities_per_tile * p_max_gameplay_entities];

  gameplay_entity_ids_per_tile()
  {
    memset(tile_buckets, -1, sizeof(tile_buckets));
  }

  void update(const tile_map<p_tile_map_width, p_tile_map_height>& p_tile_map, const gameplay_entities<p_max_gameplay_entities>& p_game_entities)
  {
    memset(tile_buckets, -1, sizeof(tile_buckets));

    for(int current_collision_vertex=0; current_collision_vertex < p_game_entities.vertex_count; ++current_collision_vertex) // vertex_count is same as collision_vertex_count
    {
      int current_gameplay_entity_id = current_collision_vertex / 4;
      if (p_game_entities.is_garbage_flags[current_gameplay_entity_id]) continue;

      int current_y_index = static_cast<int>(p_game_entities.collision_vertices[current_collision_vertex].y / p_tile_map.tile_size_y);
      int current_x_index = static_cast<int>(p_game_entities.collision_vertices[current_collision_vertex].x / p_tile_map.tile_size_x);

      int current_tile_index = (current_y_index * p_tile_map.width) + current_x_index;
      int current_tile_bucket_index = current_tile_index * p_max_entities_per_tile;
      int current_max_tile_bucket_index_limit = current_tile_bucket_index + p_max_entities_per_tile;

      // find open tile_bucket for current_tile_index
      for(; ( current_tile_bucket_index < current_max_tile_bucket_index_limit )     &&
            (tile_buckets[current_tile_bucket_index] != -1)                         &&
            (tile_buckets[current_tile_bucket_index] != current_gameplay_entity_id); ++current_tile_bucket_index
         ) {};

      tile_buckets[current_tile_bucket_index] = current_gameplay_entity_id;
    }
  }

  #ifdef _DEBUG
    inline void print_tile_buckets()
    {
      std::cout << "\n";
      for(int i=0; i < p_max_entities_per_tile * p_max_gameplay_entities; i+= p_max_entities_per_tile)
      {
        std::cout << "Tile index: " << i/p_max_entities_per_tile  << std::endl;

        for(int collision_index=0; collision_index < p_max_entities_per_tile; ++collision_index)
        {
          int entity_id = tile_buckets[i + collision_index];
          std::cout << "\tid: " << entity_id << std::endl;
        }
      }
      std::cout << "\n";
    }
  #endif

}; // gameplay_entity_ids_per_tile



/* movement stuff */
template<int max_entity_count, int tile_map_width, int tile_map_height>
struct gameplay_entity_moves
{
  sf::Vector2f current_origin_positions[max_entity_count];
  sf::Vector2f destination_origin_positions[max_entity_count];
  sf::Vector2f velocities[max_entity_count];

  private:
    int tile_index_to_current_entity_id[tile_map_width * tile_map_height];      // the entity id with its origin located in specified tile
    int tile_index_to_destination_entity_id[tile_map_width * tile_map_height];  // the entity id with its destination_origin in specified tile (its currently moving into specified tile)
  public:

  gameplay_entity_moves(const sf::Vector2f* const all_origin_positions, const std::bitset<max_entity_count>& is_garbage_flags, const tile_map<tile_map_width,tile_map_height>& p_tile_map)
  {
    static_assert(tile_map_width >= tile_map_height, "tile_map height is larger than width must change chain_entity_ids_size");

    memset(tile_index_to_current_entity_id, -1, sizeof(tile_index_to_current_entity_id));
    memset(tile_index_to_destination_entity_id, -1, sizeof(tile_index_to_destination_entity_id));

    for(int id=0; id < max_entity_count; ++id)
    {
      if (is_garbage_flags[id]) continue;
      tile_index_to_current_entity_id[p_tile_map.calculate_tile_map_index(all_origin_positions[id])] = id;
      current_origin_positions[id] = all_origin_positions[id];
    }

    for(auto& position : destination_origin_positions) position = sf::Vector2f(0.0f, 0.0f);
    for(auto& velocity : velocities) velocity                   = sf::Vector2f(0.0f, 0.0f);
  }

  void submit_all_moves(gameplay_entity_move_request* const all_move_requests, const tile_map<tile_map_width,tile_map_height>& p_tile_map, const std::bitset<max_entity_count>& is_garbage_flags)
  {
    for(int request_entity_id=0; request_entity_id < max_entity_count; ++request_entity_id)
    {
      // cancel move if request id has garbage entity or has no velocity or is already moving
      if ( is_garbage_flags[request_entity_id] ||
         ( !(all_move_requests[request_entity_id].velocity.x + all_move_requests[request_entity_id].velocity.y) ) ||
         ( velocities[request_entity_id].x || velocities[request_entity_id].y ))
         { continue; }

      sf::Vector2f request_velocity = all_move_requests[request_entity_id].velocity;
      int chain_destination_tile_index = p_tile_map.calculate_tile_map_index(all_move_requests[request_entity_id].destination_origin_position);
      int chain_index = 0;
      int chain_entity_ids[tile_map_width];
      memset(chain_entity_ids, -1, sizeof(chain_entity_ids));


      // start chain with request entity
      chain_entity_ids[chain_index] = request_entity_id;
      ++chain_index;

      for(int chain_loop_index=0; chain_loop_index < sizeof(chain_entity_ids); ++chain_loop_index)
      {
        // cancel move if entity is moving into a wall
        if ( p_tile_map.bitmap[chain_destination_tile_index] == static_cast<int>(tile_map_bitmap_type::WALL) )
        {
          memset(chain_entity_ids, -1, sizeof(chain_entity_ids));
          chain_index = 0;
          break;
        }

        int current_other_entity_id     = tile_index_to_current_entity_id[chain_destination_tile_index];
        int destination_other_entity_id = tile_index_to_destination_entity_id[chain_destination_tile_index];

        // submit chained moves if chain leads to empty tile
        if ( (current_other_entity_id == -1) && (destination_other_entity_id == -1) ) break;

        // cancel move if tile has moving entity or add stationary entity to chain
        if (current_other_entity_id != -1)
        {
          if (velocities[current_other_entity_id].x || velocities[current_other_entity_id].y) // cancel move if chain leads to tile with moving entities
          {
            memset(chain_entity_ids, -1, sizeof(chain_entity_ids));
            chain_index = 0;
            break;
          }
          else // tile has stationary entity so add other entity id to chain
          {
            chain_entity_ids[chain_index] = current_other_entity_id;
            ++chain_index;
          }
        }

        // if tile is has destination entity the destination entity must be moving so cancel move
        if (destination_other_entity_id != -1)
        {
          memset(chain_entity_ids, -1, sizeof(chain_entity_ids));
          chain_index = 0;
          break;
        }

        chain_destination_tile_index = ( (chain_destination_tile_index + static_cast<int>(all_move_requests[request_entity_id].direction().x)) * static_cast<int>(request_velocity.x != 0) ) +
                                       ( (chain_destination_tile_index + (tile_map_width * static_cast<int>(all_move_requests[request_entity_id].direction().y))) * static_cast<int>(request_velocity.y != 0) );
      }


      // register moves for each chained entity
      for(int chain_entity_ids_index=0; chain_entity_ids_index < chain_index; ++chain_entity_ids_index)
      {
        int id = chain_entity_ids[chain_entity_ids_index];
        sf::Vector2f offset = sf::Vector2f( (p_tile_map.tile_size_x * all_move_requests[request_entity_id].direction().x * static_cast<float>(chain_entity_ids_index)), (p_tile_map.tile_size_y * all_move_requests[request_entity_id].direction().y * static_cast<float>(chain_entity_ids_index)) );

        current_origin_positions[id]     = all_move_requests[request_entity_id].current_origin_position     + offset;
        destination_origin_positions[id] = all_move_requests[request_entity_id].destination_origin_position + offset;
        velocities[id]                   = request_velocity / static_cast<float>(chain_index);

        tile_index_to_current_entity_id[ p_tile_map.calculate_tile_map_index(current_origin_positions[id]) ]         = id;
        tile_index_to_destination_entity_id[ p_tile_map.calculate_tile_map_index(destination_origin_positions[id]) ] = id;
      }
    }
  }

  void update_by_velocities(const float timestep, const tile_map<tile_map_width,tile_map_height>& p_tile_map)
  {
    for(int id=0; id < max_entity_count; ++id)
    {
      if ( !(velocities[id].x || velocities[id].y) ) continue;  // nothing to update if not moving

      current_origin_positions[id] += (velocities[id] * timestep);

      bool reached_destination = ( (velocities[id].x > 0.0f) && (current_origin_positions[id].x >= destination_origin_positions[id].x) ) ||
                                 ( (velocities[id].x < 0.0f) && (current_origin_positions[id].x <= destination_origin_positions[id].x) ) ||
                                 ( (velocities[id].y > 0.0f) && (current_origin_positions[id].y >= destination_origin_positions[id].y) ) ||
                                 ( (velocities[id].y < 0.0f) && (current_origin_positions[id].y <= destination_origin_positions[id].y) );
                                
      if (reached_destination)
      {
        current_origin_positions[id] = destination_origin_positions[id];

        int tile_index          = p_tile_map.calculate_tile_map_index(current_origin_positions[id]);
        int previous_tile_index = ( (tile_index - 1)              * static_cast<int>(velocities[id].x > 0.0f) ) +
                                  ( (tile_index + 1)              * static_cast<int>(velocities[id].x < 0.0f) ) +
                                  ( (tile_index - tile_map_width) * static_cast<int>(velocities[id].y > 0.0f) ) +
                                  ( (tile_index + tile_map_width) * static_cast<int>(velocities[id].y < 0.0f) );
   
        velocities[id] = sf::Vector2f(0.0f,0.0f);
        if (tile_index_to_current_entity_id[previous_tile_index] == id) tile_index_to_current_entity_id[previous_tile_index] = -1;
        tile_index_to_current_entity_id[tile_index]     = id;
        tile_index_to_destination_entity_id[tile_index] = -1;
      }
    }
  }

};



