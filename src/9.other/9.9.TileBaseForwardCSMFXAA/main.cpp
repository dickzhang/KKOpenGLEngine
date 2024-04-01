#include "GraphicsEngine.h"
#include "EngineUtils.h"

int main()
{
    if (!EngineUtils::InitGLFW())
    {
        EngineUtils::ReleaseGLFW();
        assert(false);
    }

    GraphicsEngine().Run();
    return 0;
}