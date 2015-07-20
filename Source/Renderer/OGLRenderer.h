#ifndef __OGLRENDERER_H__
#define __OGLRENDERER_H__

#include "glew.h"
#include "Camera.h"
struct GLFWwindow;

class COGLRenderer
{
public:
	COGLRenderer();
	~COGLRenderer();

	void RenderScene(GLFWwindow* pWindow);

private:
	GLuint m_vertexBuffer;
	GLuint m_program;

	GLuint CompileShaders(const char* filePath, GLenum type);

	CCamera m_camera;
};

#endif	//~__OGLRENDERER_H__