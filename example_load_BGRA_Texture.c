/*******************************************************************************************
*
* Example of loading a BGRA texture
* 
*
********************************************************************************************/
#include <stdlib.h>
#include "raylib.h"
#include "rlgl.h"
#include "external/glad.h"

//------------------------------------------------------------------------------------
// Texture loading functions
//------------------------------------------------------------------------------------
// Load texture from file into GPU memory (VRAM)
Texture2D LoadTextureBGRA(const char *fileName)
{
    Texture2D texture = { 0 };

    Image image = LoadImage(fileName);

    int comp=0;
    if (image.format==PIXELFORMAT_UNCOMPRESSED_R8G8B8)
        comp=3;
    if (image.format==PIXELFORMAT_UNCOMPRESSED_R8G8B8A8)
        comp=4;

    if (comp<3 || comp>4)
        {
        UnloadImage(image);
        TRACELOG(LOG_ERROR, "Format of the image %s not supported.", fileName);
        return texture;    
        }

    if (image.data != NULL)
    {
        unsigned char* pixel=image.data;
        int imgsize=image.width*image.height;
        for (int i=0;i<imgsize;i++)
        {
            unsigned char tmp=pixel[i*comp];
            pixel[i*comp]=pixel[i*comp+2];
            pixel[i*comp+2]=tmp;
        }
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
    }
    return texture;
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

    InitWindow(screenWidth, screenHeight, "raylib example");

    Texture2D rgba_tex=LoadTexture("resources/parrots.png");
    Texture2D bgra_tex=LoadTextureBGRA("resources/parrots.png");

    SetTargetFPS(10);
    while (!WindowShouldClose())
        {
        BeginDrawing();
        ClearBackground(BLACK);    

        if (IsKeyDown(KEY_ONE))
            DrawTexture(rgba_tex,0,0,WHITE);

        if (IsKeyDown(KEY_TWO))
            DrawTexture(bgra_tex,0,0,WHITE);

        DrawText("Hold pressed key 1 to show RGBA texture 1",0,0,20,WHITE);
        DrawText("Hold pressed key 2 to show BGRA texture 2",0,20,20,WHITE);

        EndDrawing();    
        }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    return 0;
}

