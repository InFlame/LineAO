#include <iostream>

#include <string>
#include <fstream>
#include <sstream>

#include <GL/glew.h>

class Shader {

public:
	Shader( const GLchar* vertexPath, const GLchar* fragmentPath );
	~Shader();

	void use( bool f);
	GLuint programID();

private:
	const GLchar* m_vertexCode;
	const GLchar* m_fragmentCode;
	GLuint m_programID, m_vertexID, m_fragmentID;

	void load( const GLchar* vertexPath, const GLchar* fragmentPath );
	
};