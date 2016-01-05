#include "GL_LineAO.h"

GL_LineAO::GL_LineAO( std::shared_ptr< const LineSet > sLines ) :
	streamlines( sLines )
{

	GLint value[4];
	glGetIntegerv( GL_VIEWPORT, value );

	int width = value[2];
	int height = value[3];

	m_width = width;
	m_height = height;

	srand( time( NULL ) );

	// create shaders
	m_lineShader = std::unique_ptr< Shader >( new Shader( 
		"/u/mai11dre/VisPrak/fantom/praktikum/shader/Line-vertex.glsl",
		"/u/mai11dre/VisPrak/fantom/praktikum/shader/Line-fragment.glsl")
	);

	m_textureShader = std::unique_ptr< Shader >( new Shader (
		"/u/mai11dre/VisPrak/fantom/praktikum/shader/Lightning-vertex.glsl",
		"/u/mai11dre/VisPrak/fantom/praktikum/shader/Lightning-fragment.glsl")
	);

	initLines();
	initQuad();
	initGBuffer();
	genNoiseTexture();

}

GL_LineAO::~GL_LineAO() {
	glDeleteVertexArrays( 1, &VAO );
	glDeleteBuffers( 1, &VBO );
	glDeleteBuffers( 1, &IBO );
	glDeleteBuffers( 1, &TBO );

	glDeleteVertexArrays( 1, &quadVAO );
	glDeleteBuffers( 1, &quadVBO );
	glDeleteBuffers( 1, &quadIBO );
	glDeleteBuffers( 1, &quadTex );

	glDeleteFramebuffers( 1, &gBuffer );
	glDeleteTextures( 1, &gColor );
	glDeleteTextures( 1, &gNDMap );
	glDeleteTextures( 1, &gZoomMap );

	glDeleteRenderbuffers( 1, &depthRBO );

	glDeleteTextures( 1, &noise );
}

void GL_LineAO::initLines() {
	std::vector< std::vector< size_t > > lines = streamlines->getLines();

	vertices.clear();
	indices.clear();
	tangents.clear();

	int offset = 0;
	for( int i=0; i<lines.size(); i++ ) {
		if( lines.at(i).size() == 0 ) continue;
		for( int j=0; j<lines.at(i).size(); j++ ) {
			// vertices
			vertices.push_back( streamlines->getPointOnLine( i, j )[0] );
			vertices.push_back( streamlines->getPointOnLine( i, j )[1] );
			vertices.push_back( streamlines->getPointOnLine( i, j )[2] );
		
			// indices
			if( j < lines.at(i).size()-1 ) {
				indices.push_back( offset );
				indices.push_back( ++offset );
			} else offset ++;

			// tangents
			Point3 point = streamlines->getPointOnLine( i, j );
			Point3 tangent;
			if( j == 0 ) {
				Point3 pointNext = streamlines->getPointOnLine( i, j + 1 );
				tangent = point - pointNext;
			} else if( j == lines.at(i).size() - 1 ) {
				Point3 pointBefore = streamlines->getPointOnLine( i, j - 1 );
				tangent = pointBefore - point;
			} else {
				Point3 pointNext = streamlines->getPointOnLine( i, j + 1 );
				Point3 pointBefore = streamlines->getPointOnLine( i, j - 1 );
				tangent = pointBefore - pointNext;
			}
			//tangent = normalized( tangent );
			tangents.push_back( tangent[0] );
			tangents.push_back( tangent[1] );
			tangents.push_back( tangent[2] );

			//std::cout << tangent[0] << " | " << tangent[1] << " | " << tangent[2] << std::endl;
		}
	}

	// generate buffers
	glGenVertexArrays( 1, &VAO );
	glGenBuffers( 1, &VBO );
	glGenBuffers( 1, &IBO );
	glGenBuffers( 1, &TBO );

	// fill vertex array and buffers
	glBindVertexArray( VAO );

	glBindBuffer( GL_ARRAY_BUFFER, VBO );
	glBufferData( GL_ARRAY_BUFFER, sizeof( GLfloat ) * vertices.size(), &vertices[0], GL_STATIC_DRAW );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof( GLfloat ), (GLvoid*)0 );
	glEnableVertexAttribArray( 0 );

	glBindBuffer( GL_ARRAY_BUFFER, TBO );
	glBufferData( GL_ARRAY_BUFFER, sizeof( GLfloat ) * tangents.size(), &tangents[0], GL_STATIC_DRAW );
	glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof( GLfloat ), (GLvoid*)0 );
	glEnableVertexAttribArray( 1 );

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, IBO );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( GLuint ) * indices.size(), &indices[0], GL_STATIC_DRAW );

	glBindVertexArray( 0 );
}

void GL_LineAO::initQuad() {
	// create quad
	glGenVertexArrays( 1, &quadVAO );
	glGenBuffers( 1, &quadVBO );
	glGenBuffers( 1, &quadIBO );
	glGenBuffers( 1, &quadTex );

	// data
	const GLfloat quadVertices[] = {
		-1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f
	};

	const GLuint quadIndices[] = {
		0, 1, 2,
		0, 2, 3
	};

	const GLfloat texCoords[] = {
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f
	};

	glBindVertexArray( quadVAO );

	glBindBuffer( GL_ARRAY_BUFFER, quadVBO );
	glBufferData( GL_ARRAY_BUFFER, sizeof( quadVertices ), quadVertices, GL_STATIC_DRAW );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof( GLfloat ), (GLvoid*)0 );
	glEnableVertexAttribArray( 0 );

	glBindBuffer( GL_ARRAY_BUFFER, quadTex );
	glBufferData( GL_ARRAY_BUFFER, sizeof( texCoords ), texCoords, GL_STATIC_DRAW );
	glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof( GLfloat ), (GLvoid*)0 );
	glEnableVertexAttribArray( 1 );

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, quadIBO );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( quadIndices ), quadIndices, GL_STATIC_DRAW );

	glBindVertexArray( 0 );
}

void GL_LineAO::initGBuffer() {
	glGenFramebuffers( 1, &gBuffer );
	glBindFramebuffer( GL_FRAMEBUFFER, gBuffer );

	glGenTextures( 1, &gColor );
	glBindTexture( GL_TEXTURE_2D, gColor );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gColor, 0 );

	glGenTextures( 1, &gNDMap );
	glBindTexture( GL_TEXTURE_2D, gNDMap );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glGenerateMipmap( GL_TEXTURE_2D );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNDMap, 0 );

	glGenTextures( 1, &gZoomMap );
	glBindTexture( GL_TEXTURE_2D, gZoomMap );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_FLOAT, NULL );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gZoomMap, 0 );

	GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers( 3, attachments );

	// add renderbuffer
	glGenRenderbuffers( 1, &depthRBO );
	glBindRenderbuffer( GL_RENDERBUFFER, depthRBO );
	glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height );
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRBO);

	if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE ) 
	std::cout << "ERROR: Framebuffer incomplete! " << std::endl;

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

void GL_LineAO::genNoiseTexture() {
	// TODO: change to texture repeating by modifying texture coordinates

	int width = m_width;
	int height = m_height;

	std::vector< float > randVectors( width * height * 3 );

	#pragma omp parallel for
	for( int y=0; y<height; y++ ) {
		for( int x=0; x<width; x++ ) {
			randVectors[ y * width * 3 + x * 3 + 0 ] = static_cast< float >( rand() ) / static_cast< float >( RAND_MAX );		
			randVectors[ y * width * 3 + x * 3 + 1 ] = static_cast< float >( rand() ) / static_cast< float >( RAND_MAX );
			randVectors[ y * width * 3 + x * 3 + 2 ] = static_cast< float >( rand() ) / static_cast< float >( RAND_MAX );
		}
	}

	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	glGenTextures( 1, &noise );
	glBindTexture( GL_TEXTURE_2D, noise );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, &randVectors[0] );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
}

// ---------------------     rendering passes     -------------------------

void GL_LineAO::lineShadingPass() const {
	glEnable( GL_DEPTH_TEST );

	// select framebuffer as render target
	glBindFramebuffer( GL_FRAMEBUFFER, gBuffer );
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_lineShader->use( true );
		glBindVertexArray( VAO );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, IBO );
		glDrawElements( GL_LINES, indices.size(), GL_UNSIGNED_INT, 0 );
		glBindVertexArray( 0 );
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );

	m_lineShader->use( false );	
}

void GL_LineAO::lightningPass() const {
	glDisable( GL_DEPTH_TEST );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	m_textureShader->use( true );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, gColor );
	glUniform1i( glGetUniformLocation( m_textureShader->programID(), "gColor" ), 0 );

	glActiveTexture( GL_TEXTURE1 );
	glBindTexture( GL_TEXTURE_2D, gNDMap );
	glUniform1i( glGetUniformLocation( m_textureShader->programID(), "gNDMap" ), 1 );

	glActiveTexture( GL_TEXTURE2 );
	glBindTexture( GL_TEXTURE_2D, gZoomMap );
	glUniform1i( glGetUniformLocation( m_textureShader->programID(), "gZoomMap" ), 2 );

	glActiveTexture( GL_TEXTURE3 );
	glBindTexture( GL_TEXTURE_2D, noise );
	glUniform1i( glGetUniformLocation( m_textureShader->programID(), "noise" ), 3 );

	glUniform1f( glGetUniformLocation( m_textureShader->programID(), "u_colorSizeX" ), m_width );
	glUniform1f( glGetUniformLocation( m_textureShader->programID(), "u_colorSizeY" ), m_height );

	glBindVertexArray( quadVAO );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, quadIBO );
	glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );
	glBindVertexArray( 0 );

	m_textureShader->use( false );
}

void GL_LineAO::draw() const {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	lineShadingPass();
	lightningPass();

	// check window resize
	/*GLint value[4];
	glGetIntegerv( GL_VIEWPORT, value );

	if( value[2] != m_width || value[3] != m_height ) const_cast< GL_LineAO* >(this)->initGBuffer();*/

}