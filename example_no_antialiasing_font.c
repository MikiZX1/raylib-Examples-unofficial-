/*******************************************************************************************
*   modified to remove antialiasing from font textures:
*   raylib [text] example - Font loading
*
*   NOTE: raylib can load fonts from multiple input file formats:
*
*     - TTF/OTF > Sprite font atlas is generated on loading, user can configure
*                 some of the generation parameters (size, characters to include)
*     - BMFonts > Angel code font fileformat, sprite font image must be provided
*                 together with the .fnt file, font generation cna not be configured
*     - XNA Spritefont > Sprite font image, following XNA Spritefont conventions,
*                 Characters in image must follow some spacing and order rules
*
*   Example originally created with raylib 1.4, last time updated with raylib 3.0
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2016-2024 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [text] example - font loading");

    // Define characters to draw
    // NOTE: raylib supports UTF-8 encoding, following list is actually codified as UTF8 internally
    const char msg[256] = "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHI\nJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmn\nopqrstuvwxyz{|}~¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓ\nÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷\nøùúûüýþÿ";

    // NOTE: Textures/Fonts MUST be loaded after Window initialization (OpenGL context is required)

    // TTF font : Font data and atlas are generated directly from TTF
    // NOTE: We define a font base size of 32 pixels tall and up-to 250 characters
    Font fontTtf = LoadFontEx("build/external/raylib-master/examples/text/resources/pixantiqua.ttf", 32, 0, 250);

    SetTextLineSpacing(16);         // Set line spacing for multiline text (when line breaks are included '\n')

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    
    //--------------------------------------------------------------------------------------
    char we_dont_want_aliasing=1; // change this to zero to use aliasing

    if (we_dont_want_aliasing)    
    {
    // remove antialiasing from font texture
    Image tmp1=LoadImageFromTexture(fontTtf.texture);
    ImageAlphaClear(&tmp1,(Color){0,0,0,0},0.95);
    UnloadTexture(fontTtf.texture);
    fontTtf.texture=LoadTextureFromImage(tmp1);
    UnloadImage(tmp1);
    SetTextureFilter(fontTtf.texture,TEXTURE_FILTER_POINT);
    }

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(BLACK);

            DrawTextEx(fontTtf, msg, (Vector2){ 20.0f, 100.0f }, (float)fontTtf.baseSize*4, 2, LIME);
            DrawText("Using TTF font generated", 20, GetScreenHeight() - 30, 20, GRAY);

        DrawTexturePro(fontTtf.texture,(Rectangle){0,0,100,100},
        (Rectangle){0,0,600,600},(Vector2){0,0},0,WHITE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadFont(fontTtf);    // TTF Font unloading

    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
