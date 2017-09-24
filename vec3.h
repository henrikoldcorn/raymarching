struct vec3 {
	float x;
	float y;
	float z;
};

typedef struct vec3 vec3;

vec3 plus(vec3 a, const vec3 b) {
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	return a;
}

vec3 subtract(vec3 a, const vec3 b) {
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	return a;
}

vec3 scale(vec3 a, float b) {
	vec3 res = {a.x*b, a.y*b, a.z*b};
	return res;
}

float dot(vec3 a, vec3 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

float modulus(vec3 a) {
	return pow(dot(a, a), 0.5);
}

vec3 normalise(vec3 a) {
	float mag = modulus(a);
	return scale(a, 1.0 / mag);
}

vec3 cross(const vec3 a, const vec3 b) {
	float x = a.y*b.z - a.z*b.y;
	float y = a.z*b.x - a.x*b.z;
	float z = a.x*b.y - a.y*b.x;
	vec3 result = {x, y, z};
	return result;
}

vec3 reflect(vec3 ray, vec3 norm) {
	ray = normalise(ray);
	norm = normalise(norm);
	return subtract(ray, scale(norm, 2.0*dot(ray, norm)));	
}

