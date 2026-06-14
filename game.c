#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>

typedef enum { MAIN_MENU, GAME_MENU, DEATH_MENU } Menus;

typedef struct {

  const int width;
  const int height;
  const int fps;

} WindowState;

typedef struct {

  int tile_width;
  int tile_height;

} PlatformState;

typedef struct {

  Texture2D *texture;
  int cols;
  int rows;
  int tile_width;
  int tile_height;

} TilesetState;

typedef struct {

  TilesetState *tileset;
  int width;
  int height;

} TilemapState;

typedef struct {

  Rectangle *player;
  float speed;
  float gravity;

  int jump_left;
  float air_time_passed;
  float falling_speed;
  float rising_speed;
  float terminal_velocity;
  bool is_grounded;
  bool in_jump;

  bool last_direction;

} PlayerState;

typedef struct {

  Texture2D sheet;
  int rows;
  int cols;

} SpriteSheetState;

typedef enum { IDLE, IDLE_LEFT, RUN, RUN_LEFT, JUMP, JUMP_LEFT } PlayerMode;

typedef struct {

  PlayerMode mode;
  float time_needed;
  float time_passed;
  SpriteSheetState sprite;

  int frame_count;
  int current_frame_no;

  Rectangle current_frame_rec;

} AnimationState;

typedef struct {

  int i;
  int j;
  int rows;
  int cols;

  Rectangle *rec;

} TileInformation;

typedef enum {

  VERTICAL,
  HORIZONTAL

} FloaterType;

typedef struct {

  FloaterType type;
  int *floater;
  int floater_width;
  Vector2 position;
  float speed;

  Vector2 upper_bound;
  Vector2 lower_bound;

  bool direction;

} Floater;

void ResetPlayerAirState(PlayerState *player_state) {
  player_state->is_grounded = true;
  player_state->falling_speed = 0;
  player_state->air_time_passed = 0;
  player_state->in_jump = false;
  player_state->jump_left = 2;
}

bool CollisionResponse(TileInformation *tile_info, PlayerState *player_state,
                       const short data[]) {

  bool Y =
      (tile_info->rec->y + tile_info->rec->height < player_state->player->y) ||
      (player_state->player->y + player_state->player->height <
       tile_info->rec->y);

  bool X =
      (tile_info->rec->x + tile_info->rec->width < player_state->player->x) ||
      (player_state->player->x + player_state->player->width <
       tile_info->rec->x);

  if (!X && !Y) {

    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;

    int i = tile_info->i;
    int j = tile_info->j;
    int rows = tile_info->rows;
    int cols = tile_info->cols;

    if (((i > 0) && (data[(i - 1) * cols + j])) || (i == 0)) {
      up = true;
    }

    if ((i <= rows - 2) && (data[(i + 1) * cols + j])) {
      down = true;
    }

    if (((j > 0) && (data[i * cols + (j - 1)])) || (j == 0)) {
      left = true;
    }

    if ((j <= cols - 2) && (data[i * cols + (j + 1)])) {
      right = true;
    }

    // printf("left: %d + right: %d + up: %d + down: %d\n", left, right, up,
    // down);

    int buffer = 15;
    float x_push_back = 0.1;
    float y_push_back = 0.1;

    if (up && down) {

      if ((player_state->player->x) >= (tile_info->rec->x)) {
        player_state->player->x =
            tile_info->rec->x + tile_info->rec->width + x_push_back;
      }

      else {
        player_state->player->x =
            tile_info->rec->x - player_state->player->width - x_push_back;
      }

    }

    else if (left && right) {

      if ((player_state->player->y) <= (tile_info->rec->y)) {
        player_state->player->y =
            tile_info->rec->y - player_state->player->height + y_push_back;

        if (!(player_state->is_grounded)) {
          ResetPlayerAirState(player_state);
        }
      }

      else {
        player_state->player->y =
            tile_info->rec->y + tile_info->rec->height + y_push_back;
      }
    }

    else if (left && down && !up && !right) {

      if (((player_state->player->y + player_state->player->height) <=
           (tile_info->rec->y + buffer)) &&
          ((player_state->player->x) <
           (tile_info->rec->x + tile_info->rec->width - 15))) {
        player_state->player->y =
            tile_info->rec->y - player_state->player->height + y_push_back;

        if (!(player_state->is_grounded)) {
          ResetPlayerAirState(player_state);
        }

      }

      else {
        player_state->player->x =
            tile_info->rec->x + tile_info->rec->width + x_push_back;
      }
    }

    else if (right && down && !up && !left) {

      if (((player_state->player->y + player_state->player->height) <=
           (tile_info->rec->y + buffer)) &&
          ((player_state->player->x + player_state->player->width) >
           (tile_info->rec->x + 15))) {
        player_state->player->y =
            tile_info->rec->y - player_state->player->height + y_push_back;

        if (!(player_state->is_grounded)) {
          ResetPlayerAirState(player_state);
        }
      }

      else {
        player_state->player->x =
            tile_info->rec->x - player_state->player->width - x_push_back;
      }
    }

    else if (right && !down && !up && !left) {

      if ((player_state->player->y + player_state->player->height) <=
          (tile_info->rec->y + buffer)) {
        player_state->player->y =
            tile_info->rec->y - player_state->player->height + y_push_back;

        if (!(player_state->is_grounded)) {
          ResetPlayerAirState(player_state);
        }
      }

      else if ((player_state->player->y) >=
               (tile_info->rec->y + tile_info->rec->height - buffer)) {
        player_state->player->y =
            tile_info->rec->y + tile_info->rec->height + y_push_back;

      }

      else {
        player_state->player->x =
            tile_info->rec->x - player_state->player->width - x_push_back;
      }
    }

    else if (left && !down && !up && !right) {

      if ((player_state->player->y + player_state->player->height) <=
          (tile_info->rec->y + buffer)) {
        player_state->player->y =
            tile_info->rec->y - player_state->player->height + y_push_back;

        if (!(player_state->is_grounded)) {
          ResetPlayerAirState(player_state);
        }
      }

      else if ((player_state->player->y) >=
               (tile_info->rec->y + tile_info->rec->height - buffer)) {
        player_state->player->y =
            tile_info->rec->y + tile_info->rec->height + y_push_back;

      }

      else {
        player_state->player->x =
            tile_info->rec->x + tile_info->rec->width + x_push_back;
      }
    }
  }

  return (!X && !Y);
}

Vector2 TileResolveFromSerial(int tile_offset, TilemapState *tilemap) {

  bool is_divisible = (tile_offset % tilemap->tileset->cols) && 1;

  int in_x_dir;
  int in_y_dir;

  if (is_divisible) {
    in_x_dir = (tile_offset % tilemap->tileset->cols - 1);
    in_y_dir = (tile_offset / tilemap->tileset->cols);
  }

  else {
    in_x_dir = tilemap->tileset->cols - 1;
    in_y_dir = (tile_offset / tilemap->tileset->cols) - 1;
  }

  return (Vector2){.x = in_x_dir * tilemap->tileset->tile_width,
                   .y = in_y_dir * tilemap->tileset->tile_height};
}

bool FloaterCollisionResponse(Floater *floater, PlayerState *player_state) {

  int buffer = 15;
  bool is_grounded = false;
  float x_push_back = 0.1;
  float y_push_back = 0.1;

  for (int i = 0; i < floater->floater_width; i++) {

    bool X = (floater->position.x + (i + 1) * 32 < player_state->player->x) ||
             (player_state->player->x + player_state->player->width <
              (floater->position.x + i * 32));

    bool Y = (floater->position.y + 32 < player_state->player->y) ||
             (player_state->player->y + player_state->player->height <
              (floater->position.y));

    if (!X && !Y) {

      if (i == 0) {

        if ((player_state->player->y + player_state->player->height) <=
            (floater->position.y + buffer)) {
          player_state->player->y = floater->position.y -
                                    player_state->player->height +
                                    floater->speed + y_push_back;

          if (!(player_state->is_grounded)) {
            ResetPlayerAirState(player_state);
          }

          is_grounded |= true;
        }

        else if ((player_state->player->y) >=
                 (floater->position.y + 32 - buffer)) {
          player_state->player->y = floater->position.y + 32 + y_push_back;

        }

        else {
          player_state->player->x =
              floater->position.x - player_state->player->width - x_push_back;
        }
      }

      else if (i == floater->floater_width - 1) {

        if ((player_state->player->y + player_state->player->height) <=
            (floater->position.y + buffer)) {
          player_state->player->y = floater->position.y -
                                    player_state->player->height +
                                    floater->speed + y_push_back;

          if (!(player_state->is_grounded)) {
            ResetPlayerAirState(player_state);
          }

          is_grounded |= true;
        }

        else if ((player_state->player->y) >=
                 (floater->position.y + 32 - buffer)) {
          player_state->player->y = floater->position.y + 32 + y_push_back;

        }

        else {
          player_state->player->x =
              floater->position.x + (i + 1) * 32 + x_push_back;
        }
      }

      else {

        if (player_state->player->y <= floater->position.y + buffer) {

          player_state->player->y = floater->position.y -
                                    player_state->player->height +
                                    floater->speed + y_push_back;

          if (player_state->is_grounded == false) {
            ResetPlayerAirState(player_state);
          }

          is_grounded |= true;
        }

        else {
          player_state->player->y = floater->position.y + 32 + y_push_back;
        }
      }
    }
  }

  return is_grounded;
}

void DrawPlatform(WindowState *window, PlatformState *platform,
                  TilemapState *tilemap, PlayerState *player_state,
                  Floater floaters[]) {

  const int cols = tilemap->width / platform->tile_width;
  const int rows = tilemap->height / platform->tile_height;
  const int data_size = (cols) * (rows);

  const short data[] = {
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 6, 7, 7, 8, 0, 0, 0, 0, 0, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 7, 7, 7, 7, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 3, 0, 0, 0, 0, 0, 71, 11, 10, 10, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 65, 24, 37, 37, 37, 37, 66, 0, 0, 0, 0, 0, 65, 4, 4, 4, 62, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  34, 11, 33, 2, 34, 10, 11, 33, 2, 2, 2, 2, 34, 37, 24, 24, 15, 14, 37, 64, 0, 0, 0, 0, 0, 65, 4, 4, 37, 62, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                  40, 4, 4, 15, 4, 4, 4, 4, 14, 14, 14, 15, 4, 15, 37, 14, 4, 4, 14, 62, 0, 0, 0, 0, 0, 65, 4, 4, 4, 35, 33, 34, 10, 10, 33, 2, 2, 34, 10, 33, 2, 2, 3, 0, 0, 0, 0, 0, 0, 0,
                  4, 4, 4, 4, 4, 39, 4, 4, 4, 4, 4, 4, 4, 4, 15, 4, 4, 4, 4, 62, 0, 0, 0, 0, 0, 53, 4, 4, 4, 4, 4, 4, 4, 4, 4, 14, 15, 4, 4, 4, 14, 15, 66, 0, 0, 0, 0, 0, 0, 0,
                  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 40, 4, 62, 0, 0, 0, 0, 0, 61, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 64, 0, 0, 0, 0, 0, 0, 0,
                  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 40, 4, 4, 4, 4, 4, 4, 4, 4, 62, 0, 0, 0, 0, 0, 61, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 35, 10, 10, 11, 11, 11, 10, 10};

  Rectangle current_tile =
      (Rectangle){.x = 0, .y = 0, .width = 32, .height = 32};

  bool is_grounded;

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {

      short tile_offset = data[cols * i + j];

      if (tile_offset == 0) {
        continue;
      }

      Rectangle current_tile_rec =
          (Rectangle){.x = j * 32, .y = i * 32, .width = 32, .height = 32};

      Vector2 tile = TileResolveFromSerial(tile_offset, tilemap);

      current_tile.x = tile.x;
      current_tile.y = tile.y;

      TileInformation tile_info = (TileInformation){
          .i = i, .j = j, .rows = rows, .cols = cols, .rec = &current_tile_rec};

      if (!i && !j) {
        is_grounded = CollisionResponse(&tile_info, player_state, data);
      }

      else {
        is_grounded |= CollisionResponse(&tile_info, player_state, data);
      }

      DrawTextureRec(*(tilemap->tileset->texture), current_tile,
                     (Vector2){.x = j * 32, .y = i * 32}, WHITE);
    }

    for (int idx = 0; idx < 3; idx++) {
      for (int i = 0; i < floaters[idx].floater_width; i++) {

        Vector2 position = (Vector2){.x = floaters[idx].position.x + i * 32,
                                     .y = floaters[idx].position.y};

        Vector2 tile = TileResolveFromSerial(floaters[idx].floater[i], tilemap);
        current_tile.x = tile.x;
        current_tile.y = tile.y;

        DrawTextureRec(*(tilemap->tileset->texture), current_tile, position,
                       WHITE);
      }

      is_grounded |= FloaterCollisionResponse(&floaters[idx], player_state);
    }
  }

  player_state->is_grounded = is_grounded;

  printf("is_grounded: %d\n", is_grounded);
}

void Update(PlayerState *player_state, Camera2D *camera, TilemapState *tilemap,
            PlayerMode *mode) {

  if (player_state->is_grounded && IsKeyDown(KEY_LEFT)) {
    player_state->player->x -= player_state->speed;
    player_state->last_direction = 0;
    *mode = RUN_LEFT;
  }

  else if (player_state->is_grounded && IsKeyDown(KEY_RIGHT)) {
    player_state->player->x += player_state->speed;
    player_state->last_direction = 1;
    *mode = RUN;
  }

  else {
    *mode = player_state->last_direction ? (IDLE) : (IDLE_LEFT);
  }

  if (!player_state->is_grounded && IsKeyDown(KEY_DOWN)) {
    player_state->gravity *= 1.5;
  }

  else {
    player_state->gravity = 1.5;
  }

  if (IsKeyPressed(KEY_P)) {
    (camera->zoom + 0.1 <= 2) ? (camera->zoom += 0.1) : (camera->zoom);
  }

  if (IsKeyPressed(KEY_M)) {
    (camera->zoom - 0.1 >= 1) ? (camera->zoom -= 0.1) : (camera->zoom);
  }

  if (player_state->player->x < 0) {
    player_state->player->x = 0;
  }

  if (player_state->player->x >
      (tilemap->width - player_state->player->width + 15)) {
    player_state->player->x = tilemap->width - player_state->player->width + 15;
  }

  if (player_state->player->y < 0) {
    player_state->player->y = 4;
  }

  if (player_state->player->y >
      (tilemap->height - player_state->player->height + 1000)) {
    player_state->player->y =
        tilemap->height - player_state->player->height + 1000;
  }

  camera->target =
      (Vector2){.x = player_state->player->x, .y = player_state->player->y};

  // if (IsKeyDown(KEY_DOWN)) {
  //   player_state->player->y += player_state->speed;
  // }

  // if (IsKeyDown(KEY_UP)) {
  //   player_state->player->y -= player_state->speed;
  // }

  if ((camera->target.x) - (GetScreenWidth() / (2.0 * camera->zoom)) <= 0) {
    camera->target.x = camera->offset.x / (camera->zoom);
  }

  if ((camera->target.x) + (GetScreenWidth() / (2.0 * camera->zoom)) >=
      (tilemap->width)) {
    camera->target.x = tilemap->width - camera->offset.x / (camera->zoom);
  }

  if ((camera->target.y) - (GetScreenHeight() / (2.0 * camera->zoom)) <= 0) {
    camera->target.y = camera->offset.y / (camera->zoom);
  }

  if ((camera->target.y) + (GetScreenHeight() / (2.0 * camera->zoom)) >=
      (tilemap->height)) {
    camera->target.y = tilemap->height - camera->offset.y / (camera->zoom);
  }
}

void AnimatePlayer(AnimationState *animation_state, PlayerState *player_state) {
  float time_passed = animation_state->time_passed + GetFrameTime();

  if (time_passed > animation_state->time_needed) {
    animation_state->time_passed = 0;

    animation_state->current_frame_no++;

    if (animation_state->current_frame_no >= animation_state->frame_count) {
      animation_state->current_frame_no = 0;
    }
  }

  else {
    animation_state->time_passed = time_passed;
  }

  animation_state->current_frame_rec.x =
      animation_state->current_frame_no *
      animation_state->current_frame_rec.width;

  if (player_state->last_direction) {

    DrawTextureRec(animation_state->sprite.sheet,
                   animation_state->current_frame_rec,
                   (Vector2){.x = player_state->player->x - 75,
                             .y = player_state->player->y - 32},
                   WHITE);
  }

  else {

    DrawTextureRec(animation_state->sprite.sheet,
                   animation_state->current_frame_rec,
                   (Vector2){.x = player_state->player->x - 82,
                             .y = player_state->player->y - 32},
                   WHITE);
  }
}

void InitAnimationStates(AnimationState animation_states[]) {

  animation_states[IDLE] = (AnimationState){
      .mode = IDLE,
      .time_needed = .5,
      .time_passed = 0,
      .sprite = (SpriteSheetState){.sheet = LoadTexture(
                                       "resources/hero/hero-idle.png"),
                                   .rows = 1,
                                   .cols = 2},
      .frame_count = 2,
      .current_frame_no = 0,
      .current_frame_rec = (Rectangle){
          .x = 0, .y = 192 / 2.0, .width = 192, .height = 192 / 2.0}};

  animation_states[IDLE_LEFT] = (AnimationState){
      .mode = IDLE_LEFT,
      .time_needed = .5,
      .time_passed = 0,
      .sprite = (SpriteSheetState){.sheet = LoadTexture(
                                       "resources/hero/hero-idle-left.png"),
                                   .rows = 1,
                                   .cols = 2},
      .frame_count = 2,
      .current_frame_no = 0,
      .current_frame_rec = (Rectangle){
          .x = 0, .y = 192 / 2.0, .width = 192, .height = 192 / 2.0}};

  animation_states[RUN] = (AnimationState){
      .mode = RUN,
      .time_needed = 0.08,
      .time_passed = 0,
      .sprite = (SpriteSheetState){.sheet = LoadTexture(
                                       "resources/hero/hero-run.png"),
                                   .rows = 1,
                                   .cols = 8},
      .frame_count = 8,
      .current_frame_no = 0,
      .current_frame_rec = (Rectangle){
          .x = 0, .y = 192 / 2.0, .width = 192, .height = 192 / 2.0}};

  animation_states[RUN_LEFT] = (AnimationState){
      .mode = RUN,
      .time_needed = 0.08,
      .time_passed = 0,
      .sprite = (SpriteSheetState){.sheet = LoadTexture(
                                       "resources/hero/hero-run-back.png"),
                                   .rows = 1,
                                   .cols = 8},
      .frame_count = 8,
      .current_frame_no = 0,
      .current_frame_rec = (Rectangle){
          .x = 0, .y = 192 / 2.0, .width = 192, .height = 192 / 2.0}};

  animation_states[JUMP] = (AnimationState){
      .mode = JUMP,
      .time_needed = 0.08,
      .time_passed = 0,
      .sprite = (SpriteSheetState){.sheet = LoadTexture(
                                       "resources/hero/hero-jump.png"),
                                   .rows = 1,
                                   .cols = 3},
      .frame_count = 3,
      .current_frame_no = 0,
      .current_frame_rec = (Rectangle){
          .x = 0, .y = 192 / 2.0, .width = 192, .height = 192 / 2.0}};

  animation_states[JUMP_LEFT] = (AnimationState){
      .mode = JUMP_LEFT,
      .time_needed = 0.08,
      .time_passed = 0,
      .sprite = (SpriteSheetState){.sheet = LoadTexture(
                                       "resources/hero/hero-jump-left.png"),
                                   .rows = 1,
                                   .cols = 3},
      .frame_count = 3,
      .current_frame_no = 0,
      .current_frame_rec = (Rectangle){
          .x = 0, .y = 192 / 2.0, .width = 192, .height = 192 / 2.0}};
}

void DrawJumpState(AnimationState animation_states[], int frame_no,
                   PlayerState *player_state) {

  if (player_state->last_direction) {
    animation_states[JUMP].current_frame_no = frame_no;

    animation_states[JUMP].current_frame_rec.x =
        animation_states[JUMP].current_frame_no *
        animation_states[JUMP].current_frame_rec.width;

    DrawTextureRec(animation_states[JUMP].sprite.sheet,
                   animation_states[JUMP].current_frame_rec,
                   (Vector2){.x = player_state->player->x - 75,
                             .y = player_state->player->y - 32},
                   WHITE);
  } else {
    animation_states[JUMP_LEFT].current_frame_no = frame_no;

    animation_states[JUMP_LEFT].current_frame_rec.x =
        animation_states[JUMP_LEFT].current_frame_no *
        animation_states[JUMP_LEFT].current_frame_rec.width;

    DrawTextureRec(animation_states[JUMP_LEFT].sprite.sheet,
                   animation_states[JUMP_LEFT].current_frame_rec,
                   (Vector2){.x = player_state->player->x - 75,
                             .y = player_state->player->y - 32},
                   WHITE);
  }
}

void HandleJump(AnimationState animation_states[], PlayerState *player_state) {

  DrawJumpState(animation_states, 1, player_state);

  if (player_state->rising_speed > 0) {
    player_state->air_time_passed += GetFrameTime();
    player_state->rising_speed -=
        (player_state->air_time_passed * player_state->gravity);

    player_state->player->y -= player_state->rising_speed;
  }

  else {
    player_state->in_jump = false;
  }

  if (IsKeyDown(KEY_LEFT)) {
    player_state->player->x -= player_state->speed;
    player_state->last_direction = 0;
  }

  else if (IsKeyDown(KEY_RIGHT)) {
    player_state->player->x += player_state->speed;
    player_state->last_direction = 1;
  }
}

void HandleHorizontalFloater(PlayerState *player_state, Floater *floater) {
    if(FloaterCollisionResponse(floater, player_state)){
        player_state -> player -> x -= floater->speed * ((floater -> direction) ? (-1) : (1));
    };
}

void UpdateFloaters(Floater floaters[], PlayerState* player_state) {
  for (int i = 0; i < 3; i++) {

    if (floaters[i].type == VERTICAL) {

      if (floaters[i].position.y <= floaters[i].upper_bound.y) {
        floaters[i].direction = 0;
      }

      if (floaters[i].position.y >= floaters[i].lower_bound.y) {
        floaters[i].direction = 1;
      }

      floaters[i].position.y +=
          (floaters[i].direction ? (-1) : (1)) * floaters[i].speed;

    }

    else {
      if (floaters[i].position.x <= floaters[i].lower_bound.x) {
        floaters[i].direction = 1;
      }

      if (floaters[i].position.x >= floaters[i].upper_bound.x) {
        floaters[i].direction = 0;
      }

      floaters[i].position.x +=
          (floaters[i].direction ? (1) : (-1)) * floaters[i].speed;

     HandleHorizontalFloater(player_state, &floaters[i]);
    }
  }
}

int main() {
  Menus menu = MAIN_MENU;

  WindowState window = (WindowState){.width = 1600, .height = 896, .fps = 60};
  PlatformState platform = (PlatformState){.tile_width = 32, .tile_height = 32};

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(window.width, window.height, "GGG");

  Texture2D tileset_texture = LoadTexture("resources/tiles/Tileset.png");

  TilesetState tileset = (TilesetState){.texture = &tileset_texture,
                                        .cols = 12,
                                        .rows = 8,
                                        .tile_width = 32,
                                        .tile_height = 32};

  TilemapState tilemap = (TilemapState){.tileset = &tileset,
                                        .width = 50 * tileset.tile_width,
                                        .height = 40 * tileset.tile_height};

  Rectangle player = (Rectangle){.x = 0, .y = 1000, .width = 32, .height = 64};
  PlayerState player_state = (PlayerState){.player = &player,
                                           .speed = 5.7,
                                           .falling_speed = 0,
                                           .gravity = 1.5,
                                           .jump_left = 0,
                                           .air_time_passed = 0,
                                           .is_grounded = false,
                                           .in_jump = false,
                                           .rising_speed = 7,
                                           .terminal_velocity = 10,
                                           .last_direction = 1

  };

  Camera2D camera = (Camera2D){
      .offset =
          (Vector2){.x = GetScreenWidth() / 2.0, .y = GetScreenHeight() / 2.0},
      .target =
          (Vector2){.x = player_state.player->x, .y = player_state.player->y},
      .rotation = 0.0,
      .zoom = 1.5};

  Floater floaters[3];

  floaters[0] = (Floater){
      .floater = (int[]){44, 45, 45, 46},
      .floater_width = 4,
      .speed = 0.7,
      .position = (Vector2){.x = 45 * tileset.tile_width,
                            .y = 34 * tileset.tile_height},
      .type = VERTICAL,
      .lower_bound = (Vector2){.x = 45 * tileset.tile_width,
                               .y = 34 * tileset.tile_height},
      .upper_bound = (Vector2){.x = 45 * tileset.tile_width,
                               .y = 25 * tileset.tile_height},
      .direction = 1,

  };

  floaters[1] =
      (Floater){.floater = (int[]){44, 45, 46},
                .floater_width = 3,
                .speed = 0.7,
                .position = (Vector2){.x = 31 * tileset.tile_width,
                                      .y = 25 * tileset.tile_height},
                .type = HORIZONTAL,
                .upper_bound = (Vector2){.x = 31 * tileset.tile_width,
                                         .y = 25 * tileset.tile_height},
                .lower_bound = (Vector2){.x = 22 * tileset.tile_width,
                                         .y = 25 * tileset.tile_height},
                .direction = 0

      };

  floaters[2] = (Floater){.floater = (int[]){44, 45, 45, 46},
                          .floater_width = 4,
                          .speed = 0.7,
                          .position = (Vector2){.x = 2 * tileset.tile_width,
                                                .y = 25 * tileset.tile_height},
                          .type = VERTICAL,
                          .upper_bound = (Vector2){.x = 2 * tileset.tile_width,
                                                .y = 9 * tileset.tile_height},
                          .lower_bound = (Vector2){.x = 2 * tileset.tile_width,
                                                .y = 25 * tileset.tile_height},
                          .direction = 1


  };

  PlayerMode current_mode = IDLE;

  AnimationState animation_states[6];

  InitAnimationStates(animation_states);

  InitAudioDevice();

  Sound sound_walking = LoadSound("resources/Audio/running_in_grass.mp3");
  SetTargetFPS(window.fps);

  while (!WindowShouldClose()) {
    if (menu == MAIN_MENU) {
      //  Draw phase
      BeginDrawing();
      // Clean, dark minimalist background
      // ClearBackground(GetColor(0x0f111aFF));
      ClearBackground((Color){15, 17, 26, 128});

      // 1. DRAW THE TITLE ("HOLLOW")
      const char titleText[] = "HOLLOW";
      int titleFontSize = 80;

      // Calculate horizontal centering for the title
      int titleWidth = MeasureText(titleText, titleFontSize);
      int titleX = (window.width - titleWidth) / 2;
      int titleY = GetScreenHeight() / 2.0 - 80; // Positioned in the upper half

      // Draw the main title text
      DrawText(titleText, titleX, titleY, titleFontSize, SKYBLUE);

      // 2. DRAW THE SUBTITLE ("Press Enter to Play Game")
      const char *subText = "Press Enter to Play Game";
      int subFontSize = 24;

      // Calculate horizontal centering for the subtitle
      int subWidth = MeasureText(subText, subFontSize);
      int subX = (window.width - subWidth) / 2;
      int subY = titleY + 100; // Positioned in the lower half

      // Draw the subtitle text
      DrawText(subText, subX, subY, subFontSize, LIGHTGRAY);

      EndDrawing();
      if (IsKeyPressed(KEY_ENTER)) {
        menu = GAME_MENU;
      }
    }
    if (menu == GAME_MENU) {

      Update(&player_state, &camera, &tilemap, &current_mode);
      UpdateFloaters(floaters, &player_state);

      BeginDrawing();

      BeginMode2D(camera);

      if (IsKeyDown(KEY_A)) {
        ToggleFullscreen();
      }

      ClearBackground(RAYWHITE);

      DrawPlatform(&window, &platform, &tilemap, &player_state, floaters);

      DrawRectangleRec(player, RED);

      if (IsKeyPressed(KEY_SPACE) && player_state.jump_left > 0) {
        player_state.in_jump = true;
        player_state.air_time_passed = 0;
        player_state.rising_speed = 5;

        player_state.jump_left--;
      }

      if (player_state.in_jump) {
        HandleJump(animation_states, &player_state);
        StopSound(sound_walking); // Needed to stop sound for playing when
                                  // player not grounded
      }

      else if (player_state.is_grounded) {
        // Sound of Walking Code//
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_LEFT)) {
          if (IsSoundPlaying(sound_walking) == 0) {
            SetSoundVolume(sound_walking, 1.0f);
            PlaySound(sound_walking);
          }
        } else {
          StopSound(sound_walking);
        }
        // Sound of Walking Code end//

        switch (current_mode) {
        case IDLE: {
          AnimatePlayer(&animation_states[IDLE], &player_state);
          break;
        };

        case IDLE_LEFT: {
          AnimatePlayer(&animation_states[IDLE_LEFT], &player_state);
          break;
        };

        case RUN: {
          AnimatePlayer(&animation_states[RUN], &player_state);
          break;
        }
        case RUN_LEFT: {
          AnimatePlayer(&animation_states[RUN_LEFT], &player_state);
          break;
        }
        }
      }

      else {
        StopSound(sound_walking); // Needed to stop sound for playing when
                                  // player not grounded

        player_state.air_time_passed += GetFrameTime();

        player_state.falling_speed +=
            (player_state.gravity * player_state.air_time_passed);

        player_state.falling_speed =
            (player_state.falling_speed <= player_state.terminal_velocity)
                ? (player_state.falling_speed)
                : (player_state.terminal_velocity);

        player_state.player->y += player_state.falling_speed;

        if (IsKeyDown(KEY_LEFT)) {
          player_state.player->x -= player_state.speed;
        }

        else if (IsKeyDown(KEY_RIGHT)) {
          player_state.player->x += player_state.speed;
        }

        DrawJumpState(animation_states, 1, &player_state);
      }

      EndMode2D();
      EndDrawing();
      if ((player_state.player->y) > 2000) {
        menu = DEATH_MENU;
      }
      // printf("%f \n", player_state.player->y);
    }

    if (menu == DEATH_MENU) {
      BeginDrawing();
      ClearBackground(GetColor(0x590404FF));

      const char *death_text = "Press Enter to Go Back To Main Menu";
      int font_size = 50;

      DrawText(death_text,
               (GetScreenWidth() - MeasureText(death_text, font_size)) / 2.0,
               GetScreenHeight() / 2.0 - font_size, font_size, LIGHTGRAY);

      EndDrawing();
      if (IsKeyPressed(KEY_ENTER)) {
        menu = MAIN_MENU;
        player_state.player->y =
            1000; // not resetting y causes insta death because player is still
                  // below falling level; see below comment;
        player_state.player->x =
            0; // Need to reset all other (x,speed etc) too , but I think it
               // will be better to write a deathfunction and handle this using
               // that instead of everything else
      }
    }
  }
  UnloadSound(sound_walking);
  CloseAudioDevice();
  UnloadTexture(tileset_texture);
}
