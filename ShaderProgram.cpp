#include "ShaderProgram.h"
#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <iostream>

ShaderProgram::ShaderProgram()
    : m_programId(-1) {

}

void ShaderProgram::load(const char* vertexPath, const char* fragmentPath, const char* geometryPath)
{
    // 1. retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::string geometryCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    std::ifstream gShaderFile;
    // ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        // open files
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        // read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // close file handlers
        vShaderFile.close();
        fShaderFile.close();
        // convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
        // if geometry shader path is present, also load a geometry shader
        if (geometryPath != nullptr)
        {
            gShaderFile.open(geometryPath);
            std::stringstream gShaderStream;
            gShaderStream << gShaderFile.rdbuf();
            gShaderFile.close();
            geometryCode = gShaderStream.str();
        }
    }
    catch (std::ifstream::failure& e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();
    // 2. compile shaders
    unsigned int vertex, fragment;
    int success;
    char infoLog[512];
    // vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);

    // print compile errors if any
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        throw std::runtime_error(infoLog);
    };

    // fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    // print compile errors if any
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        throw std::runtime_error(infoLog);
    };
    // if geometry shader is given, compile geometry shader
    unsigned int geometry;
    if (geometryPath != nullptr)
    {
        const char* gShaderCode = geometryCode.c_str();
        geometry = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometry, 1, &gShaderCode, NULL);
        glCompileShader(geometry);
        // print compile errors if any
        glGetShaderiv(geometry, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(geometry, 512, NULL, infoLog);
            throw std::runtime_error(infoLog);
        };
    }
    // shader Program
    m_programId = glCreateProgram();
    glAttachShader(m_programId, vertex);
    glAttachShader(m_programId, fragment);
    if (geometryPath != nullptr)
        glAttachShader(m_programId, geometry);
    glLinkProgram(m_programId);
    // print compile errors if any
    glGetShaderiv(m_programId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(m_programId, 512, NULL, infoLog);
        throw std::runtime_error(infoLog);
    };
    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    if (geometryPath != nullptr)
        glDeleteShader(geometry);

}


//void ShaderProgram::load(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
//{
//    std::string vertexCode;
//    std::string fragmentCode;
//    std::ifstream vShaderFile;
//    std::ifstream fShaderFile;
//    // ensure ifstream objects can throw exceptions:
//    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
//    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
//    try
//    {
//        // open files
//        vShaderFile.open(vertexShaderPath);
//        fShaderFile.open(fragmentShaderPath);
//        std::stringstream vShaderStream, fShaderStream;
//        // read file's buffer contents into streams
//        vShaderStream << vShaderFile.rdbuf();
//        fShaderStream << fShaderFile.rdbuf();
//        // close file handlers
//        vShaderFile.close();
//        fShaderFile.close();
//        // convert stream into string
//        vertexCode = vShaderStream.str();
//        fragmentCode = fShaderStream.str();
//    }
//    catch (std::ifstream::failure& e)
//    {
//        throw std::runtime_error("Failed to locate vertex or fragment shader files");
//    }
//
//    const char* vShaderCode = vertexCode.c_str();
//    const char* fShaderCode = fragmentCode.c_str();
//
//    unsigned int vertex, fragment;
//    int success;
//    char infoLog[512];
//
//    // vertex Shader
//    vertex = glCreateShader(GL_VERTEX_SHADER);
//    glShaderSource(vertex, 1, &vShaderCode, NULL);
//    glCompileShader(vertex);
//    // print compile errors if any
//    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
//    if (!success)
//    {
//        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
//        throw std::runtime_error(infoLog);
//    };
//
//    // similiar for Fragment Shader
//    fragment = glCreateShader(GL_FRAGMENT_SHADER);
//    glShaderSource(fragment, 1, &fShaderCode, NULL);
//    glCompileShader(fragment);
//    // print compile errors if any
//    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
//    if (!success)
//    {
//        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
//        throw std::runtime_error(infoLog);
//    };
//
//    // shader Program
//    m_programId = glCreateProgram();
//    glAttachShader(m_programId, vertex);
//    glAttachShader(m_programId, fragment);
//    glLinkProgram(m_programId);
//    // print linking errors if any
//    glGetProgramiv(m_programId, GL_LINK_STATUS, &success);
//    if (!success)
//    {
//        glGetProgramInfoLog(m_programId, 512, NULL, infoLog);
//        throw std::runtime_error(infoLog);
//    }
//
//    // delete the shaders as they're linked into our program now and no longer necessary
//    glDeleteShader(vertex);
//    glDeleteShader(fragment);
//}

void ShaderProgram::activate()
{
    glUseProgram(m_programId);
}

void ShaderProgram::setUniform(const std::string& uniformName, bool value)
{
    glUniform1i(glGetUniformLocation(m_programId, uniformName.c_str()), (int32_t)value);
}

void ShaderProgram::setUniform(const std::string& uniformName, int32_t value)
{
    glUniform1i(glGetUniformLocation(m_programId, uniformName.c_str()), value);
}

void ShaderProgram::setUniform(const std::string& uniformName, float_t value)
{
    glUniform1f(glGetUniformLocation(m_programId, uniformName.c_str()), value);
}

void ShaderProgram::setUniform(const std::string& uniformName, const glm::vec2& value)
{
    glUniform2fv(glGetUniformLocation(m_programId, uniformName.c_str()), 1, &value[0]);
}

void ShaderProgram::setUniform(const std::string& uniformName, const glm::vec3& value)
{
    glUniform3fv(glGetUniformLocation(m_programId, uniformName.c_str()), 1, &value[0]);
}

void ShaderProgram::setUniform(const std::string& uniformName, const glm::vec4& value)
{
    glUniform4fv(glGetUniformLocation(m_programId, uniformName.c_str()), 1, &value[0]);
}

void ShaderProgram::setUniform(const std::string& uniformName, const glm::mat2& value)
{
    glUniformMatrix2fv(glGetUniformLocation(m_programId, uniformName.c_str()), 1, false, &value[0][0]);
}

void ShaderProgram::setUniform(const std::string& uniformName, const glm::mat3& value)
{
    glUniformMatrix3fv(glGetUniformLocation(m_programId, uniformName.c_str()), 1, false, &value[0][0]);
}

void ShaderProgram::setUniform(const std::string& uniformName, const glm::mat4& value)
{
    glUniformMatrix4fv(glGetUniformLocation(m_programId, uniformName.c_str()), 1, false, &value[0][0]);
}
