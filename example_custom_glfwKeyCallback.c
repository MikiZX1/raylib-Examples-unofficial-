/*******************************************************************************************
*
*   raylib example - Custom glfw Keyboard callback
*
*
********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "raylib.h"
#include <external/glfw/include/GLFW/glfw3.h>

static void MyKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key < 0) return;    // Security check, macOS fn key generates -1

    if (action == GLFW_RELEASE) printf("Released: ");
    else if(action == GLFW_PRESS) printf("Pressed: ");
    else if(action == GLFW_REPEAT) printf("Held: ");

    // Check the exit key to set close window
    if ((scancode == 9) && (action == GLFW_PRESS)) glfwSetWindowShouldClose(glfwGetCurrentContext(), GLFW_TRUE);

    printf("Key: %d     Scancode: %d     Key's Name:%s\n",key,scancode,
        glfwGetKeyName(key,scancode));

//    printf("Key: %d = Scancode: %d",key,glfwGetKeyScancode(key));
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib example");

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    glfwSetKeyCallback(glfwGetCurrentContext(), MyKeyCallback);
    
    // Main game loop  
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

            DrawText("Press any key ... output is shown in the console...", 190, 200, 20, LIGHTGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    return 0;
}