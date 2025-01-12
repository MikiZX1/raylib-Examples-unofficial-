/*******************************************************************************************
*
*   raylib geometry shader example - A somewhat BROKEN example (vertex color problem?)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*
********************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include "raylib.h"
#include "raymath.h"
#include "external/glad.h"
#include "rlgl.h"

#define RL_DEFAULT_SHADER_ATTRIB_NAME_POSITION     "vertexPosition"    // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_NAME_POSITION
#define RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD     "vertexTexCoord"    // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD
#define RL_DEFAULT_SHADER_ATTRIB_NAME_NORMAL       "vertexNormal"      // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_NAME_NORMAL
#define RL_DEFAULT_SHADER_ATTRIB_NAME_COLOR        "vertexColor"       // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_NAME_COLOR
#define RL_DEFAULT_SHADER_ATTRIB_NAME_TANGENT      "vertexTangent"     // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_NAME_TANGENT
#define RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2    "vertexTexCoord2"   // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2
#define RL_DEFAULT_SHADER_UNIFORM_NAME_MVP         "mvp"               // model-view-projection matrix
#define RL_DEFAULT_SHADER_UNIFORM_NAME_VIEW        "matView"           // view matrix
#define RL_DEFAULT_SHADER_UNIFORM_NAME_PROJECTION  "matProjection"     // projection matrix
#define RL_DEFAULT_SHADER_UNIFORM_NAME_MODEL       "matModel"          // model matrix
#define RL_DEFAULT_SHADER_UNIFORM_NAME_NORMAL      "matNormal"         // normal matrix (transpose(inverse(matModelView))
#define RL_DEFAULT_SHADER_UNIFORM_NAME_COLOR       "colDiffuse"        // color diffuse (base tint color, multiplied by texture color)
#define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE0  "texture0"          // texture0 (texture slot active 0)
#define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE1  "texture1"          // texture1 (texture slot active 1)
#define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE2  "texture2"          // texture2 (texture slot active 2)

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
const int screenWidth = 800;
const int screenHeight = 450;

const char* vertexShaderSrc=
"#version 150 core\n"
"in vec3 vertexPosition;\n"
"in vec4 vertexColor;\n"
"in vec2 vertexTexCoord;\n"
"in vec3 vertexNormal;\n"
"uniform mat4 mvp;\n"
"uniform vec4 colDiffuse;\n"
"out float vSides;\n"
"out vec3 vColor;\n"
"void main()\n"
"{\n"
"    vColor=vertexColor.rgb;\n"
"    vSides=32.0;\n"
"    gl_Position = mvp*vec4(vertexPosition, 1.0);\n"
"}\n";

const char* geometryShaderSrc=
    "#version 150 core\n"
    "layout(triangles) in;\n"
    "layout(line_strip, max_vertices = 64) out;\n"
    "in vec3 vColor[];\n"
    "in float vSides[];\n"
    "out vec3 fragColor;\n"
    "const float PI = 3.1415926;\n"
    "void main()\n"
    "{\n"
    "    for (int i = 0; i <= vSides[0]; i++) {\n"
    "        float ang = PI * 2.0 / vSides[0] * i;\n"
    "        vec4 offset = vec4(cos(ang) * 0.3, -sin(ang) * 0.4, 0.0, 0.0);\n"
    "        gl_Position = gl_in[0].gl_Position + offset;\n"
    "        fragColor = vColor[0];\n"
    "        EmitVertex();\n"
    "    }\n"
    "    EndPrimitive();\n"
    "}\n";

const char* fragmentShaderSrc=
"#version 150 core\n"
"in vec2 fragTexCoord;\n"
"in vec3 fragColor;\n"
"uniform sampler2D texture0;\n"
"uniform vec4 colDiffuse;\n"
"out vec4 finalColor;\n"
"void main()\n"
"{\n"
"    finalColor = texture2D(texture0,fragTexCoord)*colDiffuse*vec4(fragColor,1.0);\n"
"}\n";


// Compile custom shader and return shader id
unsigned int MyrlCompileShader(const char *shaderCode, int type)
{
    GLuint shader = 0;

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    shader = glCreateShader(type);
    glShaderSource(shader, 1, &shaderCode, NULL);

    GLint success = 0;
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (success == GL_FALSE)
    {
        switch (type)
        {
            case GL_VERTEX_SHADER: printf( "SHADER: [ID %i] Failed to compile vertex shader code", shader); break;
            case GL_FRAGMENT_SHADER: printf( "SHADER: [ID %i] Failed to compile fragment shader code", shader); break;
            case GL_GEOMETRY_SHADER: printf( "SHADER: [ID %i] Failed to compile geometry shader code", shader); break;
            default:                 
                 break;
        }

        int maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        if (maxLength > 0)
        {
            int length = 0;
            char *log = (char *)RL_CALLOC(maxLength, sizeof(char));
            glGetShaderInfoLog(shader, maxLength, &length, log);
            printf( "SHADER: [ID %i] Compile error: %s\n", shader, log);
            RL_FREE(log);
        }

        shader = 0;
    }
    else
    {
        switch (type)
        {
            case GL_VERTEX_SHADER: printf("SHADER: [ID %i] Vertex shader compiled successfully", shader); break;
            case GL_FRAGMENT_SHADER: printf( "SHADER: [ID %i] Fragment shader compiled successfully", shader); break;
            case GL_GEOMETRY_SHADER:printf( "SHADER: [ID %i] Geometry shader compiled successfully", shader); break;
            default: break;
        }
    }
#endif

    return shader;
}


unsigned int MyrlLoadShaderCode(const char *vsCode, const char *gsCode, const char *fsCode)
{
    unsigned int id = 0;

#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
    unsigned int vertexShaderId = 0;
    unsigned int geometryShaderId = 0;
    unsigned int fragmentShaderId = 0;

    if (vsCode != NULL) vertexShaderId = MyrlCompileShader(vsCode, GL_VERTEX_SHADER);
    if (gsCode != NULL) geometryShaderId = MyrlCompileShader(gsCode, GL_GEOMETRY_SHADER);
    if (fsCode != NULL) fragmentShaderId = MyrlCompileShader(fsCode, GL_FRAGMENT_SHADER);

    id = glCreateProgram();
    glAttachShader(id, vertexShaderId);
    glAttachShader(id, geometryShaderId);
    glAttachShader(id, fragmentShaderId);
    glLinkProgram(id);
    glUseProgram(id);

    if (id > 0) glDetachShader(id, vertexShaderId);
    glDeleteShader(vertexShaderId);
    if (id > 0) glDetachShader(id, geometryShaderId);
    glDeleteShader(geometryShaderId);
    if (id > 0) glDetachShader(id, fragmentShaderId);
    glDeleteShader(fragmentShaderId);

    if (id == 0)
       {
       printf( "SHADER: Failed to load custom shader code, using default shader");
       }
    return id;
#endif
}


// Load shader from code strings and bind default locations
Shader MyLoadShaderFromMemory(const char *vsCode, const char *gsCode, const char *fsCode)
{
    Shader shader = { 0 };

    shader.id = MyrlLoadShaderCode(vsCode, gsCode, fsCode);

    // After shader loading, we TRY to set default location names
    if (shader.id > 0)
    {
        shader.locs = (int *)RL_CALLOC(RL_MAX_SHADER_LOCATIONS, sizeof(int));
        for (int i = 0; i < RL_MAX_SHADER_LOCATIONS; i++) shader.locs[i] = -1;

        // Get handles to GLSL input attribute locations
        shader.locs[SHADER_LOC_VERTEX_POSITION] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_POSITION);
        shader.locs[SHADER_LOC_VERTEX_TEXCOORD01] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD);
        shader.locs[SHADER_LOC_VERTEX_TEXCOORD02] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2);
        shader.locs[SHADER_LOC_VERTEX_NORMAL] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_NORMAL);
        shader.locs[SHADER_LOC_VERTEX_TANGENT] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_TANGENT);
        shader.locs[SHADER_LOC_VERTEX_COLOR] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_COLOR);

        // Get handles to GLSL uniform locations (vertex shader)
        shader.locs[SHADER_LOC_MATRIX_MVP] = rlGetLocationUniform(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_MVP);
        shader.locs[SHADER_LOC_MATRIX_VIEW] = rlGetLocationUniform(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_VIEW);
        shader.locs[SHADER_LOC_MATRIX_PROJECTION] = rlGetLocationUniform(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_PROJECTION);
        shader.locs[SHADER_LOC_MATRIX_MODEL] = rlGetLocationUniform(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_MODEL);
        shader.locs[SHADER_LOC_MATRIX_NORMAL] = rlGetLocationUniform(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_NORMAL);

        // Get handles to GLSL uniform locations (fragment shader)
        shader.locs[SHADER_LOC_COLOR_DIFFUSE] = rlGetLocationUniform(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_COLOR);
        shader.locs[SHADER_LOC_MAP_DIFFUSE] = rlGetLocationUniform(shader.id, RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE0);  // SHADER_LOC_MAP_ALBEDO
        shader.locs[SHADER_LOC_MAP_SPECULAR] = rlGetLocationUniform(shader.id, RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE1); // SHADER_LOC_MAP_METALNESS
        shader.locs[SHADER_LOC_MAP_NORMAL] = rlGetLocationUniform(shader.id, RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE2);
    }

    return shader;
}


//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    //SetTraceLogLevel(LOG_WARNING);
    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "raylib example - geometry shaders");


    SetTargetFPS(60);   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    Model testmodel=LoadModel("resources/well.obj");

    Camera3D camera;
    camera.fovy=90;
    camera.position=(Vector3){0,0,-10};
    camera.target=(Vector3){0,0,0};
    camera.up=(Vector3){0,1,0};

    printf("About to compile shader....\n");   
    Shader base;
    base=MyLoadShaderFromMemory(vertexShaderSrc,geometryShaderSrc,fragmentShaderSrc);

    testmodel.materials[0].shader=base;
    float rotation=0;

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
    rotation+=GetFrameTime();
        BeginDrawing();
        ClearBackground(GRAY);
        BeginMode3D(camera);
        testmodel.transform=MatrixMultiply( MatrixMultiply(MatrixIdentity(),MatrixRotateY(rotation)),
                                MatrixTranslate(0,-2,-6));
        DrawModel(testmodel,(Vector3){0,0,5},1.3,WHITE);
        EndMode3D();

        DrawFPS(0,0);

        EndDrawing();
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
