#version 330
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in mat4 instanceTransform;
uniform mat4 mvp;
uniform mat4 matNormal;
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;
uniform float time;

float rand(vec2 c){
	return fract(sin(dot(c.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float noise(vec2 p, float freq ){
	float unit = 100.0/freq;
	vec2 ij = floor(p/unit);
	vec2 xy = mod(p,unit)/unit;
	//xy = 3.*xy*xy-2.*xy*xy*xy;
	xy = .5*(1.-cos(3.14159*xy));
	float a = rand((ij+vec2(0.,0.)));
	float b = rand((ij+vec2(1.,0.)));
	float c = rand((ij+vec2(0.,1.)));
	float d = rand((ij+vec2(1.,1.)));
	float x1 = mix(a, b, xy.x);
	float x2 = mix(c, d, xy.x);
	return mix(x1, x2, xy.y);
}

float pNoise(vec2 p, int res){
	float persistance = .5;
	float n = 0.;
	float normK = 0.;
	float f = 4.;
	float amp = 1.;
	int iCount = 0;
	for (int i = 0; i<50; i++){
		n+=amp*noise(p, f);
		f*=2.;
		normK+=amp;
		amp*=persistance;
		if (iCount == res) break;
		iCount++;
	}
	float nf = n/normK;
	return nf*nf*nf*nf;
}
void main()
{

vec3 tmpPosition = vec3(instanceTransform[3][0],instanceTransform[3][1],instanceTransform[3][2]);//*vec4(vertexPosition, 1.0));
float skala=12.0;
float noise1=4.0*clamp(pNoise(vec2(-time*12.0+tmpPosition.x/skala,tmpPosition.z/skala),10),0.0,1.0);

vec2 sway1=vec2(cos(rand(vec2(tmpPosition.x,tmpPosition.z))*time*3.0+tmpPosition.x+tmpPosition.z),sin(time*2.6+tmpPosition.x+tmpPosition.z));
vec3 myVertexPosition0=3.0*pow(1.0-(vertexPosition.z+7.5)/15.0,4.0)*vec3(-sway1.x,sway1.y,0.0);
sway1=vec2(cos(time*3.0+tmpPosition.x/150.0+tmpPosition.z/150.0),sin(rand(vec2(tmpPosition.x,tmpPosition.z))*time*4.0+tmpPosition.x/150.0+tmpPosition.z/150.0));
myVertexPosition0+=3.0*pow(1.0-(vertexPosition.z+7.5)/15.0,4.0)*vec3(-sway1.x,sway1.y,0.0);
  
sway1=vec2(clamp(noise1*2.0,0.0,2.0)+abs(cos(time))/2.0,0.0);
float skala7=pow((7.5-vertexPosition.z)/15.0,1.5);
vec3 myVertexPosition3=6.0*(skala7)*vec3(sway1.x,0.0,0.0);
myVertexPosition3.z-=-sway1.x*(skala7)*5;

vec3 offset = mix((-myVertexPosition0)/2.0,myVertexPosition3,        noise1);
vec3 myVertexPosition = vertexPosition+offset;

    fragPosition = vec3(instanceTransform*vec4(myVertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    offset.z/=5.0;
    offset.z=clamp(offset.z/2.0,0.1,0.99);
    fragColor = vec4(0.9+(-15.0+7.5-vertexPosition.z)/15.0*1.50,
	(0.95+(-15.0+7.5-vertexPosition.z)/15.0)/1.0+0.4+offset.z/1.0,
	0.2-(-15.0+7.5-vertexPosition.z)/15.0*0.50*0.26,
	1.0);
	fragColor.a=(15.0-7.5-vertexPosition.z)/15.0;
    gl_Position = mvp*instanceTransform*vec4(myVertexPosition, 1.0);
} 