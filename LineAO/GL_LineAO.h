#pragma once

#include <stdlib.h>
#include <time.h>

#include <fantom/algorithm.hpp>
#include <fantom/register.hpp>
#include <fantom/graphics.hpp>
#include <fantom/datastructures/LineSet.hpp>

#include <GL/glew.h>

#include "Shader.h"

using namespace fantom;


class GL_LineAO : public CustomDrawer {

public:
	GL_LineAO( std::shared_ptr< const LineSet > sLines );
	~GL_LineAO();

	virtual void draw() const;

private:
	std::shared_ptr< const LineSet > streamlines;
	std::vector< GLfloat > vertices;
	std::vector< GLuint > indices;
	std::vector< GLfloat > tangents;

	GLuint VAO, VBO, IBO, TBO;
	GLuint quadVAO, quadVBO, quadIBO, quadTex;

	// G-Buffer
	GLuint gBuffer;
	GLuint gColor, gNDMap, gZoomMap;
	GLuint depthRBO;
	GLuint noise;

	std::unique_ptr< Shader > m_lineShader;
	std::unique_ptr< Shader > m_textureShader;

	GLuint m_width, m_height;

	void initLines();
	void initQuad();
	void initGBuffer();
	void genNoiseTexture();

	// rendering passes
	void lineShadingPass() const;
	void lightningPass() const;
};	
