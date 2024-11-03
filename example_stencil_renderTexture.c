/*******************************************************************************************
*
* STENCIL Example (using RenderTexture) for raylib
* not tested for cross-platform compatibility (works on Desktop)
* originally taken from https://github.com/raysan5/raylib/discussions/2964
*
********************************************************************************************/
#include <stdlib.h>
#include "raylib.h"
#include "rlgl.h"
#include "external/glad.h"

void BeginStencilMode()
{
	rlDrawRenderBatchActive();
	glEnable(GL_STENCIL_TEST);
}

void ClearStencilMask()
{
    glClearStencil(0);
    glClear( GL_STENCIL_BUFFER_BIT );
}

void BeginStencilMask()
{
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
}

void EndStencilMask()
{
	rlDrawRenderBatchActive();	
	glStencilFunc(GL_EQUAL, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

void EndStencilMode()
{
	rlDrawRenderBatchActive();
	glDisable(GL_STENCIL_TEST);
}

// Load depth texture/renderbuffer (to be attached to fbo)
// WARNING: OpenGL ES 2.0 requires GL_OES_depth_texture and WebGL requires WEBGL_depth_texture extensions
unsigned int MyrlLoadTextureDepth(int width, int height, bool useRenderBuffer)
{
    unsigned int id = 0;
    glGenRenderbuffers(1, &id);
    glBindRenderbuffer(GL_RENDERBUFFER, id);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    TRACELOG(RL_LOG_INFO, "TEXTURE: [ID %i] Depth/Stencil renderbuffer loaded successfully", id);
    return id;
}


RenderTexture2D MyLoadRenderTexture(int width, int height)
{
    RenderTexture2D target = { 0 };

    target.id = rlLoadFramebuffer(); // Load an empty framebuffer

    if (target.id > 0)
    {
        rlEnableFramebuffer(target.id);

        // Create color texture (default to RGBA)
        target.texture.id = rlLoadTexture(NULL, width, height, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);
        target.texture.width = width;
        target.texture.height = height;
        target.texture.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
        target.texture.mipmaps = 1;

        // Create depth renderbuffer/texture
        target.depth.id = MyrlLoadTextureDepth(width, height, true);
        target.depth.width = width;
        target.depth.height = height;
        target.depth.format = 19;       //DEPTH_COMPONENT_24BIT?
        target.depth.mipmaps = 1;

        // Attach color texture and depth renderbuffer/texture to FBO
        rlFramebufferAttach(target.id, target.texture.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, target.id);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, target.depth.id );

        // Check if fbo is complete with attachments (valid)
        if (rlFramebufferComplete(target.id)) TRACELOG(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully", target.id);

        rlDisableFramebuffer();
    }
    else TRACELOG(LOG_WARNING, "FBO: Framebuffer object can not be created");

    return target;
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

    InitWindow(800, 600, "raylib example");

    RenderTexture2D rendetex=MyLoadRenderTexture(screenWidth,screenHeight);

    Image noise=GenImagePerlinNoise(256,256,0,0,100);
    Texture2D tex_noise=LoadTextureFromImage(noise);

    SetTargetFPS(10);
    while (!WindowShouldClose())
        {
        BeginDrawing();
        BeginTextureMode(rendetex);
        ClearBackground(SKYBLUE);    

        DrawRectanglePro((Rectangle){250,100,60,10},(Vector2){0,0},-40,BLACK);
        DrawRectanglePro((Rectangle){250,100,60,10},(Vector2){0,0},-130,BLACK);
        DrawRectangle(100,100,300,200,WHITE);
        DrawRectangleRounded((Rectangle){110,110,260,180},0.5,10,BLACK);
        for (int i=0;i<100;i+=30)
            DrawCircle(385,130+i,10,ORANGE);

        if (IsKeyDown(KEY_SPACE))
        {
        ClearStencilMask();
        BeginStencilMode();      
            BeginStencilMask();  
            /// Draw the actual stencil
            /// tv screen
            DrawRectangleRounded((Rectangle){110,110,260,180},0.5,10,BLACK);

            EndStencilMask();

        /// Draw noise texture using stencil mask
        DrawTexturePro(tex_noise, 
            (Rectangle){0,0,256,256},(Rectangle){200+GetRandomValue(0,20),250,400,400},
            (Vector2){200,200},0,WHITE);

            EndStencilMode();
            }

        /// Draw without the stencil mask
        DrawRectangle(0,300,screenWidth,20,GREEN);

        DrawText("Hold SPACE key to draw using the stencil mask.",10,10,20,BLACK);
        DrawText("Hold SPACE key to draw using the stencil mask.",11,9,20,WHITE);

        EndTextureMode();

        DrawTexturePro(rendetex.texture,
        (Rectangle){0,0,screenWidth,-screenHeight},
        (Rectangle){0,0,screenWidth,screenHeight},
        (Vector2){0.0,0.0},
        0,WHITE);

        EndDrawing();    
        }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    return 0;
}

