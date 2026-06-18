// ============================================================
// main.cpp
// ============================================================
// Compile:
//     g++ -std=c++17 main.cpp ScaleShiftCinematicsCameraSystem.cpp -o directional_camera_test
// Run:
//     ./directional_camera_test
// ============================================================

#include "ScaleShiftCinematicsCameraSystem.hpp"

#include <iostream>

static const char* CameraModeToString(SSCCameraMode mode)
{
    switch (mode)
    {
        case SSCCameraMode::FirstPerson:
            return "FirstPerson";
        case SSCCameraMode::ThirdPerson:
            return "ThirdPerson";
        case SSCCameraMode::TopDown:
            return "TopDown";
    }

    return "Unknown";
}

static void PrintCameraState(const ScaleShiftCinematicsCameraSystem& cameraSystem)
{
    const SSCCameraState& camera = cameraSystem.GetCamera();
    const SSCVec3& morphDir = cameraSystem.GetMorphDirection();

    std::cout << "Mode: " << CameraModeToString(cameraSystem.GetMode()) << ' ';
    std::cout << "Position: " << camera.position.x << ", " << camera.position.y << ", " << camera.position.z << ' ';
    std::cout << "Rotation: Pitch " << camera.rotation.pitch << ", Yaw " << camera.rotation.yaw << ", Roll " << camera.rotation.roll << ' ';
    std::cout << "FOV: " << camera.fieldOfView << ' ';
    std::cout << "Morph Direction: " << morphDir.x << ", " << morphDir.y << ", " << morphDir.z << ' ';
    std::cout << "Transition Alpha: " << cameraSystem.GetTransitionAlpha() << " ";
}

int main()
{
    ScaleShiftCinematicsCameraSystem cameraSystem;

    SSCPlayerCameraAnchors player;
    player.root.position = SSCVec3(0.0f, 0.0f, 0.0f);
    player.root.rotation = SSCRotator(0.0f, 35.0f, 0.0f);

    player.head.position = SSCVec3(0.0f, 0.0f, 1.72f);
    player.head.rotation = SSCRotator(0.0f, 35.0f, 0.0f);

    player.neckShoulders.position = SSCVec3(0.0f, 0.0f, 1.45f);
    player.neckShoulders.rotation = SSCRotator(0.0f, 35.0f, 0.0f);

    SSCCameraInput input;
    input.lookX = 4.0f;
    input.lookY = -1.0f;
    input.moveForward = 1.0f;
    input.moveRight = 0.5f;

    constexpr float deltaTime = 1.0f / 60.0f;

    std::cout << "--- Directional Morph: First Person to Third Person ---";
    cameraSystem.FlowToMode(SSCCameraMode::ThirdPerson, 0.8f, SSCCameraTransitionCurve::EaseInOut);

    for (int i = 0; i < 60; ++i)
    {
        cameraSystem.Update(deltaTime, player, input);
    }

    PrintCameraState(cameraSystem);

    std::cout << "--- Directional Morph: Third Person to First Person ---";
    cameraSystem.FlowToMode(SSCCameraMode::FirstPerson, 0.55f, SSCCameraTransitionCurve::EaseInOut);

    for (int i = 0; i < 60; ++i)
    {
        cameraSystem.Update(deltaTime, player, input);
    }

    PrintCameraState(cameraSystem);

    std::cout << "Directional morph camera test completed.";
    return 0;
}
