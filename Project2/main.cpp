/**
* Author: Kelly Chen
* Assignment: Pong Clone
* Date due: 10/16/2026
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/


#include "CS3113/cs3113.h"

// Enums
enum GameStatus { PLAYING, PLAYER_1_WON, PLAYER_2_WON };
enum PaddleMode { PLAYER_CONTROLLED, AUTOMATIC };

// Global Constants
constexpr int SCREEN_WIDTH  = 1600 / 2,
              SCREEN_HEIGHT = 900 / 2,
              FPS           = 60,
              PADDLE_SPEED  = 320,   
              BALL_SPEED    = 300,
              MAX_BALLS     = 3,
              MAX_LIVES     = 3;

constexpr Vector2 ORIGIN                = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 },
                  BACKGROUND_SCALE      = { SCREEN_WIDTH, SCREEN_HEIGHT },
                  PADDLE_SCALE          = { 24.0f, 110.0f },
                  BALL_SCALE            = { 36.0f, 36.0f },
                  HEART_SCALE           = { 28.0f, 28.0f },
                  WIN_MESSAGE_SCALE     = { 480.0f, 120.0f },
                  LEFT_PADDLE_INIT_POS  = { 40.0f, ORIGIN.y },
                  RIGHT_PADDLE_INIT_POS = { SCREEN_WIDTH - 40.0f, ORIGIN.y };

constexpr float HEART_MARGIN = 30.0f,
                HEART_GAP    = 34.0f,
                HEART_Y      = 28.0f;

constexpr Vector2 BALL_INIT_MOVEMENTS[MAX_BALLS] = {
    {  1.0f,  1.0f },
    { -1.0f,  1.0f },
    {  1.0f, -1.0f }
};

constexpr char BACKGROUND_FP[]   = "assets/background.png";
constexpr char LEFT_PADDLE_FP[]  = "assets/shield_earth.png";
constexpr char RIGHT_PADDLE_FP[] = "assets/shield_mars.png";
constexpr char LEFT_HEART_FP[]   = "assets/heart_earth.png";
constexpr char RIGHT_HEART_FP[]  = "assets/heart_mars.png";
constexpr char P1_WINS_FP[]      = "assets/p1_wins.png";
constexpr char P2_WINS_FP[]      = "assets/p2_wins.png";

constexpr const char *BALL_FPS[MAX_BALLS] = {
    "assets/meteor_rock.png",
    "assets/meteor_fire.png",
    "assets/meteor_ice.png"
};

// Global Variables
AppStatus  gAppStatus       = RUNNING;
GameStatus gGameStatus      = PLAYING;
PaddleMode gRightPaddleMode = PLAYER_CONTROLLED;

float gPreviousTicks = 0.0f;

int gActiveBalls = 1, 
    gLeftLives   = MAX_LIVES,
    gRightLives  = MAX_LIVES;

Vector2 gLeftPaddlePosition  = LEFT_PADDLE_INIT_POS,
        gLeftPaddleMovement  = { 0.0f, 0.0f },
        gLeftPaddleScale     = PADDLE_SCALE,

        gRightPaddlePosition = RIGHT_PADDLE_INIT_POS,
        gRightPaddleMovement = { 0.0f, 0.0f },
        gRightPaddleScale    = PADDLE_SCALE;

Vector2 gBallPositions[MAX_BALLS],
        gBallMovements[MAX_BALLS],
        gBallScales[MAX_BALLS];

Texture2D gBackgroundTexture,
          gLeftPaddleTexture,
          gRightPaddleTexture,
          gLeftHeartTexture,
          gRightHeartTexture,
          gP1WinsTexture,
          gP2WinsTexture,
          gBallTextures[MAX_BALLS];

// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();
bool isColliding(const Vector2 *positionA, const Vector2 *scaleA,
                 const Vector2 *positionB, const Vector2 *scaleB);
void renderObject(const Texture2D *texture, const Vector2 *position,
                  const Vector2 *scale);
void resetBall(int index, float directionX);
void clampY(Vector2 *position, const Vector2 *scale); 



// Function Definitions
bool isColliding(const Vector2 *positionA, const Vector2 *scaleA,
                 const Vector2 *positionB, const Vector2 *scaleB) {
    float xDistance = fabs(positionA->x - positionB->x) - ((scaleA->x + scaleB->x) / 2.0f);
    float yDistance = fabs(positionA->y - positionB->y) - ((scaleA->y + scaleB->y) / 2.0f);

    return xDistance < 0.0f && yDistance < 0.0f;
}


void renderObject(const Texture2D *texture, const Vector2 *position,
                  const Vector2 *scale) {
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

    Vector2 originOffset = {
        scale->x / 2.0f,
        scale->y / 2.0f
    };

    DrawTexturePro(*texture, textureArea, destinationArea, originOffset, 0.0f, WHITE);
}


void resetBall(int index, float directionX) {
    gBallPositions[index] = ORIGIN;
    gBallMovements[index] = { directionX, BALL_INIT_MOVEMENTS[index].y };
    Normalise(&gBallMovements[index]);
}

void clampY(Vector2 *position, const Vector2 *scale) {
    float halfHeight = scale->y / 2.0f;

    if (position->y < halfHeight) position->y = halfHeight;
    if (position->y > SCREEN_HEIGHT - halfHeight) position->y = SCREEN_HEIGHT - halfHeight;
}

void initialise() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Planet Defense Pong");

    gBackgroundTexture  = LoadTexture(BACKGROUND_FP);
    gLeftPaddleTexture  = LoadTexture(LEFT_PADDLE_FP);
    gRightPaddleTexture = LoadTexture(RIGHT_PADDLE_FP);
    gLeftHeartTexture   = LoadTexture(LEFT_HEART_FP);
    gRightHeartTexture  = LoadTexture(RIGHT_HEART_FP);
    gP1WinsTexture      = LoadTexture(P1_WINS_FP);
    gP2WinsTexture      = LoadTexture(P2_WINS_FP);

    for (int i = 0; i < MAX_BALLS; i++) {
        gBallTextures[i]  = LoadTexture(BALL_FPS[i]);
        gBallPositions[i] = ORIGIN;
        gBallScales[i]    = BALL_SCALE;
        gBallMovements[i] = BALL_INIT_MOVEMENTS[i];

        // so that diagonal meteors are not faster than straight ones
        Normalise(&gBallMovements[i]);
    }

    SetTargetFPS(FPS);
}

void processInput() {
    gLeftPaddleMovement = { 0.0f, 0.0f };
    
    if (IsKeyDown(KEY_W)) gLeftPaddleMovement.y = -1.0f;
    else if (IsKeyDown(KEY_S)) gLeftPaddleMovement.y = 1.0f;


    if (gRightPaddleMode == PLAYER_CONTROLLED) {
        gRightPaddleMovement = { 0.0f, 0.0f };
        if (IsKeyDown(KEY_UP)) gRightPaddleMovement.y = -1.0f;
        else if (IsKeyDown(KEY_DOWN)) gRightPaddleMovement.y = 1.0f;
    }

    if (IsKeyPressed(KEY_T)) {
        gRightPaddleMode = (gRightPaddleMode == PLAYER_CONTROLLED) ? AUTOMATIC : PLAYER_CONTROLLED;
    }

    if (IsKeyPressed(KEY_ONE)) gActiveBalls = 1;
    else if (IsKeyPressed(KEY_TWO)) gActiveBalls = 2;
    else if (IsKeyPressed(KEY_THREE)) gActiveBalls = 3;

    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;
}

void update() {
    float ticks     = static_cast<float>(GetTime());
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;

    if (gGameStatus != PLAYING) return;

    if (gRightPaddleMode == AUTOMATIC) {
        if (gRightPaddleMovement.y == 0.0f) gRightPaddleMovement.y = 1.0f;
        if (gRightPaddlePosition.y - gRightPaddleScale.y / 2.0f <= 0.0f) {
            gRightPaddleMovement.y = 1.0f; 
        } else if (gRightPaddlePosition.y + gRightPaddleScale.y / 2.0f >= SCREEN_HEIGHT) {
            gRightPaddleMovement.y = -1.0f;
        }
    }

    gLeftPaddlePosition.y += PADDLE_SPEED * gLeftPaddleMovement.y * deltaTime;
    gRightPaddlePosition.y += PADDLE_SPEED * gRightPaddleMovement.y * deltaTime;

    clampY(&gLeftPaddlePosition,  &gLeftPaddleScale);
    clampY(&gRightPaddlePosition, &gRightPaddleScale);



    for (int i = 0; i < gActiveBalls; i++) {
        gBallPositions[i].x += BALL_SPEED * gBallMovements[i].x * deltaTime;
        gBallPositions[i].y += BALL_SPEED * gBallMovements[i].y * deltaTime;

        float halfHeight = gBallScales[i].y / 2.0f;
        if (gBallPositions[i].y - halfHeight < 0.0f) {
            gBallPositions[i].y = halfHeight;
            gBallMovements[i].y *= -1.0f;
        } 
        else if (gBallPositions[i].y + halfHeight > SCREEN_HEIGHT) {
            gBallPositions[i].y = SCREEN_HEIGHT - halfHeight;
            gBallMovements[i].y *= -1.0f;
        }

        if (isColliding(&gBallPositions[i], &gBallScales[i],
                        &gLeftPaddlePosition, &gLeftPaddleScale)) {
            gBallMovements[i].x *= -1.0f;
            gBallPositions[i].x = gLeftPaddlePosition.x + (gLeftPaddleScale.x + gBallScales[i].x) / 2.0f;
        } 
        else if (isColliding(&gBallPositions[i], &gBallScales[i],
                             &gRightPaddlePosition, &gRightPaddleScale)) {
            gBallMovements[i].x *= -1.0f;
            gBallPositions[i].x = gRightPaddlePosition.x - (gRightPaddleScale.x + gBallScales[i].x) / 2.0f;
        }

        if (gBallPositions[i].x - gBallScales[i].x / 2.0f < 0.0f) {
            gLeftLives--;
            resetBall(i, 1.0f);
        } 
        else if (gBallPositions[i].x + gBallScales[i].x / 2.0f > SCREEN_WIDTH) {
            gRightLives--;
            resetBall(i, -1.0f);
        }

        if (gLeftLives <= 0) {
            gGameStatus = PLAYER_2_WON;
        } 
        else if (gRightLives <= 0) {
            gGameStatus = PLAYER_1_WON;
        }
        
    }
}

void render() {
    BeginDrawing();
    ClearBackground(BLACK);

    renderObject(&gBackgroundTexture, &ORIGIN, &BACKGROUND_SCALE);

    for (int i = 0; i < gLeftLives; i++) {
        Vector2 heartPosition = { HEART_MARGIN + i * HEART_GAP, HEART_Y };
        renderObject(&gLeftHeartTexture, &heartPosition, &HEART_SCALE);
    }
    for (int i = 0; i < gRightLives; i++) {
        Vector2 heartPosition = { SCREEN_WIDTH - HEART_MARGIN - i * HEART_GAP, HEART_Y };
        renderObject(&gRightHeartTexture, &heartPosition, &HEART_SCALE);
    }

    renderObject(&gLeftPaddleTexture,  &gLeftPaddlePosition,  &gLeftPaddleScale);
    renderObject(&gRightPaddleTexture, &gRightPaddlePosition, &gRightPaddleScale);

    for (int i = 0; i < gActiveBalls; i++) {
        renderObject(&gBallTextures[i], &gBallPositions[i], &gBallScales[i]);
    }

    if (gGameStatus == PLAYER_1_WON) {
        renderObject(&gP1WinsTexture, &ORIGIN, &WIN_MESSAGE_SCALE);
    } 
    else if (gGameStatus == PLAYER_2_WON) {
        renderObject(&gP2WinsTexture, &ORIGIN, &WIN_MESSAGE_SCALE);
    }

    EndDrawing();
}

void shutdown() {
    UnloadTexture(gBackgroundTexture);
    UnloadTexture(gLeftPaddleTexture);
    UnloadTexture(gRightPaddleTexture);
    UnloadTexture(gLeftHeartTexture);
    UnloadTexture(gRightHeartTexture);
    UnloadTexture(gP1WinsTexture);
    UnloadTexture(gP2WinsTexture);

    for (int i = 0; i < MAX_BALLS; i++) UnloadTexture(gBallTextures[i]);

    CloseWindow();
}

int main(void) {
    initialise();

    while (gAppStatus == RUNNING) {
        processInput();
        update();
        render();
    }

    shutdown();

    return 0;
}