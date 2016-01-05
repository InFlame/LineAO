#version 330 compatibility

layout( location = 0 ) in vec3 position;
layout( location = 1 ) in vec3 tangent;

out vec3 Color;
out vec3 Normal;
out vec3 FragPos;
out float Zoom;

void main() {
	gl_Position = gl_ModelViewProjectionMatrix * vec4( position, 1.0f );

	vec3 view = vec3( 0.0f, 0.0f, -1.0f );
	vec3 pos = ( gl_ModelViewMatrix * vec4( position, 0.0f ) ).xyz;
	vec3 c = pos - view;
	vec3 t = normalize( gl_ModelViewMatrix * vec4( tangent, 0.0f ) ).xyz;

	vec3 offset = normalize( cross( view, t ) );
	vec3 normal = normalize( cross( offset, t ) );
	normal *= sign( dot( normal, vec3( 0.0f, 0.0f, 1.0f ) ) );
	Normal = ( vec4( normal, 0.0f ) ).xyz;

	Zoom = length( ( gl_ModelViewMatrix * normalize( vec4( 1.0f, 1.0f, 1.0f, 0.0f ) ) ).xyz );

	FragPos = pos;

	Color = normalize( abs( tangent ) );
}