#version 330 compatibility

in vec2 TexCoords;
out vec4 Color;

uniform sampler2D gColor;
uniform sampler2D gNDMap;
uniform sampler2D gZoomMap;
uniform sampler2D noise;

uniform float u_colorSizeX;
uniform float u_colorSizeY;

uniform float u_lineAORadius = 2.0f;
uniform float u_lineAODensityWeight = 1.0f;
uniform float u_lineAOTotalStrength;

const float blurSizeH = 1.0 / 300.0;
const float blurSizeV = 1.0 / 200.0;

#define SCALES 4
#define SAMPLES 32

vec3 lightSource = vec3( 0.0f, 0.0f, -1.0f );

vec4 blur() {
	vec4 sum = vec4( 0.0f );
	for( int x=-4; x<4; x++ ) {
		for( int y=-4; y<4; y++ ) {
			sum += texture( 
				gColor,
				vec2( TexCoords.x + x * blurSizeH, TexCoords.y + y * blurSizeV )
			) / 81.0;
		}
	}

	return sum;
}

float AmbientOcclusion() {

    const float falloff = 0.00001;

    float occlusion = 0.0f;

	vec3 normal = texture( gNDMap, TexCoords ).xyz;
	float currentPixelDepth = texture( gNDMap, TexCoords ).w;

	vec3 ray;
	vec3 hemispherePoint;

	// grab random normal and transform to interval [-1, 1]
	vec3 randNormal = normalize( ( texture( noise, TexCoords ).xyz * 2.0f ) - vec3( 1.0f ) );

	float radiusScaler = 0.0f;
	float maxPixels = max( u_colorSizeX , u_colorSizeY );
	float radius = ( texture( gZoomMap, TexCoords ).x * u_lineAORadius / maxPixels ) / ( 1.0f - currentPixelDepth );

	vec3 occluderNormal;
	float occluderDepth;
	float depthDifference;
	float normalDifference;

	for( int l=0; l<SCALES; l++ ) {

		float occluseionStep = 0.0f;

		#define radScaleMin 1.5
		radiusScaler += radScaleMin + l;

		int numSamplesAdded = 0;
		for( int i=0; i<SAMPLES; i++ ) {

			// visibility
			vec3 randSphereNormal = ( texture( noise, vec2( float( i ) / float( SAMPLES ),
															float( l + 1 ) / float( SCALES ) ) ).rgb * 2.0f ) - vec3( 1.0f );
			vec3 hemisphereVector = reflect( randSphereNormal, randNormal );			
			ray = radiusScaler * radius * hemisphereVector;
			ray *= sign( dot( ray, normal ) );

			hemispherePoint = ray + vec3( TexCoords, currentPixelDepth );

           if( ( hemispherePoint.x < 0.0 ) || ( hemispherePoint.x > 1.0 ) ||
                ( hemispherePoint.y < 0.0 ) || ( hemispherePoint.y > 1.0 )
              )
            {
                continue;
            }

			numSamplesAdded++;

			occluderDepth = texture( gNDMap, hemispherePoint.xy ).w;
			occluderNormal = texture( gNDMap, hemispherePoint.xy ).xyz;

			depthDifference = currentPixelDepth - occluderDepth;

			// weight 	might be occluderNormal instead of hemisphereVector
			float pointDiffuse = max( dot( hemisphereVector, normal ), 0.0f );

			// illumination weight
			float occluderDiffuse = 0.0f; // replaceable with more realistic effect
			vec3 H = normalize( lightSource + normalize( hemisphereVector ) );
			float occluderSpecular = pow( max( dot( H, occluderNormal ), 0.0f ), 100.0 );

			normalDifference = pointDiffuse * ( occluderSpecular + occluderDiffuse );
			normalDifference = 1.5f - normalDifference;

			// depth weight
			float scaler = 1.0 - ( l / ( float( SCALES - 1 ) ) );
			float densityInfluence = scaler * scaler * u_lineAODensityWeight;
			float densityWeight = 1.0f - smoothstep( falloff, densityInfluence, depthDifference );

			occluseionStep += normalDifference * densityWeight * step( falloff, depthDifference );
		}

		occlusion += ( 1.0f / float( numSamplesAdded ) ) * occluseionStep;

	}

	float occlusionScaleFactor = 1.0f / SCALES;
	occlusionScaleFactor *= u_lineAOTotalStrength;

	return clamp( ( 1.0f - ( occlusionScaleFactor * occlusion ) ), 0, 1 );

}

void main() {
	/*Color.xyz = texture( gColor, TexCoords ).rgb * AmbientOcclusion();
	Color.w = texture( gColor, TexCoords ).a;*/
	//Color = blur();

	Color = texture( gColor, TexCoords );
}
