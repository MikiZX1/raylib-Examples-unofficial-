#include "raylib.h"
#include "raymath.h"
//------------------------------------------------------------------------------------
// player always upright
// camera offset if you want to move the camera to aim
// you can also change default camera offset to get a kind of 3rd person view
//------------------------------------------------------------------------------------
int main(void)
{
InitWindow(1280, 1024, "Simple FPS movement");

Vector3 rotation=(Vector3){0.0f,0.0f,0.0f};
Vector3 camera_offset=(Vector3){0.0f,2.0f,0.0f};
Vector3 desired_camera_offset=(Vector3){0.0f,2.0f,0.0f};

Vector3 player_position=(Vector3){0.0f,5.0f,-20.0f};;

float movement_speed=4.0f;

Camera cam1;
cam1.fovy=90;
cam1.up=(Vector3){0.0f,1.0f,0.0f};
cam1.projection=CAMERA_PERSPECTIVE;

DisableCursor();

while (!WindowShouldClose())
{
    rotation.x-=GetMouseDelta().x/100.0;
    rotation.y+=GetMouseDelta().y/100.0;
    rotation.y=Clamp(rotation.y,-80.0f*DEG2RAD,80.0f*DEG2RAD);

    Vector3 forward_vector=(Vector3){0.0f,0.0f,-1.0f};
    forward_vector=Vector3RotateByAxisAngle(forward_vector,(Vector3){0.0f,1.0f,0.0f},rotation.x);

    Vector3 left_vector=(Vector3){-1.0f,0.0f,0.0f};
    left_vector=Vector3RotateByAxisAngle(left_vector,(Vector3){0.0f,1.0f,0.0f},rotation.x);

    if (IsKeyDown(KEY_UP)) player_position=Vector3Add(player_position,Vector3Scale(forward_vector,movement_speed*GetFrameTime()));
    if (IsKeyDown(KEY_DOWN)) player_position=Vector3Subtract(player_position,Vector3Scale(forward_vector,movement_speed*GetFrameTime()));

    if (IsKeyDown(KEY_LEFT)) player_position=Vector3Add(player_position,Vector3Scale(left_vector,movement_speed*GetFrameTime()));
    if (IsKeyDown(KEY_RIGHT)) player_position=Vector3Subtract(player_position,Vector3Scale(left_vector,movement_speed*GetFrameTime()));     
    
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
        desired_camera_offset=(Vector3){1.0f,2.0f,-1.0f};
        }
    else
        {
        desired_camera_offset=(Vector3){0.0f,2.0f,-0.5f};
        }

    // smooth camera transition between no-aim and aim position
    camera_offset=Vector3Add(camera_offset,
                    Vector3Scale(
                        Vector3Subtract(desired_camera_offset,camera_offset)
                        ,GetFrameTime()*10.0f));

    Vector3 camera_position=camera_offset;
    camera_position=Vector3RotateByAxisAngle(camera_position,(Vector3){0.0f,1.0f,0.0f},rotation.x);
    camera_position=Vector3Add(player_position,camera_position);

    cam1.position=camera_position;
    cam1.target=Vector3Add(camera_position,Vector3RotateByAxisAngle(forward_vector,left_vector,rotation.y));

    BeginDrawing();
    ClearBackground(BLACK);
        BeginMode3D(cam1);
            DrawGrid(10,10);
            // world object
            DrawCube((Vector3){0,4,-10},2,4,2,GREEN);
            // player
            DrawCube(player_position,0.2,4,0.2,RED);
        EndMode3D();
    EndDrawing();
}
CloseWindow();
return 0;
}
