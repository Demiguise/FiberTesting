#ifndef __OGLRENDERER_H__
#define __OGLRENDERER_H__

struct GLFWwindow;

class COGLRenderer
{
public:
	COGLRenderer();
	~COGLRenderer();

	void RenderScene(GLFWwindow* pWindow);

private:

};

#endif	//~__OGLRENDERER_H__