/*******************************************************************************************
*   raylib [shaders] example - deferred rendering
*   Example complexity rating: [★★★★] 4/4
*   NOTE: This example requires raylib OpenGL 3.3 or OpenGL ES 3.0
*   Example originally created with raylib 4.5, last time updated for raylib 5.6
*   Example contributed by Justin Andreas Lacoste (@27justin) and reviewed by Ramon Santamaria (@raysan5)
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*   Copyright (c) 2023 Justin Andreas Lacoste (@27justin)

*   When compiling your raylib make sure to compile raylib for GLES3
*   COMPILE this example FOR WEB AS USUAL BUT ADD: -sFULL_ES3=1 flag to the emcc command

*
********************************************************************************************/
#if !defined(PLATFORM_DESKTOP)
    #define GRAPHICS_API_OPENGL_ES3
#endif

#include <stdlib.h>         // Required for: NULL

#ifdef PLATFORM_WEB
    #include <GLES3/gl3.h>
#else
    #include <external/glad.h>
#endif

#include "raylib.h"

#include "rlgl.h"
#include "raymath.h"

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

const char* gbufferShader_vs="#version 300 es\n"
"precision highp float;\n"
"in vec3 vertexPosition;\n"
"in vec2 vertexTexCoord;\n"
"in vec3 vertexNormal;\n"
"in vec4 vertexColor;\n"
"out vec3 fragPosition;\n"
"out vec2 fragTexCoord;\n"
"out vec3 fragNormal;\n"
"out vec4 fragColor;\n"
"uniform mat4 matModel;\n"
"uniform mat4 matView;\n"
"uniform mat4 matProjection;\n"
"void main()\n"
"{\n"
"    vec4 worldPos = matModel * vec4(vertexPosition, 1.0);\n"
"    fragPosition = worldPos.xyz; \n"
"    fragTexCoord = vertexTexCoord;\n"
"    fragColor = vertexColor;\n"
"    mat3 normalMatrix = transpose(inverse(mat3(matModel)));\n"
"    fragNormal = normalMatrix * vertexNormal;\n"
"    gl_Position = matProjection * matView * worldPos;\n"
"}\n";

const char* gbufferShader_fs="#version 300 es\n"
"precision highp float;\n"
"layout (location = 0) out vec4 gPosition;\n"
"layout (location = 1) out vec4 gNormal;\n"
"layout (location = 2) out vec4 gAlbedoSpec;\n"
"in vec3 fragPosition;\n"
"in vec2 fragTexCoord;\n"
"in vec3 fragNormal;\n"
"in vec4 fragColor;\n"
"uniform vec4 colDiffuse;\n"
"uniform sampler2D texture0;\n"
"void main() {\n"
"    gPosition = vec4(fragPosition,1.0);\n"
"    gNormal = vec4(normalize(fragNormal),1.0);\n"
"    gAlbedoSpec.rgb = texture(texture0, fragTexCoord).rgb * colDiffuse.rgb;\n"
"    gAlbedoSpec.a = texture(texture0, fragTexCoord).a;\n"
"}\n";

const char* deferredShader_vs="#version 300 es\n"
"precision highp float;\n"
"in vec3 vertexPosition;\n"
"in vec2 vertexTexCoord;\n"
"out vec2 texCoord;\n"
"void main() \n"
"{\n"
"    gl_Position = vec4(vertexPosition, 1.0);\n"
"    texCoord = vertexTexCoord;\n"
"}\n";

const char* deferredShader_fs="#version 300 es\n"
"precision highp float;\n"
"out vec4 finalColor;\n"
"in vec2 texCoord;\n"
"uniform sampler2D gPosition;\n"
"uniform sampler2D gNormal;\n"
"uniform sampler2D gAlbedoSpec;\n"
"struct Light {\n"
"    int enabled;\n"
"    int type; \n"
"    vec3 position;\n"
"    vec3 target; \n"
"    vec4 color;\n"
"};\n"
"const int NR_LIGHTS = 4;\n"
"uniform Light lights[NR_LIGHTS];\n"
"uniform vec3 viewPosition;\n"
"const float QUADRATIC = 0.35;\n"
"const float LINEAR = 0.15;\n"
"void main() {\n"
"    vec3 fragPosition = texture(gPosition, texCoord).rgb;\n"
"    vec3 normal = texture(gNormal, texCoord).rgb;\n"
"    vec3 albedo = texture(gAlbedoSpec, texCoord).rgb;\n"
"    float specular = texture(gAlbedoSpec, texCoord).a;\n"
"    vec3 ambient = albedo * vec3(0.03f);\n"
"    vec3 viewDirection = normalize(viewPosition - fragPosition);\n"
"    for(int i = 0; i < NR_LIGHTS; ++i)\n"
"    {\n"
"        if(lights[i].enabled == 0) continue;\n"
"        vec3 lightDirection = lights[i].position - fragPosition;\n"
"        vec3 diffuse = max(dot(normal, lightDirection), 0.0) * albedo * lights[i].color.xyz;\n"
"        vec3 halfwayDirection = normalize(lightDirection + viewDirection);\n"
"        float spec = pow(max(dot(normal, halfwayDirection), 0.0), 32.0);\n"
"        vec3 specular = vec3(0.1,0.1,0.1) + specular * spec * lights[i].color.xyz;\n"
"        float distance = length(lights[i].position - fragPosition);\n"
"        float attenuation = 1.0 / (1.0 + LINEAR * distance + QUADRATIC * distance * distance);\n"
"        diffuse *= attenuation;\n"
"        specular *= attenuation*attenuation;\n"
"        ambient += diffuse + specular;\n"
"    }\n"
"    finalColor = vec4(ambient, 1.0);\n"
"}\n";

#define MAX_SPHERES   10

// GBuffer data
typedef struct GBuffer {
    unsigned int framebuffer;

    unsigned int positionTexture;
    unsigned int normalTexture;
    unsigned int albedoSpecTexture;
    
    unsigned int depthRenderbuffer;
} GBuffer;

// Deferred mode passes
typedef enum {
   DEFERRED_POSITION,
   DEFERRED_NORMAL,
   DEFERRED_ALBEDO,
   DEFERRED_SHADING
} DeferredMode;

// Load depth texture/renderbuffer (to be attached to fbo)
unsigned int custom_LoadRenderbufferDepth(int width, int height)
{
    unsigned int id = 0;
    // GL_DEPTH24_STENCIL8 is the default for GLFW OpenGL context and raylib uses it, you might need to change this for 
    // different raylib's backends like SDL, RGFW, ...
    // Possible formats: GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT32F and GL_DEPTH32F_STENCIL8 (the list might vary depending on the platform the code is run on)
    unsigned int glInternalFormat = GL_DEPTH24_STENCIL8;
    glGenRenderbuffers(1, &id);
    glBindRenderbuffer(GL_RENDERBUFFER, id);
    glRenderbufferStorage(GL_RENDERBUFFER, glInternalFormat, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    return id;
}

unsigned int custom_LoadTexture(int w,int h, unsigned int opengl_format)
{
unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, opengl_format, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  
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
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [shaders] example - deferred render");

    Camera camera = { 0 };
    camera.position = (Vector3){ 5.0f, 4.0f, 5.0f };    // Camera position
    camera.target = (Vector3){ 0.0f, 1.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 60.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

    // Load plane model from a generated mesh
    Model model = LoadModelFromMesh(GenMeshPlane(10.0f, 10.0f, 3, 3));
    Model sphere = LoadModelFromMesh(GenMeshSphere(1.0f, 10.0f, 10.0f));

    // Load geometry buffer (G-buffer) shader and deferred shader
    Shader gbufferShader = LoadShaderFromMemory(gbufferShader_vs, gbufferShader_fs);
    Shader deferredShader = LoadShaderFromMemory(deferredShader_vs, deferredShader_fs);

    deferredShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(deferredShader, "viewPosition");

    // Initialize the G-buffer
    GBuffer gBuffer = { 0 };
    gBuffer.framebuffer = rlLoadFramebuffer();

    if (!gBuffer.framebuffer)
    {
        TraceLog(LOG_WARNING, "Failed to create framebuffer");
        exit(1);
    }
    
    rlEnableFramebuffer(gBuffer.framebuffer);

    // NOTE: Vertex positions are stored in a texture for simplicity. A better approach would use a depth texture
    // (instead of a depth renderbuffer) to reconstruct world positions in the final render shader via clip-space position, 
    // depth, and the inverse view/projection matrices.

    // 32-bit precision used on desktop as well as for OpenGL ES 3. 
    #ifdef PLATFORM_WEB
    gBuffer.positionTexture = custom_LoadTexture(screenWidth,screenHeight,GL_RGBA32F);
    #else
    // But as mentioned above, the positions could be reconstructed instead of stored. 
    gBuffer.positionTexture = rlLoadTexture(NULL, screenWidth, screenHeight, RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16, 1);
    #endif

    // Similarly, 32-bit precision is used for normals on desktop as well as OpenGL ES 3.
    #ifdef PLATFORM_WEB
    gBuffer.normalTexture = custom_LoadTexture(screenWidth,screenHeight,GL_RGBA32F);
    #else
    gBuffer.normalTexture = rlLoadTexture(NULL, screenWidth, screenHeight, RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16, 1);
    #endif

    // Albedo (diffuse color) and specular strength can be combined into one texture.
    // The color in RGB, and the specular strength in the alpha channel.
    gBuffer.albedoSpecTexture = rlLoadTexture(NULL, screenWidth, screenHeight, RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);

    // Activate the draw buffers for our framebuffer
    rlActiveDrawBuffers(3);

    // Now we attach our textures to the framebuffer.
    rlFramebufferAttach(gBuffer.framebuffer, gBuffer.positionTexture, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);
    rlFramebufferAttach(gBuffer.framebuffer, gBuffer.normalTexture, RL_ATTACHMENT_COLOR_CHANNEL1, RL_ATTACHMENT_TEXTURE2D, 0);
    rlFramebufferAttach(gBuffer.framebuffer, gBuffer.albedoSpecTexture, RL_ATTACHMENT_COLOR_CHANNEL2, RL_ATTACHMENT_TEXTURE2D, 0);

    // Finally we attach the depth buffer.
    gBuffer.depthRenderbuffer = custom_LoadRenderbufferDepth(screenWidth, screenHeight);
    rlFramebufferAttach(gBuffer.framebuffer, gBuffer.depthRenderbuffer, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_RENDERBUFFER, 0);

    // Make sure our framebuffer is complete.
    // NOTE: rlFramebufferComplete() automatically unbinds the framebuffer, so we don't have
    // to rlDisableFramebuffer() here.
    if (!rlFramebufferComplete(gBuffer.framebuffer))
    {
        TraceLog(LOG_WARNING, "Framebuffer is not complete");
        exit(1);
    }

    // Now we initialize the sampler2D uniform's in the deferred shader.
    // We do this by setting the uniform's values to the texture units that
    // we later bind our g-buffer textures to.
    rlEnableShader(deferredShader.id);
    int texUnitPosition = 0;
    int texUnitNormal = 1;
    int texUnitAlbedoSpec = 2;
    SetShaderValue(deferredShader, rlGetLocationUniform(deferredShader.id, "gPosition"), &texUnitPosition, RL_SHADER_UNIFORM_SAMPLER2D);
    SetShaderValue(deferredShader, rlGetLocationUniform(deferredShader.id, "gNormal"), &texUnitNormal, RL_SHADER_UNIFORM_SAMPLER2D);
    SetShaderValue(deferredShader, rlGetLocationUniform(deferredShader.id, "gAlbedoSpec"), &texUnitAlbedoSpec, RL_SHADER_UNIFORM_SAMPLER2D);
    rlDisableShader();

    // Assign out lighting shader to model
    model.materials[0].shader = gbufferShader;
    sphere.materials[0].shader = gbufferShader;
    
    // add some specular detail to the model material
    Image tmp_img2=GenImageChecked(256,256,32,32,BLACK,LIGHTGRAY);
    unsigned char* pixel=tmp_img2.data;
    for (int i=0;i<256*256;i++)
        if (pixel[i*4]<128)
            pixel[i*4+3]=255;
        else
            pixel[i*4+3]=10;

    Texture texture_albedo_specular=LoadTextureFromImage(tmp_img2);
    UnloadImage(tmp_img2);
    model.materials[0].maps[MATERIAL_MAP_ALBEDO].texture=texture_albedo_specular;



    // Create lights
    //--------------------------------------------------------------------------------------
    Light lights[MAX_LIGHTS] = { 0 };
    lights[0] = CreateLight(LIGHT_POINT, (Vector3){ -2, 1, -2 }, Vector3Zero(), YELLOW, deferredShader);
    lights[1] = CreateLight(LIGHT_POINT, (Vector3){ 2, 1, 2 }, Vector3Zero(), RED, deferredShader);
    lights[2] = CreateLight(LIGHT_POINT, (Vector3){ -2, 1, 2 }, Vector3Zero(), GREEN, deferredShader);
    lights[3] = CreateLight(LIGHT_POINT, (Vector3){ 2, 1, -2 }, Vector3Zero(), BLUE, deferredShader);

    const float SPHERE_SCALE = 0.5;
    Vector3 spherePositions[MAX_SPHERES] = { 0 };
    float sphereRotations[MAX_SPHERES] = { 0 };
    Color sphereColors[MAX_SPHERES] = { 0 };
    
    for (int i = 0; i < MAX_SPHERES; i++)
    {
        spherePositions[i] = (Vector3){
            .x = (float)(rand()%10) - 5,
            .y = (float)(rand()%5),
            .z = (float)(rand()%10) - 5,
        };
        
        sphereRotations[i] = (float)(rand()%360);

        sphereColors[i] = (Color){GetRandomValue(0,128),GetRandomValue(0,128),GetRandomValue(0,128),255};
        
    }

    float time=0;

    DeferredMode mode = DEFERRED_SHADING;

    rlEnableDepthTest();

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //---------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())
    {
        // Update
        //----------------------------------------------------------------------------------
        time-=GetFrameTime()/1.5;

        UpdateCamera(&camera, CAMERA_ORBITAL);

        // Update the shader with the camera view vector (points towards { 0.0f, 0.0f, 0.0f })
        float cameraPos[3] = { camera.position.x, camera.position.y, camera.position.z };
        SetShaderValue(deferredShader, deferredShader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);
        
        // Check key inputs to enable/disable lights
        if (IsKeyPressed(KEY_Y)) { lights[0].enabled = !lights[0].enabled; }
        if (IsKeyPressed(KEY_R)) { lights[1].enabled = !lights[1].enabled; }
        if (IsKeyPressed(KEY_G)) { lights[2].enabled = !lights[2].enabled; }
        if (IsKeyPressed(KEY_B)) { lights[3].enabled = !lights[3].enabled; }

        // Check key inputs to switch between G-buffer textures
        if (IsKeyPressed(KEY_ONE)) mode = DEFERRED_POSITION;
        if (IsKeyPressed(KEY_TWO)) mode = DEFERRED_NORMAL;
        if (IsKeyPressed(KEY_THREE)) mode = DEFERRED_ALBEDO;
        if (IsKeyPressed(KEY_FOUR)) mode = DEFERRED_SHADING;

        // Update light values (actually, only enable/disable them)
        for (int i = 0; i < MAX_LIGHTS; i++) UpdateLightValues(deferredShader, lights[i]);
        //----------------------------------------------------------------------------------

        // Draw
        // ---------------------------------------------------------------------------------
        BeginDrawing();

            // Draw to the geometry buffer by first activating it
            rlEnableFramebuffer(gBuffer.framebuffer);
            rlClearColor(0, 0, 0, 0);
            rlClearScreenBuffers();  // Clear color and depth buffer
            
            rlDisableColorBlend();
            BeginMode3D(camera);
                // NOTE: We have to use rlEnableShader here. `BeginShaderMode` or thus `rlSetShader`
                // will not work, as they won't immediately load the shader program.
                rlEnableShader(gbufferShader.id);
                    // When drawing a model here, make sure that the material's shaders
                    // are set to the gbuffer shader!
                    DrawModel(model, Vector3Zero(), 1.0f, WHITE);
                    DrawModel(sphere, (Vector3) { 0.0, 1.0f, 0.0 }, 1.0f, WHITE);

                    for (int i = 0; i < MAX_SPHERES; i++)
                    {
                        Vector3 position = spherePositions[i];
                        DrawModelEx(sphere, position, (Vector3) { 1, 1, 1 }, sphereRotations[i], (Vector3) { SPHERE_SCALE, SPHERE_SCALE, SPHERE_SCALE }, sphereColors[i]);
                    }

                rlDisableShader();
            EndMode3D();
            rlEnableColorBlend();

            // Go back to the default framebuffer (0) and draw our deferred shading.
            rlDisableFramebuffer();
            rlClearScreenBuffers(); // Clear color & depth buffer

            switch (mode)
            {
                case DEFERRED_SHADING:
                {
                    BeginMode3D(camera);
                        rlDisableColorBlend();
                        rlEnableShader(deferredShader.id);
                            // Bind our g-buffer textures
                            // We are binding them to locations that we earlier set in sampler2D uniforms `gPosition`, `gNormal`,
                            // and `gAlbedoSpec`
                            rlActiveTextureSlot(texUnitPosition);
                            rlEnableTexture(gBuffer.positionTexture);
                            rlActiveTextureSlot(texUnitNormal);
                            rlEnableTexture(gBuffer.normalTexture);
                            rlActiveTextureSlot(texUnitAlbedoSpec);
                            rlEnableTexture(gBuffer.albedoSpecTexture);

                            // Finally, we draw a fullscreen quad to our default framebuffer
                            // This will now be shaded using our deferred shader
                            rlLoadDrawQuad();
                        rlDisableShader();
                        rlEnableColorBlend();
                    EndMode3D();

                    // As a last step, we now copy over the depth buffer from our g-buffer to the default framebuffer.
                    // This step is only needed if you plan to continue drawing in the deffer-rendered scene
                    rlBindFramebuffer(RL_READ_FRAMEBUFFER, gBuffer.framebuffer);
                    rlBindFramebuffer(RL_DRAW_FRAMEBUFFER, 0);
                    rlBlitFramebuffer(0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, 0x00000100);    // GL_DEPTH_BUFFER_BIT

                    rlDisableFramebuffer();
                    // Since our shader is now done and disabled, we can draw spheres
                    // that represent light positions in default forward rendering
                    BeginMode3D(camera);
                        rlEnableShader(rlGetShaderIdDefault());
                            for (int i = 0; i < MAX_LIGHTS; i++)
                            {
                                lights[i].position.x=cos((i+3*i*time)*i*1.57*0.002+time)*5;
                                lights[i].position.z=sin((i+3*i*time)*i*1.57*0.001+time)*5;
                                lights[i].position.y=Clamp(sin(i*0.77+time*0.1*i)*3,0,3)+0.5;
                                if (lights[i].enabled) DrawSphereEx(lights[i].position, 0.2f, 8, 8, lights[i].color);
                                else DrawSphereWires(lights[i].position, 0.2f, 8, 8, ColorAlpha(lights[i].color, 0.3f));
                            }
                        rlDisableShader();
                    EndMode3D();                   
                    DrawText("FINAL RESULT", 10, screenHeight - 30, 20, DARKGREEN);
                } break;
                case DEFERRED_POSITION:
                {
                    DrawTextureRec((Texture2D){
                        .id = gBuffer.positionTexture,
                        .width = screenWidth,
                        .height = screenHeight,
                    }, (Rectangle) { 0, 0, (float)screenWidth, (float)-screenHeight }, Vector2Zero(), RAYWHITE);
                    
                    DrawText("POSITION TEXTURE", 10, screenHeight - 30, 20, DARKGREEN);
                } break;
                case DEFERRED_NORMAL:
                {
                    DrawTextureRec((Texture2D){
                        .id = gBuffer.normalTexture,
                        .width = screenWidth,
                        .height = screenHeight,
                    }, (Rectangle) { 0, 0, (float)screenWidth, (float)-screenHeight }, Vector2Zero(), RAYWHITE);
                    
                    DrawText("NORMAL TEXTURE", 10, screenHeight - 30, 20, DARKGREEN);
                } break;
                case DEFERRED_ALBEDO:
                {
                    DrawTextureRec((Texture2D){
                        .id = gBuffer.albedoSpecTexture,
                        .width = screenWidth,
                        .height = screenHeight,
                    }, (Rectangle) { 0, 0, (float)screenWidth, (float)-screenHeight }, Vector2Zero(), RAYWHITE);
                    
                    DrawText("ALBEDO TEXTURE", 10, screenHeight - 30, 20, DARKGREEN);
                } break;
                default: break;
            }

            DrawText("Toggle lights keys: [Y][R][G][B]", 10, 40, 20, DARKGRAY);
            DrawText("Switch G-buffer textures: [1][2][3][4]", 10, 70, 20, DARKGRAY);

            DrawFPS(10, 10);
            
        EndDrawing();
        // -----------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadModel(model);     // Unload the models
    UnloadModel(sphere);

    UnloadTexture(texture_albedo_specular);

    UnloadShader(deferredShader); // Unload shaders
    UnloadShader(gbufferShader);

    // Unload geometry buffer and all attached textures
    rlUnloadFramebuffer(gBuffer.framebuffer);
    rlUnloadTexture(gBuffer.positionTexture);
    rlUnloadTexture(gBuffer.normalTexture);
    rlUnloadTexture(gBuffer.albedoSpecTexture);
    rlUnloadTexture(gBuffer.depthRenderbuffer);

    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

