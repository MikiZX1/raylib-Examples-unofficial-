/* Sets the GLSL sampler and accompanying texture slot in the specified material.
   Use this function to specify:
      - material textures
      - and their respective custom samplers in the material's shader.
  NOTE:
   1) Materials are used when drawing 3d models using DrawModel or DrawMesh and they contain information about:
      - shader
      - texture maps
      - and their tints.
   2) Drawing Models and Meshes using a material does not require BeginShaderMode block to work and will break
       this block if drawn within it.
   3) If you are drawing using raylib's batching system (text, primitives, DrawTexture, ...) then this function is not
       what you should be using. Rather. in case of batched rendering, you should specify the additional textures for
       your GLSL shader using SetShaderValueTexture, after the BeginShaderMode command.
  
  Requirements: Material with a custom glsl shader
  
  Input arguments:  material  = memory address of the material to modify
                    texture_unit  = which texture unit in the material to change
                    tex = Texture2D structure to assign to the texture slot in the material
                    sampler_name  = GLSL sampler name to be bound to the texture unit

  Return values: 0  SUCCESS                    
                -1  [ERROR] TRYING TO MODIFY THE DEFAULT SHADER 
                -2  [ERROR] SAMPLER NAME NOT FOUND IN THE MATERIAL's SHADER
                -3  [ERROR] TEXTURE NOT VALID
                -4  [ERROR] TEXTURE UNIT INDEX OUT OF BOUNDS
*/
int SetMaterialShaderTexture(Material *material, int texture_unit, Texture2D tex, const char* sampler_name)
{
  Material default_material=LoadMaterialDefault();

  int default_shader_id=default_material.shader.id;
  UnloadMaterial(default_material);
  if (material->shader.id==default_shader_id)
    {
      return -1; //we cannot modify the default shader
    }

  if (texture_unit>=7 && texture_unit<=9) 
    {
      // texture you are uploading will be treated as a cubemap type of texture
    }

  int shader_location=GetShaderLocation(material->shader,sampler_name);
  if (shader_location==-1)
    {
      return -2;  // could not find the specified GLSL sampler name
    }

  if (texture_unit>-1 && texture_unit<12)
    {
    if (IsTextureValid(tex))
      {
      material->shader.locs[SHADER_LOC_MAP_DIFFUSE+texture_unit]=shader_location;
      material->maps[MATERIAL_MAP_DIFFUSE+texture_unit].texture=tex;
      }
      else
      {
        return -3;  // texture not valid
      }
    }
    else
    {
      return -4;  // only 12 texture units are available (units 0 to 11)
    }

  return 0;
}
