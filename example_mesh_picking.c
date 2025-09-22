/*******************************************************************************************
*   screen space mesh picking / using "occlusion queries" or "color picking"
*   OpenGL queries code taken from: https://stackoverflow.com/questions/36258142/opengl-c-occlusion-query
*   
********************************************************************************************/
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "external/glad.h"

#define MIN(a,b) (((a)<(b))? (a):(b))
#define MAX(a,b) (((a)>(b))? (a):(b))

#define MAX_OBJECTS 35

typedef struct _EXAMPLE_OBJECT
{
    Mesh mesh;
    Color picking_color;    // color picking color
    Color color;            // drawing color
    Matrix transform;
    char selected;
} EXAMPLE_OBJECT;

EXAMPLE_OBJECT objects[MAX_OBJECTS];

char box_select;
Vector2     box_select_area_start={0,0};
Vector2     box_select_area_end={0,0};
Rectangle   box_select_area={0,0,0,0};

char selection_method = 0; // 0 = OCCLUSION QUERY, 1 = COLOR PICKING

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
    camera.position = (Vector3){ 0.0f, 60.0f, 100.0f };    // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };              // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };                  // Camera up vector (rotation towards target)
    camera.fovy = 50.0f;                                        // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;                     // Camera projection type

    // create mesh objects, choose position and drawing color for them
    // as well as write down their picking colors (their unique ID)
    for (int i=0;i<MAX_OBJECTS;i++)
        {
        objects[i].picking_color=(Color){i/65536,i/256,i%256, 255};
        objects[i].color=(Color){GetRandomValue(0,230),
                            GetRandomValue(0,230),GetRandomValue(0,230),255};
        objects[i].transform=MatrixMultiply(MatrixIdentity(), MatrixTranslate(GetRandomValue(-50,50),
                            GetRandomValue(-50,50),GetRandomValue(-50,50)));
        int random_mesh_id=GetRandomValue(0,4);
        switch (random_mesh_id)
            {
            case 0: objects[i].mesh=GenMeshCube(6,7,10); break;
            case 1: objects[i].mesh=GenMeshCone(6,6,4); break;
            case 2: objects[i].mesh=GenMeshCylinder(6,7,4); break;
            case 3: objects[i].mesh=GenMeshSphere(6,10,10); break;
            case 4: objects[i].mesh=GenMeshPlane(6,6,1,1); break;           
            }
        objects[i].selected=0;
        }

    Material material = LoadMaterialDefault();

    box_select=0;
    selection_method=1;

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second

    //--------------------------------------------------------------------------------------
    // Main game loop

    glGenQueries(1, query);             // Init OpenGL queries

    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyDown(KEY_ONE)) selection_method=1;
        if (IsKeyDown(KEY_TWO)) selection_method=2;

        // init mouse box selection
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && box_select==0)
            {
                    box_select_area_start=GetMousePosition();
                    box_select_area_end=GetMousePosition();
                    box_select_area.x=GetMousePosition().x;
                    box_select_area.y=GetMousePosition().y;
                    box_select_area.width=0;
                    box_select_area.height=0;
                    box_select=1;
            }

        // draw the scene to a render texture
        // render texture is used to obtain the picking color of the objects under the mouse cursor
        // as well as to do occlusion queries used to determine which objects are rendered within the box selected area
        BeginTextureMode(rt);

        if (selection_method==2)
            {
            // do color picking (read back color of pixel, under mouse cursor, from the 
            // previous frame rendered to our render texture)
            glReadPixels((int)GetMousePosition().x,screenHeight-(int)GetMousePosition().y,
                1,1, GL_RGB, GL_UNSIGNED_BYTE, &colors[0] );
            // find which object was drawn using this color
            for (int i=0;i<MAX_OBJECTS;i++)
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
        
        // now clear the render texture and draw this frame
        ClearBackground(WHITE);
        if ( selection_method == 1)
            {
            // restrict drawing to the selection box if using occlusion queries
            BeginScissorMode(box_select_area.x,box_select_area.y, box_select_area.width, box_select_area.height);
            }
      
        // start drawing the scene for our selection methods
        BeginMode3D(camera);    
        rlDisableDepthTest();
        for (int i=0;i<MAX_OBJECTS;i++)
        {
        if ( selection_method == 1)
            {
            // do OpenGL query (test to see how many pixels of this object/mesh got drawn)
            // this method is executed as the meshes are actually drawn to the render texture
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
            // just draw the mesh using its unique ID as its color (picking color)
            // we read the color under the mouse position in the next frame of this game loop
            material.maps[MATERIAL_MAP_DIFFUSE].color = objects[i].picking_color;
            DrawMesh(objects[i].mesh,material,objects[i].transform);
            }
        }
        rlEnableDepthTest();
        EndMode3D();

        EndScissorMode();
        EndTextureMode();

        // while box selecting re-calculate the selection rectangle
        if (box_select==1)
            {
                box_select_area_end=GetMousePosition();
                box_select_area.x=MIN(box_select_area_start.x,box_select_area_end.x);
                box_select_area.y=MIN(box_select_area_start.y,box_select_area_end.y);
                box_select_area.width=MAX(box_select_area_start.x,box_select_area_end.x)-box_select_area.x;
                box_select_area.height=MAX(box_select_area_start.y,box_select_area_end.y)-box_select_area.y;
            }

        // end box selection
        if (IsMouseButtonUp(MOUSE_BUTTON_LEFT))
            {
                box_select=0;
            }

        UpdateCamera(&camera, CAMERA_ORBITAL);

        // Draw the same scene to the screen using the same camera transform
        // but color the meshes as we want, texture them, or use shaders...
        // this part does not affect the screen space selection methods
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(DARKGRAY);

            BeginMode3D(camera);

            for (int i=0;i<MAX_OBJECTS;i++)
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

        if (selection_method == 1)
            {
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
    
    glDeleteQueries(1, query);

     CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
