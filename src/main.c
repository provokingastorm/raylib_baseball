#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>

/* -------------------------------------------------------------------------
   Screenshot-inspired 8/16-bit baseball screen.
   No external textures are required. Everything is drawn with raylib
   primitives so the project is easy to build and modify.
   ------------------------------------------------------------------------- */

/* Window / virtual screen */
#define SCREEN_WIDTH 993
#define SCREEN_HEIGHT 728
#define TARGET_FPS 60

/* Field layout */
#define FIELD_LEFT 252
#define FIELD_RIGHT 747
#define FIELD_TOP 0
#define FIELD_BOTTOM 728
#define INFIELD_TOP_Y 505
#define HOME_PLATE_X 496
#define HOME_PLATE_Y 645
#define PITCHER_MOUND_X 496
#define PITCHER_MOUND_Y 213
#define BASE_HALF_SIZE 58

/* HUD layout */
#define HUD_LEFT_X 0
#define HUD_RIGHT_X 748
#define HUD_WIDTH 245
#define HUD_TOP_Y 0
#define HUD_HEIGHT 728
#define SCORE_BOX_X (HUD_RIGHT_X + (HUD_WIDTH - SCORE_BOX_WIDTH) / 2)
#define SCORE_BOX_Y 340
#define SCORE_BOX_WIDTH 140
#define SCORE_BOX_HEIGHT 88
#define COUNT_BOX_X (HUD_RIGHT_X + (HUD_WIDTH - COUNT_BOX_WIDTH) / 2)
#define COUNT_BOX_Y 449
#define COUNT_BOX_WIDTH 140
#define COUNT_BOX_HEIGHT 81

/* Pixel-art sizing */
#define PIXEL_UNIT 7
#define PLAYER_SCALE 1
#define TEXT_PIXEL_SIZE 3
#define SCORE_FONT_SIZE 29
#define SMALL_FONT_SIZE 22

/* Gameplay timing */
#define PITCH_FLIGHT_SECONDS 0.82f
#define BAT_SWING_SECONDS 0.18f
#define MESSAGE_SECONDS 1.25f
#define BALL_RADIUS 5

/* Palette */
static const Color COLOR_FIELD = {43, 113, 2, 255};
static const Color COLOR_FIELD_DARK = {32, 88, 2, 255};
static const Color COLOR_DIRT = {175, 71, 16, 255};
static const Color COLOR_DIRT_DARK = {145, 55, 12, 255};
static const Color COLOR_LINE = {242, 242, 236, 255};
static const Color COLOR_BLACK = {0, 0, 0, 255};
static const Color COLOR_WHITE = {245, 245, 239, 255};
static const Color COLOR_RED = {221, 57, 30, 255};
static const Color COLOR_BLUE = {11, 42, 152, 255};
static const Color COLOR_BLUE_DARK = {5, 25, 105, 255};
static const Color COLOR_SKIN = {247, 151, 112, 255};
static const Color COLOR_HAIR = {117, 54, 25, 255};
static const Color COLOR_ORANGE = {217, 77, 18, 255};

/* ------------------------------------------------------------------------- */
typedef enum GameState {
    GAME_STATE_PLAYING,
    GAME_STATE_SWING,
    GAME_STATE_RESULT
} GameState;

typedef struct GameData {
    GameState game_state;
    float pitch_timer;
    float swing_timer;
    float message_timer;
    float ball_x;
    float ball_y;
    float ball_start_x;
    float ball_start_y;
    float ball_target_x;
    float ball_target_y;
    int balls;
    int strikes;
    int outs;
    int inning;
    int player_score;
    int opponent_score;
    bool pitch_in_zone;
    bool swing_hit;
} GameData;

/* ------------------------------------------------------------------------- */
static float clamp_float_value(float value, float minimum, float maximum)
{
    if (value < minimum) return minimum;
    if (value > maximum) return maximum;
    return value;
}

static float ease_in_out(float value)
{
    value = clamp_float_value(value, 0.0f, 1.0f);
    return value * value * (3.0f - 2.0f * value);
}

static void draw_pixel_rect(int x, int y, int width, int height, Color color)
{
    DrawRectangle(x, y, width, height, color);
}

static void draw_pixel_text(const char *text, int x, int y, int font_size, Color color)
{
    DrawText(text, x, y, font_size, color);
}

static void draw_centered_pixel_text(const char *text, int panel_x, int panel_width, int y, int font_size, Color color)
{
    int text_width = MeasureText(text, font_size);
    draw_pixel_text(text, panel_x + (panel_width - text_width) / 2, y, font_size, color);
}

/* -------------------------------------------------------------------------
   Field and bases
   ------------------------------------------------------------------------- */
static void draw_baseball_field(void)
{
    DrawRectangle(FIELD_LEFT, FIELD_TOP, FIELD_RIGHT - FIELD_LEFT, FIELD_BOTTOM, COLOR_FIELD);

    /* Infield dirt */
    DrawEllipse(HOME_PLATE_X, INFIELD_TOP_Y + 94, 228, 91, COLOR_DIRT);
    DrawRectangle(HOME_PLATE_X - 115, INFIELD_TOP_Y + 43, 230, 74, COLOR_DIRT);

    /* Subtle center-field mowing stripe */
    DrawRectangle(FIELD_LEFT + 7, 0, 7, SCREEN_HEIGHT, COLOR_FIELD_DARK);
    DrawRectangle(FIELD_RIGHT - 14, 0, 7, SCREEN_HEIGHT, COLOR_FIELD_DARK);

    /* Foul lines */
    DrawLineEx((Vector2){FIELD_LEFT + 7, 225}, (Vector2){HOME_PLATE_X, HOME_PLATE_Y}, 4.0f, COLOR_LINE);
    DrawLineEx((Vector2){FIELD_RIGHT - 7, 225}, (Vector2){HOME_PLATE_X, HOME_PLATE_Y}, 4.0f, COLOR_LINE);

    /* First and third base dirt */
    DrawPoly((Vector2){FIELD_RIGHT - 50, 240}, 4, 45.0f, 10.0f, COLOR_DIRT);
    DrawPoly((Vector2){FIELD_LEFT + 50, 240}, 4, 45.0f, 10.0f, COLOR_DIRT);

    /* Pitcher's mound */
    DrawEllipse(PITCHER_MOUND_X, PITCHER_MOUND_Y + 1, 139, 27, COLOR_DIRT);
    DrawEllipse(PITCHER_MOUND_X, PITCHER_MOUND_Y - 1, 126, 22, COLOR_DIRT_DARK);
    DrawRectangle(PITCHER_MOUND_X - 20, PITCHER_MOUND_Y - 5, 40, 7, COLOR_WHITE);

    /* Base paths */
    DrawLineEx((Vector2){HOME_PLATE_X, HOME_PLATE_Y}, (Vector2){HOME_PLATE_X - BASE_HALF_SIZE, INFIELD_TOP_Y + 5}, 5.0f, COLOR_LINE);
    DrawLineEx((Vector2){HOME_PLATE_X, HOME_PLATE_Y}, (Vector2){HOME_PLATE_X + BASE_HALF_SIZE, INFIELD_TOP_Y + 5}, 5.0f, COLOR_LINE);
    DrawLineEx((Vector2){HOME_PLATE_X - BASE_HALF_SIZE, INFIELD_TOP_Y + 5}, (Vector2){HOME_PLATE_X - 1, INFIELD_TOP_Y - 45}, 5.0f, COLOR_LINE);
    DrawLineEx((Vector2){HOME_PLATE_X + BASE_HALF_SIZE, INFIELD_TOP_Y + 5}, (Vector2){HOME_PLATE_X + 1, INFIELD_TOP_Y - 45}, 5.0f, COLOR_LINE);

    /* Bases */
    DrawRectangle(HOME_PLATE_X - 10, HOME_PLATE_Y - 2, 20, 12, COLOR_WHITE);
    DrawTriangle(
        (Vector2){HOME_PLATE_X - 15, HOME_PLATE_Y - 5},
        (Vector2){HOME_PLATE_X + 15, HOME_PLATE_Y - 5},
        (Vector2){HOME_PLATE_X, HOME_PLATE_Y + 11},
        COLOR_WHITE);

    DrawRectangle(FIELD_LEFT + 31, INFIELD_TOP_Y - 52, 20, 13, COLOR_WHITE);
    DrawRectangle(FIELD_RIGHT - 51, INFIELD_TOP_Y - 52, 20, 13, COLOR_WHITE);
    DrawRectangle(HOME_PLATE_X - 10, INFIELD_TOP_Y - 64, 20, 13, COLOR_WHITE);
}

/* -------------------------------------------------------------------------
   Pixel-art players
   ------------------------------------------------------------------------- */
static void draw_pitcher(Vector2 position, float animation_phase)
{
    int x = (int)position.x;
    int y = (int)position.y;
    int arm_offset = (animation_phase > 0.5f) ? 8 : 0;

    /* shadow */
    DrawEllipse(x, y + 37, 39, 9, COLOR_DIRT_DARK);

    /* legs */
    draw_pixel_rect(x - 18, y + 17, 13, 20, COLOR_WHITE);
    draw_pixel_rect(x + 5, y + 17, 13, 20, COLOR_WHITE);
    draw_pixel_rect(x - 22, y + 34, 18, 7, COLOR_BLUE_DARK);
    draw_pixel_rect(x + 4, y + 34, 18, 7, COLOR_BLUE_DARK);

    /* torso */
    draw_pixel_rect(x - 20, y - 3, 40, 27, COLOR_WHITE);
    draw_pixel_rect(x - 16, y + 2, 32, 19, COLOR_RED);

    /* arms */
    draw_pixel_rect(x - 29, y - 1, 11, 23, COLOR_WHITE);
    draw_pixel_rect(x + 18, y - 1 - arm_offset, 11, 23, COLOR_WHITE);
    draw_pixel_rect(x - 32, y + 17, 13, 8, COLOR_SKIN);
    draw_pixel_rect(x + 20, y - 4 - arm_offset, 13, 8, COLOR_SKIN);

    /* head and cap */
    draw_pixel_rect(x - 17, y - 31, 34, 27, COLOR_SKIN);
    draw_pixel_rect(x - 21, y - 38, 42, 9, COLOR_ORANGE);
    draw_pixel_rect(x - 12, y - 45, 24, 9, COLOR_ORANGE);
    draw_pixel_rect(x - 7, y - 37, 13, 5, COLOR_WHITE);
    draw_pixel_rect(x - 17, y - 13, 34, 7, COLOR_SKIN);
}

static void draw_batter(Vector2 position, float swing_amount)
{
    int x = (int)position.x;
    int y = (int)position.y;
    int bat_lift = (int)(swing_amount * 34.0f);

    /* shadow */
    DrawEllipse(x + 5, y + 48, 50, 10, COLOR_DIRT_DARK);

    /* legs */
    draw_pixel_rect(x - 21, y + 15, 15, 30, COLOR_WHITE);
    draw_pixel_rect(x + 7, y + 15, 15, 30, COLOR_WHITE);
    draw_pixel_rect(x - 24, y + 41, 22, 8, COLOR_BLUE_DARK);
    draw_pixel_rect(x + 5, y + 41, 22, 8, COLOR_BLUE_DARK);

    /* body */
    draw_pixel_rect(x - 25, y - 12, 48, 34, COLOR_WHITE);
    draw_pixel_rect(x - 19, y - 7, 38, 23, COLOR_BLUE);
    draw_pixel_rect(x - 24, y + 2, 12, 22, COLOR_BLUE_DARK);

    /* head */
    draw_pixel_rect(x - 19, y - 43, 35, 29, COLOR_SKIN);
    draw_pixel_rect(x - 25, y - 48, 42, 10, COLOR_BLUE_DARK);
    draw_pixel_rect(x - 20, y - 54, 28, 9, COLOR_BLUE);
    draw_pixel_rect(x - 18, y - 22, 30, 7, COLOR_SKIN);

    /* arms */
    draw_pixel_rect(x + 10, y - 7, 11, 28, COLOR_WHITE);
    draw_pixel_rect(x + 19, y + 8, 10, 10, COLOR_SKIN);
    draw_pixel_rect(x + 22, y - 2 - bat_lift, 9, 20, COLOR_SKIN);

    /* bat */
    DrawLineEx(
        (Vector2){x + 27, y + 3 - bat_lift},
        (Vector2){x + 66, y - 25 - bat_lift},
        7.0f,
        COLOR_DIRT_DARK);
}

/* -------------------------------------------------------------------------
   HUD panels
   ------------------------------------------------------------------------- */
static void draw_score_panel(const GameData *game_data)
{
    char text_buffer[32];

    DrawRectangle(HUD_LEFT_X, HUD_TOP_Y, HUD_WIDTH, HUD_HEIGHT, COLOR_BLACK);
    DrawRectangle(HUD_RIGHT_X, HUD_TOP_Y, HUD_WIDTH, HUD_HEIGHT, COLOR_BLACK);

    /* thin field-edge highlights */
    DrawRectangle(HUD_WIDTH - 3, 0, 3, SCREEN_HEIGHT, COLOR_FIELD_DARK);
    DrawRectangle(HUD_RIGHT_X, 0, 3, SCREEN_HEIGHT, COLOR_FIELD_DARK);

    /* Score / count boxes */
    DrawRectangle(SCORE_BOX_X, SCORE_BOX_Y, SCORE_BOX_WIDTH, SCORE_BOX_HEIGHT, COLOR_BLACK);
    DrawRectangle(COUNT_BOX_X, COUNT_BOX_Y, COUNT_BOX_WIDTH, COUNT_BOX_HEIGHT, COLOR_BLACK);

    draw_pixel_text("B", COUNT_BOX_X + 8, COUNT_BOX_Y + 6, SCORE_FONT_SIZE, COLOR_WHITE);
    draw_pixel_text("S", COUNT_BOX_X + 8, COUNT_BOX_Y + 32, SCORE_FONT_SIZE, COLOR_WHITE);
    draw_pixel_text("O", COUNT_BOX_X + 8, COUNT_BOX_Y + 58, SCORE_FONT_SIZE, COLOR_WHITE);

    for (int index = 0; index < game_data->balls; ++index)
    {
        DrawRectangle(COUNT_BOX_X + 47 + index * 25, COUNT_BOX_Y + 12, 17, 17, COLOR_WHITE);
    }

    for (int index = 0; index < game_data->strikes; ++index)
    {
        DrawRectangle(COUNT_BOX_X + 47 + index * 25, COUNT_BOX_Y + 38, 17, 17, COLOR_WHITE);
    }

    for (int index = 0; index < game_data->outs; ++index)
    {
        DrawRectangle(COUNT_BOX_X + 47 + index * 25, COUNT_BOX_Y + 64, 17, 17, COLOR_WHITE);
    }

    /* Bottom player labels centered within their HUD panels. */
    draw_centered_pixel_text("3 Carter", HUD_RIGHT_X, HUD_WIDTH, 596, SMALL_FONT_SIZE, COLOR_WHITE);
    draw_centered_pixel_text("Avg .271", HUD_RIGHT_X, HUD_WIDTH, 663, SMALL_FONT_SIZE, COLOR_WHITE);
    draw_centered_pixel_text("Hr. 24", HUD_RIGHT_X, HUD_WIDTH, 691, SMALL_FONT_SIZE, COLOR_WHITE);

    draw_centered_pixel_text("O Bennett", HUD_LEFT_X, HUD_WIDTH, 596, SMALL_FONT_SIZE, COLOR_WHITE);
    draw_centered_pixel_text("Era. .281", HUD_LEFT_X, HUD_WIDTH, 663, SMALL_FONT_SIZE, COLOR_WHITE);
}

/* -------------------------------------------------------------------------
   Game logic
   ------------------------------------------------------------------------- */
static void reset_game(GameData *game_data)
{
    game_data->game_state = GAME_STATE_PLAYING;
    game_data->pitch_timer = 0.0f;
    game_data->swing_timer = 0.0f;
    game_data->message_timer = 0.0f;
    game_data->ball_start_x = PITCHER_MOUND_X;
    game_data->ball_start_y = PITCHER_MOUND_Y - 30;
    game_data->ball_target_x = HOME_PLATE_X + 6;
    game_data->ball_target_y = HOME_PLATE_Y - 35;
    game_data->ball_x = game_data->ball_start_x;
    game_data->ball_y = game_data->ball_start_y;
    game_data->balls = 0;
    game_data->strikes = 0;
    game_data->outs = 0;
    game_data->inning = 1;
    game_data->player_score = 0;
    game_data->opponent_score = 0;
    game_data->pitch_in_zone = true;
    game_data->swing_hit = false;
}

static void start_pitch(GameData *game_data)
{
    game_data->game_state = GAME_STATE_PLAYING;
    game_data->pitch_timer = 0.0f;
    game_data->ball_x = game_data->ball_start_x;
    game_data->ball_y = game_data->ball_start_y;
    game_data->swing_hit = false;
}

static void register_strike(GameData *game_data)
{
    game_data->strikes += 1;
    if (game_data->strikes >= 3)
    {
        game_data->balls = 0;
        game_data->strikes = 0;
        game_data->outs += 1;
        if (game_data->outs >= 3)
        {
            game_data->outs = 0;
        }
    }
}

static void register_swing(GameData *game_data)
{
    if (game_data->game_state != GAME_STATE_PLAYING)
    {
        return;
    }

    game_data->game_state = GAME_STATE_SWING;
    game_data->swing_timer = 0.0f;
    game_data->swing_hit = game_data->pitch_timer > PITCH_FLIGHT_SECONDS * 0.55f;
}

static void update_game(GameData *game_data, float delta_time)
{
    if (IsKeyPressed(KEY_R))
    {
        reset_game(game_data);
        return;
    }

    if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_Z) || IsKeyPressed(KEY_ENTER))
    {
        register_swing(game_data);
    }

    if (game_data->game_state == GAME_STATE_PLAYING)
    {
        game_data->pitch_timer += delta_time;
        const float progress = ease_in_out(game_data->pitch_timer / PITCH_FLIGHT_SECONDS);
        game_data->ball_x = Lerp(game_data->ball_start_x, game_data->ball_target_x, progress);
        game_data->ball_y = Lerp(game_data->ball_start_y, game_data->ball_target_y, progress);

        if (game_data->pitch_timer >= PITCH_FLIGHT_SECONDS)
        {
            register_strike(game_data);
            game_data->message_timer = MESSAGE_SECONDS;
            start_pitch(game_data);
        }
    }
    else if (game_data->game_state == GAME_STATE_SWING)
    {
        game_data->swing_timer += delta_time;
        if (game_data->swing_timer >= BAT_SWING_SECONDS)
        {
            if (game_data->swing_hit)
            {
                game_data->player_score += 1;
                game_data->balls = 0;
                game_data->strikes = 0;
            }
            else
            {
                register_strike(game_data);
            }

            game_data->message_timer = MESSAGE_SECONDS;
            start_pitch(game_data);
        }
    }

    if (game_data->message_timer > 0.0f)
    {
        game_data->message_timer -= delta_time;
    }
}

/* -------------------------------------------------------------------------
   Main rendering
   ------------------------------------------------------------------------- */
static void draw_game_scene(const GameData *game_data)
{
    BeginDrawing();
    ClearBackground(COLOR_BLACK);

    draw_baseball_field();
    draw_score_panel(game_data);

    float swing_amount = 0.0f;
    if (game_data->game_state == GAME_STATE_SWING)
    {
        swing_amount = clamp_float_value(game_data->swing_timer / BAT_SWING_SECONDS, 0.0f, 1.0f);
    }

    float pitcher_animation = 0.0f;
    if (game_data->game_state == GAME_STATE_PLAYING)
    {
        pitcher_animation = game_data->pitch_timer / PITCH_FLIGHT_SECONDS;
    }

    draw_pitcher((Vector2){PITCHER_MOUND_X, PITCHER_MOUND_Y}, pitcher_animation);
    draw_batter((Vector2){HOME_PLATE_X - 95, HOME_PLATE_Y - 7}, swing_amount);

    /* Home plate markings */
    DrawLineEx((Vector2){HOME_PLATE_X - 31, HOME_PLATE_Y + 5}, (Vector2){HOME_PLATE_X - 19, HOME_PLATE_Y + 35}, 4, COLOR_LINE);
    DrawLineEx((Vector2){HOME_PLATE_X + 31, HOME_PLATE_Y + 5}, (Vector2){HOME_PLATE_X + 19, HOME_PLATE_Y + 35}, 4, COLOR_LINE);
    DrawLineEx((Vector2){HOME_PLATE_X - 19, HOME_PLATE_Y + 35}, (Vector2){HOME_PLATE_X + 19, HOME_PLATE_Y + 35}, 4, COLOR_LINE);

    /* Ball */
    if (game_data->game_state == GAME_STATE_PLAYING)
    {
        DrawCircle((int)game_data->ball_x, (int)game_data->ball_y, BALL_RADIUS, COLOR_WHITE);
        DrawCircle((int)game_data->ball_x - 2, (int)game_data->ball_y - 1, 2, COLOR_RED);
    }

    if (game_data->message_timer > 0.0f)
    {
        const char *message = game_data->swing_hit ? "HIT!" : "STRIKE";
        int message_width = MeasureText(message, 34);
        draw_pixel_text(message, HOME_PLATE_X - message_width / 2, 300, 34, COLOR_WHITE);
    }

    /* Tiny control hint, hidden enough to preserve the reference feel. */
    draw_pixel_text("SPACE: SWING   R: RESET", FIELD_LEFT + 35, SCREEN_HEIGHT - 29, 16, COLOR_WHITE);

    EndDrawing();
}

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Baseball Arcade - Raylib");
    SetTargetFPS(TARGET_FPS);

    GameData game_data = {0};
    reset_game(&game_data);

    while (!WindowShouldClose())
    {
        float delta_time = GetFrameTime();
        update_game(&game_data, delta_time);
        draw_game_scene(&game_data);
    }

    CloseWindow();
    return 0;
}
