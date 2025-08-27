#include "utility/includes.h"
#include "engine/engine.h"

/* TODO
clean up shader variables
readme

pbr pathtracing
	refraction
	mirror
	roughness
	subsurface scattering
scroll
	speed

fix free move camera
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
