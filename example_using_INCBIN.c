#include "raylib.h"

#include <stdlib.h>
#include <string.h>

// Include incbin.h, you can download this file directly from INCBIN's GitHub repository
#define INCBIN_PREFIX
#include "incbin.h"

// These are our resources, located in this example at the same root as this file; you can use subfolders
// each of the following 3 lines defines variables with ..Data, ..Size suffixes 
// for example INCBIN(TestImage, "test.png"); will give us TestImageData and TestImageSize variables to use in code
INCBIN(TestImage, "test.png");
INCBIN(OBJ_untitled, "untitled.obj");
INCBIN(GLB_untitled, "untitled.glb");

// temporary memory blocks we will pass to raylib so loading completes correctly
// After testing, it appears these will be released by raylib Unload... functions (double check yourself)
unsigned char* OBJ_untitled_tmp=NULL;
unsigned char* GLB_untitled_tmp=NULL;

// my loading function that redirects Load... function to resource when loading of binary data is needed
unsigned char* MyLoadFileData(const char* fileName, int* dataSize)
{
  if (strstr(fileName,"untitled.glb")!=NULL)
  {
    *dataSize = GLB_untitledSize;
    GLB_untitled_tmp=malloc(GLB_untitledSize);
    if (GLB_untitled_tmp!=NULL)
      memcpy(GLB_untitled_tmp,&GLB_untitledData[0],GLB_untitledSize);
    return GLB_untitled_tmp;
  }
}

// my loading function that redirects Load... function to resource when loading of text data is needed
char* MyLoadFileText(const char* fileName)
{
    OBJ_untitled_tmp=malloc(OBJ_untitledSize);
    if (OBJ_untitled_tmp!=NULL)
      memcpy(OBJ_untitled_tmp,&OBJ_untitledData[0],OBJ_untitledSize);
    return OBJ_untitled_tmp;
}

int main()
{
InitWindow(800, 450, "raylib - test incbin");

  SetLoadFileDataCallback(&MyLoadFileData);   // tell raylib we want to provide data instead raylib doing this for us
  Model MyModel_GLB=LoadModel("untitled.glb");  // now LoadModel will use our "loading" function MyLoadFileData (GLB format=binary)
  SetLoadFileTextCallback(&MyLoadFileText);   // tell raylib we want to provide data instead raylib doing this for us
  Model MyModel_OBJ=LoadModel("model.obj");   // now LoadModel will use our "loading" function MyLoadFileText (OBJ format=text)
  SetLoadFileDataCallback(NULL);    // tell raylib it should be doing the data/text loading from now on
  SetLoadFileTextCallback(NULL);

  Image image1 = LoadImageFromMemory(".PNG",&TestImageData[0],TestImageSize);  // we can also use incbin like this
  Texture texture1=LoadTextureFromImage(image1);

  while (!WindowShouldClose()) 
    {
    BeginDrawing();
      ClearBackground(GRAY);
      DrawTexture(texture1,0,0,WHITE);
    EndDrawing();
    }

UnloadTexture(texture1);
UnloadImage(image1);
UnloadModel(MyModel_GLB);
UnloadModel(MyModel_OBJ);
CloseWindow();

}
