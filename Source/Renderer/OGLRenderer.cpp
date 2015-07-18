#include "stdafx.h"
#include "OGLRenderer.h"
#include "RayTracer.h"

#include "glew.h"

COGLRenderer::COGLRenderer()
{

}

COGLRenderer::~COGLRenderer()
{
}

void COGLRenderer::RenderScene(GLFWwindow* pWindow)
{
	GLuint vertexArrayId;
	glGenVertexArrays(1, &vertexArrayId);
	glBindVertexArray(vertexArrayId);
	//GLfloat* image = (GLfloat*)malloc(sizeof(GLfloat) * gWinHeight * gWinWidth * 4);
	//g_pRayTracer->GetGLPixelOutput(image);

	//glDrawPixels(gWinWidth, gWinHeight, GL_RGBA, GL_FLOAT, image);

	//free(image);

	//glfwSwapBuffers(pWindow);
}

