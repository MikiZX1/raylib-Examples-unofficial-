/*******************************************************************************************
*
*   raylib drawing non-batched full-screen quad using 
*   a shader that takes 6 textures
*
********************************************************************************************/

#include <stdlib.h>                     // Required for: malloc(), free()
#include <stdio.h>

#include "raylib.h"
#include "rlgl.h"

const char* vs2="#version 330 core              \n"
"layout (location = 0) in vec3 vertexPosition;\n"
"layout (location = 1) in vec2 vertexTexCoord;\n"
"out vec2 texCoord;\n"
"void main() {\n"
"    gl_Position = vec4(vertexPosition, 1.0);\n"
"    texCoord = vertexTexCoord;\n"
"}\n\0";

const char* fs2="#version 330 core              \n"
"out vec4 finalColor;\n"
"in vec2 texCoord;\n"
"uniform sampler2D mytexture0;\n"
"uniform sampler2D mytexture1;\n"
"uniform sampler2D mytexture2;\n"
"uniform sampler2D mytexture3;\n"
"uniform sampler2D mytexture4;\n"
"uniform sampler2D mytexture5;\n"
"uniform float tex_select;\n"
"void main() {\n"
"finalColor = vec4(1.0);\n"
"    if (tex_select==0.0) finalColor = texture(mytexture0, texCoord);\n"
"    if (tex_select==1.0) finalColor = texture(mytexture1, texCoord);\n"
"    if (tex_select==2.0) finalColor = texture(mytexture2, texCoord);\n"
"    if (tex_select==3.0) finalColor = texture(mytexture3, texCoord);\n"
"    if (tex_select==4.0) finalColor = texture(mytexture4, texCoord);\n"
"    if (tex_select==5.0) finalColor = texture(mytexture5, texCoord);\n"
"}\n\0";

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    // -------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib example - 6 textures shader");

    // Load shader
    Shader shader_display = LoadShaderFromMemory(vs2,fs2);

    // used to visualize one selected draw buffer from shader
    int selectTexLoc = GetShaderLocation(shader_display, "tex_select");

    Image tmp_image;
    tmp_image=GenImageColor(screenWidth,screenHeight,RED);
    Texture2D tex_mytexture0=LoadTextureFromImage(tmp_image);
    UnloadImage(tmp_image);
    tmp_image=GenImageColor(screenWidth,screenHeight,GREEN);
    Texture2D tex_mytexture1=LoadTextureFromImage(tmp_image);
    UnloadImage(tmp_image);
    tmp_image=GenImageColor(screenWidth,screenHeight,BLUE);
    Texture2D tex_mytexture2=LoadTextureFromImage(tmp_image);
    UnloadImage(tmp_image);
    tmp_image=GenImageColor(screenWidth,screenHeight,MAGENTA);
    Texture2D tex_mytexture3=LoadTextureFromImage(tmp_image);
    UnloadImage(tmp_image);
    tmp_image=GenImageColor(screenWidth,screenHeight,ORANGE);
    Texture2D tex_mytexture4=LoadTextureFromImage(tmp_image);
    UnloadImage(tmp_image);
    tmp_image=GenImageColor(screenWidth,screenHeight,GRAY);
    Texture2D tex_mytexture5=LoadTextureFromImage(tmp_image);
    UnloadImage(tmp_image);


    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //---------------------------------------------------------------------------------------
    float tex_select=0.0;
    // Main game loop
    while (!WindowShouldClose())
    {
        // Update
        //----------------------------------------------------------------------------------
     
        // Check key inputs to switch between textures
        if (IsKeyPressed(KEY_ONE))      tex_select=0.0;
        if (IsKeyPressed(KEY_TWO))      tex_select=1.0;
        if (IsKeyPressed(KEY_THREE))    tex_select=2.0;
        if (IsKeyPressed(KEY_FOUR))    tex_select=3.0;
        if (IsKeyPressed(KEY_FIVE))    tex_select=4.0;
        if (IsKeyPressed(KEY_SIX))    tex_select=5.0;
        
        // Draw
        // ---------------------------------------------------------------------------------
        BeginDrawing();
        
        rlClearScreenBuffers(); // Clear color & depth buffer

        // setup shader uniforms 
        SetShaderValue(shader_display, selectTexLoc, &tex_select, SHADER_UNIFORM_FLOAT);
        rlEnableShader(shader_display.id);

        // activate OpenGL's texture units
        rlActiveTextureSlot(0);        rlEnableTexture(tex_mytexture0.id);
        rlActiveTextureSlot(1);        rlEnableTexture(tex_mytexture1.id);
        rlActiveTextureSlot(2);        rlEnableTexture(tex_mytexture2.id);
        rlActiveTextureSlot(3);        rlEnableTexture(tex_mytexture3.id);
        rlActiveTextureSlot(4);        rlEnableTexture(tex_mytexture4.id);
        rlActiveTextureSlot(5);        rlEnableTexture(tex_mytexture5.id);

        // link our shader's locs to the correct OpenGL texture unit
        int temp_tex_unit=0;
        rlSetUniform(rlGetLocationUniform(shader_display.id, "mytexture0"), &temp_tex_unit,RL_SHADER_UNIFORM_INT,1);
        temp_tex_unit++;
        rlSetUniform(rlGetLocationUniform(shader_display.id, "mytexture1"), &temp_tex_unit,RL_SHADER_UNIFORM_INT,1);
        temp_tex_unit++;
        rlSetUniform(rlGetLocationUniform(shader_display.id, "mytexture2"), &temp_tex_unit,RL_SHADER_UNIFORM_INT,1);
        temp_tex_unit++;
        rlSetUniform(rlGetLocationUniform(shader_display.id, "mytexture3"), &temp_tex_unit,RL_SHADER_UNIFORM_INT,1);
        temp_tex_unit++;
        rlSetUniform(rlGetLocationUniform(shader_display.id, "mytexture4"), &temp_tex_unit,RL_SHADER_UNIFORM_INT,1);
        temp_tex_unit++;
        rlSetUniform(rlGetLocationUniform(shader_display.id, "mytexture5"), &temp_tex_unit,RL_SHADER_UNIFORM_INT,1);

        rlLoadDrawQuad();
        
        EndShaderMode();

        rlEnableColorBlend();
        DrawText("Show textures (press key): [1][2][3][4][5][6]", 10, 70, 20, DARKGRAY);

        DrawFPS(10, 10);
            
        EndDrawing();
        // -----------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadShader(shader_display);

    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
