float clamp(float in, float min, float max) {
	if (in < min)
		return min;
	else if (in > max)
		return max;
	return in;
}

RGB mix(RGB a, RGB b, float weight) {
	float red = a.R * (1 -weight) + b.R * weight;
	float green = a.G * (1 -weight) + b.G * weight;
	float blue = a.B * (1 -weight) + b.B * weight;
	RGB ans = {red, green, blue};
	return ans;
}

float max(float a, float b) {
	return a > b ? a : b;
}

float min(float a, float b) {
	return a < b ? a : b;
}

float upness(vec3 dir) {
	vec3 up = {0.0, 1.0, 0.0};
	return dot(normalise(dir), up);
}

float fresnel(float n1, float n2, float costheta) {
	//not perfect
	float R0 = /*0.3;/*/(n1-n2) / (n1+n2);
	return R0 + (1.0-R0) * pow((1-costheta), 5);
}

double pow(double x, double y);

RGB gammaCorrect(RGB col, float gamma) {
	float r = (float) (col.R);
	float g = (float) (col.G);
	float b = (float) (col.B);
	r = 255.0 * pow(r/255.0, gamma);
	g = 255.0 * pow(g/255.0, gamma);
	b = 255.0 * pow(b/255.0, gamma);
	
	unsigned char R = (unsigned char) (r);
	unsigned char G = (unsigned char) (g);
	unsigned char B = (unsigned char) (b);
	RGB result = {R, G, B};
	
	return result;
}

float unirand() {
	return rand() / RAND_MAX;
}

float degToRad(float deg) {
	return deg * M_PI / 180.0; 
}

float mixFloat(float a, float b, float k) {
	return a * (1.0 - k) + b * k;
}
// polynomial smooth min (k = 0.1);
float smin(float a, float b, float k) {
    float h = clamp(0.5 + 0.5 * (b - a) / k, 0.0, 1.0 );
    return mixFloat(b, a, h) - k * h * (1.0 - h);
}

float smoothstep(float edge0, float edge1, float x) {
	x = clamp( (x-edge0) / (edge1=edge0), 0.0, 1.0);
	return x*x*(3-2*x);
}

float smootherstep(float edge0, float edge1, float x) {
	x = clamp( (x-edge0) / (edge1=edge0), 0.0, 1.0);
	return x*x*x*(x*(x*6 - 15) + 10);
}






