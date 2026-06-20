#include <raylib.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stdio.h>

// #define DEBUG

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
  Rectangle attack_light_current_frame_rec;

  int attack_light_frame_count;
  int attack_light_current_frame_no;
  int jump_left;

  float speed;
  float gravity;
  float hp;
  float air_time_passed;
  float falling_speed;
  float rising_speed;
  float sliding_speed;
  float terminal_velocity;
  float attack_light_damage;
  float attack_light_time_passed;
  float attack_light_time_needed;

  bool is_grounded;
  bool in_jump;
  bool last_direction;
  bool is_attack_hit;
  bool in_attack_light;
  bool is_being_hit;
  bool death_sound;

  Texture2D attack_light_sprite;
  Texture2D attack_light_sprite_back;

} PlayerState;

typedef struct {

  Texture2D sheet;
  int rows;
  int cols;

} SpriteSheetState;

typedef enum {
  IDLE,
  IDLE_LEFT,
  RUN,
  RUN_LEFT,
  JUMP,
  JUMP_LEFT,
  ATTACK_LIGHT
} PlayerMode;

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

typedef struct {

  int width;
  int height;
  float damage;
  Vector2 position;
  Vector2 lower_bound;
  Vector2 upper_bound;
  Rectangle hitbox;
  Texture2D sprite_sheet;

  int frame_count;
  Rectangle current_frame_rec;
  int current_frame_no;
  float time_needed;
  float time_passed;

  float speed;
  float max_speed;
  bool direction;

} Trap;

typedef struct {

  Texture2D sprite;
  int frame_count;
  int current_frame_no;
  Rectangle current_frame_rec;
  float time_needed;
  float time_passed;

} MobAnimation;

typedef enum {
  MOB_WALK,
  MOB_RUN,
  MOB_ATTACK,
  MOB_IDLE,
  MOB_DYING,
  MOB_DEAD
} MobMode;

typedef struct {

  MobMode mode;

  float hp;

  Rectangle hurtbox;
  Vector2 lower_bound;
  Vector2 upper_bound;

  float damage;
  float walk_speed;
  float run_speed;
  float idle_buffer;

  bool direction;
  bool saw;
  bool player_direction_rel;

  MobAnimation golem_walk;
  MobAnimation golem_walk_back;
  MobAnimation golem_run;
  MobAnimation golem_run_back;
  MobAnimation golem_attack;
  MobAnimation golem_attack_back;
  MobAnimation golem_death;
  MobAnimation golem_death_back;

} MobGolem;

typedef struct {

  MobMode mode;

  float hp;

  Rectangle hurtbox;
  Rectangle bullet_hitbox;

  Vector2 lower_bound;
  Vector2 upper_bound;
  Vector2 bullet_destination;

  float damage;
  float walk_speed;
  float bullet_speed;
  float idle_buffer;
  float no_damage_time;

  bool direction;
  bool player_direction_rel;
  bool bullet_released;
  bool bullet_direction;

  MobAnimation golemr_walk;
  MobAnimation golemr_attack;
  MobAnimation golemr_bullet;
  MobAnimation golemr_death;

  MobAnimation golemr_bullet_back;
  MobAnimation golemr_walk_back;
  MobAnimation golemr_attack_back;
  MobAnimation golemr_death_back;

} MobGolemR;

typedef enum { TRAMPOLINE_IDLE, TRAMPOLINE_ACTIVE } TrampolineMode;

typedef struct {

  TrampolineMode mode;

  Vector2 position;

  int width;
  int height;
  int frame_count;
  int current_frame_no;

  Texture2D sprite;

  float time_needed;
  float time_passed;

  Rectangle current_frame_rec;

  int height_trampoline[2];

} Trampoline;

float diff(float a, float b) { return ((a - b > 0) ? (a - b) : (b - a)); }

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

      player_state->player->y += player_state->sliding_speed;

      // if(jk)

    }

    else if (left && right) {

      if ((player_state->player->y) <= (tile_info->rec->y)) {
        player_state->player->y =
            tile_info->rec->y - player_state->player->height + y_push_back;

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

      }

      else {
        player_state->player->x =
            tile_info->rec->x + tile_info->rec->width + x_push_back;

        player_state->player->y += player_state->sliding_speed;
      }
    }

    else if (right && down && !up && !left) {

      if (((player_state->player->y + player_state->player->height) <=
           (tile_info->rec->y + buffer)) &&
          ((player_state->player->x + player_state->player->width) >
           (tile_info->rec->x + 15))) {
        player_state->player->y =
            tile_info->rec->y - player_state->player->height + y_push_back;

      }

      else {
        player_state->player->x =
            tile_info->rec->x - player_state->player->width - x_push_back;

        player_state->player->y += player_state->sliding_speed;
      }
    }

    else if (right && !down && !up && !left) {

      if ((player_state->player->y + player_state->player->height) <=
          (tile_info->rec->y + buffer)) {
        player_state->player->y =
            tile_info->rec->y - player_state->player->height + y_push_back;

      }

      else if ((player_state->player->y) >=
               (tile_info->rec->y + tile_info->rec->height - buffer)) {
        player_state->player->y =
            tile_info->rec->y + tile_info->rec->height + y_push_back;

      }

      else {
        player_state->player->x =
            tile_info->rec->x - player_state->player->width - x_push_back * 2.5;

        player_state->player->y += player_state->sliding_speed * 2.5;
      }
    }

    else if (left && !down && !up && !right) {

      if ((player_state->player->y + player_state->player->height) <=
          (tile_info->rec->y + buffer)) {
        player_state->player->y =
            tile_info->rec->y - player_state->player->height + y_push_back;

      }

      else if ((player_state->player->y) >=
               (tile_info->rec->y + tile_info->rec->height - buffer)) {
        player_state->player->y =
            tile_info->rec->y + tile_info->rec->height + y_push_back;

      }

      else {
        player_state->player->x =
            tile_info->rec->x + tile_info->rec->width + x_push_back * 2.5;

        player_state->player->y += player_state->sliding_speed * 2.5;
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
                (floater->position.y + buffer) &&
            (player_state->player->x + player_state->player->width >=
             floater->position.x + 6)) {
          player_state->player->y = floater->position.y -
                                    player_state->player->height +
                                    floater->speed + y_push_back;

          is_grounded |= true;
        }

        else if ((player_state->player->y) >=
                 (floater->position.y + 32 - buffer)) {
          player_state->player->y = floater->position.y + 32 + y_push_back;

        }

        else {
          player_state->player->x =
              floater->position.x - player_state->player->width - x_push_back;

          player_state->player->y += player_state->sliding_speed;
        }
      }

      else if (i == floater->floater_width - 1) {

        if ((player_state->player->y + player_state->player->height) <=
                (floater->position.y + buffer) &&
            (player_state->player->x <
             floater->position.x + (i + 1) * 32 - 6)) {
          player_state->player->y = floater->position.y -
                                    player_state->player->height +
                                    floater->speed + y_push_back;

          is_grounded |= true;
        }

        else if ((player_state->player->y) >=
                 (floater->position.y + 32 - buffer)) {
          player_state->player->y = floater->position.y + 32 + y_push_back;

        }

        else {
          player_state->player->x =
              floater->position.x + (i + 1) * 32 + x_push_back;

          player_state->player->y += player_state->sliding_speed;
        }
      }

      else {

        if (player_state->player->y <= floater->position.y + buffer) {

          player_state->player->y = floater->position.y -
                                    player_state->player->height +
                                    floater->speed + y_push_back;

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
      0,  0,  6,  7,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  6,
      7,  7,  7,  7,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
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
      0,  0,  0,  0,  0,  0,  0,  0,  0,  6,  7,  7,  7,  7,  7,  7,  7,  7,
      7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  6,  7,  7,  7,  7,
      7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  0,  0,  0,  0,  0,  0,
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
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  6,  7,  7,  7,  7,  7,  7,  7,
      7,  7,  7,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  6,  7,  7,  7,  7,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
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
      0,  1,  2,  2,  2,  2,  2,  3,  0,  0,  0,  0,  0,  71, 11, 10, 10, 12,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  65, 24, 37,
      37, 37, 37, 66, 0,  0,  0,  0,  0,  65, 4,  4,  4,  62, 0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  34, 11,
      33, 2,  34, 10, 11, 33, 2,  2,  2,  2,  34, 37, 24, 24, 15, 14, 37, 64,
      0,  0,  0,  0,  0,  65, 4,  4,  37, 62, 0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  40, 4,  4,  15, 4,  4,
      4,  4,  14, 14, 14, 15, 4,  15, 37, 14, 4,  4,  14, 62, 0,  0,  0,  0,
      0,  65, 4,  4,  4,  35, 33, 34, 10, 10, 33, 2,  2,  34, 10, 33, 2,  2,
      3,  0,  0,  0,  0,  0,  0,  0,  4,  4,  4,  4,  4,  39, 4,  4,  4,  4,
      4,  4,  4,  4,  15, 4,  4,  4,  4,  62, 0,  0,  0,  0,  0,  53, 4,  4,
      4,  4,  4,  4,  4,  4,  4,  14, 15, 4,  4,  4,  14, 15, 66, 0,  0,  0,
      0,  0,  0,  0,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
      4,  4,  4,  40, 4,  62, 0,  0,  0,  0,  0,  61, 4,  4,  4,  4,  4,  4,
      4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  64, 0,  0,  0,  0,  0,  0,  0,
      4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  40, 4,  4,  4,  4,  4,  4,  4,
      4,  62, 0,  0,  0,  0,  0,  61, 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
      4,  4,  4,  4,  4,  4,  35, 10, 10, 11, 11, 11, 10, 10, 0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  62, 0,  0,
      0,  0,  0,  61, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  62, 0,  0,  0,  0,  0,  61,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0};

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

      if (is_grounded) {
        ResetPlayerAirState(player_state);
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

  if (is_grounded) {
    ResetPlayerAirState(player_state);
  }
}

void Update(PlayerState *player_state, Camera2D *camera, TilemapState *tilemap,
            PlayerMode *mode) {

  if (IsKeyDown(KEY_A) && !player_state->in_attack_light &&
      !player_state->is_being_hit) {

    player_state->last_direction = 0;

    if (player_state->is_grounded) {
      *mode = RUN_LEFT;
      player_state->player->x -= player_state->speed;
    }

    else {
      player_state->player->x -= player_state->speed / 1.5;
    }

  }

  else if (IsKeyDown(KEY_D) && !player_state->in_attack_light &&
           !player_state->is_being_hit) {
    player_state->last_direction = 1;

    if (player_state->is_grounded) {
      *mode = RUN;
      player_state->player->x += player_state->speed;
    }

    else {
      player_state->player->x += player_state->speed / 1.5;
    }
  }

  else {
    *mode = player_state->last_direction ? (IDLE) : (IDLE_LEFT);
  }

  if (!player_state->is_grounded && IsKeyDown(KEY_S) &&
      !player_state->in_attack_light) {
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

  if ((camera->target.y) - (GetScreenHeight() / (2.0 * camera->zoom)) <= 160) {
    camera->target.y = (camera->offset.y) / (camera->zoom) + 160;
  }

  if ((camera->target.y) + (GetScreenHeight() / (2.0 * camera->zoom)) >=
      (tilemap->height - 64)) {
    camera->target.y = tilemap->height - 64 - camera->offset.y / (camera->zoom);
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

  // if (IsKeyDown(KEY_A) && !player_state->in_attack_light) {
  //   player_state->player->x -= player_state->speed;
  //   player_state->last_direction = 0;
  // }

  // else if (IsKeyDown(KEY_D) && !player_state->in_attack_light) {
  //   player_state->player->x += player_state->speed;
  //   player_state->last_direction = 1;
  // }
}

void HandleHorizontalFloater(PlayerState *player_state, Floater *floater) {
  if (FloaterCollisionResponse(floater, player_state)) {
    player_state->player->x -=
        floater->speed * ((floater->direction) ? (-1) : (1));
  };
}

void UpdateFloaters(Floater floaters[], PlayerState *player_state) {
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

void DrawTrap(Trap *trap) {

  if (trap->time_needed >= trap->time_passed) {
    trap->current_frame_no++;
    trap->time_passed = 0;
  }

  if (trap->current_frame_no > trap->frame_count) {
    trap->current_frame_no = 1;
  }

  trap->current_frame_rec.x =
      (trap->current_frame_no - 1) * trap->current_frame_rec.width;

  DrawTextureRec(trap->sprite_sheet, trap->current_frame_rec, trap->position,
                 WHITE);
}

bool SimpleCollisionCheck(Rectangle *rec1, Rectangle *rec2) {
  bool X =
      (rec1->x + rec1->width <= rec2->x) || (rec2->x + rec2->width <= rec1->x);
  bool Y = (rec1->y + rec1->height <= rec2->y) ||
           (rec2->y + rec2->height <= rec1->y);

  if (!X && !Y) {
    return 1;
  } else {
    return 0;
  }
}

void UpdateTrap(Trap *trap, PlayerState *player_state, Sound *death_scream) {

  if (trap->direction) {
    trap->speed =
        (trap->max_speed) * ((trap->position.x) / (float)(trap->upper_bound.x));
  } else {
    trap->speed =
        (trap->max_speed) * ((float)(trap->lower_bound.x) / (trap->position.x));
  }

  if (trap->position.x + trap->speed > trap->upper_bound.x) {
    trap->direction = 0;
  }

  else if (trap->position.x + trap->speed < trap->lower_bound.x) {
    trap->direction = 1;
  }

  trap->position.x += (trap->direction ? (1) : (-1)) * (trap->speed);
  trap->hitbox.x = trap->position.x;

  if (SimpleCollisionCheck(&trap->hitbox, player_state->player)) {
    player_state->hp -= trap->damage;
  };

  if (player_state->hp <= 0) {
    PlaySound(*death_scream);
  }
}

void DrawHP(PlayerState *player_state) {

  Rectangle back = (Rectangle){.width = 55,
                               .height = 4,
                               .x = player_state->player->x +
                                    player_state->player->width / 2.0 - 27.5,
                               .y = player_state->player->y - 15};

  Rectangle hp_bar = (Rectangle){.width = back.width * player_state->hp,
                                 .height = 4,
                                 .x = back.x,
                                 .y = player_state->player->y - 15};

  Color color;

  if (player_state->hp > 0.56) {
    color = (Color){22, 196, 127, 255};
  }

  else if (player_state->hp <= 0.56 && player_state->hp >= 20.6) {
    color = (Color){255, 214, 90, 255};
  }

  else {

    color = (Color){249, 56, 39, 255};
  }

  DrawRectangleRounded(back, 1, 0, LIGHTGRAY);
  DrawRectangleRounded(hp_bar, 1, 0, color);
}

void HandleAttackLight(PlayerState *player_state, Sound *sound_attack_light,
                       MobGolem *golem, Sound *sound_golem_hit,
                       MobGolemR *golemr) {

  if (player_state->is_being_hit) {
    return;
  }

  player_state->in_attack_light = true;

  player_state->attack_light_time_passed += GetFrameTime();

  if (player_state->attack_light_time_passed >=
      player_state->attack_light_time_needed) {
    player_state->attack_light_current_frame_no++;
    player_state->attack_light_time_passed = 0;
  }

  if (player_state->attack_light_current_frame_no >
      player_state->attack_light_frame_count) {
    player_state->attack_light_current_frame_no = 1;

    player_state->in_attack_light = false;
    player_state->is_attack_hit = 1;
    return;
  }

  if (player_state->attack_light_current_frame_no == 1) {
    PlaySound(*sound_attack_light);
  }

  player_state->attack_light_current_frame_rec.x =
      (player_state->attack_light_current_frame_no - 1) *
      player_state->attack_light_current_frame_rec.width;

  Rectangle hitbox;

  if (player_state->last_direction) {

    hitbox = (Rectangle){.x = player_state->player->x,
                         .y = player_state->player->y,
                         .width = 150,
                         .height = 64};

#ifdef DEBUG
    DrawRectangleRec(hitbox, GREEN);

    DrawRectangle(player_state->player->x, player_state->player->y, 32, 64,
                  RED);

#endif

    DrawTextureRec(player_state->attack_light_sprite,
                   player_state->attack_light_current_frame_rec,
                   (Vector2){.x = player_state->player->x - 85,
                             .y = player_state->player->y - 127},
                   WHITE);
  }

  else {

    hitbox = (Rectangle){.x = player_state->player->x - 150 +
                              player_state->player->width,
                         .y = player_state->player->y,
                         .width = 150,
                         .height = 64};
#ifdef DEBUG
    DrawRectangleRec(hitbox, GREEN);

    DrawRectangle(player_state->player->x, player_state->player->y, 32, 64,
                  RED);
#endif

    DrawTextureRec(player_state->attack_light_sprite_back,
                   player_state->attack_light_current_frame_rec,
                   (Vector2){.x = player_state->player->x - 85 - 150 -
                                  player_state->player->width,
                             .y = player_state->player->y - 127},
                   WHITE);
  }

  if (player_state->is_attack_hit &&
      SimpleCollisionCheck(&hitbox, &golem->hurtbox)) {
    golem->hp -= player_state->attack_light_damage;
    player_state->is_attack_hit = 0;
    golem->idle_buffer = .6;

    if (player_state->player->x >= golem->hurtbox.x) {
      golem->direction = 1;
    } else {
      golem->direction = 0;
    }

    if (golem->hp > 0) {
      PlaySound(*sound_golem_hit);
    }
  }

  if (player_state->is_attack_hit &&
      SimpleCollisionCheck(&hitbox, &golemr->hurtbox)) {
    golemr->hp -= player_state->attack_light_damage;
    player_state->is_attack_hit = 0;
    golemr->idle_buffer = .7;


    if (golemr->hp > 0) {
      PlaySound(*sound_golem_hit);
    }
  }

  // if (golem->hp > 0) {
  //   PlaySound(*sound_golem_hit);
  // }
  // }
}

void UpdateGolem(MobGolem *golem, PlayerState *player_state,
                 Sound *death_scream, Sound *golem_attack) {

  if (golem->hp <= 0) {
    if (!(golem->mode == MOB_DEAD)) {
      golem->mode = MOB_DYING;
    }
    return;
  }

  if (golem->idle_buffer > 0) {
    golem->idle_buffer -= GetFrameTime();
    golem->mode = MOB_IDLE;

    if (golem->idle_buffer <= 0) {
      golem->mode = MOB_WALK;
    }
    return;
  } else {
    golem->idle_buffer = 0;
  }

  if ((golem->mode == MOB_ATTACK ||
       SimpleCollisionCheck(&golem->hurtbox, player_state->player)) &&
      (!player_state->in_attack_light)) {

    golem->mode = MOB_ATTACK;
    player_state->is_being_hit = true;

    return;
  }

  if (golem->direction) {
    golem->player_direction_rel = (player_state->player->x >= golem->hurtbox.x);
  } else {
    golem->player_direction_rel =
        !(player_state->player->x >= golem->hurtbox.x);
  }

  if (

      (golem->player_direction_rel) &&
      (diff(golem->hurtbox.x + golem->hurtbox.width / 2.0,
            player_state->player->x) <= 400) &&
      (player_state->player->y <= golem->hurtbox.y + golem->hurtbox.height) &&
      (player_state->player->y + player_state->player->height >=
       golem->hurtbox.y)

      && (golem->mode != MOB_IDLE)

  ) {
    golem->mode = MOB_RUN;

    if (golem->saw) {
      golem->saw = 0;

      if (player_state->player->x >
          golem->hurtbox.x + golem->hurtbox.width / 2.0) {
        golem->direction = 1;
      } else {
        golem->direction = 0;
      }
    }

  }

  else {
    if (golem->mode != MOB_IDLE) {
      golem->mode = MOB_WALK;
    }
    golem->saw = 1;
  }

  if (golem->hurtbox.x <= golem->lower_bound.x) {
    golem->direction = 1;
  }

  else if (golem->hurtbox.x >= golem->upper_bound.x - 32) {
    golem->direction = 0;
  }

  if (golem->mode == MOB_WALK) {
    golem->hurtbox.x += (golem->direction ? (1) : (-1)) * (golem->walk_speed);
  }

  else if (golem->mode == MOB_RUN) {
    golem->hurtbox.x += (golem->direction ? (1) : (-1)) * (golem->run_speed);
  }
}

void DrawGolemHP(MobGolem *golem) {

  Rectangle back =
      (Rectangle){.width = 55,
                  .height = 4,
                  .x = golem->hurtbox.x + golem->hurtbox.width / 2.0 - 27.5,
                  .y = golem->hurtbox.y - 20};

  Rectangle hp_bar = (Rectangle){.width = back.width * golem->hp,
                                 .height = 4,
                                 .x = back.x,
                                 .y = golem->hurtbox.y - 20};

  Color color;

  if (golem->hp > 0.56) {
    color = (Color){22, 196, 127, 255};
  }

  else if (golem->hp <= 0.56 && golem->hp >= 20.6) {
    color = (Color){255, 214, 90, 255};
  }

  else {
    color = (Color){249, 56, 39, 255};
  }

  DrawRectangleRounded(back, 1, 0, LIGHTGRAY);
  DrawRectangleRounded(hp_bar, 1, 0, color);
}

void DrawGolem(MobGolem *golem, PlayerState *player_state, Sound *golem_attack,
               Sound *golem_dead) {

  if (golem->mode == MOB_DEAD) {
    return;
  }

#ifdef DEBUG
  DrawRectangleRec(golem->hurtbox, BLUE);
#endif

  DrawGolemHP(golem);

  if (golem->mode == MOB_IDLE) {

    if (golem->direction) {

      DrawTextureRec(
          golem->golem_run.sprite, golem->golem_run.current_frame_rec,
          (Vector2){.x = golem->hurtbox.x - 25, .y = golem->hurtbox.y - 32},
          WHITE);
    }

    else {
      DrawTextureRec(
          golem->golem_run_back.sprite, golem->golem_run.current_frame_rec,
          (Vector2){.x = golem->hurtbox.x - 29, .y = golem->hurtbox.y - 32},
          WHITE);
    }
  }

  if (golem->mode == MOB_ATTACK) {

    if (player_state->player->x >=
        golem->hurtbox.x + golem->hurtbox.width / 2.0) {
      golem->direction = 1;
      player_state->player->x = golem->hurtbox.x + golem->hurtbox.width - 10;

    }

    else {
      golem->direction = 0;
      player_state->player->x =
          golem->hurtbox.x - player_state->player->width + 10;
    }

    if (golem->direction) {

      golem->golem_attack.time_passed += GetFrameTime();

      if (golem->golem_attack.time_passed >= golem->golem_attack.time_needed) {
        golem->golem_attack.current_frame_no++;
        golem->golem_attack.time_passed = 0;
      }

      if (golem->golem_attack.current_frame_no == 4) {
        PlaySound(*golem_attack);
      }

      if (golem->golem_attack.current_frame_no >
          golem->golem_attack.frame_count) {
        golem->golem_attack.current_frame_no = 1;
        golem->mode = MOB_IDLE;
        golem->idle_buffer = .5;
        player_state->is_being_hit = false;
        player_state->hp -= golem->damage;

        return;
      }

      golem->golem_attack.current_frame_rec.x =
          (golem->golem_attack.current_frame_no - 1) *
          golem->golem_attack.current_frame_rec.width;

      DrawTextureRec(
          golem->golem_attack.sprite, golem->golem_attack.current_frame_rec,
          (Vector2){.x = golem->hurtbox.x - 25, .y = golem->hurtbox.y - 32},
          WHITE);
    }

    else {

      golem->golem_attack_back.time_passed += GetFrameTime();

      if (golem->golem_attack_back.time_passed >=
          golem->golem_attack_back.time_needed) {
        golem->golem_attack_back.current_frame_no++;
        golem->golem_attack_back.time_passed = 0;
      }

      if (golem->golem_attack_back.current_frame_no == 4) {
        PlaySound(*golem_attack);
      }

      if (golem->golem_attack_back.current_frame_no >
          golem->golem_attack_back.frame_count) {
        golem->golem_attack_back.current_frame_no = 1;
        golem->mode = MOB_IDLE;
        golem->idle_buffer = .5;
        player_state->is_being_hit = false;
        player_state->hp -= golem->damage;

        return;
      }

      golem->golem_attack_back.current_frame_rec.x =
          (golem->golem_attack_back.current_frame_no - 1) *
          golem->golem_attack_back.current_frame_rec.width;

      DrawTextureRec(
          golem->golem_attack_back.sprite,
          golem->golem_attack_back.current_frame_rec,
          (Vector2){.x = golem->hurtbox.x - 29, .y = golem->hurtbox.y - 32},
          WHITE);
    }
  }

  if (golem->mode == MOB_DYING) {

    if (golem->direction) {

      golem->golem_death.time_passed += GetFrameTime();

      if (golem->golem_death.time_passed >= golem->golem_death.time_needed) {
        golem->golem_death.current_frame_no++;
        golem->golem_death.time_passed = 0;
      }

      if (golem->golem_death.current_frame_no == 2) {
        PlaySound(*golem_dead);
      }

      if (golem->golem_death.current_frame_no >
          golem->golem_death.frame_count) {
        golem->golem_death.current_frame_no = 1;
        golem->mode = MOB_DEAD;
        StopSound(*golem_dead);
        return;
      }

      golem->golem_death.current_frame_rec.x =
          (golem->golem_death.current_frame_no - 1) *
          golem->golem_death.current_frame_rec.width;

      DrawTextureRec(
          golem->golem_death.sprite, golem->golem_death.current_frame_rec,
          (Vector2){.x = golem->hurtbox.x - 25, .y = golem->hurtbox.y - 32},
          WHITE);
    }

    else {

      golem->golem_death_back.time_passed += GetFrameTime();

      if (golem->golem_death_back.time_passed >=
          golem->golem_death_back.time_needed) {
        golem->golem_death_back.current_frame_no++;
        golem->golem_death_back.time_passed = 0;
      }

      if (golem->golem_death_back.current_frame_no == 2) {
        PlaySound(*golem_dead);
      }

      if (golem->golem_death_back.current_frame_no >
          golem->golem_death_back.frame_count) {
        golem->golem_death_back.current_frame_no = 1;
        golem->mode = MOB_DEAD;
        StopSound(*golem_dead);
        return;
      }

      golem->golem_death_back.current_frame_rec.x =
          (golem->golem_death_back.current_frame_no - 1) *
          golem->golem_death_back.current_frame_rec.width;

      DrawTextureRec(
          golem->golem_death_back.sprite,
          golem->golem_death_back.current_frame_rec,
          (Vector2){.x = golem->hurtbox.x - 29, .y = golem->hurtbox.y - 32},
          WHITE);
    }
  }

  if (golem->mode == MOB_WALK) {

    if (golem->direction) {

      golem->golem_walk.time_passed += GetFrameTime();

      if (golem->golem_walk.time_passed >= golem->golem_walk.time_needed) {
        golem->golem_walk.current_frame_no++;
        golem->golem_walk.time_passed = 0;
      }

      if (golem->golem_walk.current_frame_no > golem->golem_walk.frame_count) {
        golem->golem_walk.current_frame_no = 1;
        return;
      }

      golem->golem_walk.current_frame_rec.x =
          (golem->golem_walk.current_frame_no - 1) *
          golem->golem_walk.current_frame_rec.width;

      DrawTextureRec(
          golem->golem_walk.sprite, golem->golem_walk.current_frame_rec,
          (Vector2){.x = golem->hurtbox.x - 25, .y = golem->hurtbox.y - 32},
          WHITE);
    }

    else {

      golem->golem_walk_back.time_passed += GetFrameTime();

      if (golem->golem_walk_back.time_passed >=
          golem->golem_walk_back.time_needed) {
        golem->golem_walk_back.current_frame_no++;
        golem->golem_walk_back.time_passed = 0;
      }

      if (golem->golem_walk_back.current_frame_no >
          golem->golem_walk_back.frame_count) {
        golem->golem_walk_back.current_frame_no = 1;
        return;
      }

      golem->golem_walk_back.current_frame_rec.x =
          (golem->golem_walk_back.current_frame_no - 1) *
          golem->golem_walk_back.current_frame_rec.width;

      DrawTextureRec(
          golem->golem_walk_back.sprite,
          golem->golem_walk_back.current_frame_rec,
          (Vector2){.x = golem->hurtbox.x - 29, .y = golem->hurtbox.y - 32},
          WHITE);
    }
  }

  if (golem->mode == MOB_RUN) {

    if (golem->direction) {

      golem->golem_run.time_passed += GetFrameTime();

      if (golem->golem_run.time_passed >= golem->golem_run.time_needed) {
        golem->golem_run.current_frame_no++;
        golem->golem_run.time_passed = 0;
      }

      if (golem->golem_run.current_frame_no > golem->golem_run.frame_count) {
        golem->golem_run.current_frame_no = 1;
        return;
      }

      golem->golem_run.current_frame_rec.x =
          (golem->golem_run.current_frame_no - 1) *
          golem->golem_run.current_frame_rec.width;

      DrawTextureRec(
          golem->golem_run.sprite, golem->golem_run.current_frame_rec,
          (Vector2){.x = golem->hurtbox.x - 25, .y = golem->hurtbox.y - 32},
          WHITE);
    }

    else {

      golem->golem_run_back.time_passed += GetFrameTime();

      if (golem->golem_run_back.time_passed >=
          golem->golem_run_back.time_needed) {
        golem->golem_run_back.current_frame_no++;
        golem->golem_run_back.time_passed = 0;
      }

      if (golem->golem_run_back.current_frame_no >
          golem->golem_run_back.frame_count) {
        golem->golem_run_back.current_frame_no = 1;
        return;
      }

      golem->golem_run_back.current_frame_rec.x =
          (golem->golem_run_back.current_frame_no - 1) *
          golem->golem_run_back.current_frame_rec.width;

      DrawTextureRec(
          golem->golem_run_back.sprite, golem->golem_run_back.current_frame_rec,
          (Vector2){.x = golem->hurtbox.x - 29, .y = golem->hurtbox.y - 32},
          WHITE);
    }
  }
}

bool SimpleCollisionResolver(Rectangle *rec1, Rectangle *rec2,
                             PlayerState *player_state) {
  if (SimpleCollisionCheck(rec1, rec2)) {
    if (rec1->y + rec1->height <= rec2->y + 10) {
      rec1->y = rec2->y - rec1->height;
      player_state->jump_left = 2;
    }

    else if (rec1->x + rec1->width <= rec2->x + rec2->width / 2.0) {
      rec1->x = rec2->x - rec1->width - 0.5;
    }

    else {
      rec1->x = rec2->x + rec2->width + 0.5;
    }

    return 1;
  }

  return 0;
}

void UpdateTrampoline(Trampoline *trampoline, PlayerState *player_state) {

  Rectangle trampoline_hitbox = (Rectangle){.x = trampoline->position.x,
                                            .y = trampoline->position.y + 20,
                                            .height = 8,
                                            .width = 28};

  if (SimpleCollisionCheck(player_state->player, &trampoline_hitbox)) {
    trampoline->mode = TRAMPOLINE_ACTIVE;
  }

  if (trampoline->mode == TRAMPOLINE_ACTIVE) {

    if (trampoline->current_frame_no == 2) {
      player_state->player->y -= 20;
    } else if (trampoline->current_frame_no <= 5) {
      player_state->player->y -= 6 + (.1 * trampoline->current_frame_no);

      if (IsKeyDown(KEY_D)) {
        player_state->player->x += .9;
      }

      if (IsKeyDown(KEY_A)) {
        player_state->player->x -= .9;
      }
    }
  }
}

void DrawTrampoline(Trampoline *trampoline, Sound* sound_spring) {
  if (trampoline->mode == TRAMPOLINE_IDLE) {
    DrawTextureRec(
        trampoline->sprite, trampoline->current_frame_rec,
        (Vector2){.x = trampoline->position.x, .y = trampoline->position.y},
        WHITE);
  }

  if (trampoline->mode == TRAMPOLINE_ACTIVE) {
    trampoline->time_passed += GetFrameTime();

    if(trampoline->current_frame_no == 1){
       PlaySound(*sound_spring);
    }

    if (trampoline->time_passed >= trampoline->time_needed) {
      trampoline->time_passed = 0;

      trampoline->current_frame_no++;
    }

    if (trampoline->current_frame_no > trampoline->frame_count) {
      trampoline->current_frame_no = 1;
      trampoline->mode = TRAMPOLINE_IDLE;
      return;
    }

    trampoline->current_frame_rec.x =
        (trampoline->current_frame_no - 1) * (trampoline->width);

    DrawTextureRec(
        trampoline->sprite, trampoline->current_frame_rec,
        (Vector2){.x = trampoline->position.x, .y = trampoline->position.y},
        WHITE);
  }
}

void UpdateGolemR(MobGolemR *golemr, PlayerState *player_state, Sound* sound_golemr_collision) {

  if (golemr->hp <= 0) {
    if (!(golemr->mode == MOB_DEAD)) {
      golemr->mode = MOB_DYING;
    }
    return;
  }

  if (golemr->idle_buffer > 0) {
    golemr->idle_buffer -= GetFrameTime();
    golemr->mode = MOB_IDLE;
    return;
  }

  else {
    golemr->idle_buffer = 0;
    golemr -> mode = MOB_WALK;
  }

  if (golemr->no_damage_time > 0) {
    golemr->no_damage_time -= GetFrameTime();
  } else {
    golemr->no_damage_time = 0;
  }

  if (SimpleCollisionResolver(player_state->player, &golemr->hurtbox,
                              player_state)) {
    if (golemr->no_damage_time == 0) {
      player_state->hp -= 0.5;
      golemr->no_damage_time = 1;

      PlaySound(*sound_golemr_collision);
    }
  };

  if (golemr->mode == MOB_IDLE) {
  }

  if (golemr->mode == MOB_WALK) {
    if (golemr->hurtbox.x <= golemr->lower_bound.x) {
      golemr->direction = 1;
    }

    if (golemr->hurtbox.x >= golemr->upper_bound.x) {
      golemr->direction = 0;
    }
    golemr->hurtbox.x += (golemr->direction ? (1) : (-1)) * golemr->walk_speed;
  }

  if (diff(golemr->hurtbox.x + golemr->hurtbox.width / 2.0,
           player_state->player->x) <= 7 * 32 &&
      (player_state->player->y + player_state->player->height >
       golemr->hurtbox.y) &&
      (player_state->player->y < golemr->hurtbox.height + golemr->hurtbox.y) &&
      !(golemr->bullet_released)) {
    golemr->mode = MOB_ATTACK;

    if (player_state->player->x < golemr->hurtbox.x) {
      golemr->direction = 0;
    } else {
      golemr->direction = 1;
    }
  }
}

void DrawGolemRHP(MobGolemR *golemr) {

#ifdef DEBUG
  DrawRectangleRec(golemr->hurtbox, BLUE);
#endif

  Rectangle back =
      (Rectangle){.width = 55,
                  .height = 4,
                  .x = golemr->hurtbox.x + golemr->hurtbox.width / 2.0 - 27.5,
                  .y = golemr->hurtbox.y - 20};

  Rectangle hp_bar = (Rectangle){.width = back.width * golemr->hp,
                                 .height = 4,
                                 .x = back.x,
                                 .y = golemr->hurtbox.y - 20};

  Color color;

  if (golemr->hp > 0.56) {
    color = (Color){22, 196, 127, 255};
  }

  else if (golemr->hp <= 0.56 && golemr->hp >= 20.6) {
    color = (Color){255, 214, 90, 255};
  }

  else {
    color = (Color){249, 56, 39, 255};
  }

  DrawRectangleRounded(back, 1, 0, LIGHTGRAY);
  DrawRectangleRounded(hp_bar, 1, 0, color);
}

void UpdateBullet(MobGolemR *golemr, PlayerState *player_state, Sound* sound_bullet_hit) {

  if (golemr->bullet_direction) {

    if (SimpleCollisionCheck(player_state->player, &golemr->bullet_hitbox)) {
      player_state->hp -= .20;
      golemr->bullet_released = false;

          PlaySound(*sound_bullet_hit);
    }

    else if (golemr->bullet_hitbox.x >= golemr->bullet_destination.x) {
      golemr->bullet_released = false;
    }

    else {
      golemr->bullet_hitbox.x += golemr->bullet_speed;
    }
  }

  else {
    if (SimpleCollisionCheck(player_state->player, &golemr->bullet_hitbox)) {
      player_state->hp -= .20;
      golemr->bullet_released = false;
      PlaySound(*sound_bullet_hit);
    }

    else if (golemr->bullet_hitbox.x <= golemr->bullet_destination.x) {
      golemr->bullet_released = false;
    }

    else {
      golemr->bullet_hitbox.x -= golemr->bullet_speed;
    }
  }
}

void DrawBullet(MobGolemR *golemr) {

  if (golemr->bullet_direction) {

    if (golemr->golemr_bullet.time_passed >=
        golemr->golemr_bullet.time_needed) {
      golemr->golemr_bullet.time_passed = 0;
      golemr->golemr_bullet.current_frame_no++;
    }

    if (golemr->golemr_bullet.current_frame_no >
        golemr->golemr_bullet.frame_count) {
      golemr->golemr_bullet.current_frame_no = 1;
    }

    golemr->golemr_bullet.current_frame_rec.x =
        (golemr->golemr_bullet.current_frame_no - 1) * 64;

    DrawTextureRec(golemr->golemr_bullet.sprite,
                   golemr->golemr_bullet.current_frame_rec,
                   (Vector2){.x = golemr->bullet_hitbox.x - 23,
                             .y = golemr->bullet_hitbox.y - 28},
                   WHITE);
  }

  else {
    if (golemr->golemr_bullet_back.time_passed >=
        golemr->golemr_bullet_back.time_needed) {
      golemr->golemr_bullet_back.time_passed = 0;
      golemr->golemr_bullet_back.current_frame_no++;
    }

    if (golemr->golemr_bullet_back.current_frame_no >
        golemr->golemr_bullet_back.frame_count) {
      golemr->golemr_bullet_back.current_frame_no = 1;
    }

    golemr->golemr_bullet_back.current_frame_rec.x =
        (golemr->golemr_bullet_back.current_frame_no - 1) * 64;

    DrawTextureRec(golemr->golemr_bullet_back.sprite,
                   golemr->golemr_bullet_back.current_frame_rec,
                   (Vector2){.x = golemr->bullet_hitbox.x - 23,
                             .y = golemr->bullet_hitbox.y - 28},
                   WHITE);
  }
}

void DrawGolemR(MobGolemR *golemr, PlayerState *player_state,
                Sound *golemr_dead, Sound* golemr_bullet, Sound* sound_bullet_hit) {

  DrawGolemRHP(golemr);

  if (golemr->mode == MOB_IDLE) {

    if (golemr->direction) {
      DrawTextureRec(
          golemr->golemr_walk.sprite,
          (Rectangle){.x = 0, .y = 0, .width = 64, .height = 64},
          (Vector2){.x = golemr->hurtbox.x - 25, .y = golemr->hurtbox.y - 32},
          WHITE);
    }

    else {
      DrawTextureRec(
          golemr->golemr_walk_back.sprite,
          (Rectangle){.x = 0, .y = 0, .width = 64, .height = 64},
          (Vector2){.x = golemr->hurtbox.x - 25, .y = golemr->hurtbox.y - 32},
          WHITE);
    }
  }

  if (golemr->mode == MOB_WALK) {

    if (golemr->direction) {

      golemr->golemr_walk.time_passed += GetFrameTime();

      if (golemr->golemr_walk.time_passed >= golemr->golemr_walk.time_needed) {
        golemr->golemr_walk.time_passed = 0;
        golemr->golemr_walk.current_frame_no++;
      }

      if (golemr->golemr_walk.current_frame_no >
          golemr->golemr_walk.frame_count) {
        golemr->golemr_walk.current_frame_no = 1;
      }

      golemr->golemr_walk.current_frame_rec.x =
          (golemr->golemr_walk.current_frame_no - 1) * (96.75);

      DrawTextureRec(
          golemr->golemr_walk.sprite, golemr->golemr_walk.current_frame_rec,
          (Vector2){.x = golemr->hurtbox.x - 25, .y = golemr->hurtbox.y - 32},
          WHITE);

    }

    else {

      golemr->golemr_walk_back.time_passed += GetFrameTime();

      if (golemr->golemr_walk_back.time_passed >=
          golemr->golemr_walk_back.time_needed) {
        golemr->golemr_walk_back.time_passed = 0;
        golemr->golemr_walk_back.current_frame_no++;
      }

      if (golemr->golemr_walk_back.current_frame_no >
          golemr->golemr_walk_back.frame_count) {
        golemr->golemr_walk_back.current_frame_no = 1;
      }

      golemr->golemr_walk_back.current_frame_rec.x =
          (golemr->golemr_walk_back.current_frame_no - 1) * (96.75);

      DrawTextureRec(
          golemr->golemr_walk_back.sprite,
          golemr->golemr_walk_back.current_frame_rec,
          (Vector2){.x = golemr->hurtbox.x - 29, .y = golemr->hurtbox.y - 32},
          WHITE);
    }
  }

  if (golemr->mode == MOB_ATTACK) {
    if (player_state->player->x >= golemr->hurtbox.x) {

      golemr->golemr_attack.time_passed += GetFrameTime();

      if (golemr->golemr_attack.time_passed >=
          golemr->golemr_attack.time_needed) {
        golemr->golemr_attack.time_passed = 0;
        golemr->golemr_attack.current_frame_no++;
      }

      if (golemr->golemr_attack.current_frame_no == 7) {


          PlaySound(*golemr_bullet);
        golemr->bullet_released = true;
        golemr->bullet_direction = golemr->direction;
        golemr->bullet_destination =
            (Vector2){.x = golemr->hurtbox.x +
                           7 * 32 * (golemr->bullet_direction ? (1) : (-1)),
                      .y = golemr->hurtbox.y};

        golemr->bullet_hitbox.x = golemr->hurtbox.x + 20;
        golemr->bullet_hitbox.y = golemr->hurtbox.y + 20;
      }

      if (golemr->golemr_attack.current_frame_no >
          golemr->golemr_attack.frame_count) {
        golemr->golemr_attack.current_frame_no = 1;
        golemr->mode = MOB_WALK;
        return;
      }

      golemr->golemr_attack.current_frame_rec.x =
          (golemr->golemr_attack.current_frame_no - 1) * (99.3);

      DrawTextureRec(
          golemr->golemr_attack.sprite, golemr->golemr_attack.current_frame_rec,
          (Vector2){.x = golemr->hurtbox.x - 25, .y = golemr->hurtbox.y - 32},
          WHITE);

    } else {

      golemr->golemr_attack_back.time_passed += GetFrameTime();

      if (golemr->golemr_attack_back.time_passed >=
          golemr->golemr_attack_back.time_needed) {
        golemr->golemr_attack_back.time_passed = 0;
        golemr->golemr_attack_back.current_frame_no++;
      }

      if (golemr->golemr_attack_back.current_frame_no == 7) {

          PlaySound(*golemr_bullet);

        golemr->bullet_released = true;
        golemr->bullet_direction = golemr->direction;
        golemr->bullet_destination =
            (Vector2){.x = golemr->hurtbox.x +
                           7 * 32 * (golemr->bullet_direction ? (1) : (-1)),
                      .y = golemr->hurtbox.y};

        golemr->bullet_hitbox.x = golemr->hurtbox.x + 20;
        golemr->bullet_hitbox.y = golemr->hurtbox.y + 20;
      }

      if (golemr->golemr_attack_back.current_frame_no >
          golemr->golemr_attack_back.frame_count) {
        golemr->golemr_attack_back.current_frame_no = 1;
        golemr->mode = MOB_WALK;
        return;
      }

      golemr->golemr_attack_back.current_frame_rec.x =
          (golemr->golemr_attack_back.current_frame_no - 1) * (99.3);

      DrawTextureRec(
          golemr->golemr_attack_back.sprite,
          golemr->golemr_attack_back.current_frame_rec,
          (Vector2){.x = golemr->hurtbox.x - 25, .y = golemr->hurtbox.y - 32},
          WHITE);
    }
  }

  if (golemr->mode == MOB_DYING) {

    if (golemr->direction) {

      golemr->golemr_death.time_passed += GetFrameTime();

      if (golemr->golemr_death.time_passed >=
          golemr->golemr_death.time_needed) {
        golemr->golemr_death.current_frame_no++;
        golemr->golemr_death.time_passed = 0;
      }

      if (golemr->golemr_death.current_frame_no == 2) {
        PlaySound(*golemr_dead);
      }

      if (golemr->golemr_death.current_frame_no >
          golemr->golemr_death.frame_count) {
        golemr->golemr_death.current_frame_no = 1;
        golemr->mode = MOB_DEAD;
        StopSound(*golemr_dead);
        return;
      }

      golemr->golemr_death.current_frame_rec.x =
          (golemr->golemr_death.current_frame_no - 1) *
          golemr->golemr_death.current_frame_rec.width;

      DrawTextureRec(
          golemr->golemr_death.sprite, golemr->golemr_death.current_frame_rec,
          (Vector2){.x = golemr->hurtbox.x - 25, .y = golemr->hurtbox.y - 32},
          WHITE);
    }

    else {

      golemr->golemr_death_back.time_passed += GetFrameTime();

      if (golemr->golemr_death_back.time_passed >=
          golemr->golemr_death_back.time_needed) {
        golemr->golemr_death_back.current_frame_no++;
        golemr->golemr_death_back.time_passed = 0;
      }

      if (golemr->golemr_death_back.current_frame_no == 2) {
        PlaySound(*golemr_dead);
      }

      if (golemr->golemr_death_back.current_frame_no >
          golemr->golemr_death_back.frame_count) {
        golemr->golemr_death_back.current_frame_no = 1;
        golemr->mode = MOB_DEAD;
        StopSound(*golemr_dead);
        return;
      }

      golemr->golemr_death_back.current_frame_rec.x =
          (golemr->golemr_death_back.current_frame_no - 1) *
          golemr->golemr_death_back.current_frame_rec.width;

      DrawTextureRec(
          golemr->golemr_death_back.sprite,
          golemr->golemr_death_back.current_frame_rec,
          (Vector2){.x = golemr->hurtbox.x - 29, .y = golemr->hurtbox.y - 32},
          WHITE);
    }
  }

  if (golemr->bullet_released && golemr->mode != MOB_IDLE) {

      #ifdef DEBUG

    DrawRectangleRec(golemr->bullet_hitbox, YELLOW);

    #endif


    UpdateBullet(golemr, player_state, sound_bullet_hit);
    DrawBullet(golemr);
  }
}

int main() {
  Menus menu = MAIN_MENU;

  WindowState window = (WindowState){.width = 1600, .height = 896, .fps = 60};
  PlatformState platform = (PlatformState){.tile_width = 32, .tile_height = 32};

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(window.width, window.height, "GGG");

  Texture2D bg = LoadTexture("resources/bg/bg.png");


  Texture2D tileset_texture = LoadTexture("resources/tiles/Tileset.png");

  TilesetState tileset = (TilesetState){.texture = &tileset_texture,
                                        .cols = 12,
                                        .rows = 8,
                                        .tile_width = 32,
                                        .tile_height = 32};

  TilemapState tilemap = (TilemapState){.tileset = &tileset,
                                        .width = 50 * tileset.tile_width,
                                        .height = 48 * tileset.tile_height};

  Rectangle player = (Rectangle){.x = 0, .y = 1160, .width = 32, .height = 64};
  PlayerState player_state = (PlayerState){
      .player = &player,
      .speed = 5.7,
      .falling_speed = 0,
      .gravity = 1.5,
      .jump_left = 0,
      .air_time_passed = 0,
      .is_grounded = false,
      .in_jump = false,
      .rising_speed = 7,
      .sliding_speed = 2,
      .terminal_velocity = 10,
      .last_direction = 1,
      .hp = 1,
      .in_attack_light = false,
      .is_attack_hit = 1,
      .attack_light_damage = 0.4,
      .attack_light_time_needed = 0.05,
      .attack_light_time_passed = 0,
      .attack_light_frame_count = 15,
      .attack_light_current_frame_no = 1,
      .attack_light_current_frame_rec =
          (Rectangle){.width = 384, .height = 192, .x = 0, .y = 0},
      .attack_light_sprite = LoadTexture("resources/hero/hero-attack.png"),
      .attack_light_sprite_back =
          LoadTexture("resources/hero/hero-attack-back.png"),
      .is_being_hit = false,
      .death_sound = false

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
      .speed = 1.7,
      .position = (Vector2){.x = 45 * tileset.tile_width,
                            .y = 40 * tileset.tile_height},
      .type = VERTICAL,
      .lower_bound = (Vector2){.x = 45 * tileset.tile_width,
                               .y = 40 * tileset.tile_height},
      .upper_bound = (Vector2){.x = 45 * tileset.tile_width,
                               .y = 31 * tileset.tile_height},
      .direction = 1,

  };

  floaters[1] =
      (Floater){.floater = (int[]){44, 45, 46},
                .floater_width = 3,
                .speed = 1.7,
                .position = (Vector2){.x = 31 * tileset.tile_width,
                                      .y = 31 * tileset.tile_height},
                .type = HORIZONTAL,
                .upper_bound = (Vector2){.x = 31 * tileset.tile_width,
                                         .y = 31 * tileset.tile_height},
                .lower_bound = (Vector2){.x = 22 * tileset.tile_width,
                                         .y = 31 * tileset.tile_height},
                .direction = 0

      };

  floaters[2] = (Floater){
      .floater = (int[]){44, 45, 45, 46},
      .floater_width = 4,
      .speed = 1.9,
      .position =
          (Vector2){.x = 2 * tileset.tile_width, .y = 31 * tileset.tile_height},
      .type = VERTICAL,
      .upper_bound =
          (Vector2){.x = 2 * tileset.tile_width, .y = 15 * tileset.tile_height},
      .lower_bound =
          (Vector2){.x = 2 * tileset.tile_width, .y = 31 * tileset.tile_height},
      .direction = 0

  };

  PlayerMode current_mode = IDLE;

  int animation_states_count = 6;
  AnimationState animation_states[animation_states_count];

  InitAnimationStates(animation_states);

  InitAudioDevice();

  Sound sound_walking = LoadSound("resources/audio/running_in_grass.mp3");
  Sound death_scream = LoadSound("resources/audio/death_scream.mp3");
  Sound sound_jump = LoadSound("resources/audio/jump.mp3");
  Sound sound_attack_light = LoadSound("resources/audio/attack_light.mp3");
  Sound sound_golem_hit = LoadSound("resources/audio/golem-hit.mp3");
  Sound sound_golem_attack = LoadSound("resources/audio/golem-attack.wav");
  Sound sound_golem_dead = LoadSound("resources/audio/golem-dead.mp3");
  Sound sound_golemr_collision = LoadSound("resources/audio/golemr_collision.wav");
  Sound sound_bullet =  LoadSound("resources/audio/bullet.wav");
  Sound sound_bullet_hit =  LoadSound("resources/audio/bullet_hit.mp3");
  Sound sound_spring=  LoadSound("resources/audio/spring.mp3");
  Sound sound_nature=  LoadSound("resources/audio/nature.mp3");
  SetTargetFPS(window.fps);

  Trap trap = (Trap){

      .width = 38,
      .height = 38,
      .damage = 1,
      .position =
          (Vector2){.x = 8 * tilemap.tileset->tile_width,
                    .y = 31 * tilemap.tileset->tile_height - trap.height},
      .hitbox = (Rectangle){.width = trap.width,
                            .height = trap.height,
                            .x = trap.position.x,
                            .y = trap.position.y},
      .sprite_sheet = LoadTexture("resources/traps/saw.png"),
      .frame_count = 10,
      .current_frame_no = 1,
      .current_frame_rec =
          (Rectangle){
              .width = trap.width, .height = trap.height, .x = 0, .y = 0},
      .time_needed = 0.05,
      .time_passed = 0,
      .lower_bound =
          (Vector2){.x = 8 * tilemap.tileset->tile_width + 15,
                    .y = 31 * tilemap.tileset->tile_height - trap.height},
      .upper_bound =
          (Vector2){.x = 20 * tilemap.tileset->tile_width,
                    .y = 31 * tilemap.tileset->tile_height - trap.height},
      .speed = 1,
      .max_speed = 10,
      .direction = 0

  };

  Font font_press_start =
      LoadFontEx("resources/fonts/PressStart2P-Regular.ttf", 80, 0, 250);
  Font font_jetbrains_mono =
      LoadFontEx("resources/fonts/JetBrainsMono-Regular.ttf", 40, 0, 250);

  MobGolem golem = {

      .mode = MOB_WALK,
      .hp = 1,
      .hurtbox = (Rectangle){.x = 12 * tilemap.tileset->tile_width,
                             .y = 21 * tilemap.tileset->tile_height + 24,
                             .width = 40,
                             .height = 40},
      .lower_bound = (Vector2){.x = 11 * tilemap.tileset->tile_width,
                               .y = 21 * tilemap.tileset->tile_height + 24},
      .upper_bound = (Vector2){.x = 21 * tilemap.tileset->tile_width,
                               .y = 21 * tilemap.tileset->tile_height + 24},

      .damage = 0.35,
      .walk_speed = 1,
      .run_speed = 3,
      .idle_buffer = 0,

      .direction = 1,
      .saw = 1,
      .player_direction_rel = 1,

      .golem_walk =
          (MobAnimation){
              .sprite = LoadTexture("resources/mob/golem/golem_walk.png"),
              .frame_count = 4,
              .current_frame_no = 1,
              .current_frame_rec =
                  (Rectangle){.x = 0, .y = 0, .width = 95.25, .height = 64},
              .time_needed = .12,
              .time_passed = 0

          },

      .golem_walk_back =
          (MobAnimation){
              .sprite = LoadTexture("resources/mob/golem/golem_walk_back.png"),
              .frame_count = 4,
              .current_frame_no = 1,
              .current_frame_rec =
                  (Rectangle){.x = 0, .y = 0, .width = 95.25, .height = 64},
              .time_needed = .12,
              .time_passed = 0

          },

      .golem_run =
          (MobAnimation){

              .sprite = LoadTexture("resources/mob/golem/golem_run.png"),
              .frame_count = 4,
              .current_frame_no = 1,
              .current_frame_rec =
                  (Rectangle){.x = 0, .y = 0, .width = 96.25, .height = 64},
              .time_needed = .07,
              .time_passed = 0},

      .golem_run_back =
          (MobAnimation){

              .sprite = LoadTexture("resources/mob/golem/golem_run_back.png"),
              .frame_count = 4,
              .current_frame_no = 1,
              .current_frame_rec =
                  (Rectangle){.x = 0, .y = 0, .width = 96.25, .height = 64},
              .time_needed = .07,
              .time_passed = 0},

      .golem_attack =
          (MobAnimation){

              .sprite = LoadTexture("resources/mob/golem/golem_attack.png"),
              .frame_count = 7,
              .current_frame_no = 1,
              .current_frame_rec =
                  (Rectangle){.x = 0, .y = 0, .width = 98, .height = 64},
              .time_needed = .07,
              .time_passed = 0},

      .golem_attack_back =
          (MobAnimation){

              .sprite =
                  LoadTexture("resources/mob/golem/golem_attack_back.png"),
              .frame_count = 7,
              .current_frame_no = 1,
              .current_frame_rec =
                  (Rectangle){.x = 0, .y = 0, .width = 95, .height = 64},
              .time_needed = .07,
              .time_passed = 0},

      .golem_death =
          (MobAnimation){

              .sprite = LoadTexture("resources/mob/golem/golem_death.png"),
              .frame_count = 9,
              .current_frame_no = 1,
              .current_frame_rec =
                  (Rectangle){.x = 0, .y = 0, .width = 101.5, .height = 64},
              .time_needed = .07,
              .time_passed = 0},

      .golem_death_back =
          (MobAnimation){

              .sprite = LoadTexture("resources/mob/golem/golem_death_back.png"),
              .frame_count = 9,
              .current_frame_no = 1,
              .current_frame_rec =
                  (Rectangle){.x = 0, .y = 0, .width = 101, .height = 64},
              .time_needed = .07,
              .time_passed = 0}

  };

  MobGolemR golemr = {

      .mode = MOB_ATTACK,
      .hp = 1,

      .bullet_hitbox = (Rectangle){.x = 0, .y = 0, .width = 20, .height = 5},

      .hurtbox = (Rectangle){.x = 24 * tilemap.tileset->tile_width,
                             .y = 13 * tilemap.tileset->tile_height + 24,
                             .width = 40,
                             .height = 40},
      .lower_bound = (Vector2){.x = 24 * tilemap.tileset->tile_width,
                               .y = 13 * tilemap.tileset->tile_height + 24},
      .upper_bound = (Vector2){.x = 28 * tilemap.tileset->tile_width,
                               .y = 13 * tilemap.tileset->tile_height + 24},

      .damage = 0.45,
      .walk_speed = 1,
      .bullet_speed = 9,
      .idle_buffer = 0,
      .no_damage_time = 0,

      .direction = 1,
      .player_direction_rel = 1,
      .bullet_released = false,
      .bullet_direction = 1,

      .bullet_destination = (Vector2){.x = 0, .y = 0},

      .golemr_walk =
          (MobAnimation){
              .sprite = LoadTexture("resources/mob/golem_r/golem_r_idle.png"),
              .frame_count = 4,
              .current_frame_no = 1,
              .current_frame_rec =
                  (Rectangle){.x = 0, .y = 0, .width = 96.75, .height = 64},
              .time_needed = .15,
              .time_passed = 0

          },
      .golemr_walk_back =
          (MobAnimation){
              .sprite =
                  LoadTexture("resources/mob/golem_r/golem_r_idle_back.png"),
              .frame_count = 4,
              .current_frame_no = 1,
              .current_frame_rec =
                  (Rectangle){.x = 0, .y = 0, .width = 97, .height = 64},
              .time_needed = .15,
              .time_passed = 0

          },

      .golemr_attack =
          (MobAnimation){

              .sprite = LoadTexture("resources/mob/golem_r/golem_r_attack.png"),
              .frame_count = 10,
              .current_frame_no = 1,
              .current_frame_rec =
                  (Rectangle){.x = 0, .y = 0, .width = 99.3, .height = 64},
              .time_needed = .09,
              .time_passed = 0},

      .golemr_attack_back =
          (MobAnimation){

              .sprite =
                  LoadTexture("resources/mob/golem_r/golem_r_attack_back.png"),
              .frame_count = 10,
              .current_frame_no = 1,
              .current_frame_rec =
                  (Rectangle){.x = 0, .y = 0, .width = 99, .height = 64},
              .time_needed = .11,
              .time_passed = 0},

      .golemr_bullet =
          (MobAnimation){

              .sprite = LoadTexture("resources/mob/golem_r/golem_r_bullet.png"),
              .frame_count = 3,
              .current_frame_no = 1,
              .current_frame_rec =
                  (Rectangle){.x = 0, .y = 0, .width = 64, .height = 64},
              .time_needed = .07,
              .time_passed = 0},

      .golemr_bullet_back =
          (MobAnimation){

              .sprite =
                  LoadTexture("resources/mob/golem_r/golem_r_bullet_back.png"),
              .frame_count = 3,
              .current_frame_no = 1,
              .current_frame_rec =
                  (Rectangle){.x = 0, .y = 0, .width = 64, .height = 64},
              .time_needed = .07,
              .time_passed = 0},

      .golemr_death =
          (MobAnimation){

              .sprite = LoadTexture("resources/mob/golem/golem_death.png"),
              .frame_count = 9,
              .current_frame_no = 1,
              .current_frame_rec =
                  (Rectangle){.x = 0, .y = 0, .width = 101.5, .height = 64},
              .time_needed = .07,
              .time_passed = 0},

      .golemr_death_back =
          (MobAnimation){

              .sprite = LoadTexture("resources/mob/golem/golem_death_back.png"),
              .frame_count = 9,
              .current_frame_no = 1,
              .current_frame_rec =
                  (Rectangle){.x = 0, .y = 0, .width = 101, .height = 64},
              .time_needed = .07,
              .time_passed = 0}

  };

  Trampoline trampoline = {

      .mode = TRAMPOLINE_IDLE,
      .width = 28,
      .height = 28,
      .position = (Vector2){.x = 10 * 32, .y = 15 * 32 - 28},
      .sprite = LoadTexture("resources/traps/trampoline.png"),
      .time_needed = 0.1,
      .time_passed = 0,
      .frame_count = 8,
      .current_frame_no = 1,
      .current_frame_rec =
          (Rectangle){.x = 0, .y = 0, .width = 28, .height = 28},
  };

  while (!WindowShouldClose()) {

      if(!IsSoundPlaying(sound_nature)){
          PlaySound(sound_nature);
      }


    if (menu == MAIN_MENU) {
      //  Draw phase
      BeginDrawing();
      // Clean, dark minimalist background
      // ClearBackground(GetColor(0x0f111aFF));
      ClearBackground((Color){15, 17, 26, 128});

      // 1. DRAW THE TITLE ("HOLLOW")
      const char titleText[] = "HOLLOW";
      float titleFontSize = 80.0;
      float titleSpacing = 2.0;

      // Calculate horizontal centering for the title
      Vector2 titleDim = MeasureTextEx(font_press_start, titleText,
                                       titleFontSize, titleSpacing);
      float titleX = (GetScreenWidth() - titleDim.x) / 2.0;
      float titleY =
          GetScreenHeight() / 2.0 - titleDim.y; // Positioned in the upper half

      // Draw the main title text
      // DrawText(titleText, titleX, titleY, titleFontSize, SKYBLUE);
      DrawTextEx(font_press_start, titleText,
                 (Vector2){.x = titleX, .y = titleY}, titleFontSize, 2,
                 SKYBLUE);

      // 2. DRAW THE SUBTITLE ("Press Enter to Play Game")
      const char *subText = "Press Enter to Play";
      float subFontSize = 40.0;
      float subSpacing = 2.0;

      // Calculate horizontal centering for the subtitle

      Vector2 subDim =
          MeasureTextEx(font_jetbrains_mono, subText, subFontSize, subSpacing);

      float subX = (GetScreenWidth() - subDim.x) / 2.0;
      float subY = titleY + 100; // Positioned in the lower half

      // Draw the subtitle text
      // DrawText(subText, subX, subY, subFontSize, LIGHTGRAY);
      DrawTextEx(font_jetbrains_mono, subText, (Vector2){.x = subX, .y = subY},
                 subFontSize, subSpacing, LIGHTGRAY);

      EndDrawing();
      if (IsKeyPressed(KEY_ENTER)) {
        menu = GAME_MENU;
      }
    }
    if (menu == GAME_MENU) {

      Update(&player_state, &camera, &tilemap, &current_mode);
      UpdateFloaters(floaters, &player_state);
      UpdateTrap(&trap, &player_state, &death_scream);
      UpdateGolem(&golem, &player_state, &death_scream, &sound_golem_attack);
      UpdateGolemR(&golemr, &player_state, &sound_golemr_collision);
      UpdateTrampoline(&trampoline, &player_state);

      BeginDrawing();

      BeginMode2D(camera);


      DrawTexture(bg, 0, 0, WHITE);



      if (IsKeyDown(KEY_T)) {
        ToggleFullscreen();
      }

      ClearBackground(RAYWHITE);

      DrawPlatform(&window, &platform, &tilemap, &player_state, floaters);
      DrawHP(&player_state);

#ifdef DEBUG
      DrawRectangleRec(trap.hitbox, BLUE);
      DrawRectangleRec((Rectangle){.x = trampoline.position.x,
                                   .y = trampoline.position.y + 20,
                                   .width = 28,
                                   .height = 8},
                       BLUE);
#endif
      DrawTrap(&trap);
      DrawTrampoline(&trampoline, &sound_spring);

#ifdef DEBUG
      DrawRectangleRec(player, RED);
#endif

      if (player_state.in_attack_light || IsKeyPressed(KEY_J)) {
        current_mode = ATTACK_LIGHT;
        HandleAttackLight(&player_state, &sound_attack_light, &golem,
                          &sound_golem_hit, &golemr);

      }

      else if (IsKeyPressed(KEY_SPACE) && !player_state.is_being_hit &&
               player_state.jump_left > 0) {

        PlaySound(sound_jump);

        player_state.in_jump = true;
        player_state.air_time_passed = 0;
        player_state.rising_speed = 5;

        player_state.jump_left--;
      }

      if (player_state.in_jump) {
        HandleJump(animation_states, &player_state);
        if ((golem.mode != MOB_DEAD)) {
          DrawGolem(&golem, &player_state, &sound_golem_attack,
                    &sound_golem_dead);
        }
        if ((golemr.mode != MOB_DEAD)) {
            DrawGolemR(&golemr, &player_state, &sound_golem_dead, &sound_bullet, &sound_bullet_hit);
        }
      }

      else if (player_state.is_grounded) {

        StopSound(sound_jump);

        // Sound of Walking Code//
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_A)) {
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

        if ((golem.mode != MOB_DEAD)) {
          DrawGolem(&golem, &player_state, &sound_golem_attack,
                    &sound_golem_dead);
        }

        if ((golemr.mode != MOB_DEAD)) {
          DrawGolemR(&golemr, &player_state, &sound_golem_dead, &sound_bullet, &sound_bullet_hit);
        }

      }

      else {

        StopSound(sound_walking);

        player_state.air_time_passed += GetFrameTime();

        player_state.falling_speed +=
            (player_state.gravity * player_state.air_time_passed);

        player_state.falling_speed =
            (player_state.falling_speed <= player_state.terminal_velocity)
                ? (player_state.falling_speed)
                : (player_state.terminal_velocity);

        player_state.player->y += player_state.falling_speed;

        if (!player_state.in_attack_light) {
          DrawJumpState(animation_states, 1, &player_state);
        }

        if ((golem.mode != MOB_DEAD)) {
          DrawGolem(&golem, &player_state, &sound_golem_attack,
                    &sound_golem_dead);
        }

        if ((golemr.mode != MOB_DEAD)) {
            DrawGolemR(&golemr, &player_state, &sound_golem_dead, &sound_bullet, &sound_bullet_hit);
        }
      }

      EndMode2D();
      EndDrawing();

      if ((player_state.player->y) > (tilemap.height - 32)) {
        player_state.hp = 0;
        StopSound(sound_walking);
      }
      // printf("%f \n", player_state.player->y);
    }

    if (player_state.hp <= 0) {

      if (!player_state.death_sound) {
        PlaySound(death_scream);
        player_state.death_sound = true;
      }

      menu = DEATH_MENU;
      BeginDrawing();
      ClearBackground(GetColor(0x590404FF));

      const char *subText = "Press Enter to Go to Main Menu";
      float subFontSize = 40.0;
      float subSpacing = 2.0;

      // Calculate horizontal centering for the subtitle

      Vector2 subDim =
          MeasureTextEx(font_jetbrains_mono, subText, subFontSize, subSpacing);

      float subX = (GetScreenWidth() - subDim.x) / 2.0;
      float subY =
          (GetScreenHeight() - subDim.y) / 2.0; // Positioned in the lower half

      // Draw the subtitle text
      // DrawText(subText, subX, subY, subFontSize, LIGHTGRAY);
      DrawTextEx(font_jetbrains_mono, subText, (Vector2){.x = subX, .y = subY},
                 subFontSize, subSpacing, LIGHTGRAY);

      EndDrawing();

      if (IsKeyPressed(KEY_ENTER)) {
        menu = MAIN_MENU;
        player_state.player->y =
            1000; // not resetting y causes insta death because player is still
                  // below falling level; see below comment;

#ifdef DEBUG
        player_state.player->x = 100;
        player_state.player->y = 0;
#endif

        player_state.player->x =
            0; // Need to reset all other (x,speed etc) too , but I think it
        // will be better to write a deathfunction and handle this using
        // that instead of everything else

        player_state.death_sound = false;
        player_state.last_direction = 1;
        player_state.in_jump = 0;
        player_state.in_attack_light = 0;
        player_state.hp = 1;
        golem.hp = 1;
        golem.mode = MOB_WALK;
        golemr.hp = 1;
        golemr.mode = MOB_WALK;
        ResetPlayerAirState(&player_state);
      }
    }
  }

  StopSound(sound_nature);

  UnloadSound(sound_walking);
  UnloadSound(death_scream);
  UnloadSound(sound_jump);
  UnloadSound(sound_attack_light);
  UnloadSound(sound_golem_hit);
  UnloadSound(sound_golem_attack);
  UnloadSound(sound_golem_attack);
  UnloadSound(sound_golem_dead);
  UnloadSound(sound_golemr_collision);
  UnloadSound(sound_bullet);
  UnloadSound(sound_bullet_hit);

  CloseAudioDevice();

  UnloadTexture(tileset_texture);
  for (int i = 0; i < animation_states_count; i++) {
    UnloadTexture(animation_states[i].sprite.sheet);
  }

  UnloadTexture(player_state.attack_light_sprite);
  UnloadTexture(player_state.attack_light_sprite_back);

  UnloadTexture(golem.golem_walk.sprite);
  UnloadTexture(golem.golem_run.sprite);
  UnloadTexture(golem.golem_attack.sprite);
  UnloadTexture(golem.golem_death.sprite);

  UnloadTexture(golem.golem_walk_back.sprite);
  UnloadTexture(golem.golem_run_back.sprite);
  UnloadTexture(golem.golem_attack_back.sprite);
  UnloadTexture(golem.golem_death_back.sprite);

  UnloadTexture(trampoline.sprite);

  UnloadTexture(golemr.golemr_walk.sprite);
  UnloadTexture(golemr.golemr_attack.sprite);
  UnloadTexture(golemr.golemr_bullet.sprite);

  UnloadTexture(golemr.golemr_walk_back.sprite);
  UnloadTexture(golemr.golemr_attack_back.sprite);
  UnloadTexture(golemr.golemr_bullet_back.sprite);

  CloseWindow();
}
