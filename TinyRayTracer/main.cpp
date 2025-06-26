#include <vector>
#include <fstream>
#define _USE_MATH_DEFINES
#include <math.h>
#include <limits>
#include <algorithm>
#include <assert.h>

bool IsNearlyEqual(float a, float b, float epsilon = 1e-3f)
{
	return std::abs(a - b) < epsilon;
}

double DegreesToRadians(double degrees) 
{
	return degrees * M_PI / 180.0;
}

struct Color
{
	Color() = default;
	Color(float inR, float inG, float inB)
		:r(inR), g(inG), b(inB)
	{}
	float r = 0;
	float g = 0;
	float b = 0;

	Color operator*(float f) const { return Color(r * f, g * f, b * f); }

	Color operator+(const Color& c) const { return Color(r + c.r, g + c.g, b + c.b); }
	Color& operator+=(const Color& c) { *this = *this + c; return *this; }

	static const Color ruby;
	static const Color emerald;
	static const Color sapphire;
	static const Color granite;
	static const Color sky;
	static const Color white;
};
Color operator*(float f, const Color& color) { return color * f; }

std::ostream& operator<<(std::ostream& stream, const Color& color) 
{
	auto toByte = [](float c) -> char {
		return static_cast<char>(std::clamp(c, 0.0f, 1.0f) * 255.0f);
	};
	
	stream << toByte(color.r) << toByte(color.g) << toByte(color.b);
	return stream;
}

const Color Color::ruby = { 0.6f, 0.05f, 0.1f };
const Color Color::emerald = { 0.0f, 0.4f, 0.2f };
const Color Color::sapphire = { 0.1f, 0.2f, 0.7f };
const Color Color::granite = { 0.35f, 0.35f, 0.35f };
const Color Color::sky = { 0.52f, 0.68f, 0.92f };
const Color Color::white = { 1.0f, 1.0f, 1.0f };

struct Material
{
	Color diffuseColor = Color::white;
	Color specularColor = Color::white;
	float specularExp = 0.0f; // shininess exponent
	float refractiveIndex = 1.0f;

	float kd = 0.0f; // diffuse coefficient
	float ks = 0.0f; // specular coefficient
	float kr = 0.0f;  // reflection coefficient
	float kt = 0.0f;  // refraction coefficient

	static const Material ruby;
	static const Material emerald;
	static const Material sapphire;
	static const Material matteGranite;
	static const Material mirror;
	static const Material glass;
};

const Material Material::ruby =			{ Color::ruby,		Color::white,	150.0f,  1.0f, 0.5f, 0.9f,	0.1f, 0.0f };
const Material Material::emerald =		{ Color::emerald,	Color::white,	100.0f,  1.0f, 0.6f, 0.8f,	0.1f, 0.0f };
const Material Material::sapphire =		{ Color::sapphire,	Color::white,	200.0f,  1.0f, 0.4f, 1.0f,	0.1f, 0.0f };
const Material Material::matteGranite = { Color::granite,	Color::white,	10.0f,	 1.0f, 0.3f, 0.05f,	0.0f, 0.0f };
const Material Material::mirror =		{ Color::white,		Color::white,	1000.0f, 1.0f, 0.1f, 1.0f,	0.8f, 0.0f };
const Material Material::glass =		{ Color::white,		Color::white,	1000.0f, 1.6f, 0.1f, 1.0f,	0.2f, 0.8f };

struct Vector2
{
	Vector2() = default;
	Vector2(float inX, float inY)
		:x(inX), y(inY)
	{}
	float x = 0;
	float y = 0;
};

struct Vector3
{
	Vector3() = default;
	Vector3(float inX, float inY, float inZ)
		:x(inX), y(inY), z(inZ)
	{}
	float Square() const { return x * x + y * y + z * z; }
	float Length() const { return std::sqrt(Square()); }
	void Normalize() { float len = Length(); if (len > 0.0f) *this /= len; }
	Vector3 Normalized() const { Vector3 v = *this; v.Normalize(); return v; }
	float Dot(const Vector3& other) const { return x * other.x + y * other.y + z * other.z; }

	Vector3 operator*(float f) const { return Vector3(x * f, y * f, z * f); }
	Vector3 operator/(float f) const { return Vector3(x / f, y / f, z / f); }
	Vector3& operator/=(float f) { *this = *this / f; return *this; }

	Vector3 operator-(const Vector3& v) const {	return Vector3(x - v.x, y - v.y, z - v.z); }
	Vector3 operator+(const Vector3& v) const {	return Vector3(x + v.x, y + v.y, z + v.z); }

	float x = 0;
	float y = 0;
	float z = 0;

	static const Vector3 zero;
	static const Vector3 unit;
};
const Vector3 Vector3::zero = Vector3(0.0f, 0.0f, 0.0f);
const Vector3 Vector3::unit = Vector3(1.0f, 1.0f, 1.0f);

Vector3 operator*(float f, const Vector3& v) { return v * f; }
Vector3 operator-(const Vector3& v) { return v * -1.0f; }

struct Ray
{
	Ray(const Vector3& inPos, const Vector3& inDir) : pos(inPos), dir(inDir) {}
	void ApplyPosBias(const Vector3& inNormal, float bias = 1e-3f) { pos = dir.Dot(inNormal) < 0 ? pos - inNormal * bias : pos + inNormal * bias; } // Ray 충돌 검사시 출발 지점과의 충돌을 피해 Bias 적용
	Vector3 pos;
	Vector3 dir;
};

struct Sphere 
{
	Vector3 center = Vector3::zero;
	float radius = 0.0f;
	Material material = Material::matteGranite;
};

struct Light
{
	Vector3 pos = Vector3::zero;
	float intensity = 0;
};

bool IsIntersect(const Ray& ray, const Sphere& sphere, float& outDist)
{
	Vector3 centerToRay = ray.pos - sphere.center;

	// float a = ray.dir.Square(); 항상 1이기 때문에 근의 공식에서 생략.
	assert(IsNearlyEqual(ray.dir.Square(), 1.0f) && "Ray direction must be normalized");
	float b = 2.0f * (centerToRay.Dot(ray.dir));
	float c = centerToRay.Square() - sphere.radius * sphere.radius;

	float discriminant = b * b - 4 * c;

	if (discriminant < 0.0f)
		return false;

	float sqrtDiscriminant = std::sqrt(discriminant);

	// 광선과 구의 교차점 (t가 작을 수록 더 가까운 교차점)
	float t1 = (-b - sqrtDiscriminant) / (2.0f);
	float t2 = (-b + sqrtDiscriminant) / (2.0f);

	// 양수 t만 유효 (광선의 앞쪽에서 교차)
	if (t1 >= 0.0f) 
	{
		outDist = t1;
		return true;
	}
	else if (t2 >= 0.0f) 
	{
		outDist = t2;
		return true;
	}

	return false; // 둘 다 음수면 교차 지점이 광선 뒤에 있음
}

struct HitResult
{
	Vector3 pos = Vector3::zero;
	Vector3 normal = Vector3::unit;
	Material material = Material::matteGranite;
};

struct Scene
{
	std::vector<Sphere> spheres;
	std::vector<Light> lights;
	Color backgroundColor = Color::white;
};

// 카메라 위치 항상 0,0,0 기준.
struct Camera
{
	Camera(float inHorizontalFov, float inVerticalFov)
		:pos(0.0f, 0.0f, 0.0f)
	{
		screenSize.x = static_cast<float>(std::tan(DegreesToRadians(inHorizontalFov * 0.5f)));
		screenSize.y = static_cast<float>(std::tan(DegreesToRadians(inVerticalFov * 0.5f)));
	}

	Vector2 screenSize;
	Vector3 pos;
};

void LoadScene(Scene& outScene)
{
	outScene.spheres = {
		{ Vector3(0,	50,		400),	50.0f,	Material::ruby },
		{ Vector3(150,	200,	700),	150.0f, Material::emerald },
		{ Vector3(-80,	-80,	400),	60.0f,	Material::glass },
		{ Vector3(150,	-100,	500),	80.0f,	Material::matteGranite },
		{ Vector3(-250,	150,	700),	150.0f, Material::mirror },
		{ Vector3(50,	-100,	750),	150.0f, Material::sapphire }
	};

	outScene.lights = {
		{ {-100, 150, 100}, 1.0f},
		{ {0, 500, 300},	1.0f},
		{ {200, 400, 200},	1.0f}
	};

	outScene.backgroundColor = Color::sky;
}

bool CheckIntersect(const Ray& ray, const Scene& scene, HitResult& outHitResult)
{
	const Sphere* intersectedSphere = nullptr;
	float minT = std::numeric_limits<float>::max();
	for (const Sphere& sphere : scene.spheres)
	{
		float t = 0.0f;
		if (IsIntersect(ray, sphere, t))
		{
			if (t < minT)
			{
				minT = t;
				intersectedSphere = &sphere;
			}
		}
	}

	if (intersectedSphere)
	{
		outHitResult.material = intersectedSphere->material;
		outHitResult.pos = ray.pos + (ray.dir * minT);
		outHitResult.normal = (outHitResult.pos - intersectedSphere->center).Normalized();

		return true;
	}
	else
	{
		return false;
	}
}

Vector3 Reflect(const Vector3& inDir, const Vector3& normal) 
{
	return inDir - 2.f * normal * (inDir.Dot(normal));
}

bool Refract(const Vector3& inDir, const Vector3& normal, float refractiveIndex, Vector3& outRefractDir)
{ 
	assert(refractiveIndex > 0 && "refractiveIndex must always be greater than 0.");

	float cosi = - std::clamp(inDir.Dot(normal), -1.0f, 1.0f);
	float etai = 1;
	float etat = refractiveIndex;
	Vector3 n = normal;
	if (cosi < 0) { // if the ray is inside the object, swap the indices and invert the normal to get the correct result
		cosi = -cosi;
		std::swap(etai, etat); 
		n = -normal;
	}

	float eta = etai / etat;
	float k = 1 - eta * eta * (1 - cosi * cosi); 
	if (k < 0)
	{
		return false;
	}
	else
	{
		outRefractDir = (inDir * eta + n * (eta * cosi - sqrtf(k))).Normalized();
		return true;
	}
}

Color CalcLight(const Vector3& viewDir, const Vector3& pos, const Vector3& normal, const Material& material, const Scene& scene)
{
	float diffuseIntensitySum = 0.0f;
	float specularIntensitySum = 0.0f;

	for (const Light& light : scene.lights)
	{
		const Vector3 lightDir = (light.pos - pos).Normalized();
		
		Ray shadowRay(pos, lightDir);
		shadowRay.ApplyPosBias(normal);

		HitResult hitResult;
		if (CheckIntersect(shadowRay, scene, hitResult))
		{
			const float lightDistanceSquare = (light.pos - shadowRay.pos).Square();
			if ((hitResult.pos - shadowRay.pos).Square() < lightDistanceSquare)
			{
				continue;
			}
		}

		diffuseIntensitySum += light.intensity * std::max(0.0f, (lightDir.Dot(normal)));
		const Vector3 outLightDir = Reflect(lightDir, normal).Normalized();
		specularIntensitySum += std::pow(std::max(0.f, outLightDir.Dot(viewDir)), material.specularExp);
	}

	Color result = material.kd * material.diffuseColor * diffuseIntensitySum;
	result += material.ks * material.specularColor * specularIntensitySum;
	return result;
}

Color CastRay(const Ray& ray, const Scene& scene, int depth)
{
	HitResult hitResult;
	if (depth == 0 || !CheckIntersect(ray, scene, hitResult))
	{
		return scene.backgroundColor;
	}

	Color reflectedColor;
	if (hitResult.material.kr > 0.0f)
	{
		const Vector3 reflectionDir = Reflect(ray.dir, hitResult.normal).Normalized();

		Ray reflectionRay(hitResult.pos, reflectionDir);
		reflectionRay.ApplyPosBias(hitResult.normal);

		reflectedColor = hitResult.material.kr * CastRay(reflectionRay, scene, depth - 1);
	}

	Color refractColor;
	if (hitResult.material.kt > 0.0f && hitResult.material.refractiveIndex > 0)
	{
		Vector3 refractionDir;
		
		if(Refract(ray.dir, hitResult.normal, hitResult.material.refractiveIndex, refractionDir))
		{ 
			Ray refractionRay(hitResult.pos, refractionDir);
			refractionRay.ApplyPosBias(hitResult.normal);

			refractColor = hitResult.material.kt * CastRay(refractionRay, scene, depth - 1);
		}
	}

	Color lightColor;
	if (hitResult.material.kd > 0.0f || hitResult.material.ks > 0.0f)
	{
		lightColor = CalcLight(ray.dir, hitResult.pos, hitResult.normal, hitResult.material, scene);
	}

	return  lightColor + refractColor + reflectedColor;
}

int main() 
{
	constexpr int width = 1024;
	constexpr int height = 768;
	constexpr int numPixel = width * height;
	const Camera camera(120.f, 100.f);
	constexpr int rayTracingDepth = 10;

	Scene scene;
	LoadScene(scene);

	std::vector<Color> frameBuffer(numPixel);

	// 좌표계 기준: +x 우측, +y 상단, +z 정면 
	const float hRatio = camera.screenSize.x / width;
	const float vRatio = camera.screenSize.y / height;
	for (int x = 0; x < width; ++x)
	{
		for (int y = 0; y < height; ++y)
		{
			Vector3 rayDir;
			rayDir.x = (x - (width * 0.5f)) * hRatio;
			rayDir.y = (y - (height * 0.5f)) * vRatio * (-1); // frame buffer 기준 +y가 하단이기 때문에 Flip
			rayDir.z = 1.0f;
			rayDir.Normalize();

			const Ray ray(camera.pos, rayDir);
			const int index = y * width + x;
			frameBuffer[index] = CastRay(ray, scene, rayTracingDepth);
		}
	}

	std::ofstream stream("./out.ppm", std::ios::binary);
	stream << "P6\n" << width << " " << height << "\n255\n";
	for (Color& pixel : frameBuffer) 
	{
		stream << pixel;
	}

	stream.close();
	return 0;
}