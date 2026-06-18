// ============================================================
// ScaleShiftCinematicsCameraSystem.h
// ============================================================
// Directional ScaleShift camera morph system.
//
// Purpose:
// - First Person: camera is linked directly to the player's head.
// - Third Person: camera orbits from the player's neck/shoulder anchor.
// - Top Down: camera is free-world/map based.
// - Directional Morphing: when entering/exiting the player, the transition
//   direction is influenced by mouse look, player forward, movement direction,
//   and current camera forward.
// ============================================================

#pragma once

struct SSCVec3
{
    float x;
    float y;
    float z;

    SSCVec3();
    SSCVec3(float x_, float y_, float z_);

    SSCVec3 operator+(const SSCVec3& rhs) const;
    SSCVec3 operator-(const SSCVec3& rhs) const;
    SSCVec3 operator*(float scalar) const;
    SSCVec3& operator+=(const SSCVec3& rhs);
};

struct SSCRotator
{
    float pitch;
    float yaw;
    float roll;

    SSCRotator();
    SSCRotator(float pitch_, float yaw_, float roll_);
};

struct SSCTransform
{
    SSCVec3 position;
    SSCRotator rotation;
};

struct SSCPlayerCameraAnchors
{
    SSCTransform root;
    SSCTransform head;
    SSCTransform neckShoulders;
};

struct SSCCameraInput
{
    float lookX = 0.0f;
    float lookY = 0.0f;

    float moveForward = 0.0f;
    float moveRight = 0.0f;
    float moveUp = 0.0f;

    float zoomAxis = 0.0f;
};

enum class SSCCameraMode
{
    FirstPerson,
    ThirdPerson,
    TopDown
};

enum class SSCCameraTransitionProfile
{
    Fast,
    Cinematic
};

enum class SSCThirdPersonAnchor
{
    OverShoulderLeftBehind,
    OverShoulderRightBehind,
    OverShoulderLeftFront,
    OverShoulderRightFront,
    CenterBehind,
    CenterFront
};

enum class SSCCameraFlowState
{
    Idle,
    DirectionalMorphing
};

enum class SSCCameraTransitionCurve
{
    Linear,
    SmoothStep,
    EaseIn,
    EaseOut,
    EaseInOut
};

struct SSCCameraState
{
    SSCVec3 position;
    SSCRotator rotation;
    float fieldOfView = 75.0f;
};

struct SSCDirectionalMorphSettings
{
    float currentCameraForwardWeight = 0.55f;
    float playerForwardWeight = 0.25f;
    float movementDirectionWeight = 0.20f;

    float exitDistance = 4.25f;
    float entryDistance = 1.15f;
    float verticalLift = 0.35f;
};

struct SSCCameraTransitionSettings
{
    float fastDuration = 0.35f;
    SSCCameraTransitionCurve fastCurve = SSCCameraTransitionCurve::SmoothStep;

    float cinematicDuration = 1.25f;
    SSCCameraTransitionCurve cinematicCurve = SSCCameraTransitionCurve::EaseInOut;
};

class ScaleShiftCinematicsCameraSystem
{
public:
    ScaleShiftCinematicsCameraSystem();

    void SetMode(SSCCameraMode newMode);
    void FlowToMode(SSCCameraMode newMode, float durationSeconds, SSCCameraTransitionCurve curve = SSCCameraTransitionCurve::SmoothStep);

    SSCCameraMode GetMode() const;
    SSCCameraMode GetSourceMode() const;
    SSCCameraMode GetTargetMode() const;
    SSCCameraFlowState GetFlowState() const;

    bool IsTransitioning() const;
    float GetTransitionAlpha() const;

    const SSCCameraState& GetCamera() const;
    const SSCVec3& GetMorphDirection() const;

    void FlowToModeWithProfile(SSCCameraMode newMode, SSCCameraTransitionProfile profile);

    void SetTransitionProfile(SSCCameraTransitionProfile profile);
    SSCCameraTransitionProfile GetTransitionProfile() const;
    void ToggleTransitionProfile();

    void SetTransitionSettings(const SSCCameraTransitionSettings& settings);
    const SSCCameraTransitionSettings& GetTransitionSettings() const;

    void SetDirectionalMorphSettings(const SSCDirectionalMorphSettings& settings);
    const SSCDirectionalMorphSettings& GetDirectionalMorphSettings() const;

    void Update(float deltaTime, const SSCPlayerCameraAnchors& player, const SSCCameraInput& input);

private:
    SSCCameraState BuildFirstPersonCamera(const SSCPlayerCameraAnchors& player, const SSCCameraInput& input, float deltaTime);
    SSCCameraState BuildThirdPersonCamera(const SSCPlayerCameraAnchors& player, const SSCCameraInput& input, float deltaTime);
    SSCCameraState BuildTopDownCamera(const SSCCameraInput& input, float deltaTime);

    SSCCameraTransitionProfile transitionProfile;
    SSCCameraTransitionSettings transitionSettings;

    void BeginDirectionalMorph(SSCCameraMode newMode, float durationSeconds, SSCCameraTransitionCurve curve);
    SSCVec3 CalculateDirectionalMorphVector(const SSCPlayerCameraAnchors& player, const SSCCameraInput& input) const;
    SSCCameraState ApplyDirectionalMorphPath(const SSCCameraState& target, const SSCPlayerCameraAnchors& player, float t) const;

    static float DegToRad(float degrees);
    static float RadToDeg(float radians);
    static float Clamp(float value, float minValue, float maxValue);
    static float Length(const SSCVec3& v);
    static SSCVec3 Normalize(const SSCVec3& v, const SSCVec3& fallback = SSCVec3(1.0f, 0.0f, 0.0f));
    static float SmoothStep(float t);
    static float ApplyTransitionCurve(float t, SSCCameraTransitionCurve curve);
    static SSCVec3 Lerp(const SSCVec3& a, const SSCVec3& b, float t);
    static float LerpFloat(float a, float b, float t);
    static SSCRotator LerpRotator(const SSCRotator& a, const SSCRotator& b, float t);
    static SSCVec3 ForwardFromYaw(float yawDegrees);
    static SSCVec3 RightFromYaw(float yawDegrees);
    static SSCVec3 ForwardFromRotator(const SSCRotator& rotation);
    static SSCRotator LookAtRotation(const SSCVec3& from, const SSCVec3& to);

private:
    SSCCameraMode mode;
    SSCCameraMode sourceMode;
    SSCCameraMode targetMode;
    SSCCameraFlowState flowState;
    SSCCameraTransitionCurve transitionCurve;

    SSCThirdPersonAnchor activeThirdPersonAnchor =
    SSCThirdPersonAnchor::CenterBehind;

    static SSCThirdPersonAnchor ChooseThirdPersonAnchor(float yawDegrees);

    SSCCameraState camera;
    SSCCameraState previousCamera;

    bool isTransitioning;
    float transitionAlpha;
    float transitionDuration;

    float firstPersonFOV;
    float thirdPersonFOV;
    float topDownFOV;

    float mouseSensitivity;

    float firstPersonPitch;
    float firstPersonYaw;

    float thirdPersonPitch;
    float thirdPersonYaw;
    float thirdPersonDistance;
    float thirdPersonShoulderOffset;
    float thirdPersonHeightOffset;

    SSCVec3 topDownPosition;
    SSCRotator topDownRotation;
    float topDownMoveSpeed;
    float topDownZoomSpeed;
    float topDownMinHeight;
    float topDownMaxHeight;

    SSCDirectionalMorphSettings morphSettings;
    SSCVec3 morphDirection;
    SSCVec3 morphStartPosition;
    SSCRotator morphStartRotation;
};
