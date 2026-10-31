/**
* Author: Kelly Chen
* Assignment: Lunar Lander!
* Date due: 10/30/2026
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "CS3113/Entity.h"

// Enums
enum GameStatus { PLAYING, MISSION_ACCOMPLISHED, MISSION_FAILED };

// Global Constants
constexpr int SCREEN_WIDTH  = 1000,
              SCREEN_HEIGHT = 600,
              FPS           = 120;

constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;

constexpr Vector2 ORIGIN           = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 },
                  BACKGROUND_SCALE = { SCREEN_WIDTH, SCREEN_HEIGHT },
                  MESSAGE_SCALE    = { 720.0f, 180.0f };

constexpr float ACCELERATION_OF_GRAVITY = 90.0f,  
                THRUST_POWER            = 220.0f,  
                DRAG_COEFFICIENT        = 1.5f;   

constexpr float MAX_FUEL       = 100.0f,
                FUEL_BURN_RATE = 20.0f;    

constexpr Vector2 LANDER_START_POSITION = { ORIGIN.x, 70.0f },
                  LANDER_SCALE          = { 80.0f, 60.0f },
                  LANDER_COLLIDER       = { 64.0f, 56.0f };

constexpr int NUMBER_OF_PLATFORMS = 6,
              STREET              = 0,
              LEFT_BUILDING       = 1,
              ANTENNA             = 2,
              RIGHT_BUILDING      = 3,
              HELIPAD             = 4,
              TRUCK               = 5;  

constexpr Vector2 PLATFORM_POSITIONS[NUMBER_OF_PLATFORMS] = {
    { ORIGIN.x, 580.0f },  
    { 130.0f,   410.0f },  
    { 100.0f,   226.0f },  
    { 870.0f,   450.0f },  
    { 870.0f,   314.0f },  
    { 480.0f,   530.0f }   
};

constexpr Vector2 PLATFORM_SCALES[NUMBER_OF_PLATFORMS] = {
    { SCREEN_WIDTH, 40.0f },
    { 200.0f, 320.0f },
    {  24.0f,  48.0f },
    { 160.0f, 240.0f },
    {  96.0f,  32.0f },
    { 120.0f,  60.0f }
};

constexpr float TRUCK_SPEED       = 120.0f,   
                TRUCK_LEFT_BOUND  = 300.0f,  
                TRUCK_RIGHT_BOUND = 720.0f;

constexpr float BAR_LEFT   = 20.0f,
                BAR_TOP    = 20.0f,
                BAR_WIDTH  = 200.0f,
                BAR_HEIGHT = 20.0f;

constexpr char BACKGROUND_FP[]     = "assets/background.png";
constexpr char DRONE_FP[]          = "assets/drone.png";
constexpr char DRONE_SHEET_FP[]    = "assets/drone_sheet.png";   // 2 rows x 4 columns
constexpr char ACCOMPLISHED_FP[]   = "assets/mission_accomplished.png";
constexpr char FAILED_FP[]         = "assets/mission_failed.png";
constexpr char BATTERY_LABEL_FP[]  = "assets/battery_label.png";
constexpr char BAR_BACKGROUND_FP[] = "assets/bar_background.png";
constexpr char BAR_FILL_FP[]       = "assets/bar_fill.png";

constexpr const char *PLATFORM_FPS[NUMBER_OF_PLATFORMS] = {
    "assets/street.png",
    "assets/building_a.png",
    "assets/antenna.png",
    "assets/building_b.png",
    "assets/helipad.png",
    "assets/truck.png"
};


// Global Variables
AppStatus  gAppStatus  = RUNNING;
GameStatus gGameStatus = PLAYING;

float gPreviousTicks   = 0.0f,
      gTimeAccumulator = 0.0f;

float gFuel        = MAX_FUEL;
bool  gIsThrusting = false;  

Entity *gLander    = nullptr;
Entity *gPlatforms = nullptr;

Texture2D gBackgroundTexture,
          gAccomplishedTexture,
          gFailedTexture,
          gBatteryLabelTexture,
          gBarBackgroundTexture,
          gBarFillTexture;


// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();
void renderObject(const Texture2D *texture, const Vector2 *position, const Vector2 *scale);
void checkMissionStatus();


// Function Definitions
void renderObject(const Texture2D *texture, const Vector2 *position,
                  const Vector2 *scale)
{
    Rectangle textureArea = {
        0.0f, 0.0f,
        static_cast<float>(texture->width),
        static_cast<float>(texture->height)
    };

    Rectangle destinationArea = { position->x, position->y, scale->x, scale->y };
    Vector2   originOffset    = { scale->x / 2.0f, scale->y / 2.0f };

    DrawTexturePro(*texture, textureArea, destinationArea, originOffset, 0.0f, WHITE);
}

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Rooftop Drone Delivery");

    gBackgroundTexture    = LoadTexture(BACKGROUND_FP);
    gAccomplishedTexture  = LoadTexture(ACCOMPLISHED_FP);
    gFailedTexture        = LoadTexture(FAILED_FP);
    gBatteryLabelTexture  = LoadTexture(BATTERY_LABEL_FP);
    gBarBackgroundTexture = LoadTexture(BAR_BACKGROUND_FP);
    gBarFillTexture       = LoadTexture(BAR_FILL_FP);


    std::map<Direction, std::vector<int>> droneAtlas = {
        { DOWN,  { 0, 1, 2, 3 } },  
        { UP,    { 4, 5, 6, 7 } },  
        { LEFT,  { 4, 5, 6, 7 } },
        { RIGHT, { 4, 5, 6, 7 } }
    };

    gLander = new Entity(
        LANDER_START_POSITION,  
        LANDER_SCALE, 
        DRONE_SHEET_FP,  
        ATLAS,             
        { 2, 4 }, 
        droneAtlas,  
        PLAYER      
    );
    gLander->setDirection(DOWN);  
    gLander->setColliderDimensions(LANDER_COLLIDER);

    gLander->setAcceleration({ 0.0f, ACCELERATION_OF_GRAVITY });



    gPlatforms = new Entity[NUMBER_OF_PLATFORMS];

    for (int i = 0; i < NUMBER_OF_PLATFORMS; i++)
    {
        gPlatforms[i].setTexture(PLATFORM_FPS[i]);
        gPlatforms[i].setPosition(PLATFORM_POSITIONS[i]);
        gPlatforms[i].setScale(PLATFORM_SCALES[i]);
        gPlatforms[i].setColliderDimensions(PLATFORM_SCALES[i]);

        gPlatforms[i].setEntityType(HAZARD);
    }

    gPlatforms[HELIPAD].setEntityType(LANDING_PAD);
    gPlatforms[TRUCK].setEntityType(LANDING_PAD);

    gPlatforms[TRUCK].setSpeed(TRUCK_SPEED);
    gPlatforms[TRUCK].moveRight();

    SetTargetFPS(FPS);
}

void processInput()
{
    Vector2 acceleration   = { 0.0f, ACCELERATION_OF_GRAVITY };
    bool    sidewaysThrust = false;
    gIsThrusting           = false;

    if (gFuel > 0.0f)
    {
        if (IsKeyDown(KEY_A))
        {
            acceleration.x -= THRUST_POWER;
            sidewaysThrust  = true;
        }
        else if (IsKeyDown(KEY_D))
        {
            acceleration.x += THRUST_POWER;
            sidewaysThrust  = true;
        }

        if (IsKeyDown(KEY_W))
        {
            acceleration.y -= THRUST_POWER;
            gIsThrusting    = true;
        }

        if (sidewaysThrust) gIsThrusting = true;
    }


    if (!sidewaysThrust)
        acceleration.x -= gLander->getVelocity().x * DRAG_COEFFICIENT;

    gLander->setAcceleration(acceleration);

    gLander->setDirection(gIsThrusting ? UP : DOWN);

    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;
}


void checkMissionStatus()
{
    EntityType touched = gLander->getLastCollisionType();

    if (touched == LANDING_PAD && gLander->isCollidingBottom())
        gGameStatus = MISSION_ACCOMPLISHED;  
    else if (touched != NONE)
        gGameStatus = MISSION_FAILED;  

    Vector2 position = gLander->getPosition();

    if (position.x < 0.0f || position.x > SCREEN_WIDTH ||
        position.y < 0.0f || position.y > SCREEN_HEIGHT)
        gGameStatus = MISSION_FAILED;
}

void update()
{
    // Delta time
    float ticks     = static_cast<float>(GetTime());
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;

    if (gGameStatus != PLAYING) return;

    deltaTime += gTimeAccumulator;

    if (deltaTime < FIXED_TIMESTEP)
    {
        gTimeAccumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP)
    {
        if (gPlatforms[TRUCK].getPosition().x > TRUCK_RIGHT_BOUND)
            gPlatforms[TRUCK].moveLeft();
        else if (gPlatforms[TRUCK].getPosition().x < TRUCK_LEFT_BOUND)
            gPlatforms[TRUCK].moveRight();

        gLander->update(FIXED_TIMESTEP, gPlatforms, NUMBER_OF_PLATFORMS, nullptr, 0);

        for (int i = 0; i < NUMBER_OF_PLATFORMS; i++)
            gPlatforms[i].update(FIXED_TIMESTEP, nullptr, 0, nullptr, 0);


        if (gIsThrusting)
        {
            gFuel -= FUEL_BURN_RATE * FIXED_TIMESTEP;
            if (gFuel < 0.0f) gFuel = 0.0f;
        }

        deltaTime -= FIXED_TIMESTEP;

        checkMissionStatus();
        if (gGameStatus != PLAYING) break;
    }

    gTimeAccumulator = deltaTime;
}

void render()
{
    BeginDrawing();
    ClearBackground(BLACK);

    renderObject(&gBackgroundTexture, &ORIGIN, &BACKGROUND_SCALE);

    for (int i = 0; i < NUMBER_OF_PLATFORMS; i++) gPlatforms[i].render();

    gLander->render();


    Vector2 barCentre = { BAR_LEFT + BAR_WIDTH / 2.0f, BAR_TOP + BAR_HEIGHT / 2.0f },
            barScale  = { BAR_WIDTH, BAR_HEIGHT };
    renderObject(&gBarBackgroundTexture, &barCentre, &barScale);

    float   fillWidth  = BAR_WIDTH * gFuel / MAX_FUEL;
    Vector2 fillCentre = { BAR_LEFT + fillWidth / 2.0f, barCentre.y },
            fillScale  = { fillWidth, BAR_HEIGHT - 4.0f };
    if (fillWidth > 0.0f) renderObject(&gBarFillTexture, &fillCentre, &fillScale);

    Vector2 labelScale    = { static_cast<float>(gBatteryLabelTexture.width),
                              static_cast<float>(gBatteryLabelTexture.height) };
    Vector2 labelPosition = { BAR_LEFT + labelScale.x / 2.0f,
                              BAR_TOP + BAR_HEIGHT + 4.0f + labelScale.y / 2.0f };
    renderObject(&gBatteryLabelTexture, &labelPosition, &labelScale);

    if (gGameStatus == MISSION_ACCOMPLISHED)
        renderObject(&gAccomplishedTexture, &ORIGIN, &MESSAGE_SCALE);
    else if (gGameStatus == MISSION_FAILED)
        renderObject(&gFailedTexture, &ORIGIN, &MESSAGE_SCALE);

    EndDrawing();
}

void shutdown()
{
    delete gLander;
    delete[] gPlatforms;

    UnloadTexture(gBackgroundTexture);
    UnloadTexture(gAccomplishedTexture);
    UnloadTexture(gFailedTexture);
    UnloadTexture(gBatteryLabelTexture);
    UnloadTexture(gBarBackgroundTexture);
    UnloadTexture(gBarFillTexture);

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