/**
* Author: Kelly Chen
* Assignment: Simple 2D Scene
* Date due: 10/05/2026
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "CS3113/cs3113.h"

// Enums
enum TimeOfDay { DAY, NIGHT };

// Global Constants
constexpr int SCREEN_WIDTH  = 633,  
              SCREEN_HEIGHT = 900,  
              FPS           = 60;

constexpr char DAY_BG_FP[]   = "assets/background_daytime.png";
constexpr char NIGHT_BG_FP[] = "assets/background_nighttime.png";
constexpr char WALKER_FP[]   = "assets/main_character_2.png"; 
constexpr char ORBITER_FP[]  = "assets/main_character_1.png"; 
constexpr char SUN_FP[]      = "assets/sun.png";
constexpr char MOON_FP[]     = "assets/moon.png";

constexpr Vector2 ORIGIN = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };


constexpr Vector2 BACKGROUND_POSITION = ORIGIN,
                  BACKGROUND_SCALE    = { static_cast<float>(SCREEN_WIDTH) - 48.0f,
                                          static_cast<float>(SCREEN_HEIGHT) - 48.0f };


constexpr Vector2 WALK_START       = { SCREEN_WIDTH * 0.48f, SCREEN_HEIGHT * 0.95f },
                  WALK_END         = { SCREEN_WIDTH * 0.67f, SCREEN_HEIGHT * 0.55f },
                  WALKER_BASE_SIZE = { 900.0f, 900.0f };
constexpr float   WALK_DURATION    = 8.0f;  


constexpr Vector2 ORBITER_BASE_SIZE = { 340.0f, 340.0f };
constexpr float   ORBIT_RADIUS      = 500.0f, 
                  ORBIT_SPEED       = 1.0f;  


constexpr Vector2 SUN_ARC_CENTRE  = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 4.0f },
                  MOON_ARC_CENTRE = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 3.0f },
                  SUN_SIZE        = { 140.0f, 140.0f },
                  MOON_SIZE       = { 110.0f, 110.0f };
constexpr float   SUN_ARC_RADIUS  = SCREEN_WIDTH / 4.0f,
                  MOON_ARC_RADIUS = SCREEN_WIDTH / 4.0f,
                  SUN_SPEED       = 0.4f, 
                  MOON_SPEED      = 0.5f,
                  SUN_SPIN_SPEED  = 30.0f,  
                  MOON_SPIN_SPEED = 20.0f;


constexpr float ARC_START_ANGLE = PI,
                ARC_END_ANGLE   = 0.0f;




// Global Variables
AppStatus gAppStatus     = RUNNING;
TimeOfDay gTimeOfDay     = DAY;
float     gPreviousTicks = 0.0f;


float   gWalkProgress   = 0.0f;         // 0 = bottom of the stairs, 1 = gone
Vector2 gWalkerPosition = WALK_START,
        gWalkerScale    = WALKER_BASE_SIZE;

float   gOrbitAngle      = 0.0f;        // radians
Vector2 gOrbiterPosition = WALK_START,
        gOrbiterScale    = ORBITER_BASE_SIZE;

float   gSunArcAngle  = ARC_START_ANGLE,
        gSunSpinAngle = 0.0f;           // degrees
Vector2 gSunPosition  = { SUN_ARC_CENTRE.x - SUN_ARC_RADIUS, SUN_ARC_CENTRE.y };

float   gMoonArcAngle = ARC_START_ANGLE,
        gMoonSpinAngle = 0.0f;          // degrees
Vector2 gMoonPosition = { MOON_ARC_CENTRE.x - MOON_ARC_RADIUS, MOON_ARC_CENTRE.y };

Texture2D gDayBackgroundTexture,
          gNightBackgroundTexture,
          gWalkerTexture,
          gOrbiterTexture,
          gSunTexture,
          gMoonTexture;




// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();
void renderObject(const Texture2D *texture, const Vector2 *position,
                  const Vector2 *scale, float angle);




// Function Definitions
void renderObject(const Texture2D *texture, const Vector2 *position,
                  const Vector2 *scale, float angle)
{
    Rectangle textureArea = {
        0.0f, 0.0f,
        static_cast<float>(texture->width),
        static_cast<float>(texture->height)
    };

    Rectangle destinationArea = {
        position->x,
        position->y,
        scale->x,
        scale->y
    };

    Vector2 originOffset = { scale->x / 2.0f, scale->y / 2.0f };

    DrawTexturePro(*texture, textureArea, destinationArea, originOffset,
                   angle, WHITE);
}

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Project 1: Simple 2D Scene");

    gDayBackgroundTexture   = LoadTexture(DAY_BG_FP);
    gNightBackgroundTexture = LoadTexture(NIGHT_BG_FP);
    gWalkerTexture          = LoadTexture(WALKER_FP);
    gOrbiterTexture         = LoadTexture(ORBITER_FP);
    gSunTexture             = LoadTexture(SUN_FP);
    gMoonTexture            = LoadTexture(MOON_FP);

    SetTargetFPS(FPS);
}

void processInput()
{
    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;
}

void update()
{
    // delta time
    float ticks     = static_cast<float>(GetTime());
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;



    gWalkProgress += deltaTime / WALK_DURATION;
    if (gWalkProgress > 1.0f) {
        gWalkProgress = 0.0f;
    }

    gWalkerPosition.x = WALK_START.x + (WALK_END.x - WALK_START.x) * gWalkProgress;
    gWalkerPosition.y = WALK_START.y + (WALK_END.y - WALK_START.y) * gWalkProgress;

    gWalkerScale.x = WALKER_BASE_SIZE.x * (1.0f - gWalkProgress * 0.5f); // Shrink the walker as it climbs
    gWalkerScale.y = WALKER_BASE_SIZE.y * (1.0f - gWalkProgress * 0.5f); // Shrink the walker as it climbs  
    


    gOrbitAngle += ORBIT_SPEED * deltaTime;
    float orbitRadius = ORBIT_RADIUS * (1.0f - gWalkProgress);

    gOrbiterPosition.x = gWalkerPosition.x + orbitRadius * cosf(gOrbitAngle);
    gOrbiterPosition.y = gWalkerPosition.y + orbitRadius * sinf(gOrbitAngle);

    gOrbiterScale.x = ORBITER_BASE_SIZE.x * (1.0f - gWalkProgress * 0.5f);
    gOrbiterScale.y = ORBITER_BASE_SIZE.y * (1.0f - gWalkProgress * 0.5f);



    if (gTimeOfDay == DAY) {
        gSunArcAngle -= SUN_SPEED * deltaTime;
        gSunPosition.x = SUN_ARC_CENTRE.x + SUN_ARC_RADIUS * cosf(gSunArcAngle) * 1.01f;
        gSunPosition.y = SUN_ARC_CENTRE.y - SUN_ARC_RADIUS * sinf(gSunArcAngle) * 1.01f;
        gSunSpinAngle += SUN_SPIN_SPEED * deltaTime;

        if (gSunArcAngle <= ARC_END_ANGLE) {
            gTimeOfDay = NIGHT;
            gMoonArcAngle = ARC_START_ANGLE;
            gMoonSpinAngle = 0.0f;
        }
    } else {
        gMoonArcAngle -= MOON_SPEED * deltaTime;
        gMoonPosition.x = MOON_ARC_CENTRE.x + MOON_ARC_RADIUS * cosf(gMoonArcAngle);
        gMoonPosition.y = MOON_ARC_CENTRE.y - MOON_ARC_RADIUS * sinf(gMoonArcAngle);
        gMoonSpinAngle += MOON_SPIN_SPEED * deltaTime;

        if (gMoonArcAngle <= ARC_END_ANGLE) {
            gTimeOfDay = DAY;
            gSunArcAngle = ARC_START_ANGLE;
            gSunSpinAngle = 0.0f;
        }
    }

}

void render()
{
    BeginDrawing();
    if (gTimeOfDay == DAY) {
        ClearBackground(SKYBLUE);
    } else {
        ClearBackground(DARKBLUE);
    }

    if (gTimeOfDay == DAY) {
        renderObject(&gDayBackgroundTexture, &BACKGROUND_POSITION, &BACKGROUND_SCALE, 0.0f);
    }
    else {
        renderObject(&gNightBackgroundTexture, &BACKGROUND_POSITION, &BACKGROUND_SCALE, 0.0f);
    }

    if (gTimeOfDay == DAY) {
        renderObject(&gSunTexture, &gSunPosition, &SUN_SIZE, gSunSpinAngle);
    }
    else {
        renderObject(&gMoonTexture, &gMoonPosition, &MOON_SIZE, gMoonSpinAngle);
    }


    renderObject(&gOrbiterTexture, &gOrbiterPosition, &gOrbiterScale, 0.0f);
    renderObject(&gWalkerTexture,  &gWalkerPosition,  &gWalkerScale,  0.0f);
    

    EndDrawing();
}

void shutdown()
{
    UnloadTexture(gDayBackgroundTexture);
    UnloadTexture(gNightBackgroundTexture);
    UnloadTexture(gWalkerTexture);
    UnloadTexture(gOrbiterTexture);
    UnloadTexture(gSunTexture);
    UnloadTexture(gMoonTexture);

    CloseWindow();
}

int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();
        render();
    }

    shutdown();

    return 0;
}