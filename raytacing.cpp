#version 400

in vec3 dir;
out vec4 outcolour;

uniform mat4 mMatrix;
uniform mat4 mvMatrix;
uniform mat4 mvMatrixScene;
uniform mat4 pMatrix;
uniform mat3 normalMatrix; //mv matrix without translation

const int raytraceDepth = 1;
const int numSpheres = 6;

vec4 ambient = vec4(0, 0, 0, 1);
vec4 diffuse;
vec4 specular = vec4(1, 1, 1, 1);
float shininess = 15;
float ambientCoefficent = 1;
float diffuseCoefficent = 1;
float specularCoefficent = 0.2;

vec3 lightPosition_camSpace = vec3(6, 4, 3); //light Position in camera space

//example data structures
struct Ray {
    vec3 origin;
    vec3 dir;
};
struct Sphere {
    vec3 centre;
    float radius;
    vec3 colour;
};
struct Plane {
    vec3 point;
    vec3 normal;
    vec3 colour;
};

struct Intersection {
    float t; //closest hit
    vec3 point; // hit point
    vec3 normal; // normal
    int hit; //did it hit?
    vec3 colour; // colour accumulation, can be also implemented in struct Ray
};
//Compute light
vec4 light(vec3 pos, vec3 normal)
{
    // Distance
    float dsquared = pow(length(vec3(lightPosition_camSpace - pos)), 2);

    //Light direction
    vec3 l = normalize(vec3(lightPosition_camSpace - pos));
    //normal
    vec3 n = normalize(normal);

    // ============================Diffuse coeeficent ==========================
    float diffuseN = diffuseCoefficent * max(dot(l, n), 0);
    // =============================Specular coefficient ===========================
    vec3 v = normalize(-vec3(pos));
    //GSLS reflect command, from object to light source, so need to flip
    vec3 r = reflect(-l, n);

    float specularN = specularCoefficent * pow(max(dot(v, r), 0), shininess);
    float attenuation = 1000 / (4 * 3.14 * dsquared);
    return (diffuseN * diffuse + specularN * specular) * attenuation + ambient * ambientCoefficent;
}

void shpere_intersect(Sphere sph, Ray ray, inout Intersection intersect)
{
    vec3 d_p = ray.origin - sph.centre;
    float discriminant = pow(dot(ray.dir, d_p), 2) - pow(length(d_p), 2) + pow(sph.radius, 2);
    // If the discriminant is non-negative, there is an intersection
    if (discriminant >= 0.0f) {
        float mu = min(dot(-ray.dir, ray.origin - sph.centre) + sqrt(discriminant), dot(-ray.dir, ray.origin - sph.centre) - sqrt(discriminant));
        vec3 new_intersect = ray.origin + mu * ray.dir;
        float dist = distance(ray.origin, new_intersect);
        intersect.hit = 0;
        if (dist < intersect.t || intersect.t == 0) {
            intersect.t = dist;
            intersect.point = new_intersect;
            intersect.normal = normalize((intersect.point - sph.centre) / sph.radius);
            intersect.hit = 1;
            intersect.colour = sph.colour;
        }
    }
}

void plane_intersect(Plane pl, Ray ray, inout Intersection intersect)
{
    float mu = -dot((ray.origin - pl.point), pl.normal) / dot(ray.dir, pl.normal);

    if (mu > 0) {
        intersect.hit = 1;

        intersect.point = ray.origin + mu * ray.dir;
        float dist = distance(ray.origin, intersect.point);

        float x = mod(intersect.point.x, 2.);
        float y = mod(intersect.point.y, 2.);
        float z = mod(intersect.point.z, 2.);
        vec3 col = vec3(1, 1, 1);
        if (x > 1 && z > 1 || (x < 1 && z < 1)) {
            col = vec3(0, 0, 0);
        }

        intersect.hit = 0;
        //dist < intersect.t || intersect.t == 0
        if (dist < intersect.t || intersect.t == 0) {
            intersect.t = dist;
            intersect.point = ray.origin + mu * ray.dir;
            intersect.hit = 1;
            intersect.normal = pl.normal;
            intersect.colour = col;
        }
    }
}

Sphere sphere[numSpheres];
Plane plane;
void Intersect(Ray r, inout Intersection inter)
{
    plane_intersect(plane, r, inter);
    for (int i = 0; i < numSpheres; i++) {
        shpere_intersect(sphere[i], r, inter);
    }
}


int seed = 0;
float rnd()
{
    seed = int(mod(float(seed) * 1364.0 + 626.0, 509.0));
    return float(seed) / 509.0;
}

bool computeShadow(in Intersection intersect)
{
	Intersection temp;
	// Where it hits
	// Now check if we can reach light source
	vec3 ShadowVec = (lightPosition_camSpace- intersect.point  );
	Ray r;

	// Note: This was the best implementation I could find,
	// For some reason moving it along(+)the normal gave me incorrect results
	// I also negated the intersection, i.e. if it intersected I output the object's colours
	// And if no intersection I output black, which seems to work
	// I think it's due to the sphere intersecting itself, but I was unable to fix it
	r.origin = intersect.point -(0.05) *intersect.normal;
	r.dir=normalize(ShadowVec);
	temp.hit = 0;
	temp.t = 0;
	Intersect(r,temp);
	if (temp.hit ==1){
		return true;
	}


	return false;

}
vec3 reflect(vec3 dir, vec3 n)
{
    return dir - dot(2 * dir, n) * n;
    //float cosI = -dot(n, dir);
    // return dir + 2.0*cosI*n;
}
void main()
{
    //please leave the scene config unaltered for marking

     sphere[0].centre = vec3(-2.0, 1.5, -3.5);
    sphere[0].radius = 1.5;
    sphere[0].colour = vec3(0.8, 0.8, 0.8);
    sphere[1].centre = vec3(-0.5, 0.0, -2.0);
    sphere[1].radius = 0.6;
    sphere[1].colour = vec3(0.3, 0.8, 0.3);
    sphere[2].centre = vec3(1.0, 0.7, -2.2);
    sphere[2].radius = 0.8;
    sphere[2].colour = vec3(0.3, 0.8, 0.8);
    sphere[3].centre = vec3(0.7, -0.3, -1.2);
    sphere[3].radius = 0.2;
    sphere[3].colour = vec3(0.8, 0.8, 0.3);
    sphere[4].centre = vec3(-0.7, -0.3, -1.2);
    sphere[4].radius = 0.2;
    sphere[4].colour = vec3(0.8, 0.3, 0.3);
    sphere[5].centre = vec3(0.2, -0.2, -1.2);
    sphere[5].radius = 0.3;
    sphere[5].colour = vec3(0.8, 0.3, 0.8);

    plane.point = vec3(0, -0.5, 0);
    plane.normal = vec3(0, 1.0, 0);
    plane.colour = vec3(1, 1, 1);
    seed = int(mod(dir.x * dir.y * 39786038.0, 65536.0));
    //scene definition end

Ray ray;
//Note since view matrix starts at -40, need to scroll downwards until z = 0 to see correctly
// I have added an offset to fix this so it starts where it should be
mat4 mvMatrixOffset = mvMatrixScene;
mvMatrixOffset[3].z = mvMatrixOffset[3].z +40;
vec4 mouseOrigin = (mvMatrixOffset * vec4(0,0,0,1));
vec4 mouseDir = mvMatrixOffset * vec4(dir, 0);


    ray.origin = vec3(mouseOrigin.xyz) / mouseOrigin.w;
    ray.dir = normalize(mouseDir.xyz);
    Intersection i;
    i.hit = 0;
    i.t = 0;
    vec4 sumColour;
    for (int j = 0; j <raytraceDepth; j++) {

        Intersect(ray, i);
        if (i.hit != 0) {
            if (j == 0) {

				if(computeShadow(i)) {
				// Refer to notes in compute shadow function
					diffuse = vec4(i.colour, 1);
				}
            	else {
					diffuse = vec4(0,0,0,0);

				}
                sumColour = light(i.point, i.normal);
            }
            else {
				//reflected colour
                diffuse = vec4(i.colour, 1);
                sumColour += light(i.point, i.normal)*0.5;
            }
        }
        ray.origin = i.point;
        ray.origin += 0.001 * i.normal;
        ray.dir = normalize(reflect(ray.dir, i.normal));
		i.hit=0;
    }
    outcolour = sumColour;
}
