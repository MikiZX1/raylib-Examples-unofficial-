/*******************************************************************************************
*
*  raylib write buffers example (modified raylib's example - deferred rendering)
*
*  ORIGINALLY taken from:
*   raylib [shaders] example - deferred rendering
*
*   NOTE: This example requires raylib OpenGL 3.3 or OpenGL ES 3.0
*
*   Example originally created with raylib 4.5, last time updated with raylib 4.5
*
*   Example contributed by Justin Andreas Lacoste (@27justin) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2023 Justin Andreas Lacoste (@27justin)
*
********************************************************************************************/

#include <stdlib.h>                     // Required for: malloc(), free()
#include <stdio.h>
#include <string.h>                     // Required for: strcmp(), strlen() [Used in rlglInit(), on extensions loading]
#include <math.h>                       // Required for: sqrtf(), sinf(), cosf(), floor(), log()

#include "raylib.h"
#include "rlgl.h"
#include "external/glad.h"

#define GLSL_VERSION            330

#define MAX_CUBES   30

// GBuffer data
typedef struct GBuffer {
    unsigned int framebuffer;

    unsigned int color0_attach;
    unsigned int color1_attach;
    unsigned int color2_attach;
    unsigned int color3_attach;
    unsigned int color4_attach;
    unsigned int color5_attach;
    
    unsigned int depthRenderbuffer;
} GBuffer;

const char* vs1="#version 330 core              \n"
"layout (location = 0) in vec3 vertexPosition;   \n"
"uniform mat4 matModel;                          \n"
"uniform mat4 matView;                           \n"
"uniform mat4 matProjection;                     \n"
"void main()                                     \n"
"{                                               \n"
"    vec4 worldPos = matModel * vec4(vertexPosition, 1.0);   \n"
"    gl_Position = matProjection * matView * worldPos;       \n"
"}                                               \n"
"\0";

const char* fs1="#version 330 core              \n"
"layout (location = 0) out vec4 color0;\n"
"layout (location = 1) out vec4 color1;\n"
"layout (location = 2) out vec4 color2;\n"
"layout (location = 3) out vec4 color3;\n"
"layout (location = 4) out vec4 color4;\n"
"layout (location = 5) out vec4 color5;\n"

"void main() {\n"
"    color0=vec4(0.5,0.5,0.5,1.0);\n"
"    color1=vec4(1.0,0.0,0.0,1.0);\n"
"    color2=vec4(0.0,1.0,0.0,1.0);\n"
"    color3=vec4(0.0,0.0,1.0,1.0);\n"
"    color4=vec4(0.0,0.5,1.0,1.0);\n"
"    color5=vec4(0.0,0.0,0.0,1.0);\n"
"}\n\0";

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
"uniform sampler2D color0Tex;\n"
"uniform sampler2D color1Tex;\n"
"uniform sampler2D color2Tex;\n"
"uniform sampler2D color3Tex;\n"
"uniform sampler2D color4Tex;\n"
"uniform sampler2D color5Tex;\n"
"uniform float tex_select;\n"
"void main() {\n"
"finalColor = vec4(1.0);\n"
"    if (tex_select==0.0) finalColor = texture(color0Tex, texCoord);\n"
"    if (tex_select==1.0) finalColor = texture(color1Tex, texCoord);\n"
"    if (tex_select==2.0) finalColor = texture(color2Tex, texCoord);\n"
"    if (tex_select==3.0) finalColor = texture(color3Tex, texCoord);\n"
"    if (tex_select==4.0) finalColor = texture(color4Tex, texCoord);\n"
"    if (tex_select==5.0) finalColor = texture(color5Tex, texCoord);\n"
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

    InitWindow(screenWidth, screenHeight, "raylib [shaders] example - deferred render");

    Camera camera = { 0 };
    camera.position = (Vector3){ 5.0f, 4.0f, 5.0f };    // Camera position
    camera.target = (Vector3){ 0.0f, 1.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 60.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

    Model cube = LoadModelFromMesh(GenMeshCube(2.0f, 2.0f, 2.0f));

    // Load geometry buffer (G-buffer) shader and deferred shader
    Shader shader_drawing = LoadShaderFromMemory(vs1,fs1);    
    Shader shader_display = LoadShaderFromMemory(vs2,fs2);

    // used to visualize one selected draw buffer from shader
    int selectTexLoc = GetShaderLocation(shader_display, "tex_select");

    // Initialize the G-buffer
    GBuffer gBuffer = { 0 };
    gBuffer.framebuffer = rlLoadFramebuffer();

    if (!gBuffer.framebuffer)
    {
        TraceLog(LOG_WARNING, "Failed to create framebuffer");
        exit(1);
    }
    
    rlEnableFramebuffer(gBuffer.framebuffer);

    gBuffer.color0_attach = rlLoadTexture(NULL, screenWidth, screenHeight, RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);
    gBuffer.color1_attach = rlLoadTexture(NULL, screenWidth, screenHeight, RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);
    gBuffer.color2_attach = rlLoadTexture(NULL, screenWidth, screenHeight, RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);
    gBuffer.color3_attach = rlLoadTexture(NULL, screenWidth, screenHeight, RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);
    gBuffer.color4_attach = rlLoadTexture(NULL, screenWidth, screenHeight, RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);
    gBuffer.color5_attach = rlLoadTexture(NULL, screenWidth, screenHeight, RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);

    GLint maxDrawBuffers = 0;
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);
    printf("GL_MAX_DRAW_BUFFERS: %d",maxDrawBuffers);

    // Activate the draw buffers for our framebuffer
    rlActiveDrawBuffers(6);

    // Now we attach our textures to the framebuffer.
    rlFramebufferAttach(gBuffer.framebuffer, gBuffer.color0_attach, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);
    rlFramebufferAttach(gBuffer.framebuffer, gBuffer.color1_attach, RL_ATTACHMENT_COLOR_CHANNEL1, RL_ATTACHMENT_TEXTURE2D, 0);
    rlFramebufferAttach(gBuffer.framebuffer, gBuffer.color2_attach, RL_ATTACHMENT_COLOR_CHANNEL2, RL_ATTACHMENT_TEXTURE2D, 0);
    rlFramebufferAttach(gBuffer.framebuffer, gBuffer.color3_attach, RL_ATTACHMENT_COLOR_CHANNEL3, RL_ATTACHMENT_TEXTURE2D, 0);
    rlFramebufferAttach(gBuffer.framebuffer, gBuffer.color4_attach, RL_ATTACHMENT_COLOR_CHANNEL4, RL_ATTACHMENT_TEXTURE2D, 0);
    rlFramebufferAttach(gBuffer.framebuffer, gBuffer.color5_attach, RL_ATTACHMENT_COLOR_CHANNEL5, RL_ATTACHMENT_TEXTURE2D, 0);

    // Finally we attach the depth buffer.
    gBuffer.depthRenderbuffer = rlLoadTextureDepth(screenWidth, screenHeight, true);
    rlFramebufferAttach(gBuffer.framebuffer, gBuffer.depthRenderbuffer, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_RENDERBUFFER, 0);

    // Make sure our framebuffer is complete.
    // NOTE: rlFramebufferComplete() automatically unbinds the framebuffer, so we don't have
    // to rlDisableFramebuffer() here.
    if (!rlFramebufferComplete(gBuffer.framebuffer))
    {
        TraceLog(LOG_WARNING, "Framebuffer is not complete");
        exit(1);
    }

//    printf("************* %d\n",RL_DEFAULT_BATCH_MAX_TEXTURE_UNITS);

    // Assign shader to model
    cube.materials[0].shader = shader_drawing;

    const float CUBE_SCALE = 0.25;
    Vector3 cubePositions[MAX_CUBES] = { 0 };
    float cubeRotations[MAX_CUBES] = { 0 };
    
    for (int i = 0; i < MAX_CUBES; i++)
    {
        cubePositions[i] = (Vector3){
            .x = (float)(rand()%10) - 5,
            .y = (float)(rand()%5),
            .z = (float)(rand()%10) - 5,
        };
        
        cubeRotations[i] = (float)(rand()%360);
    }

    rlEnableDepthTest();

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //---------------------------------------------------------------------------------------
    float tex_select=0.0;
    // Main game loop
    while (!WindowShouldClose())
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera, CAMERA_ORBITAL);
     
        // Check key inputs to switch between G-buffer textures
        if (IsKeyPressed(KEY_ONE))      tex_select=0.0;
        if (IsKeyPressed(KEY_TWO))      tex_select=1.0;
        if (IsKeyPressed(KEY_THREE))    tex_select=2.0;
        if (IsKeyPressed(KEY_FOUR))    tex_select=3.0;
        if (IsKeyPressed(KEY_FIVE))    tex_select=4.0;
        if (IsKeyPressed(KEY_SIX))    tex_select=5.0;
        
        // Draw
        // ---------------------------------------------------------------------------------
        BeginDrawing();
        
            ClearBackground(RAYWHITE);
        
            // Draw to the geometry buffer by first activating it
            rlEnableFramebuffer(gBuffer.framebuffer);
            rlClearScreenBuffers();  // Clear color and depth buffer
            
            BeginMode3D(camera);
                // NOTE: We have to use rlEnableShader here. `BeginShaderMode` or thus `rlSetShader`
                // will not work, as they won't immediately load the shader program.
                rlEnableShader(shader_drawing.id);

                    // When drawing a model here, make sure that the material's shaders
                    // are set to the gbuffer shader!
                    for (int i = 0; i < MAX_CUBES; i++)
                    {
                        Vector3 position = cubePositions[i];
                        DrawModelEx(cube, position, (Vector3) { 1, 1, 1 }, cubeRotations[i], (Vector3) { CUBE_SCALE, CUBE_SCALE, CUBE_SCALE }, WHITE);
                    }

                rlDisableShader();
            EndMode3D();

            // Go back to the default framebuffer (0) and draw our deferred shading.
            rlDisableFramebuffer();
            rlClearScreenBuffers(); // Clear color & depth buffer

        // setup shader uniforms 
        SetShaderValue(shader_display, selectTexLoc, &tex_select, SHADER_UNIFORM_FLOAT);
        rlEnableShader(shader_display.id);

        // activate OpenGL's texture units
        rlActiveTextureSlot(0);        rlEnableTexture(gBuffer.color0_attach);
        rlActiveTextureSlot(1);        rlEnableTexture(gBuffer.color1_attach);
        rlActiveTextureSlot(2);        rlEnableTexture(gBuffer.color2_attach);
        rlActiveTextureSlot(3);        rlEnableTexture(gBuffer.color3_attach);
        rlActiveTextureSlot(4);        rlEnableTexture(gBuffer.color4_attach);
        rlActiveTextureSlot(5);        rlEnableTexture(gBuffer.color5_attach);

        // link our shader's locs to the correct OpenGL texture unit
        glUniform1i(rlGetLocationUniform(shader_display.id, "color0Tex"), 0);
        glUniform1i(rlGetLocationUniform(shader_display.id, "color1Tex"), 1);
        glUniform1i(rlGetLocationUniform(shader_display.id, "color2Tex"), 2);
        glUniform1i(rlGetLocationUniform(shader_display.id, "color3Tex"), 3);
        glUniform1i(rlGetLocationUniform(shader_display.id, "color4Tex"), 4);
        glUniform1i(rlGetLocationUniform(shader_display.id, "color5Tex"), 5);

        rlLoadDrawQuad();
        EndShaderMode();

        rlEnableColorBlend();
        DrawText("Show attachment textures (press key): [0][1][2][3][4][5]", 10, 70, 20, DARKGRAY);

        DrawFPS(10, 10);
            
        EndDrawing();
        // -----------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadModel(cube);

    UnloadShader(shader_drawing); // Unload shaders
    UnloadShader(shader_display);

    // Unload geometry buffer and all attached textures
    rlUnloadFramebuffer(gBuffer.framebuffer);
    rlUnloadTexture(gBuffer.depthRenderbuffer);
    rlUnloadTexture(gBuffer.color0_attach);
    rlUnloadTexture(gBuffer.color1_attach);
    rlUnloadTexture(gBuffer.color2_attach);
    rlUnloadTexture(gBuffer.color3_attach);
    rlUnloadTexture(gBuffer.color4_attach);
    rlUnloadTexture(gBuffer.color5_attach);

    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}