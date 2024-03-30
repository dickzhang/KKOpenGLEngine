#pragma once
class Camera;

class ModuleBase
{
public:
	ModuleBase();
	virtual~ModuleBase();
	virtual void Init();
	virtual void PreRender(Camera* camera);
	virtual void Render(Camera* camera);
	virtual void Exit();
private:

};
