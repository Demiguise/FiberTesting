#include "stdafx.h"
#include "OGLRenderer.h"
#include "RayTracer.h"

#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string>

COGLRenderer::COGLRenderer()
{
	if (glewInit() != GLEW_OK)
	{
		return;
	}

	GLuint vertexArrayId = 0;
	glGenVertexArrays(1, &vertexArrayId);
	glBindVertexArray(vertexArrayId);

	static const GLfloat g_vertex_buffer_data[] = {
	-1.0f, -1.0f, 0.0f,
   1.0f, -1.0f, 0.0f,
   0.0f,  1.0f, 0.0f,
	};

	glGenBuffers(1, &m_vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	m_program = glCreateProgram();

	GLuint vertexShader = CompileShaders("Source/Shaders/vertex.shader", GL_VERTEX_SHADER);
	glAttachShader(m_program, vertexShader);

	GLuint fragmentShader = CompileShaders("Source/Shaders/fragment.shader", GL_FRAGMENT_SHADER);
	glAttachShader(m_program, fragmentShader);

	glLinkProgram(m_program);

	GLint result = GL_FALSE;
	int resultLogLength;
	glGetProgramiv(m_program, GL_LINK_STATUS, &result);
	glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &resultLogLength);
	if (resultLogLength > 0)
	{
		std::vector<char> errorMessage(resultLogLength + 1);
		glGetProgramInfoLog(m_program, result, NULL, &errorMessage[0]);
		DebugLog("[Renderer] Failed to compile. Error: %s", errorMessage);
		DebugBreak();
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	m_camera.m_position = EnVector3::Zero();
}

COGLRenderer::~COGLRenderer()
{
}

void COGLRenderer::RenderScene(GLFWwindow* pWindow)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(m_program);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0 );

	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDisableVertexAttribArray(0);

	//GLfloat* image = (GLfloat*)malloc(sizeof(GLfloat) * gWinHeight * gWinWidth * 4);
	//g_pRayTracer->GetGLPixelOutput(image);

	//glDrawPixels(gWinWidth, gWinHeight, GL_RGBA, GL_FLOAT, image);

	//free(image);

}

GLuint COGLRenderer::CompileShaders(const char* filePath, GLenum type)
{
	GLuint shaderId = glCreateShader(type);
	std::string shaderCode;
	std::ifstream shaderCodeStream(filePath, std::ios::in);
	if (shaderCodeStream.is_open())
	{
		std::string line = "";
		while (getline(shaderCodeStream, line))
		{
			shaderCode += "\n" + line;
		}
		shaderCodeStream.close();
	}
	else
	{
		DebugLog("Couldn't find shader file to open: '%s'", filePath);
		return false;
	}

	const char* shaderCodeChar = shaderCode.c_str();
	glShaderSource(shaderId, 1, &shaderCodeChar, NULL);
	glCompileShader(shaderId);

	GLint result = GL_FALSE;
	int resultLogLength;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);
	glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &resultLogLength);
	if (resultLogLength > 0)
	{
		std::vector<char> errorMessage(resultLogLength);
		glGetShaderInfoLog(shaderId, result, NULL, &errorMessage[0]);
		DebugLog("[Renderer] Failed to compile. Error: %s", errorMessage);
	}

	return shaderId;
}
