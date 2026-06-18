// ============================================================
// ScaleShiftCinematicsCameraSystem.cpp
// ============================================================

#include "ScaleShiftCinematicsCameraSystem.hpp"

#include <algorithm>
#include <cmath>

extern "C" void pt_camera_sphere_angles(
    const float* cameraPoint,
    const float* sphereCentre,
    float* outData
);

SSCVec3::SSCVec3()
: x(0.0f), y(0.0f), z(0.0f)
{
}

SSCVec3::SSCVec3(float x_, float y_, float z_)
: x(x_), y(y_), z(z_)
{
}

SSCVec3 SSCVec3::operator+(const SSCVec3& rhs) const
{
    return SSCVec3(x + rhs.x, y + rhs.y, z + rhs.z);
}

SSCVec3 SSCVec3::operator-(const SSCVec3& rhs) const
{
    return SSCVec3(x - rhs.x, y - rhs.y, z - rhs.z);
}

SSCVec3 SSCVec3::operator*(float scalar) const
{
    return SSCVec3(x * scalar, y * scalar, z * scalar);
}

SSCVec3& SSCVec3::operator+=(const SSCVec3& rhs)
{
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
}

SSCRotator::SSCRotator()
: pitch(0.0f), yaw(0.0f), roll(0.0f)
{
}

SSCRotator::SSCRotator(float pitch_, float yaw_, float roll_)
: pitch(pitch_), yaw(yaw_), roll(roll_)
{
}

ScaleShiftCinematicsCameraSystem::ScaleShiftCinematicsCameraSystem()
: mode(SSCCameraMode::FirstPerson),
sourceMode(SSCCameraMode::FirstPerson),
targetMode(SSCCameraMode::FirstPerson),
flowState(SSCCameraFlowState::Idle),
transitionCurve(SSCCameraTransitionCurve::SmoothStep),
transitionProfile(SSCCameraTransitionProfile::Fast),
isTransitioning(false),
transitionAlpha(1.0f),
transitionDuration(0.55f),
firstPersonFOV(82.0f),
thirdPersonFOV(75.0f),
topDownFOV(60.0f),
mouseSensitivity(0.12f),
firstPersonPitch(0.0f),
firstPersonYaw(0.0f),
thirdPersonPitch(-8.0f),
thirdPersonYaw(0.0f),
thirdPersonDistance(4.25f),
thirdPersonShoulderOffset(0.45f),
thirdPersonHeightOffset(0.2f),
topDownPosition(0.0f, 0.0f, 35.0f),
topDownRotation(-70.0f, 0.0f, 0.0f),
topDownMoveSpeed(18.0f),
topDownZoomSpeed(12.0f),
topDownMinHeight(12.0f),
topDownMaxHeight(90.0f),
morphDirection(1.0f, 0.0f, 0.0f),
morphStartPosition(0.0f, 0.0f, 2.0f),
morphStartRotation(0.0f, 0.0f, 0.0f)
{
    camera.position = SSCVec3(0.0f, 0.0f, 2.0f);
    camera.rotation = SSCRotator(0.0f, 0.0f, 0.0f);
    camera.fieldOfView = firstPersonFOV;

    previousCamera = camera;
}

void ScaleShiftCinematicsCameraSystem::SetMode(SSCCameraMode newMode)
{
    FlowToMode(newMode, transitionDuration, SSCCameraTransitionCurve::SmoothStep);
}

void ScaleShiftCinematicsCameraSystem::FlowToMode(SSCCameraMode newMode, float durationSeconds, SSCCameraTransitionCurve curve)
{
    if (mode == newMode && !isTransitioning)
        return;

    sourceMode = mode;
    targetMode = newMode;
    mode = newMode;

    if (newMode == SSCCameraMode::FirstPerson)
    {
        firstPersonYaw = 0.0f;
        firstPersonPitch = 0.0f;
    }

    transitionDuration = std::max(durationSeconds, 0.001f);
    transitionCurve = curve;
    transitionAlpha = 0.0f;

    previousCamera = camera;
    morphStartPosition = camera.position;
    morphStartRotation = camera.rotation;

    isTransitioning = true;
    flowState = SSCCameraFlowState::DirectionalMorphing;
}

void ScaleShiftCinematicsCameraSystem::FlowToModeWithProfile(
    SSCCameraMode newMode,
    SSCCameraTransitionProfile profile
)
{
    transitionProfile = profile;

    if (profile == SSCCameraTransitionProfile::Fast)
    {
        FlowToMode(newMode, transitionSettings.fastDuration, transitionSettings.fastCurve);
    }
    else
    {
        FlowToMode(newMode, transitionSettings.cinematicDuration, transitionSettings.cinematicCurve);
    }
}

void ScaleShiftCinematicsCameraSystem::SetTransitionProfile(SSCCameraTransitionProfile profile)
{
    transitionProfile = profile;
}

SSCCameraTransitionProfile ScaleShiftCinematicsCameraSystem::GetTransitionProfile() const
{
    return transitionProfile;
}

void ScaleShiftCinematicsCameraSystem::ToggleTransitionProfile()
{
    transitionProfile =
    transitionProfile == SSCCameraTransitionProfile::Fast
    ? SSCCameraTransitionProfile::Cinematic
    : SSCCameraTransitionProfile::Fast;
}

void ScaleShiftCinematicsCameraSystem::SetTransitionSettings(const SSCCameraTransitionSettings& settings)
{
    transitionSettings = settings;
}

const SSCCameraTransitionSettings& ScaleShiftCinematicsCameraSystem::GetTransitionSettings() const
{
    return transitionSettings;
}

SSCCameraMode ScaleShiftCinematicsCameraSystem::GetMode() const
{
    return mode;
}

SSCCameraMode ScaleShiftCinematicsCameraSystem::GetSourceMode() const
{
    return sourceMode;
}

SSCCameraMode ScaleShiftCinematicsCameraSystem::GetTargetMode() const
{
    return targetMode;
}

SSCCameraFlowState ScaleShiftCinematicsCameraSystem::GetFlowState() const
{
    return flowState;
}

bool ScaleShiftCinematicsCameraSystem::IsTransitioning() const
{
    return isTransitioning;
}

float ScaleShiftCinematicsCameraSystem::GetTransitionAlpha() const
{
    return transitionAlpha;
}

const SSCCameraState& ScaleShiftCinematicsCameraSystem::GetCamera() const
{
    return camera;
}

const SSCVec3& ScaleShiftCinematicsCameraSystem::GetMorphDirection() const
{
    return morphDirection;
}

void ScaleShiftCinematicsCameraSystem::SetDirectionalMorphSettings(const SSCDirectionalMorphSettings& settings)
{
    morphSettings = settings;
}

const SSCDirectionalMorphSettings& ScaleShiftCinematicsCameraSystem::GetDirectionalMorphSettings() const
{
    return morphSettings;
}

void ScaleShiftCinematicsCameraSystem::Update(float deltaTime, const SSCPlayerCameraAnchors& player, const SSCCameraInput& input)
{
    SSCCameraState target;

    switch (mode)
    {
        case SSCCameraMode::FirstPerson:
            target = BuildFirstPersonCamera(player, input, deltaTime);
            break;

        case SSCCameraMode::ThirdPerson:
            target = BuildThirdPersonCamera(player, input, deltaTime);
            break;

        case SSCCameraMode::TopDown:
            target = BuildTopDownCamera(input, deltaTime);
            break;
    }

    if (isTransitioning)
    {
        if (transitionAlpha <= 0.0f)
        {
            morphDirection = CalculateDirectionalMorphVector(player, input);
        }

        transitionAlpha += deltaTime / transitionDuration;
        const float curvedT = ApplyTransitionCurve(Clamp(transitionAlpha, 0.0f, 1.0f), transitionCurve);

        camera = ApplyDirectionalMorphPath(target, player, curvedT);

        if (transitionAlpha >= 1.0f)
        {
            isTransitioning = false;
            flowState = SSCCameraFlowState::Idle;
            sourceMode = mode;
            targetMode = mode;
            camera = target;
        }
    }
    else
    {
        camera = target;
    }
}

SSCCameraState ScaleShiftCinematicsCameraSystem::BuildFirstPersonCamera(const SSCPlayerCameraAnchors& player, const SSCCameraInput& input, float deltaTime)
{
    (void)deltaTime;

    firstPersonYaw += input.lookX * mouseSensitivity;
    firstPersonPitch += input.lookY * mouseSensitivity;
    firstPersonPitch = Clamp(firstPersonPitch, -89.0f, 89.0f);

    SSCCameraState result;
    result.position = player.head.position;
    result.rotation = SSCRotator(firstPersonPitch, firstPersonYaw, 0.0f);
    result.fieldOfView = firstPersonFOV;

    return result;
}

SSCCameraState ScaleShiftCinematicsCameraSystem::BuildThirdPersonCamera(const SSCPlayerCameraAnchors& player, const SSCCameraInput& input, float deltaTime)
{
    (void)deltaTime;

    thirdPersonYaw += input.lookX * mouseSensitivity;
    thirdPersonPitch += input.lookY * mouseSensitivity;
    thirdPersonPitch = Clamp(thirdPersonPitch, -45.0f, 35.0f);

    const SSCVec3 targetCenter = player.neckShoulders.position + SSCVec3(0.0f, 0.0f, thirdPersonHeightOffset);

    // Directional morphing matters here:
    // when entering third person, the camera exits along morphDirection instead of always pulling from a fixed angle.
    SSCVec3 exitDirection = morphDirection;
    if (!isTransitioning)
    {
        exitDirection = ForwardFromYaw(thirdPersonYaw);
    }

    const SSCVec3 right = RightFromYaw(thirdPersonYaw);
    const float pitchRad = DegToRad(thirdPersonPitch);
    const float horizontalDistance = thirdPersonDistance * std::cos(pitchRad);
    const float verticalDistance = thirdPersonDistance * std::sin(pitchRad);

    SSCVec3 cameraOffset = exitDirection * -horizontalDistance;
    cameraOffset += right * thirdPersonShoulderOffset;
    cameraOffset += SSCVec3(0.0f, 0.0f, -verticalDistance);

    SSCCameraState result;
    result.position = targetCenter + cameraOffset;
    result.rotation = LookAtRotation(result.position, targetCenter);
    result.fieldOfView = thirdPersonFOV;

    return result;
}

SSCThirdPersonAnchor
ScaleShiftCinematicsCameraSystem::ChooseThirdPersonAnchor(float yawDegrees)
{
    float yaw = std::fmod(yawDegrees, 360.0f);

    if (yaw < 0.0f)
        yaw += 360.0f;

    if (yaw >= 330.0f || yaw < 30.0f)
        return SSCThirdPersonAnchor::CenterBehind;

    if (yaw >= 30.0f && yaw < 90.0f)
        return SSCThirdPersonAnchor::OverShoulderRightBehind;

    if (yaw >= 90.0f && yaw < 150.0f)
        return SSCThirdPersonAnchor::OverShoulderRightFront;

    if (yaw >= 150.0f && yaw < 210.0f)
        return SSCThirdPersonAnchor::CenterFront;

    if (yaw >= 210.0f && yaw < 270.0f)
        return SSCThirdPersonAnchor::OverShoulderLeftFront;

    return SSCThirdPersonAnchor::OverShoulderLeftBehind;
}

SSCCameraState ScaleShiftCinematicsCameraSystem::BuildTopDownCamera(const SSCCameraInput& input, float deltaTime)
{
    const SSCVec3 forward = ForwardFromYaw(topDownRotation.yaw);
    const SSCVec3 right = RightFromYaw(topDownRotation.yaw);

    topDownPosition += forward * (input.moveForward * topDownMoveSpeed * deltaTime);
    topDownPosition += right * (input.moveRight * topDownMoveSpeed * deltaTime);
    topDownPosition.z += input.moveUp * topDownMoveSpeed * deltaTime;
    topDownPosition.z -= input.zoomAxis * topDownZoomSpeed * deltaTime;
    topDownPosition.z = Clamp(topDownPosition.z, topDownMinHeight, topDownMaxHeight);

    topDownRotation.yaw += input.lookX * mouseSensitivity;
    topDownRotation.pitch += input.lookY * mouseSensitivity;
    topDownRotation.pitch = Clamp(topDownRotation.pitch, -89.0f, -35.0f);

    SSCCameraState result;
    result.position = topDownPosition;
    result.rotation = topDownRotation;
    result.fieldOfView = topDownFOV;

    return result;
}

SSCVec3 ScaleShiftCinematicsCameraSystem::CalculateDirectionalMorphVector(const SSCPlayerCameraAnchors& player, const SSCCameraInput& input) const
{
    const SSCVec3 cameraForward = ForwardFromRotator(camera.rotation);
    const SSCVec3 playerForward = ForwardFromYaw(player.root.rotation.yaw);
    const SSCVec3 playerRight = RightFromYaw(player.root.rotation.yaw);

    SSCVec3 movementDirection = playerForward * input.moveForward + playerRight * input.moveRight;
    movementDirection = Normalize(movementDirection, playerForward);

    SSCVec3 blendedDirection =
    cameraForward * morphSettings.currentCameraForwardWeight +
    playerForward * morphSettings.playerForwardWeight +
    movementDirection * morphSettings.movementDirectionWeight;

    return Normalize(blendedDirection, cameraForward);
}

SSCCameraState ScaleShiftCinematicsCameraSystem::ApplyDirectionalMorphPath(const SSCCameraState& target, const SSCPlayerCameraAnchors& player, float t) const
{
    SSCCameraState result;

    const SSCVec3 head = player.head.position;
    const SSCVec3 shoulder = player.neckShoulders.position;

    SSCVec3 controlPoint = shoulder + morphDirection * morphSettings.exitDistance;
    controlPoint.z += morphSettings.verticalLift;

    if (targetMode == SSCCameraMode::FirstPerson)
    {
        controlPoint = head - morphDirection * morphSettings.entryDistance;
        controlPoint.z += morphSettings.verticalLift * 0.5f;
    }

    // Quadratic Bezier path:
    // start -> directional control point -> target
    const SSCVec3 a = Lerp(morphStartPosition, controlPoint, t);
    const SSCVec3 b = Lerp(controlPoint, target.position, t);
    result.position = Lerp(a, b, t);

    result.rotation = LerpRotator(morphStartRotation, target.rotation, t);
    result.fieldOfView = LerpFloat(previousCamera.fieldOfView, target.fieldOfView, t);

    return result;
}

float ScaleShiftCinematicsCameraSystem::DegToRad(float degrees)
{
    return degrees * 3.1415926535f / 180.0f;
}

float ScaleShiftCinematicsCameraSystem::RadToDeg(float radians)
{
    return radians * 180.0f / 3.1415926535f;
}

float ScaleShiftCinematicsCameraSystem::Clamp(float value, float minValue, float maxValue)
{
    return std::max(minValue, std::min(value, maxValue));
}

float ScaleShiftCinematicsCameraSystem::Length(const SSCVec3& v)
{
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

SSCVec3 ScaleShiftCinematicsCameraSystem::Normalize(const SSCVec3& v, const SSCVec3& fallback)
{
    const float len = Length(v);
    if (len <= 0.0001f)
        return fallback;

    return SSCVec3(v.x / len, v.y / len, v.z / len);
}

float ScaleShiftCinematicsCameraSystem::SmoothStep(float t)
{
    t = Clamp(t, 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

float ScaleShiftCinematicsCameraSystem::ApplyTransitionCurve(float t, SSCCameraTransitionCurve curve)
{
    t = Clamp(t, 0.0f, 1.0f);

    switch (curve)
    {
        case SSCCameraTransitionCurve::Linear:
            return t;

        case SSCCameraTransitionCurve::SmoothStep:
            return SmoothStep(t);

        case SSCCameraTransitionCurve::EaseIn:
            return t * t;

        case SSCCameraTransitionCurve::EaseOut:
            return 1.0f - ((1.0f - t) * (1.0f - t));

        case SSCCameraTransitionCurve::EaseInOut:
            if (t < 0.5f)
                return 2.0f * t * t;
        return 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) * 0.5f;
    }

    return t;
}

SSCVec3 ScaleShiftCinematicsCameraSystem::Lerp(const SSCVec3& a, const SSCVec3& b, float t)
{
    t = Clamp(t, 0.0f, 1.0f);
    return a * (1.0f - t) + b * t;
}

float ScaleShiftCinematicsCameraSystem::LerpFloat(float a, float b, float t)
{
    t = Clamp(t, 0.0f, 1.0f);
    return a + (b - a) * t;
}

SSCRotator ScaleShiftCinematicsCameraSystem::LerpRotator(const SSCRotator& a, const SSCRotator& b, float t)
{
    t = Clamp(t, 0.0f, 1.0f);

    return SSCRotator(
        LerpFloat(a.pitch, b.pitch, t),
                      LerpFloat(a.yaw, b.yaw, t),
                      LerpFloat(a.roll, b.roll, t)
    );
}

SSCVec3 ScaleShiftCinematicsCameraSystem::ForwardFromYaw(float yawDegrees)
{
    const float yaw = DegToRad(yawDegrees);
    return SSCVec3(std::cos(yaw), std::sin(yaw), 0.0f);
}

SSCVec3 ScaleShiftCinematicsCameraSystem::RightFromYaw(float yawDegrees)
{
    const float yaw = DegToRad(yawDegrees + 90.0f);
    return SSCVec3(std::cos(yaw), std::sin(yaw), 0.0f);
}

SSCVec3 ScaleShiftCinematicsCameraSystem::ForwardFromRotator(const SSCRotator& rotation)
{
    const float pitch = DegToRad(rotation.pitch);
    const float yaw = DegToRad(rotation.yaw);

    return Normalize(
        SSCVec3(
            std::cos(pitch) * std::cos(yaw),
                std::cos(pitch) * std::sin(yaw),
                std::sin(pitch)
        ),
        SSCVec3(1.0f, 0.0f, 0.0f)
    );
}

SSCRotator ScaleShiftCinematicsCameraSystem::LookAtRotation(const SSCVec3& from, const SSCVec3& to)
{
    const float cameraPoint[3] =
    {
        from.x,
        from.y,
        from.z
    };

    const float sphereCentre[3] =
    {
        to.x,
        to.y,
        to.z
    };

    float outData[8] = {};

    pt_camera_sphere_angles(cameraPoint, sphereCentre, outData);

    const float yawDegrees = RadToDeg(outData[5]);
    const float pitchDegrees = RadToDeg(outData[6]);

    return SSCRotator(pitchDegrees, yawDegrees, 0.0f);
}
