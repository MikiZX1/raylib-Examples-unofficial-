
// fades render texture content to black over time

#include "raylib.h"
int scrw = 900;
int scrh = 500;

#include "rlgl.h"

#include "external/glad.h"

RenderTexture rendertexture1;
RenderTexture rendertexture2;
int current_rendertexture = 0;

Shader myshader;

const char* myshader_vertsrc="#version 330 core\n"
"in vec3 vertexPosition;\n"
"in vec2 vertexTexCoord;\n"
"in vec4 vertexColor;\n"
"uniform mat4 mvp;\n"
"out vec2 texCoord;\n"
"out vec4 fragColor;\n"
"void main() \n"
"{\n"
"    gl_Position = mvp*vec4(vertexPosition, 1.0);\n"
"    texCoord = vertexTexCoord;\n"
"	 fragColor=vertexColor;\n"
"}\n";

const char* myshader_fragsrc="#version 330 core\n"
"in vec2 texCoord; \n"
"in vec4 fragColor; \n"
"uniform sampler2D texture01; \n"
"uniform vec4 colDiffuse; \n"
"out vec4 finalColor; \n"
"void main() \n"
"{ \n"
"vec4 texelColor = texture(texture01, vec2(texCoord.x,1.0-texCoord.y))-vec4(1.0/255.0,1.0/255.0,1.0/255.0,0.0); \n"
"finalColor = texelColor*colDiffuse*fragColor; \n"
"} \n";


int main(void) {

	InitWindow(scrw, scrh, "Demo");

	rendertexture1=LoadRenderTexture(scrw,scrh);
	rendertexture2=LoadRenderTexture(scrw,scrh);

	myshader=LoadShaderFromMemory(myshader_vertsrc,myshader_fragsrc);

  SetTargetFPS(60);
  while (!WindowShouldClose()) {
    BeginDrawing();

    ClearBackground(BLACK);
	// select which render texture to draw to and draw on it the faded out previous frame
    if (current_rendertexture==0)
        {
        BeginTextureMode(rendertexture1); 
		glClear(GL_DEPTH_BUFFER_BIT);
        BeginShaderMode(myshader);
        DrawTexture(rendertexture2.texture,0,0,WHITE);
        EndShaderMode();
        }
    else
        {
        BeginTextureMode(rendertexture2);
		glClear(GL_DEPTH_BUFFER_BIT);
		BeginShaderMode(myshader);
        DrawTexture(rendertexture1.texture,0,0,WHITE);
        EndShaderMode();
        }
    
	// draw your scene here
    // draw your scene here
	DrawCircle(GetRandomValue(0,scrw),GetRandomValue(0,scrh),20,WHITE);

    EndTextureMode();

	// draw our current rendered frame on the screen
	if (current_rendertexture==0)
        {
        DrawTexture(rendertexture1.texture,0,0,WHITE); 
        }
    else
        {
        DrawTexture(rendertexture2.texture,0,0,WHITE);
        }

    EndDrawing();
    current_rendertexture^=1; // to alternate between 0 and 1 (since we start at value 0)
  }
  UnloadShader(myshader);
  // unload textures
  UnloadRenderTexture(rendertexture1);
  UnloadRenderTexture(rendertexture2);
  CloseWindow();
  return 0;
}