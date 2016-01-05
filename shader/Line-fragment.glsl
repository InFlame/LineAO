#version 330 compatibility

layout( location = 0 ) out vec3 gColor;
layout( location = 1 ) out vec4 gNDMap;
layout( location = 2 ) out vec3 gZoomMap;

in vec3 Color;
in vec3 Normal;
in vec3 FragPos;
in float Zoom;

vec3 lightColor = vec3( 1.0f, 1.0f, 1.0f );
vec3 lightPos = gl_LightSource[0].position.xyz;

void main() {

	float depth = gl_FragCoord.z * gl_FragCoord.w;

	gNDMap = vec4( normalize( Normal ), depth );
	gZoomMap = vec3( Zoom, Zoom, Zoom );

	// illumination
	vec3 viewDir = vec3( 0.0f, 0.0f, 1.0f );
	vec3 H = normalize( lightPos + viewDir );
	// ambient
	float ambientStrength = 0.1f;
	vec3 ambient = lightColor * ambientStrength;
	// diffuse
	float diffuseStrength = max( dot( lightPos, Normal ), 0.0f );
	vec3 diffuse = lightColor * diffuseStrength;
	// specular
	float specularStrength = pow( max( dot( H, Normal ), 0.0f ), 64.0f );
	if( diffuseStrength <= 0.0f ) specularStrength = 0.0f;
	vec3 specular = lightColor * specularStrength;

	vec3 result = ( ambient + diffuse + specular ) * Color;

	gColor = result;
}