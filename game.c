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
  float acceleration;

} PlayerState;

void DrawPlatform(WindowState *window, PlatformState *platform,
                  TilemapState *tilemap) {

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

      DrawTextureRec(*(tilemap->tileset->texture), current_tile,
                     (Vector2){.x = j * 32, .y = i * 32}, WHITE);
    }
  }
}

void Update(PlayerState *player_state, Camera2D *camera,
            TilemapState *tilemap) {

  if (IsKeyDown(KEY_LEFT)) {
    player_state->player->x -= player_state->speed;
  }

   if (IsKeyDown(KEY_RIGHT)) {
    player_state->player->x += player_state->speed;
  }

   if (IsKeyDown(KEY_DOWN)) {
    player_state->player->y += player_state->speed;
  }

   if (IsKeyDown(KEY_UP)) {
    player_state->player->y -= player_state->speed;
  }



   int buffer_y = 4;

  if (player_state->player->x < 0) {
    player_state->player->x = 0;
  }

   if (player_state->player->x >
           (tilemap->width - player_state->player->width)) {
    player_state->player->x = tilemap->width - player_state->player->width;
  }


   if (player_state->player->y < 0) {
     player_state->player->y = buffer_y;
   }

   if (player_state->player->y >
           (tilemap->height - player_state->player->height)) {
    player_state->player->y = tilemap->height - player_state->player->height - buffer_y;
  }

  camera->target =
      (Vector2){.x = player_state->player->x, .y = player_state->player->y};

  // KEY_D checking is for only debugging.

  if (!IsKeyDown(KEY_D)) {

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
  PlayerState player_state =
      (PlayerState){.player = &player, .speed = 10, .acceleration = 0.0};

  Camera2D camera = (Camera2D){
      .offset =
          (Vector2){.x = GetScreenWidth() / 2.0, .y = GetScreenHeight() / 2.0},
      .target =
          (Vector2){.x = player_state.player->x, .y = player_state.player->y},
      .rotation = 0.0,
      .zoom = 1.7};

  SetTargetFPS(window.fps);

  while (!WindowShouldClose()) {

    Update(&player_state, &camera, &tilemap);

    BeginDrawing();

    BeginMode2D(camera);

    if (IsKeyDown(KEY_A)) {
      ToggleFullscreen();
    }

    ClearBackground(RAYWHITE);
    DrawPlatform(&window, &platform, &tilemap);
    DrawRectangleRec(player, RED);

    EndMode2D();
    EndDrawing();
  }

  UnloadTexture(tileset_texture);
}
