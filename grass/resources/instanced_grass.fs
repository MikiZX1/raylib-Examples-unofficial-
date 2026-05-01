#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;

// Output fragment color
out vec4 finalColor;

void main()
{
    finalColor = pow(fragColor, vec4(1.0/1.0));
    finalColor.a=1.0;
    finalColor.rgb*=1.0-fragColor.a/2.0;
    finalColor.rg+=finalColor.rg*fragColor.a/4.0;
    finalColor.b+=finalColor.r/2.0;
}
