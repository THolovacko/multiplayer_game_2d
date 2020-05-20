#pragma once

#include <SFML/Graphics.hpp>
#include <bitset>


enum class gameplay_entity_type : int  // these are also the sprite sheet indexes
{
  NONE  = 0,
  MARIO = 1,
  BOMB  = 2
};

template<int p_max_size>
struct gameplay_entities
{
  /* @remember: all hitboxes at max are tile-width */
  /* @remember: velocities should never move an entity more than half a tile per frame */
  /* @remember: can only have x or y velocity at a time */

  sf::Vertex vertex_buffer[p_max_size * 4];           // 4 vertices per entity
  sf::Texture sprite_sheet_texture;                   // a sprite sheet where each row is a separate entity and each column is a different frame for an animation (the first row is transparent)
  sf::Vector2f collision_vertices[p_max_size * 4];    // 4 vertices per entity [top-left, top-right, bottom-right, bottom-left]
  const int sprite_sheet_side_length;                 // the pixel length and width of each entity animation frame
  const int max_size = p_max_size;
  const int vertex_count = p_max_size * 4;            // 4 vertices per entity

  gameplay_entity_type types[p_max_size] = {gameplay_entity_type::NONE}; // type of gameplay entity that's also used to specify row in sprite_sheet
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

  void update_positions_by_velocity(const float elapsed_frame_time_seconds)
  {
    for(int entity_index=0,vertex=0; entity_index < max_size; ++entity_index,vertex += 4)
    {
      current_velocity_position_offset = velocities[entity_index] * (elapsed_frame_time_seconds * !is_garbage_flags[entity_index]);

      vertex_buffer[vertex].position   += current_velocity_position_offset;
      vertex_buffer[vertex+1].position += current_velocity_position_offset;
      vertex_buffer[vertex+2].position += current_velocity_position_offset;
      vertex_buffer[vertex+3].position += current_velocity_position_offset;

      collision_vertices[vertex]   += current_velocity_position_offset;
      collision_vertices[vertex+1] += current_velocity_position_offset;
      collision_vertices[vertex+2] += current_velocity_position_offset;
      collision_vertices[vertex+3] += current_velocity_position_offset;
    }
  }

  void update_tex_coords(const float elapsed_frame_time_seconds)  // update tex coords based on type and animation index
  {
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
      sf::Vector2f current_velocity_position_offset;
      float current_sprite_sheet_y_position;
      float current_sprite_sheet_x_position;

};



enum class test_tile_map_bitmap_type : int // these are also the sprite sheet indexes
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
    for(int y=0,vertex=4; y < height  ; ++y)
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


template<int p_tile_map_width, int p_tile_map_height, int p_max_gameplay_entities, int p_max_collisions_per_tile>
struct gameplay_entity_ids_per_tile
{
  // @remember: the second vertex in tile is not considered overlapping the first vertex in the next tile; the same goes for the 3rd vertex of a tile not sharing position of the 4th vertex of the next tile
  // @remember:     ex) if the position of a gameplay vertex is same position as top-left vertex in tile, it is considered in that tile and not also in the previous tile

  int tile_buckets[p_max_collisions_per_tile * p_max_gameplay_entities];
  std::bitset<p_max_gameplay_entities> off_map_bitfield;  // gameplay entity is partially or fully off the tile map

  gameplay_entity_ids_per_tile()
  {
    memset(tile_buckets, -1, sizeof(tile_buckets));
    off_map_bitfield.reset();
  }

  inline void update(const tile_map<p_tile_map_width, p_tile_map_height>& p_tile_map, const gameplay_entities<p_max_gameplay_entities>& p_game_entities, int (&p_gameplay_entity_id_to_tile_bucket_index_of_first_vertex)[p_max_gameplay_entities])
  {
    memset(tile_buckets, -1, sizeof(tile_buckets));
    memset(p_gameplay_entity_id_to_tile_bucket_index_of_first_vertex, -1, sizeof(p_gameplay_entity_id_to_tile_bucket_index_of_first_vertex));
    off_map_bitfield.reset();

    for(current_collision_vertex=0; current_collision_vertex < p_game_entities.vertex_count; ++current_collision_vertex) // vertex_count is same as collision_vertex_count
    {
      current_gameplay_entity_id = current_collision_vertex / 4;
      if (p_game_entities.is_garbage_flags[current_gameplay_entity_id]) continue;

      // check if vertex is not visible
      if(   (p_game_entities.collision_vertices[current_collision_vertex].y < 0) 
         || (p_game_entities.collision_vertices[current_collision_vertex].y > ( (p_tile_map.height * p_tile_map.tile_size_y) - 1 ))
         || (p_game_entities.collision_vertices[current_collision_vertex].x < 0)
         || (p_game_entities.collision_vertices[current_collision_vertex].x > ( (p_tile_map.width  * p_tile_map.tile_size_x) - 1 ))
        )
      {
        off_map_bitfield[current_gameplay_entity_id] = true;
        continue;
      }

      current_y_index = static_cast<int>(p_game_entities.collision_vertices[current_collision_vertex].y / p_tile_map.tile_size_y);
      current_x_index = static_cast<int>(p_game_entities.collision_vertices[current_collision_vertex].x / p_tile_map.tile_size_x);

      current_tile_index = (current_y_index * p_tile_map.width) + current_x_index;
      current_tile_bucket_index = current_tile_index * p_max_collisions_per_tile;
      current_max_tile_bucket_index_limit = current_tile_bucket_index + p_max_collisions_per_tile;

      if( (current_collision_vertex % 4) == 0 ) // checking if top-left vertex
      {
        p_gameplay_entity_id_to_tile_bucket_index_of_first_vertex[current_gameplay_entity_id] = current_tile_index;
      }

      // find open tile_bucket for current_tile_index
      for(; ( current_tile_bucket_index < current_max_tile_bucket_index_limit )
                        && (tile_buckets[current_tile_bucket_index] != -1)
                        && (tile_buckets[current_tile_bucket_index] != current_gameplay_entity_id); ++current_tile_bucket_index) {};

      tile_buckets[current_tile_bucket_index] = current_gameplay_entity_id;
    }

  }

  inline void update(const tile_map<p_tile_map_width,p_tile_map_height>& p_tile_map, const gameplay_entities<p_max_gameplay_entities>& p_game_entities, int (&p_gameplay_entity_id_to_tile_bucket_index_of_first_vertex)[p_max_gameplay_entities], const int target_gameplay_entity_id, const int first_vertex_bucket_index)
  {
    current_potential_bucket_indexes[0] = first_vertex_bucket_index;
    current_potential_bucket_indexes[1] = first_vertex_bucket_index + p_max_collisions_per_tile;
    current_potential_bucket_indexes[2] = current_potential_bucket_indexes[1] + (p_max_collisions_per_tile * p_tile_map_width);
    current_potential_bucket_indexes[3] = current_potential_bucket_indexes[2] - p_max_collisions_per_tile;

    for(auto& potential_vertex_bucket_index : current_potential_bucket_indexes)
    {
      // remove and re-sort target target_gameplay_entity_id in bucket index and then the other potential buckets
      if (potential_vertex_bucket_index <= ( (p_max_collisions_per_tile * p_tile_map.tile_count) - p_max_collisions_per_tile ) )
      {
        current_gameplay_entity_id_bucket_index_limit = potential_vertex_bucket_index + p_max_collisions_per_tile;

        // find bucket with target target_gameplay_entity_id then swap until end of bucket
        for(current_tile_bucket_index=potential_vertex_bucket_index; current_tile_bucket_index < current_gameplay_entity_id_bucket_index_limit; ++current_tile_bucket_index)
        {
          if(tile_buckets[current_tile_bucket_index] == -1) break; // the rest of the bucket is empty
     
          if(tile_buckets[current_tile_bucket_index] == target_gameplay_entity_id)
          {
            // swap until next bucket index is the current_gameplay_entity_id_bucket_index_limit then set last value in bucket as -1
            for(; (current_tile_bucket_index+1) < current_gameplay_entity_id_bucket_index_limit; ++current_tile_bucket_index)
            {
              tile_buckets[current_tile_bucket_index] = tile_buckets[current_tile_bucket_index+1];
            }
            tile_buckets[current_gameplay_entity_id_bucket_index_limit - 1] = -1; // this is always true because removing an element implies a non full bucket
            break;
          }
        }
      }
    }

    // update collision hash with new positions for target_gameplay_entity
    p_gameplay_entity_id_to_tile_bucket_index_of_first_vertex[target_gameplay_entity_id] = -1;
    off_map_bitfield[target_gameplay_entity_id] = false;

    int collision_vertex_limit = (target_gameplay_entity_id * 4) + 4;
    for(current_collision_vertex=(target_gameplay_entity_id * 4); current_collision_vertex < collision_vertex_limit; ++current_collision_vertex)
    {
      current_gameplay_entity_id = current_collision_vertex / 4;
      if (p_game_entities.is_garbage_flags[current_gameplay_entity_id]) continue;

      // check if vertex is not visible
      if(   (p_game_entities.collision_vertices[current_collision_vertex].y < 0) 
         || (p_game_entities.collision_vertices[current_collision_vertex].y > ( (p_tile_map.height * p_tile_map.tile_size_y) - 1 ))
         || (p_game_entities.collision_vertices[current_collision_vertex].x < 0)
         || (p_game_entities.collision_vertices[current_collision_vertex].x > ( (p_tile_map.width  * p_tile_map.tile_size_x) - 1 ))
        )
      {
        off_map_bitfield[current_gameplay_entity_id] = true;
        continue;
      }

      current_y_index = static_cast<int>(p_game_entities.collision_vertices[current_collision_vertex].y / p_tile_map.tile_size_y);
      current_x_index = static_cast<int>(p_game_entities.collision_vertices[current_collision_vertex].x / p_tile_map.tile_size_x);

      current_tile_index = (current_y_index * p_tile_map.width) + current_x_index;
      current_tile_bucket_index = current_tile_index * p_max_collisions_per_tile;
      current_max_tile_bucket_index_limit = current_tile_bucket_index + p_max_collisions_per_tile;

      if( (current_collision_vertex % 4) == 0 ) // checking if top-left vertex
      {
        p_gameplay_entity_id_to_tile_bucket_index_of_first_vertex[current_gameplay_entity_id] = current_tile_index;
      }

      // find open tile_bucket for current_tile_index
      for(; ( current_tile_bucket_index < current_max_tile_bucket_index_limit )
                        && (tile_buckets[current_tile_bucket_index] != -1)
                        && (tile_buckets[current_tile_bucket_index] != current_gameplay_entity_id); ++current_tile_bucket_index) {};

      tile_buckets[current_tile_bucket_index] = current_gameplay_entity_id;
    }
  } // update with gameplay_id

  #ifdef _DEBUG
    inline void print_tile_buckets()
    {
      std::cout << "\n";
      for(int i=0; i < p_max_collisions_per_tile * p_max_gameplay_entities; i+= p_max_collisions_per_tile)
      {
        std::cout << "Tile index: " << i/p_max_collisions_per_tile  << std::endl;

        for(int collision_index=0; collision_index < p_max_collisions_per_tile; ++collision_index)
        {
          int entity_id = tile_buckets[i + collision_index];
          //if (entity_id == -1) continue;
          std::cout << "\tid: " << entity_id << std::endl;
        }
      }
      std::cout << "\n";
    }
  #endif

  private:
    int current_tile_index;
    int current_tile_bucket_index;
    int current_gameplay_entity_id;
    int current_collision_vertex;
    int current_y_index;
    int current_x_index;
    int current_max_tile_bucket_index_limit;
    int current_potential_bucket_indexes[4];
    int current_gameplay_entity_id_bucket_index_limit;
}; // gameplay_entity_ids_per_tile



