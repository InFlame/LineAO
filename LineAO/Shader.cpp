#include "Shader.h"

Shader::Shader( const GLchar* vertexPath, const GLchar* fragmentPath ) {
	load( vertexPath, fragmentPath );
}

Shader::~Shader() {

}

void Shader::load( const GLchar* vertexPath, const GLchar* fragmentPath ) {
	std::string vertexCode;
	std::string fragmentCode;

	std::ifstream vShaderFile;
	std::ifstream fShaderFile;

	vShaderFile.exceptions( std::ifstream::badbit );
	fShaderFile.exceptions( std::ifstream::badbit );
	try {
		vShaderFile.open( vertexPath );
		fShaderFile.open( fragmentPath );
		std::stringstream vShaderStream, fShaderStream;

		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();

		vShaderFile.close();
		fShaderFile.close();

		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
	} catch( std::ifstream::failure e ) {
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}

	m_vertexCode = reinterpret_cast< const GLchar* >( vertexCode.c_str() );
	m_fragmentCode = reinterpret_cast< const GLchar* >( fragmentCode.c_str() );

	//std::cout << m_vertexCode << std::endl;

	GLint success;
	GLchar infoLog[512];

	// vertex shader
	m_vertexID = glCreateShader( GL_VERTEX_SHADER );
	glShaderSource( m_vertexID, 1, &m_vertexCode, NULL );
	glCompileShader( m_vertexID );

	glGetShaderiv( m_vertexID, GL_COMPILE_STATUS, &success );
	if( !success ) {
		glGetShaderInfoLog( m_vertexID, 512, NULL, infoLog );
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	m_fragmentID = glCreateShader( GL_FRAGMENT_SHADER );
	glShaderSource( m_fragmentID, 1, &m_fragmentCode, NULL );
	glCompileShader( m_fragmentID );

	glGetShaderiv( m_fragmentID, GL_COMPILE_STATUS, &success );
	if( !success ) {
		glGetShaderInfoLog( m_fragmentID, 512, NULL, infoLog );
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;		
	}

	m_programID = glCreateProgram();
	glAttachShader( m_programID, m_vertexID );
	glAttachShader( m_programID, m_fragmentID );
	glLinkProgram( m_programID );

	glGetProgramiv( m_programID, GL_LINK_STATUS, &success );
	if( !success ) {
		glGetProgramInfoLog( m_programID, 512, NULL, infoLog );
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	glDeleteShader( m_vertexID );
	glDeleteShader( m_fragmentID );

}

GLuint Shader::programID() { return m_programID; }

void Shader::use( bool enable ) {
	if( enable ) glUseProgram( m_programID );
	else  glUseProgram(0);
}
