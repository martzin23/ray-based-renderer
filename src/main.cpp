#include "utility/includes.h"
#include "engine/engine.h"

/* TODO
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
BVH

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
