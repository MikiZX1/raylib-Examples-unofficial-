/*******************************************************************************************
*
*   raylib example - generate a sound at runtime
*
*
********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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

    InitWindow(screenWidth, screenHeight, "raylib example - generating sounds at runtime");

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    InitAudioDevice();

    Wave my_wave;
    my_wave.frameCount=10000;
    my_wave.channels=1;
    my_wave.data=malloc(my_wave.frameCount*sizeof(float));
    my_wave.sampleRate=48000;
    my_wave.sampleSize=32;

    float* tmp_pointer=(float*)my_wave.data;
    float volume=1.0;  
    // FILL IN THE SOUND's AUDIO BUFFER WITH SAMPLES
    for (int i=0;i<my_wave.frameCount;i++)
        {
        tmp_pointer[i]=sin((float)i/10.0)*volume;
        volume-=1.0/(float)my_wave.frameCount;
        }

    Sound my_sound=LoadSoundFromWave(my_wave);
    PlaySound(my_sound);

    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(BLACK);
            DrawText("Congrats! You created your first sound!", 190, 200, 20, LIGHTGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadSound(my_sound);
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    return 0;
}
