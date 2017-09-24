#include "vec3.h"
#include "rgb.h"
#include <stdlib.h>
#include <math.h>

RGB getMaterial(vec3 point, vec3 norm) {
	RGB materialColour = {255, 255, 255};
	if (point.y < 7.0) {
		materialColour = {107, 79, 41};
	}
	return materialColour;
}