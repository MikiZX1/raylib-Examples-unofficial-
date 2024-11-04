/*******************************************************************************************
*
* Example of using a single depth texture for multiple RenderTextures
* 
*
********************************************************************************************/
#include <stdlib.h>
#include "raylib.h"
#include "rlgl.h"

unsigned int depth_tex_id;

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
        target.depth.id = depth_tex_id;
        target.depth.width = width;
        target.depth.height = height;
        target.depth.format = 19;       //DEPTH_COMPONENT_24BIT?
        target.depth.mipmaps = 1;

        // Attach color texture and depth renderbuffer/texture to FBO
        rlFramebufferAttach(target.id, target.texture.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);

        rlFramebufferAttach(target.id, depth_tex_id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_RENDERBUFFER, 0);

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

    InitWindow(screenWidth, screenHeight, "raylib example");

    depth_tex_id=rlLoadTextureDepth(screenWidth,screenHeight,true);

    RenderTexture2D rendetex=MyLoadRenderTexture(screenWidth,screenHeight);
    RenderTexture2D rendetex2=MyLoadRenderTexture(screenWidth,screenHeight);

    Camera3D camera;
    camera.fovy=90;
    camera.position=(Vector3){0,0,10};
    camera.target=(Vector3){0,0,0};
    camera.up=(Vector3){0,1,0};
    camera.projection=CAMERA_PERSPECTIVE;

    SetTargetFPS(10);
    while (!WindowShouldClose())
        {
        BeginDrawing();
        ClearBackground(BLACK);    
        BeginTextureMode(rendetex);
        ClearBackground(BLACK);    
        BeginMode3D(camera);
        DrawCylinder((Vector3){0,0,3},3,2,5,12,WHITE);
        EndMode3D();
        EndTextureMode();

        BeginTextureMode(rendetex2);
        BeginMode3D(camera);
        DrawSphere((Vector3){0,0,-5},8,GREEN);
        EndMode3D();
        EndTextureMode();

        if (IsKeyDown(KEY_ONE))
        {
        DrawTexturePro(rendetex.texture,
        (Rectangle){0,0,screenWidth,-screenHeight},
        (Rectangle){0,0,screenWidth,screenHeight},
        (Vector2){0.0,0.0},
        0,WHITE);
        }

        BeginBlendMode(BLEND_ALPHA);

        if (IsKeyDown(KEY_TWO))
        {
        DrawTexturePro(rendetex2.texture,
        (Rectangle){0,0,screenWidth,-screenHeight},
        (Rectangle){0,0,screenWidth,screenHeight},
        (Vector2){0.0,0.0},
        0,WHITE);
        }

        DrawText("Hold pressed key 1 to show render texture 1",0,0,20,WHITE);
        DrawText("Hold pressed key 2 to show render texture 2",0,20,20,WHITE);

        EndDrawing();    
        }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    return 0;
}

