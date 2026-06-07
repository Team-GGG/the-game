#include <math.h>
#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>

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

} PlayerState;

typedef struct {

  Texture2D sheet;
  int rows;
  int cols;

} SpriteSheetState;

typedef enum { IDLE, RUN, RUN_BACK, JUMP } PlayerMode;

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
    int x_push_back = 1;
    int y_push_back = 1;

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

void DrawPlatform(WindowState *window, PlatformState *platform,
                  TilemapState *tilemap, PlayerState *player_state) {

  const int cols = tilemap->width / platform->tile_width;
  const int rows = tilemap->height / platform->tile_height;
  const int data_size = (cols) * (rows);

  const short data[] = {
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  6,  7,  7,  8,  0,  0,  0,  0,  0,  6,
      7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  8,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  6,  7,  7,  7,  7,  7,
      7,  7,  7,  7,  7,  7,  7,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      6,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  6,  7,
      7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  8,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  6,  7,  7,  7,  7,  8,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  43, 2,  2,  2,  2,  2,  3,  0,  0,  0,  0,
      0,  71, 11, 10, 10, 12, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  43, 55, 24, 37, 37, 37, 37, 66, 0,  0,  0,  0,  0,  65, 4,  4,
      4,  62, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  34, 11, 33, 2,  34, 10, 11, 33, 2,  2,  2,  2,  55, 37,
      24, 24, 15, 14, 37, 64, 0,  0,  0,  0,  0,  65, 4,  4,  37, 62, 0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      40, 4,  4,  15, 4,  4,  4,  4,  14, 14, 14, 15, 15, 24, 37, 14, 4,  4,
      14, 62, 0,  0,  0,  0,  0,  65, 4,  4,  4,  35, 33, 34, 10, 10, 33, 2,
      2,  34, 10, 33, 2,  2,  3,  0,  0,  0,  0,  0,  0,  0,  4,  4,  4,  4,
      4,  39, 4,  4,  4,  4,  4,  4,  4,  14, 15, 4,  4,  4,  4,  62, 0,  0,
      0,  0,  0,  53, 4,  4,  4,  4,  4,  4,  4,  4,  4,  14, 15, 4,  4,  4,
      14, 15, 66, 0,  0,  0,  0,  0,  0,  0,  4,  4,  4,  4,  4,  4,  4,  4,
      4,  4,  4,  4,  4,  4,  4,  4,  4,  40, 4,  62, 0,  0,  0,  0,  0,  61,
      4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  64, 0,
      0,  0,  0,  0,  0,  0,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  40, 4,
      4,  4,  4,  4,  4,  4,  4,  62, 0,  0,  0,  0,  0,  61, 4,  4,  4,  4,
      4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  35, 10, 10, 11, 11, 11,
      10, 10};

  Rectangle current_tile =
      (Rectangle){.x = 0, .y = 0, .width = 32, .height = 32};

  bool is_grounded;

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {

      short tile_offset = data[cols * i + j];

      if (tile_offset == 0) {
        continue;
      }

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

      current_tile.x = in_x_dir * tilemap->tileset->tile_width;
      current_tile.y = in_y_dir * tilemap->tileset->tile_height;

      Rectangle current_tile_rec =
          (Rectangle){.x = j * 32, .y = i * 32, .width = 32, .height = 32};

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
  }

  player_state->is_grounded = is_grounded;
}

void Update(PlayerState *player_state, Camera2D *camera, TilemapState *tilemap,
            PlayerMode *mode) {

  if (player_state->is_grounded && IsKeyDown(KEY_LEFT)) {
    player_state->player->x -= player_state->speed;
    *mode = RUN_BACK;
  }

  else if (player_state->is_grounded && IsKeyDown(KEY_RIGHT)) {
    player_state->player->x += player_state->speed;
    *mode = RUN;
  }

  else {
    *mode = IDLE;
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
      (tilemap->width - player_state->player->width)) {
    player_state->player->x = tilemap->width - player_state->player->width;
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

  DrawTextureRec(animation_state->sprite.sheet,
                 animation_state->current_frame_rec,
                 (Vector2){.x = player_state->player->x - 75,
                           .y = player_state->player->y - 32},
                 WHITE);
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

  animation_states[RUN_BACK] = (AnimationState){
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
}

void DrawJumpState(AnimationState animation_states[], int frame_no,
                   PlayerState *player_state) {

  animation_states[JUMP].current_frame_no = frame_no;

  animation_states[JUMP].current_frame_rec.x =
      animation_states[JUMP].current_frame_no *
      animation_states[JUMP].current_frame_rec.width;

  DrawTextureRec(animation_states[JUMP].sprite.sheet,
                 animation_states[JUMP].current_frame_rec,
                 (Vector2){.x = player_state->player->x - 75,
                           .y = player_state->player->y - 32},
                 WHITE);
}

// void HandleJump(AnimationState* jump_animation, PlayerState* player_state,
// PlayerMode* mode){

// }

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
  }

  else if (IsKeyDown(KEY_RIGHT)) {
    player_state->player->x += player_state->speed;
  }
}

int main() {

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

  Rectangle player = (Rectangle){.x = 0, .y = 0, .width = 32, .height = 64};
  PlayerState player_state = (PlayerState){.player = &player,
                                           .speed = 5,
                                           .falling_speed = 0,
                                           .gravity = 1.5,
                                           .jump_left = 0,
                                           .air_time_passed = 0,
                                           .is_grounded = false,
                                           .in_jump = false,
                                           .rising_speed = 7,
                                           .terminal_velocity = 10

  };

  Camera2D camera = (Camera2D){
      .offset =
          (Vector2){.x = GetScreenWidth() / 2.0, .y = GetScreenHeight() / 2.0},
      .target =
          (Vector2){.x = player_state.player->x, .y = player_state.player->y},
      .rotation = 0.0,
      .zoom = 1.5};

  PlayerMode current_mode = IDLE;

  AnimationState animation_states[4];

  InitAnimationStates(animation_states);

  SetTargetFPS(window.fps);

  while (!WindowShouldClose()) {

    Update(&player_state, &camera, &tilemap, &current_mode);

    BeginDrawing();

    BeginMode2D(camera);

    if (IsKeyDown(KEY_A)) {
      ToggleFullscreen();
    }

    ClearBackground(RAYWHITE);

    DrawPlatform(&window, &platform, &tilemap, &player_state);

    DrawRectangleRec(player, RED);

    if (IsKeyPressed(KEY_SPACE) && player_state.jump_left > 0) {
      player_state.in_jump = true;
      player_state.air_time_passed = 0;
      player_state.rising_speed = 5;
    }

    if (player_state.in_jump) {
      HandleJump(animation_states, &player_state);
    }

    else if (player_state.is_grounded) {


      switch (current_mode) {
      case IDLE: {
        AnimatePlayer(&animation_states[IDLE], &player_state);
        break;
      };
      case RUN: {
        AnimatePlayer(&animation_states[RUN], &player_state);
        break;
      }
      case RUN_BACK: {
        AnimatePlayer(&animation_states[RUN_BACK], &player_state);
        break;
      }
      }
    }

    else {

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
  }

  UnloadTexture(tileset_texture);
}
