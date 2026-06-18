// ============================================================
// main_raylib_demo.cpp
// ============================================================
// Save this section as main_raylib_demo.cpp
// Arch install:
//     sudo pacman -S raylib
// Compile:
//     g++ -std=c++17 main_raylib_demo.cpp ScaleShiftCinematicsCameraSystem.cpp -o directional_camera_visual_test -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
// Run:
//     ./directional_camera_visual_test
//
// Controls:
//     ESC = open/close settings menu
//     F3 = hold perspective shift wheel
//     Mouse = choose perspective while wheel is open
//     Release F3 = confirm selected perspective
//     1 = First Person
//     2 = Third Person
//     3 = Top Down
//     W/A/S/D = move player or world camera
//     Q/E = top-down height
//     Mouse = look/orbit/steer directional morph
//     Wheel = top-down zoom
// ============================================================

#include "ScaleShiftCinematicsCameraSystem.hpp"

#include <raylib.h>
#include <raymath.h>
#include <cmath>

static Vector3 ToRaylibVec3(const SSCVec3& v)
{
    return Vector3{ v.x, v.z, v.y };
}

static float DemoDegToRad(float degrees)
{
    return degrees * 3.1415926535f / 180.0f;
}

static Vector3 DemoCameraForwardFromRotator(const SSCRotator& r)
{
    const float pitch = DemoDegToRad(r.pitch);
    const float yaw = DemoDegToRad(r.yaw);

    Vector3 forward;
    forward.x = std::cos(pitch) * std::cos(yaw);
    forward.y = std::sin(pitch);
    forward.z = std::cos(pitch) * std::sin(yaw);
    return Vector3Normalize(forward);
}

static Camera3D BuildRaylibCamera(const SSCCameraState& state)
{
    Camera3D cam = {};
    cam.position = ToRaylibVec3(state.position);

    Vector3 forward = DemoCameraForwardFromRotator(state.rotation);
    cam.target = Vector3Add(cam.position, forward);

    cam.up = Vector3{ 0.0f, 1.0f, 0.0f };
    cam.fovy = state.fieldOfView;
    cam.projection = CAMERA_PERSPECTIVE;
    return cam;
}

static const char* DemoModeName(SSCCameraMode mode)
{
    switch (mode)
    {
        case SSCCameraMode::FirstPerson:
            return "FIRST PERSON - camera enters head";
        case SSCCameraMode::ThirdPerson:
            return "THIRD PERSON - exits by look/movement vector";
        case SSCCameraMode::TopDown:
            return "TOP DOWN - free map/world camera";
    }

    return "UNKNOWN";
}

static SSCCameraMode SelectPerspectiveFromWheel(Vector2 mousePosition, Vector2 center)
{
    const Vector2 delta = Vector2Subtract(mousePosition, center);
    const float angle = std::atan2(delta.y, delta.x);

    // Top slice: First Person
    if (angle < -0.75f && angle > -2.35f)
        return SSCCameraMode::FirstPerson;

    // Right/bottom slice: Third Person
    if (angle >= -0.75f && angle <= 1.55f)
        return SSCCameraMode::ThirdPerson;

    // Left/bottom slice: Top Down
    return SSCCameraMode::TopDown;
}

static const char* WheelLabel(SSCCameraMode mode)
{
    switch (mode)
    {
        case SSCCameraMode::FirstPerson:
            return "FIRST PERSON";
        case SSCCameraMode::ThirdPerson:
            return "THIRD PERSON";
        case SSCCameraMode::TopDown:
            return "TOP DOWN";
    }

    return "UNKNOWN";
}

static void DrawPerspectiveWheel(SSCCameraMode selectedMode)
{
    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();
    const Vector2 center = Vector2{ screenWidth * 0.5f, screenHeight * 0.5f };

    DrawRectangle(0, 0, screenWidth, screenHeight, Color{ 0, 0, 0, 135 });

    const float outerRadius = 165.0f;
    const float innerRadius = 58.0f;

    DrawCircleV(center, outerRadius, Color{ 18, 18, 24, 230 });
    DrawCircleLines(static_cast<int>(center.x), static_cast<int>(center.y), outerRadius, Color{ 180, 180, 210, 210 });
    DrawCircleV(center, innerRadius, Color{ 8, 8, 12, 255 });
    DrawCircleLines(static_cast<int>(center.x), static_cast<int>(center.y), innerRadius, Color{ 255, 255, 255, 170 });

    Vector2 firstPos = Vector2{ center.x, center.y - 108.0f };
    Vector2 thirdPos = Vector2{ center.x + 112.0f, center.y + 58.0f };
    Vector2 topPos = Vector2{ center.x - 112.0f, center.y + 58.0f };

    auto drawOption = [](Vector2 pos, const char* text, bool selected)
    {
        const float radius = selected ? 48.0f : 40.0f;
        const Color fill = selected ? Color{ 255, 220, 100, 245 } : Color{ 55, 55, 70, 230 };
        const Color outline = selected ? Color{ 255, 255, 255, 255 } : Color{ 150, 150, 170, 210 };
        const Color textColor = selected ? Color{ 20, 20, 24, 255 } : RAYWHITE;

        DrawCircleV(pos, radius, fill);
        DrawCircleLines(static_cast<int>(pos.x), static_cast<int>(pos.y), radius, outline);

        const int fontSize = selected ? 17 : 15;
        const int textWidth = MeasureText(text, fontSize);
        DrawText(text, static_cast<int>(pos.x - textWidth * 0.5f), static_cast<int>(pos.y - fontSize * 0.5f), fontSize, textColor);
    };

    drawOption(firstPos, "FP", selectedMode == SSCCameraMode::FirstPerson);
    drawOption(thirdPos, "TP", selectedMode == SSCCameraMode::ThirdPerson);
    drawOption(topPos, "TOP", selectedMode == SSCCameraMode::TopDown);

    DrawLineV(center, firstPos, Color{ 110, 110, 135, 160 });
    DrawLineV(center, thirdPos, Color{ 110, 110, 135, 160 });
    DrawLineV(center, topPos, Color{ 110, 110, 135, 160 });

    const char* title = "PERSPECTIVE SHIFT";
    const int titleSize = 28;
    DrawText(title, static_cast<int>(center.x - MeasureText(title, titleSize) * 0.5f), static_cast<int>(center.y - 230.0f), titleSize, RAYWHITE);

    const char* selected = WheelLabel(selectedMode);
    const int selectedSize = 22;
    DrawText(selected, static_cast<int>(center.x - MeasureText(selected, selectedSize) * 0.5f), static_cast<int>(center.y + 205.0f), selectedSize, Color{ 255, 220, 120, 255 });

    const char* hint = "Hold F3: slow-motion wheel | Release F3: shift perspective";
    const int hintSize = 16;
    DrawText(hint, static_cast<int>(center.x - MeasureText(hint, hintSize) * 0.5f), static_cast<int>(center.y + 236.0f), hintSize, LIGHTGRAY);
}

int main()
{
    InitWindow(1280, 720, "ScaleShift Directional Morph Camera Demo");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL); // We handle ESC manually so it can open the settings menu.
    DisableCursor();

    ScaleShiftCinematicsCameraSystem cameraSystem;

    SSCVec3 playerPosition(0.0f, 0.0f, 0.0f);
    float playerYaw = 0.0f;
    const float playerMoveSpeed = 4.0f;

    bool perspectiveWheelOpen = false;
    bool settingsMenuOpen = false;
    SSCCameraMode wheelSelectedMode = SSCCameraMode::ThirdPerson;

    float wheelFrameAccumulator = 0.0f;
    const float wheelVisualFrameStep = 1.0f / 5.0f;
    bool drawLowFpsFrame = true;

    while (!WindowShouldClose())
    {
        // ===== INPUT + STATE =====
        float dt = GetFrameTime();
        Vector2 mouseDelta = GetMouseDelta();
        float mouseWheel = GetMouseWheelMove();

        // ESC MENU
        if (IsKeyPressed(KEY_ESCAPE))
        {
            settingsMenuOpen = !settingsMenuOpen;

            if (settingsMenuOpen)
            {
                perspectiveWheelOpen = false;
                EnableCursor();
            }
            else
            {
                DisableCursor();
            }
        }

        // F3 WHEEL
        if (!settingsMenuOpen && IsKeyPressed(KEY_F3))
        {
            perspectiveWheelOpen = true;
            wheelSelectedMode = cameraSystem.GetMode();
            EnableCursor();
        }

        // SETTINGS MENU INPUT
        if (settingsMenuOpen)
        {
            dt *= 0.02f;

            if (IsKeyPressed(KEY_F))
                cameraSystem.SetTransitionProfile(SSCCameraTransitionProfile::Fast);

            if (IsKeyPressed(KEY_C))
                cameraSystem.SetTransitionProfile(SSCCameraTransitionProfile::Cinematic);
        }

        // WHEEL INPUT
        if (perspectiveWheelOpen)
        {
            Vector2 center = {
                GetScreenWidth() * 0.5f,
                GetScreenHeight() * 0.5f
            };

            wheelSelectedMode = SelectPerspectiveFromWheel(GetMousePosition(), center);

            dt *= 0.08f;
        }

        // RELEASE F3
        if (!settingsMenuOpen && IsKeyReleased(KEY_F3) && perspectiveWheelOpen)
        {
            perspectiveWheelOpen = false;
            DisableCursor();

            if (cameraSystem.GetMode() == SSCCameraMode::TopDown &&
                wheelSelectedMode != SSCCameraMode::TopDown)
            {
                playerYaw = cameraSystem.GetCamera().rotation.yaw;
            }

            cameraSystem.FlowToModeWithProfile(
                wheelSelectedMode,
                cameraSystem.GetTransitionProfile()
            );
        }

    SSCCameraInput input;
    input.lookX = mouseDelta.x;
    input.lookY = -mouseDelta.y;
    input.zoomAxis = mouseWheel;

    const SSCCameraMode mode = cameraSystem.GetMode();

    if (mode == SSCCameraMode::TopDown)
    {
        input.moveForward = static_cast<float>(IsKeyDown(KEY_W)) - static_cast<float>(IsKeyDown(KEY_S));
        input.moveRight = static_cast<float>(IsKeyDown(KEY_D)) - static_cast<float>(IsKeyDown(KEY_A));
        input.moveUp = static_cast<float>(IsKeyDown(KEY_E)) - static_cast<float>(IsKeyDown(KEY_Q));
    }
    else
    {
        const float forwardInput = static_cast<float>(IsKeyDown(KEY_W)) - static_cast<float>(IsKeyDown(KEY_S));
        const float rightInput = static_cast<float>(IsKeyDown(KEY_D)) - static_cast<float>(IsKeyDown(KEY_A));

        input.moveForward = forwardInput;
        input.moveRight = rightInput;

        if (mode != SSCCameraMode::FirstPerson)
        {
            playerYaw += mouseDelta.x * 0.12f;
        }
        else
        {
            playerYaw = cameraSystem.GetCamera().rotation.yaw;
        }

        const float yawRad = DemoDegToRad(playerYaw);

        const SSCVec3 forward(
            std::cos(yawRad),
                              std::sin(yawRad),
                              0.0f
        );

        const SSCVec3 right(
            -std::sin(yawRad),
                            std::cos(yawRad),
                            0.0f
        );

        playerPosition += forward * (forwardInput * playerMoveSpeed * dt);
        playerPosition += right * (rightInput * playerMoveSpeed * dt);
    }

    SSCPlayerCameraAnchors player;
    player.root.position = playerPosition;
    player.root.rotation = SSCRotator(0.0f, playerYaw, 0.0f);

    player.head.position = playerPosition + SSCVec3(0.0f, 0.0f, 1.72f);
    player.head.rotation = SSCRotator(0.0f, playerYaw, 0.0f);

    player.neckShoulders.position = playerPosition + SSCVec3(0.0f, 0.0f, 1.45f);
    player.neckShoulders.rotation = SSCRotator(0.0f, playerYaw, 0.0f);

    cameraSystem.Update(dt, player, input);
    Camera3D rayCamera = BuildRaylibCamera(cameraSystem.GetCamera());

    BeginDrawing();
    ClearBackground(Color{ 12, 12, 16, 255 });

    BeginMode3D(rayCamera);

    DrawGrid(40, 1.0f);
    DrawPlane(Vector3{ 0.0f, -0.01f, 0.0f }, Vector2{ 80.0f, 80.0f }, Color{ 24, 24, 30, 255 });

    for (int x = -4; x <= 4; ++x)
    {
        for (int z = -4; z <= 4; ++z)
        {
            if ((x + z) % 3 == 0)
            {
                DrawCube(Vector3{ x * 4.0f, 0.5f, z * 4.0f }, 1.0f, 1.0f, 1.0f, Color{ 70, 70, 85, 255 });
                DrawCubeWires(Vector3{ x * 4.0f, 0.5f, z * 4.0f }, 1.0f, 1.0f, 1.0f, Color{ 120, 120, 140, 255 });
            }
        }
    }

    if (mode != SSCCameraMode::FirstPerson)
    {
        Vector3 playerDrawPos = ToRaylibVec3(playerPosition);
        DrawCube(Vector3{ playerDrawPos.x, 0.9f, playerDrawPos.z }, 0.7f, 1.8f, 0.7f, Color{ 180, 30, 35, 255 });
        DrawCubeWires(Vector3{ playerDrawPos.x, 0.9f, playerDrawPos.z }, 0.7f, 1.8f, 0.7f, Color{ 255, 100, 100, 255 });
        DrawSphere(ToRaylibVec3(player.neckShoulders.position), 0.12f, Color{ 255, 220, 80, 255 });
    }

    const SSCVec3 morphDir = cameraSystem.GetMorphDirection();
    const Vector3 morphStart = ToRaylibVec3(player.neckShoulders.position);
    const Vector3 morphEnd = ToRaylibVec3(player.neckShoulders.position + morphDir * 3.0f);
    DrawLine3D(morphStart, morphEnd, Color{ 80, 220, 255, 255 });
    DrawSphere(morphEnd, 0.1f, Color{ 80, 220, 255, 255 });

    EndMode3D();

    DrawRectangle(12, 12, 560, 152, Color{ 0, 0, 0, 175 });
    DrawText("ScaleShift Directional Morph Camera Demo", 24, 24, 22, RAYWHITE);
    DrawText(DemoModeName(mode), 24, 54, 18, Color{ 255, 220, 120, 255 });
    DrawText("WASD move | Mouse steers look/morph | Wheel zoom top-down | Q/E top-down height", 24, 106, 16, LIGHTGRAY);
    DrawText("Cyan line = current directional morph vector", 24, 130, 16, Color{ 80, 220, 255, 255 });
    DrawText("Hold F3 = Perspective Shift Wheel | ESC = Settings", 24, 150, 16, Color{ 255, 220, 120, 255 });

    if (cameraSystem.IsTransitioning())
    {
        DrawText("FLOW STATE: DIRECTIONAL MORPHING", 900, 24, 18, Color{ 255, 220, 120, 255 });
    }
    else
    {
        DrawText("FLOW STATE: IDLE", 1030, 24, 18, Color{ 120, 255, 160, 255 });
    }

    if (perspectiveWheelOpen)
    {
        DrawPerspectiveWheel(wheelSelectedMode);
        DrawText("TIME DILATION: 5 FPS STYLE", 930, 52, 18, Color{ 255, 120, 120, 255 });
    }

    if (settingsMenuOpen)
    {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{ 0, 0, 0, 150 });

        const int panelWidth = 520;
        const int panelHeight = 260;
        const int panelX = GetScreenWidth() / 2 - panelWidth / 2;
        const int panelY = GetScreenHeight() / 2 - panelHeight / 2;

        DrawRectangle(panelX, panelY, panelWidth, panelHeight, Color{ 18, 18, 24, 245 });
        DrawRectangleLines(panelX, panelY, panelWidth, panelHeight, Color{ 210, 210, 230, 220 });

        DrawText("SCALESHIFT SETTINGS", panelX + 34, panelY + 28, 28, RAYWHITE);
        DrawText("Transition Style", panelX + 34, panelY + 82, 20, LIGHTGRAY);

        const bool fastSelected = cameraSystem.GetTransitionProfile() == SSCCameraTransitionProfile::Fast;
        const bool cinematicSelected = cameraSystem.GetTransitionProfile() == SSCCameraTransitionProfile::Cinematic;

        DrawText(fastSelected ? "> [F] Fast" : "  [F] Fast", panelX + 58, panelY + 120, 22,
                 fastSelected ? Color{ 255, 220, 120, 255 } : RAYWHITE);

        DrawText(cinematicSelected ? "> [C] Cinematic" : "  [C] Cinematic", panelX + 58, panelY + 154, 22,
                 cinematicSelected ? Color{ 255, 220, 120, 255 } : RAYWHITE);

        DrawText("Fast = responsive gameplay shifts", panelX + 58, panelY + 194, 15, GRAY);
        DrawText("Cinematic = slower dramatic morphs", panelX + 58, panelY + 214, 15, GRAY);
        DrawText("ESC = close menu", panelX + 34, panelY + 238, 16, LIGHTGRAY);
    }

    EndDrawing();
}

EnableCursor();
CloseWindow();
return 0;
}
