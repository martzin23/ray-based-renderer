#include "utility/includes.h"
#include "engine/engine.h"

/* TODO
save framebuffer 
	https://blog.42yeah.is/opengl/2023/05/27/framebuffer-export.html
load image
	heightmapping
	cube mapping

pbr pathtracing
	refraction
	mirror
	roughness
	subsurface scattering
scroll
	speed

fix free move camera
dls
importance light sampling
triangle mesh
BVG

*/

int main() {

	Engine *engine = new Engine();

	while (engine->isRunning()) {
		engine->update();
		engine->render();
	}

	delete engine;
	return 0;		
}
