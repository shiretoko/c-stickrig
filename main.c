#include "raylib.h"
#include <math.h>

enum {
    J_PELVIS, J_SPINE, J_CHEST, J_NECK, J_HEAD,
    J_L_CLAV, J_L_UARM, J_L_FARM, J_L_HAND,
    J_R_CLAV, J_R_UARM, J_R_FARM, J_R_HAND,
    J_L_HIP, J_L_THIGH, J_L_SHIN, J_L_FOOT,
    J_R_HIP, J_R_THIGH, J_R_SHIN, J_R_FOOT,
    J_COUNT
};

typedef struct {
    const char *name;
    int   parent;      // -1 = ราก
    float length;      // ความยาวกระดูก
    float angle;       // มุมท่าเริ่มต้น (ท่ายืน)
    float min_angle;   // ช่วงมุมที่ร่างกายทำได้ (ตอนนี้เป็นค่ากว้างชั่วคราว ยังไม่ได้จูน)
    float max_angle;
    float strength;    // ยังไม่ใช้ เว้นช่องไว้
    float health;      // ยังไม่ใช้ เว้นช่องไว้
    float max_speed;   // ยังไม่ใช้ เว้นช่องไว้
} Joint;



#define JOINT(n, p, l, a) { n, p, l, a, -180.0f, 180.0f, 0.0f, 0.0f, 0.0f }

static const Joint JOINTS[J_COUNT] = {
    [J_PELVIS] = JOINT("pelvis", -1,       0,   0),
    [J_SPINE]  = JOINT("spine",  J_PELVIS, 60,  180),
    [J_CHEST]  = JOINT("chest",  J_SPINE,  50,  0),
    [J_NECK]   = JOINT("neck",   J_CHEST,  15,  0),
    [J_HEAD]   = JOINT("head",   J_NECK,   30,  0),

    [J_L_CLAV] = JOINT("l_clav", J_CHEST,  25, -90),
    [J_L_UARM] = JOINT("l_uarm", J_L_CLAV, 50, -90),
    [J_L_FARM] = JOINT("l_farm", J_L_UARM, 45,  0),
    [J_L_HAND] = JOINT("l_hand", J_L_FARM, 15,  0),

    [J_R_CLAV] = JOINT("r_clav", J_CHEST,  25,  90),
    [J_R_UARM] = JOINT("r_uarm", J_R_CLAV, 50,  90),
    [J_R_FARM] = JOINT("r_farm", J_R_UARM, 45,  0),
    [J_R_HAND] = JOINT("r_hand", J_R_FARM, 15,  0),

    [J_L_HIP]   = JOINT("l_hip",   J_PELVIS, 15,  90),
    [J_L_THIGH] = JOINT("l_thigh", J_L_HIP,  70, -90),
    [J_L_SHIN]  = JOINT("l_shin",  J_L_THIGH, 65, 0),
    [J_L_FOOT]  = JOINT("l_foot",  J_L_SHIN, 15,  20),

    [J_R_HIP]   = JOINT("r_hip",   J_PELVIS, 15, -90),
    [J_R_THIGH] = JOINT("r_thigh", J_R_HIP,  70,  90),
    [J_R_SHIN]  = JOINT("r_shin",  J_R_THIGH, 65, 0),
    [J_R_FOOT]  = JOINT("r_foot",  J_R_SHIN, 15, -20),
};

typedef struct { Vector2 start, end; } Seg;

// Forward kinematics: จากมุมของทุกข้อต่อ -> ตำแหน่งบนจอ
static void compute_fk(const Joint *j, const float *ang, Vector2 root, Seg *out)
{
    float world[J_COUNT];
    for (int i = 0; i < J_COUNT; i++) {
        float a = ang[i];
        if (a < j[i].min_angle) a = j[i].min_angle;
        if (a > j[i].max_angle) a = j[i].max_angle;

        int p = j[i].parent;
        if (p < 0) {
            world[i] = a;
            out[i].start = root;
        } else {
            world[i] = world[p] + a;
            out[i].start = out[p].end;
        }
        float r = world[i] * DEG2RAD;
        out[i].end.x = out[i].start.x + sinf(r) * j[i].length;
        out[i].end.y = out[i].start.y + cosf(r) * j[i].length;
    }
}


static void fit_to_ground(Seg *seg, float ground_y)
{
    float lowest = seg[0].end.y;
    for (int i = 0; i < J_COUNT; i++) {
        if (seg[i].end.y > lowest) lowest = seg[i].end.y;
    }
    float shift = ground_y - lowest;
    for (int i = 0; i < J_COUNT; i++) {
        seg[i].start.y += shift;
        seg[i].end.y   += shift;
    }
}

int main(void)
{
    InitWindow(800, 600, "Rig test 01 - stand");
    SetTargetFPS(60);

    float ang[J_COUNT];
    for (int i = 0; i < J_COUNT; i++) ang[i] = JOINTS[i].angle;

    const float ground_y = 540.0f;

    while (!WindowShouldClose()) {
        Seg seg[J_COUNT];
        compute_fk(JOINTS, ang, (Vector2){ 400.0f, 300.0f }, seg);
        fit_to_ground(seg, ground_y);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawLine(0, (int)ground_y, 800, (int)ground_y, GRAY);

        for (int i = 1; i < J_COUNT; i++) {
            DrawLineEx(seg[i].start, seg[i].end, 4.0f, DARKGRAY);
        }
        for (int i = 0; i < J_COUNT; i++) {
            DrawCircleV(seg[i].start, 5.0f, RED);
        }

        Vector2 hc = { (seg[J_HEAD].start.x + seg[J_HEAD].end.x) * 0.5f,
                       (seg[J_HEAD].start.y + seg[J_HEAD].end.y) * 0.5f };
        DrawCircleLines((int)hc.x, (int)hc.y, 18.0f, DARKGRAY);

        DrawText("Rig test 01: stand", 10, 10, 20, DARKGRAY);
        DrawFPS(10, 40);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

