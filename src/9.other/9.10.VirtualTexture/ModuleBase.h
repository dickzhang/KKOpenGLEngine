#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
class Camera;

class ModuleBase
{
public:
	ModuleBase();
	virtual~ModuleBase();
	virtual void Init();
	virtual void PreRender(Camera* camera);
	virtual void Render(Camera* camera,glm::vec2 mouseuv);
	virtual void Exit();
private:

};
