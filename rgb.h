struct RGB {
	unsigned char R;
	unsigned char G;
	unsigned char B;
};
typedef struct RGB RGB;

struct HSV {
	double H;
	double S;
	double V;
};
typedef struct HSV HSV;

RGB scaleRGB(RGB col, float scale) {
	
	col.R *= scale;
	if (col.R > 255)
		col.R = 255;
	else if (col.R < 0)
		col.R = 0;
	
	col.G *= scale;
	if (col.G > 255)
		col.G = 255;
	else if (col.G < 0)
		col.G = 0;
	
	col.B *= scale;
	if (col.B > 255)
		col.B = 255;
	else if (col.B < 0)
		col.B = 0;
	return col;
}

float min(float a, float b);

RGB addRGB(RGB a, RGB b) {
	a.R = min(a.R + b.R, 255);
	a.G = min(a.G + b.G, 255);
	a.B = min(a.B + b.B, 255);
	return a;
}
RGB multRGB(RGB a, RGB b) {
	a.R = a.R*b.R/255.0;
	a.G = a.G*b.G/255.0;
	a.B = a.B*b.B/255.0;
	return a;
}
RGB clampRGB(RGB col) {
	col.R = col.R > 255 ? 255 : col.R;
	col.G = col.G > 255 ? 255 : col.G;
	col.B = col.B > 255 ? 255 : col.B;
	return col;
}