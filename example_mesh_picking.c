/*******************************************************************************************
*   mesh picking / using "query selection" or "color picking"
*   
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "external/glad.h"

typedef struct _EXAMPLE_OBJECT
{
    Mesh mesh;
    Color picking_color;    // color picking color
    Color color;            // drawing color
    Matrix transform;
    char selected;
} EXAMPLE_OBJECT;

EXAMPLE_OBJECT objects[5];

char box_select;
Rectangle box_select_area;

char selection_method; // 0 = OCCLUSION QUERY, 1 = COLOR PICKING

unsigned char colors[4];   // buffer to get colors under the mouse cursor

RenderTexture2D rt;        // our "work" render texture

unsigned int query[1], numSamplesRendered; // OpenGL variables

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [shaders] example - mesh selection");

    rt=LoadRenderTexture(screenWidth, screenHeight);

    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 60.0f, 70.0f };    // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };              // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };                  // Camera up vector (rotation towards target)
    camera.fovy = 50.0f;                                        // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;                     // Camera projection type

    for (int i=0;i<5;i++)
        {
        objects[i].picking_color=(Color){0,0,1+i, 255};
        objects[i].color=(Color){GetRandomValue(0,230),
                            GetRandomValue(0,230),GetRandomValue(0,230),255};
        objects[i].transform=MatrixMultiply(MatrixIdentity(), MatrixTranslate(GetRandomValue(-30,30),
                            GetRandomValue(-30,30),GetRandomValue(-30,30)));
        }

    objects[0].mesh=GenMeshCube(10,10,10);
    objects[1].mesh=GenMeshCone(10,10,2);
    objects[2].mesh=GenMeshCylinder(5,20,4);
    objects[3].mesh=GenMeshSphere(10,10,10);
    objects[4].mesh=GenMeshPlane(10,10,1,1);

    Material material = LoadMaterialDefault();

    box_select=0;

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second

    //--------------------------------------------------------------------------------------
    // Main game loop

    float timed=0;

    glGenQueries(1, query);             // Init OpenGL queries

    while (!WindowShouldClose())        // Detect window close button or ESC key
    {

        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyDown(KEY_ONE)) selection_method=0;
        if (IsKeyDown(KEY_TWO)) selection_method=1;

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && box_select==0)
            {
                    box_select_area.x=GetMousePosition().x;
                    box_select_area.y=GetMousePosition().y;
                    box_select=1;
            }

        // draw the scene to a render texture
        // render texture is used to obtain the picking color of the objects under the mouse cursor
        // as well as to do occlusion queries used to determine which objects are rendered within the box selected area
        BeginTextureMode(rt);
        ClearBackground(BLACK);
        //rlDisableDepthTest();
        //rlDisableDepthMask();
        if ( selection_method == 0)
            {
            // restrict drawing to the selection box if using occlusion queries
            BeginScissorMode(box_select_area.x,box_select_area.y, box_select_area.width, box_select_area.height);
            }
      
        // start drawing the scene
        BeginMode3D(camera);       
        for (int i=0;i<5;i++)
        {
        if ( selection_method == 0)
            {
            // do OpenGL query (test to see how many pixels of this object/mesh got drawn)
            glBeginQuery(GL_SAMPLES_PASSED, query[0]);
            DrawMesh(objects[i].mesh,material,objects[i].transform);
            glEndQuery(GL_SAMPLES_PASSED);
            glGetQueryObjectuiv(query[0], GL_QUERY_RESULT, &numSamplesRendered);
            if (numSamplesRendered != 0) 
                objects[i].selected=1;
                else
                objects[i].selected=0;
            }
        else
            {
            material.maps[MATERIAL_MAP_DIFFUSE].color = objects[i].picking_color;
            DrawMesh(objects[i].mesh,material,objects[i].transform);
            }
        }
        EndMode3D();

        EndScissorMode();
        EndTextureMode();

        if (selection_method==1)
            {
            // do color picking (read back color of pixel, under mouse cursor, from the render texture)
            BeginTextureMode(rt);
            glReadPixels((int)GetMousePosition().x,screenHeight-(int)GetMousePosition().y,
                1,1, GL_RGB, GL_UNSIGNED_BYTE, &colors[0] );
            EndTextureMode();
            // find which object was drawn using this color
            for (int i=0;i<5;i++)
                {
                if (objects[i].picking_color.r==colors[0]
                    && objects[i].picking_color.g==colors[1]
                    && objects[i].picking_color.b==colors[2])
                    objects[i].selected=1;
                    else
                    objects[i].selected=0;
                }
            box_select=0;
        }
            
        if (IsMouseButtonUp(MOUSE_BUTTON_LEFT))
            box_select=0;

        timed+=GetFrameTime()/1.0;

        UpdateCamera(&camera, CAMERA_ORBITAL);

        // Draw the scene to the screen
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(DARKGRAY);

            BeginMode3D(camera);

            for (int i=0;i<5;i++)
                {
                material.maps[MATERIAL_MAP_DIFFUSE].color = objects[i].color;
                DrawMesh(objects[i].mesh,material,objects[i].transform);

                if (objects[i].selected) 
                    {
                    rlEnableWireMode();
                    material.maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
                    DrawMesh(objects[i].mesh,material,objects[i].transform);
                    rlDisableWireMode();
                    }
            }

            EndMode3D();

            DrawFPS(10, 10);

        if (box_select==1)
            {
            box_select_area.width=GetMousePosition().x-box_select_area.x;
            box_select_area.height=GetMousePosition().y-box_select_area.y;
            if (box_select_area.width<0)
                box_select_area.width=0;
            if (box_select_area.height<0)
                box_select_area.height=0;
            DrawRectangleLines(box_select_area.x,box_select_area.y,
                box_select_area.width,box_select_area.height,WHITE);
            }

        DrawText("Press 1 to use selection box and OpenGL occlusion queries (click and drag to select)",10,30,10,WHITE);
        DrawText("Press 2 to use color picking (hover mouse over an object)",10,50,10,WHITE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    for (int i=0;i<5;i++)
        {
        UnloadMesh(objects[i].mesh);
        }

     CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}



