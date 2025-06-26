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
	Color operator+(float f) const { return Color(r + f, g + f, b + f); }

	static const Color red;
	static const Color green;
	static const Color blue;
	static const Color gray;
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

const Color Color::red = { 1.0f, 0.0f, 0.0f };
const Color Color::green = { 0.0f, 1.0f, 0.0f };
const Color Color::blue = { 0.0f, 0.0f, 1.0f };
const Color Color::gray = { 0.3f, 0.3f, 0.3f };

struct Material
{
	Color diffuseColor;
	float specularExp = 50.0f; // shininess exponent
	float kd = 1.0f; // diffuse coefficient
	float ks = 0.5f; // specular coefficient

	static const Material Ruby;
	static const Material Emerald;
	static const Material Sapphire;
	static const Material MatteGranite;
};

const Material Material::Ruby =			{ Color(0.6f, 0.05f, 0.1f),		150.0f, 0.5f, 0.9f };
const Material Material::Emerald =		{ Color(0.0f, 0.4f, 0.2f),		100.0f, 0.6f, 0.8f };
const Material Material::Sapphire =		{ Color(0.1f, 0.2f, 0.7f),		200.0f, 0.4f, 1.0f };
const Material Material::MatteGranite = { Color(0.35f, 0.35f, 0.35f),	10.0f,	0.3f, 0.05f };

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
	void Normalize() { float len = Length(); if(len > 0.0f) *this /= len; }
	float Dot(const Vector3& other) const { return x * other.x + y * other.y + z * other.z; }

	Vector3 operator*(float f) const { return Vector3(x * f, y * f, z * f); }
	Vector3 operator/(float f) const { return Vector3(x / f, y / f, z / f); }
	Vector3& operator/=(float f) { *this = *this / f; return *this; }

	Vector3 operator-(const Vector3& v) const {	return Vector3(x - v.x, y - v.y, z - v.z); }
	Vector3 operator+(const Vector3& v) const {	return Vector3(x + v.x, y + v.y, z + v.z); }

	float x = 0;
	float y = 0;
	float z = 0;
};

struct Ray
{
	Vector3 pos;
	Vector3 dir;
};

struct Sphere 
{
	Vector3 center;
	float radius;
	Material material;
};

struct Light
{
	Vector3 Pos;
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
	Vector3 pos;
	Vector3 normal;
	Material material;
};

struct Scene
{
	std::vector<Sphere> spheres;
	std::vector<Light> lights;
	Color backgroundColor;
};

// 카메라 위치 항상 0,0,0 기준.
struct Camera
{
	Camera(float inScreenDistance, float inHorizontalFov, float inVerticalFov)
		: screenDistance(inScreenDistance), pos(0.0f, 0.0f, 0.0f)
	{
		screenSize.x = static_cast<float>(std::tan(DegreesToRadians(inHorizontalFov * 0.5f))) * screenDistance;
		screenSize.y = static_cast<float>(std::tan(DegreesToRadians(inVerticalFov * 0.5f))) * screenDistance;
	}

	float screenDistance;
	Vector2 screenSize;
	Vector3 pos;
};

void LoadScene(Scene& outScene)
{
	outScene.spheres = {
		{ Vector3(0,    0, 600),	200.0f, Material::Ruby },
		{ Vector3(100,  200, 800),	200.0f, Material::Emerald },
		{ Vector3(-300, -100, 700), 100.0f, Material::Sapphire },
		{ Vector3(200, -150, 650),	150.0f, Material::MatteGranite }
	};

	outScene.lights = {
		{ {-100, 150, 100}, 1.0f},
		{ {0, 500, 300},	1.0f},
		{ {200, 400, 200},	1.0f}
	};

	outScene.backgroundColor = Color::gray;
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
		outHitResult.normal = outHitResult.pos - intersectedSphere->center;
		outHitResult.normal.Normalize();

		return true;
	}
	else
	{
		return false;
	}
}

Vector3 Reflect(const Vector3& lightDir, const Vector3& normal) {
	return lightDir - normal * 2.f * (lightDir.Dot(normal));
}
Color CalcLight(const Vector3& viewDir, const Vector3& pos, const Vector3& normal, const Material& material, const std::vector<Light>& lights)
{
	float diffuseIntensity = 0.0f;
	float specularLightIntensity = 0.0f;

	for (const Light& light : lights)
	{
		Vector3 lightDir = light.Pos - pos;
		lightDir.Normalize();
		diffuseIntensity += light.intensity * std::max(0.0f, (lightDir.Dot(normal)));
		specularLightIntensity += std::pow(std::max(0.f, Reflect(lightDir, normal).Dot(viewDir)), material.specularExp);
	}
	return (material.kd * material.diffuseColor * diffuseIntensity) + (material.ks * specularLightIntensity);
}
Color CastRay(const Ray& ray, const Scene& scene)
{
	HitResult hitResult;
	if (CheckIntersect(ray, scene, hitResult) == false)
	{
		return scene.backgroundColor;
	}

	return CalcLight(ray.dir, hitResult.pos, hitResult.normal, hitResult.material, scene.lights);
}

int main() 
{
	constexpr int width = 1024;
	constexpr int height = 768;
	constexpr int numPixel = width * height;
	const Camera camera(100.f, 120.f, 100.f);

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
			const int index = y * width + x;
			Ray ray;
			ray.dir.x = (x - (width  * 0.5f)) * hRatio;
			ray.dir.y = (y - (height * 0.5f)) * vRatio * (-1); // frame buffer 기준 +y가 하단이기 때문에 Flip
			ray.dir.z = camera.screenDistance;
			ray.dir.Normalize();
			ray.pos = camera.pos;

			frameBuffer[index] = CastRay(ray, scene);
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