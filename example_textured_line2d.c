/*******************************************************************************************
*   Draws a textured 'line' between two points on the screen
*   raylib example
********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>         // Required for: NULL
#include "raylib.h"
#include "math.h"
#include "raymath.h"

Texture2D   myTexture;
float       myTexture_size;

void DrawTexturedLine(Vector2 from, Vector2 to, 
        Texture line_texture, 
        float horizontal_scale, float line_height, Color color)
{
  double angle=atan2(from.y-to.y,from.x-to.x);
  float linelen=Vector2Distance(from,to);
  DrawTexturePro(line_texture,
    (Rectangle){0,0,linelen/horizontal_scale,myTexture_size},
    (Rectangle){from.x,from.y,linelen,line_height},
    (Vector2){0.0,line_height/2},angle*RAD2DEG,color);
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    // -------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 550;

    InitWindow(800, 600, "raylib example textured 'line'");

    // Generate the 'railroad tracks' pattern
    myTexture_size=128;
    Image tmp_image=GenImageColor(myTexture_size,myTexture_size,(Color){0,0,0,0});
    ImageClearBackground(&tmp_image,BLACK);
    for (int i=16;i<128;i+=32)
      {
        ImageDrawLineEx(&tmp_image,(Vector2){i,0},(Vector2){i,128},16,BROWN);
      }
    ImageDrawLineEx(&tmp_image,(Vector2){0,20},(Vector2){128,20},10,GRAY);
    ImageDrawLineEx(&tmp_image,(Vector2){0,100},(Vector2){128,100},10,GRAY);
    myTexture=LoadTextureFromImage(tmp_image);

    float ypos=20.0;
    int targetFPS = GetMonitorRefreshRate(GetCurrentMonitor());  
    SetTargetFPS(targetFPS);                   // Set our game to run at 60 frames-per-second
    while (!WindowShouldClose())
        {
        BeginDrawing();
        ClearBackground(BLACK);    
        DrawTexturedLine((Vector2){300,300}, 
                (Vector2){300+cos(ypos)*ypos*30.0,300+sin(ypos)*ypos*30.0},
                myTexture,0.4,32.0, WHITE);
        DrawFPS(10, 10);
        EndDrawing();    
        ypos+=GetFrameTime()*1.0;
        }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    return 0;
}
