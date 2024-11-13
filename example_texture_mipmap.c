//------------------------------------------------------------------------------------
// raylib example - lock MIPMAP level of a texture
// once the below procedure executed on a texture, all access following
// is done on the set MIPMAP level
//------------------------------------------------------------------------------------


#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"

#include "external/glad.h"


//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib example - lock MIPMAP level of a texture");

    Texture2D parrots;
    parrots=LoadTexture("resources/parrots.png");           //SetTextureFilter(wabbit,TEXTURE_FILTER_ANISOTROPIC_8X);
    GenTextureMipmaps(&parrots);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,parrots.id);
    int mipmap_level=4;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipmap_level);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, mipmap_level);
    glBindTexture(GL_TEXTURE_2D,0);
    glDisable(GL_TEXTURE_2D);

    SetTextureFilter(parrots,TEXTURE_FILTER_ANISOTROPIC_4X);

    SetTargetFPS(30);               // Set our game to run at 30 frames-per-second
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        BeginDrawing();
            ClearBackground(BLACK);
            DrawTexture(parrots,0,0,WHITE);
         EndDrawing();
    }
    CloseWindow();              // Close window and OpenGL context
    return 0;
}