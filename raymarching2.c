#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "vec3.h"
#include "rgb.h"
#include "perlin.h"
#include "funcs.h"
#include "colours.h"

#define INF 10000


int width = 400, height = 400;
float tmin = 0.000001;
float tmax = 500.0;
float stepScale = 250.0;
float stepDelta = 0.001;


double totalt = 0.0;
float biggestt = 0.0;
long int totalsteps = 0;
int biggeststeps = 0;
float totalshadowt = 0;

long int totalR = 0, totalG = 0, totalB = 0;

//SCENE
//valley:			7.5		y 6.5 is good
vec3 cameraPos = {20.0, 5.5, 3.7};  //use {5.5, 6.6, 3.8} for valley, also no terrain falloff;
//float theta; //angle between look-at and horizontal
//vec3 cameraPos = {3.0, 6.5, 4.0};
//light
vec3 sunPos = {100, 100, 1000};

float waterHeight = 4.0;
float grassHeight = 4.6;
float grassSteepness = 0.4;
float snowHeight = 5.2;
float snowSteepness = 0.3;
float fogHeight = 3.8; //fog scale height

vec3 windDir = {-1.0, 0.0, -0.2};
float windSteepness = 0.25;



//FUNCTIONS
float terrain(float x, float z);
vec3 normal(vec3 point, float delta);
vec3 castRay(const vec3 ro, const vec3 rd, const float stepScale);
RGB addFog(RGB result, vec3 point, vec3 ro, vec3 rd, float t);
RGB getMaterial(vec3 point, vec3 ro, vec3 rd, float t, vec3 norm, int depth);
RGB colour(vec3 point, vec3 ro, vec3 rd, float t, int depth);
RGB render(int x, int y);
//end




float terrain0(float u, float v) {
	//return waterHeight - 0.001;
	///shift everything far to the right, so the symmetry is hidden
	float heightMod = 0.0;
	float dist = u*u + v*v;
	if ((dist) > 25)
		heightMod = 0.1*pow(dist, 0.5);
	if (dist > 2500)
		heightMod = 100.0;
	//heightMod = 0;
	float height = (3.0 * perlin2d(fabs(u-1000.0), v, 0.5, 8) //8
					+ 6.0 * perlin2d(fabs(u-1000.0), v, 0.15, 1)) - heightMod;
	
	if (height < waterHeight) {
		height = waterHeight - 0.0055 + 0.005 * (pow(perlin2d(fabs(((u-1000.0)/5.0)), v, 20.0, 2), 1)
												+ 0.15 * pow(perlin2d(fabs(((u-1000.0)/3.0)), v, 80.0, 2), 1));
										//;//+ 0.0005 * (sin(100.0 * (v-25.0) * pow(pow(u-7.5, 2) + pow(v-12.0, 2), 0.5) 
										//			+ 0.3*perlin2d(fabs(u-1000.0), v, 2.0, 1))); 
				//+ 0.01*perlin2d(fabs(u-1000.0), v, 6.0, 1)); //extra minus sinks the sin
		//height = waterHeight - 0.05001 + 0.05 * perlin2d(fabs(u-1000.0), v, 25.0, 1);
		//height = waterHeight-0.001;
		}
	return height;
}

float terrain(float u, float v) {
	return terrain0(u, v);
}
vec3 normal(const vec3 point, const float delta) {
	//find xdir vector
	vec3 pointx1 = {point.x + delta, terrain(point.x + delta, point.z), point.z};
	vec3 pointx2 = {point.x - delta, terrain(point.x - delta, point.z), point.z};
	vec3 xdir = subtract(pointx2, pointx1);
	
	//ydir
	vec3 pointy1 = {point.x, terrain(point.x, point.z + delta), point.z + delta};
	vec3 pointy2 = {point.x, terrain(point.x, point.z - delta), point.z - delta};
	vec3 ydir = subtract(pointy2, pointy1);
	
	vec3 norm = cross(ydir, xdir);
	
	return normalise(norm);
}
/*
this is an old version, more accurate but hugely slower - about 100x so. If you 
have the CPU time to spare, go ahead and use it. 
vec3 castRay(const vec3 ro, const vec3 rd, const float stepScale) {
	//returns hitpoint
	float t;
	int ctr = 0;
	float step = stepDelta;
	vec3 p;
	float height;
	for (t=tmin; t<tmax; t+= step) {
		totalsteps++;
		ctr++;
		p = plus(ro, scale(rd, t));
		height = terrain(p.x, p.z);
		if (p.y < height) {
			p = plus(p, scale(rd, -step/ 2.0));
			p.y = height;
			if(ctr > biggeststeps)
				biggeststeps = ctr;	
			return p;
		}
		//after first, scale stepDelta for further objects -> faster rendering
		//massive speed increase, good resolution
		if (t > 0.0)
			step = t / stepScale;
	}
	//if it never hit anything, return INF distance
	vec3 result = plus(ro, scale(rd, 1.01*tmax));
	return result;
}
*/
vec3 castRay(const vec3 ro, const vec3 rd, const float stepScale) {
	float t = tmin;
	int i;
	vec3 pos;
	//float maxHeight = 10.0;
	//bastardised SDF
	float close = 0;//0.000001;
	int maxSteps = 5000;
	float step = 0;
	float mod = 1.0;
	
	for (i=0; i<maxSteps; i++) {
		totalsteps++;
		pos = plus(ro, scale(rd, t));
		float height = pos.y - terrain(pos.x, pos.z);
		
		if (height < close*t && height > -1.0 * close*t) {
			t -= fabs(step)*1.5;
			break;
		}
		if (t > tmax) {
			t = 2.0*tmax;
			break;
		}
		if (height < -close*t) {
			mod = 0.5;
		}
		step = mod * height;
		t += step;
	}
	pos = plus(ro, scale(rd, t));
	
	return pos;
}

RGB reflectedCol(const vec3 point, const vec3 ro, const vec3 rd, const float t, const vec3 norm, const int depth) {
	RGB reflectCol = {0, 0, 0};;
	if (depth < 2) {
		vec3 newPoint = subtract(point, scale(rd, max(stepDelta, t/stepScale/2.0))); //point stepped back a little
		vec3 reflectDir = normalise(reflect(rd, norm));
		vec3 reflectPoint = castRay(newPoint, reflectDir, stepScale/1.5); //less accuracy for reflection
		float reflectT = modulus(subtract(newPoint, reflectPoint));
		
		reflectCol = colour(reflectPoint, newPoint, reflectDir, reflectT, depth);
		//reflectCol = addFog(reflectCol, reflectPoint, newPoint, reflectDir, reflectT);
	}
	
	return reflectCol;
}

RGB addFog(RGB col, const vec3 point, const vec3 ro, const vec3 rd, const float t) {
	//arbitrary constants that work
	//for mountains: 
	/*
	float a = 0.01; //linear fogginess
	float b = 2.9; //fog scale height
	float c = 2500.0;
	*/
	//for ocean:
	float a = 0.01;
	float b = 2.0;
	float c = 0;
	
	float fogAmount = (1.0 - exp(-t*a));
	if (rd.y != 0)
		fogAmount += c * exp(-b * ro.y) * (1.0 - exp(-b * rd.y * t)) / rd.y;
	fogAmount = clamp(fogAmount, 0.0, 0.9);
	
	//go yellow near sun
	vec3 sunDir = normalise(subtract(sunPos, ro));
	//does ray hit anythin? if so, no sunshine
	float sunAmount = 0.0;
	if (t > 0.99*tmax)
		sunAmount = dot(sunDir, rd);
	sunAmount = clamp(pow(sunAmount, 400.0) * 1.6, 0.0, 1.0);
	
	//add red sky for low sun
	vec3 sunHorizontal = {sunDir.x, 0.0, sunDir.z};	//find how high sun is
	sunHorizontal = normalise(sunHorizontal); //and how high ray is
	vec3 rayHorizontal = {rd.x, 0.0, rd.z};
	rayHorizontal = normalise(rayHorizontal);
	
	float sunHeight = clamp(pow(dot(sunDir, sunHorizontal), 12), 0.0, 1.0);
	float rayHeight = clamp(pow(dot(rayHorizontal, rd), 7), 0.0, 1.0);
	float sunStreaks = 0;
	if (point.y < waterHeight && t > 0.99*tmax)
		sunStreaks = -0.04;
	else if (t > 0.99*tmax)																		//adjust for longer distances
		sunStreaks = -0.1 * pow(rayHeight, 10) 
					* perlin2d(8.0*pow(100.0/tmax, 2.5)*point.x+1000000.0, point.y*100.0*rayHeight*pow(100.0/tmax, 2), 0.02, 1);
	float sunsetAmount = clamp(sunHeight * rayHeight + sunStreaks, 0.0, 1.0);
	RGB newSunsetCol = sunsetCol;
	//remove green for lower sun
	//if (rd.y < 0.0 && t > 0.99*tmax)
	newSunsetCol.G = 255.0 * clamp(pow(1.0 - sunsetAmount, 0.4), 0.0, 1.0);
	RGB fogCol;
	fogCol = mix(fogBlue, newSunsetCol, sunsetAmount*sunsetAmount);
	fogCol = mix(fogCol, sunCol, sunAmount);
	fogCol.B /= 1.4 * clamp(pow(sunHeight, 2), 0.7, 1.0);
	
	col = mix(col, fogCol, fogAmount);
	
	/*
	//darken at top when sun low
	float skyDark = 1.0;
	if (rd.y > 0.0 && t > 0.9*tmax) {
		skyDark = dot(sunDir, sunHorizontal) * pow(1.0 - dot(rayHorizontal, rd), 2);
		skyDark = 1.0 + 10 * pow(skyDark, 0.6);
		col = scaleRGB(col, 1.0 / skyDark);
		col.G /= 1.0 + (1.0 - rayHeight)*sunHeight * 1.3;
	}
	*/
	return col;
}

RGB getMaterial(const vec3 point, const vec3 ro, const vec3 rd, const float t, const vec3 norm, const int depth) {
	
	float steep = upness(norm);
	//water
	if (point.y <= (waterHeight)) {
		RGB result;
		
		//only water gets reflection
		//vec3 sunDir = normalise(subtract(sunPos, point));
		
		float k = clamp(fresnel(1.0, 1.33, dot(scale(rd, -1.0), norm)), 0.0, 1.0);
		RGB reflectCol = reflectedCol(point, ro, rd, t, norm, depth+1); //max 1 reflection
		
		
		//							viewer dir		reflected ray dir
		//float spec = clamp(dot(scale(rd, -1.0), reflect(sunDir, norm)), 0.0, 1.0);
		//reflectCol = scaleRGB(reflectCol, spec);
		//fprintf(stderr, "%f\n", spec);
		result = /*reflectCol;//mix(waterCol, reflectCol, k);/*/mix(waterCol, reflectCol, k);
		return result;
	}
	
	float newGrassHeight = grassHeight - 0.15 * steep - 0.3 * perlin2d(fabs(point.x+2000.0), point.z, 0.6, 1);
	if (point.y < newGrassHeight && steep > grassSteepness) {
		return mix(grassCol, earthCol, clamp(1-pow(steep, 0.5), 0.0, 1.0));
	}
	//snow
	
	//here, attempt to get snow
	//into valleys, by lowering sH if flat
	
	float newSnowHeight = snowHeight - 0.3*steep - 0.5*perlin2d(fabs(point.x-1000.0), point.z, 0.6, 1);
	//windiness!
	vec3 newNorm = {norm.x, 0.0, norm.z}; //removes y-component of norm
	newNorm = normalise(newNorm);
	float wind = dot(windDir, newNorm);
	
	if ((point.y > newSnowHeight) && steep > snowSteepness && wind > windSteepness) {
		return snowCol;
	}
	//otherwise, grey-brown
	return mix(earthCol, stoneGrey, 0.5+0.5*(1-steep));
}

RGB colour(const vec3 point, const vec3 ro, const vec3 rd, const float t, const int depth) {
	RGB result;
	//render sky:
	if (t >= tmax) {
		result = skyCol;
	}
	else {
		//render all else
		//delta is arbitrary, smaller values look awful, dunno why, bigger looks smoother
		vec3 norm = normal(point, 0.001);
		//very cheap occlusion
		float occ = 0.5 + clamp(upness(norm), 0.0, 0.5);
		
		//better:
		RGB matCol = getMaterial(point, ro, rd, t, norm, depth);
		
		
		//sunlight
		
		//first shadows
		//scale back point to avoid self-intersection
		vec3 newPoint = /*point;/*/subtract(point, scale(rd, 0.01));
		
		vec3 sunDir = normalise(subtract(sunPos, newPoint));
		vec3 shadow = castRay(newPoint, sunDir, stepScale/5.0); //less accuracy for shadows
		float sunAmount = 0.0;
		float shadowDist = modulus(subtract(shadow, newPoint));
		
		if (shadowDist < 0.99*tmax)
			totalshadowt += shadowDist;
		float softShadow = clamp((shadowDist-5.0) / 5.0, 0.0, 1.0);
		
		
		if (shadowDist > 0.99*tmax) {
			if (point.y > waterHeight)
				sunAmount = softShadow * clamp(dot(norm, sunDir), 0.0, 1.0);
			else {
				//water gets full light, properly handled inside getMaterial()
				sunAmount = 1.0;
			}
		}
		result = scaleRGB(multRGB(matCol, sunCol), sunAmount);
		
		//result = matCol;
		//skylighting, blueish
		
		float sky = occ * 0.3;
		result = addRGB(result, multRGB(matCol, scaleRGB(skyCol, sky)));
		//indirect/ambient
		//TEMP, change back to 0.2
		vec3 reversedSunDir = {sunDir.x*-1.0, 0.0, sunDir.z*-1.0};
		float ambient = occ * 0.2 * clamp(dot(norm, normalise(reversedSunDir)), 0.0, 1.0);
		result = addRGB(result, scaleRGB(matCol, ambient));
		
	}
	//add fog:
	//if (depth == 0)
	result = addFog(result, point, ro, rd, t);
		
	return result;
}
 
RGB render(const int x, const int y) {
	float aspectRatio = width / height;
	float fovV = degToRad(90.0);//M_PI / 4.0 ;//90.0;
	
	//doesn't work
	float fovH = aspectRatio * fovV;//(4.0*aspectRatio*atan(tan(aspectRatio / (fovV / 2.0))));
	/*
	float dx = x - width/2.0;
	float dy = height/2.0 - y;
	float dz = (height/2.0) / tan(fov / 2.0);
	
	vec3 ro = cameraPos;
	vec3 rd = {dx, dy, dz};
	rd = subtract(normalise(rd), cameraPos);
	*/
	float dx = (2.0 * (x + 0.5) / (float) width - 1.0) * aspectRatio * sin(fovH / 2.0);
	float dy = (1.0 - 2.0 * (y + 0.5) / (float) height) * sin(fovV / 2.0);
	float dz = 1.0;//0.5 * pow(6.0 - dx*dx - dy*dy, 0.5);
	vec3 ro = cameraPos;
	vec3 screenPoint = {dx, dy, dz}; //point on "screen" in world coords
	screenPoint = normalise(screenPoint);
	
	vec3 rd = screenPoint;
	
	float t;
	vec3 point;
	//cheating, but faster
	
	if (y < 0/*height * (1.0/5.0)*/) {
		t = 2.0*tmax;
		point = plus(ro, scale(rd, t));
	}
	
	else {
		point = castRay(ro, rd, stepScale);//cameraPos, dir);
		t = modulus(subtract(point, cameraPos));
	}
	if (t < (1.1*tmax)) {
		totalt += t;
		if (t > biggestt)
			biggestt = t;
	}
	
	return /*gammaCorrect(*/colour(point, ro, rd, t, 0);//, 1.0/2.2);
}

int main() {
	//srand( (unsigned int) 0);
	
	//print ppm header
	printf("P3\n");
	printf("%d %d\n", width, height);
	printf("255\n");
	
	//for printing progress
	//how many prints
	float doneStep = 100.0;
	int val = (int)(width * height / doneStep);
	int done = 0, ctr = 0;
	
	int i, j;
	for (j=0; j < height; j++)  {
		for (i=0; i < width; i++) {
			
			RGB col = render(i, j);
			
			totalR += col.R;
			totalG += col.G;
			totalB += col.B;
			
			printf("%d %d %d ", col.R, col.G, col.B);
			
			//print out to stderr increments of 100/doneStep % completion
			if (ctr++ %  val == 0) {
				fprintf(stderr, "%d%s done\n", done, "%");
				done += 100.0/doneStep;
			}
		}
		printf("\n");
	}
	float pixels = (float) width * (float) height;
	float meant	= totalt / pixels;
	float meansteps = (float) totalsteps / pixels;
	
	fprintf(stderr, "100%s done\n\n", "%");
	fprintf(stderr, "total distance: %e\nmean distance: %f\nlargest distance: %f\n\n", totalt, meant, biggestt);
	fprintf(stderr, "total steps: %e\nmean steps: %f\nlargest steps: %d\n\n", (double) totalsteps, meansteps, biggeststeps);
	fprintf(stderr, "mean R: %f\nmean G: %f\nmean B: %f\n\n", (float) totalR / pixels, 
			(float) totalG / pixels, (float) totalB / pixels);
	fprintf(stderr, "total shadow distance: %f\nmean shadow distance: %f\n\n", totalshadowt, totalshadowt / pixels);
	return 0;
}