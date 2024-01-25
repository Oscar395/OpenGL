#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {
public:
	unsigned int ID;

    Shader() = default;

    Shader(const char* vertexPath, const char* fragmentPath);

    Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath);

    void use();

    void setBool(const std::string& name, bool value) const;

    void setInt(const std::string& name, int value) const;

    void setFloat(const std::string& name, float value) const;

    void setVec3(const std::string& name, float x, float y, float z) const;

    void setVec3(const std::string& name, glm::vec3 value) const;

    void setMat4(const std::string& name, glm::mat4 value) const;
};
