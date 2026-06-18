; spherical_camera_angles.asm
; Linux x86_64 / System V ABI / NASM
;
; Build:
;   nasm -f elf64 spherical_camera_angles.asm -o spherical_camera_angles.o
;
; Function:
;   pt_camera_sphere_angles(cameraPoint, sphereCentre, outData)
;
; Inputs:
;   rdi = cameraPoint float[3]   ; x,y,z camera candidate point
;   rsi = sphereCentre float[3]  ; x,y,z player / sphere centre
;   rdx = outData float[8]
;
; Output:
;   outData[0] = dx
;   outData[1] = dy
;   outData[2] = dz
;   outData[3] = radius
;   outData[4] = horizontal distance
;   outData[5] = yaw   radians
;   outData[6] = pitch radians
;   outData[7] = distance squared

BITS 64
default rel

global pt_camera_sphere_angles

section .rodata align=16
abs_mask:   dd 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff
sign_mask:  dd 0x80000000, 0x80000000, 0x80000000, 0x80000000

zero:        dd 0.0
epsilon:     dd 0.0000001
pi_over_4:   dd 0.7853981633974483
three_pi_4:  dd 2.356194490192345

section .text

; ------------------------------------------------------------
; pt_camera_sphere_angles
; ------------------------------------------------------------
pt_camera_sphere_angles:
    ; dx = sphereCentre.x - cameraPoint.x
    movss xmm0, [rsi]
    subss xmm0, [rdi]

    ; dy = sphereCentre.y - cameraPoint.y
    movss xmm1, [rsi + 4]
    subss xmm1, [rdi + 4]

    ; dz = sphereCentre.z - cameraPoint.z
    movss xmm2, [rsi + 8]
    subss xmm2, [rdi + 8]

    ; Store dx, dy, dz
    movss [rdx], xmm0
    movss [rdx + 4], xmm1
    movss [rdx + 8], xmm2

    ; distanceSquared = dx² + dy² + dz²
    movaps xmm3, xmm0
    mulss xmm3, xmm0

    movaps xmm4, xmm1
    mulss xmm4, xmm1
    addss xmm3, xmm4

    movaps xmm4, xmm2
    mulss xmm4, xmm2
    addss xmm3, xmm4

    movss [rdx + 28], xmm3

    ; radius = sqrt(distanceSquared)
    sqrtss xmm5, xmm3
    movss [rdx + 12], xmm5

    ; horizontal = sqrt(dx² + dz²)
    movaps xmm6, xmm0
    mulss xmm6, xmm0

    movaps xmm7, xmm2
    mulss xmm7, xmm2
    addss xmm6, xmm7

    sqrtss xmm6, xmm6
    movss [rdx + 16], xmm6

    ; yaw = atan2(dx, dz)
    movss xmm0, [rdx]       ; y argument = dx
    movss xmm1, [rdx + 8]   ; x argument = dz
    call pt_fast_atan2f
    movss [rdx + 20], xmm0

    ; pitch = atan2(dy, horizontal)
    movss xmm0, [rdx + 4]   ; y argument = dy
    movss xmm1, [rdx + 16]  ; x argument = horizontal
    call pt_fast_atan2f
    movss [rdx + 24], xmm0

    ret


; ------------------------------------------------------------
; pt_fast_atan2f
;
; Approximate atan2(y, x)
;
; Input:
;   xmm0 = y
;   xmm1 = x
;
; Output:
;   xmm0 = angle in radians
;
; Formula:
;   if x >= 0:
;       r = (x - abs(y)) / (x + abs(y))
;       angle = π/4 - π/4 * r
;   else:
;       r = (x + abs(y)) / (abs(y) - x)
;       angle = 3π/4 - π/4 * r
;
;   if y < 0:
;       angle = -angle
; ------------------------------------------------------------
pt_fast_atan2f:
    ; xmm2 = abs(y) + epsilon
    movaps xmm2, xmm0
    andps xmm2, [abs_mask]
    addss xmm2, [epsilon]

    ; if x < 0, jump negative-x path
    ucomiss xmm1, [zero]
    jb .x_negative

.x_positive:
    ; r = (x - abs_y) / (x + abs_y)
    movaps xmm3, xmm1
    subss xmm3, xmm2

    movaps xmm4, xmm1
    addss xmm4, xmm2

    divss xmm3, xmm4

    ; angle = pi/4 - pi/4 * r
    movss xmm5, [pi_over_4]
    movaps xmm6, xmm5
    mulss xmm6, xmm3
    subss xmm5, xmm6

    jmp .apply_sign

.x_negative:
    ; r = (x + abs_y) / (abs_y - x)
    movaps xmm3, xmm1
    addss xmm3, xmm2

    movaps xmm4, xmm2
    subss xmm4, xmm1

    divss xmm3, xmm4

    ; angle = 3pi/4 - pi/4 * r
    movss xmm5, [three_pi_4]
    movss xmm6, [pi_over_4]
    mulss xmm6, xmm3
    subss xmm5, xmm6

.apply_sign:
    ; if y < 0, angle = -angle
    ucomiss xmm0, [zero]
    jb .negate

    movaps xmm0, xmm5
    ret

.negate:
    xorps xmm5, [sign_mask]
    movaps xmm0, xmm5
    ret
