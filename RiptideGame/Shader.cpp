#include "Shader.h"
#include "glad.h"
#include <glm/matrix.hpp>

#include "vcruntime_exception.h"

GLuint CompileShader(const std::string& source, GLenum type) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        memset(infoLog, 0, 512);
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        switch (type)
        {
        case GL_VERTEX_SHADER:
            printf("shader compiler error in shader type VERTEX: ");
            break;
        case GL_FRAGMENT_SHADER:
            printf("shader compiler error in shader type FRAGMENT: ");
            break;
        }
        
        printf(infoLog);
        printf("\n");
    }
    return shader;
}


bool Shader::Compile()
{
	if (Native != 0)
	{
		glDeleteShader(Native);
	}
	Native = 0;
    GLuint vertexShader = CompileShader(VertexCode, GL_VERTEX_SHADER);
    GLuint fragmentShader = CompileShader(FragmentCode, GL_FRAGMENT_SHADER);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        memset(infoLog, 0, 512);
        printf(infoLog);
        printf("\n");
        return false;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    Native = program;
    return true;
}

int Shader::GetNative()
{
	return Native;
}

void Shader::Destroy()
{
	if (IsDestroyed()) { return; }
	Instance::Destroy();
	
	glDeleteShader(Native);
}

void Shader::SetMat4(const std::string& name, const glm::mat4& value) const {
    if (Native == 0) {
        return;
    }

    GLint location = glGetUniformLocation(Native, name.c_str());
    if (location == -1) {
        return;
    }

    glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);
}

void Shader::SetFloat(const std::string& name, float value) const {
    if (Native == 0) {
        return;
    }

    GLint location = glGetUniformLocation(Native, name.c_str());
    if (location == -1) {
        return;
    }

    glUniform1f(location, value);
}
