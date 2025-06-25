#include <vector>
#include <fstream>
#define _USE_MATH_DEFINES
#include <math.h>
#include <limits>
#include <algorithm>

double degreesToRadians(double degrees) {
	return degrees * M_PI / 180.0;
}

struct Color
{
	float r = 0;
	float g = 0;
	float b = 0;

	static const Color Red;
	static const Color Green;
	static const Color Blue;
	static const Color Yellow;
	static const Color Magenta;
	static const Color Cyan;
	static const Color White;
	static const Color Black;
	static const Color Gray;
};

std::ostream& operator<<(std::ostream& stream, const Color& color) {
	auto toByte = [](float c) -> char {
		return static_cast<char>(std::clamp(c, 0.0f, 1.0f) * 255.0f);
		};
	
	stream << toByte(color.r) << toByte(color.g) << toByte(color.b);
	return stream;
}

const Color Color::Red = { 1.0f, 0.0f, 0.0f };
const Color Color::Green = { 0.0f, 1.0f, 0.0f };
const Color Color::Blue = { 0.0f, 0.0f, 1.0f };
const Color Color::Yellow = { 1.0f, 1.0f, 0.0f };
const Color Color::Magenta = { 1.0f, 0.0f, 1.0f };
const Color Color::Cyan = { 0.0f, 1.0f, 1.0f };
const Color Color::White = { 1.0f, 1.0f, 1.0f };
const Color Color::Black = { 0.0f, 0.0f, 0.0f };
const Color Color::Gray = { 0.3f, 0.3f, 0.3f };

struct Vector2
{
	Vector2() {}
	Vector2(float inX, float inY)
		:x(inX), y(inY)
	{}
	float x = 0;
	float y = 0;
};

struct Vector3
{
	Vector3() {}
	Vector3(float inX, float inY, float inZ)
		:x(inX), y(inY), z(inZ)
	{}
	float Square() const { return x * x + y * y + z * z; }
	float Length() const { return std::sqrt(Square()); }
	void Normalize() { float len = Length(); if(len > 0.0f) *this /= len; }
	float Dot(const Vector3& other) const { return x * other.x + y * other.y + z * other.z; }

	Vector3 operator/(float f) const { return Vector3(x / f, y / f, z / f); }
	Vector3& operator/=(float f) { *this = *this / f; return *this; }
	Vector3 operator-(const Vector3& other) const {	return Vector3(x - other.x, y - other.y, z - other.z);}

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
	Sphere(const Vector3& inCenter, float inRadius, const Color& inColor)
		:center(inCenter), radius(inRadius), color(inColor)
	{}
	Vector3 center;
	float radius;
	Color color;
};

bool IsIntersect(const Ray& ray, const Sphere& sphere, float& outDist)
{
	Vector3 centerToRay = ray.pos - sphere.center;

	// float a = ray.dir.Square(); ray.dir.Square() 는 항상 1이기 때문에 근의 공식에서 생략.
	float b = 2.0f * centerToRay.Dot(ray.dir);
	float c = centerToRay.Square() - sphere.radius * sphere.radius;

	float discriminant = b * b - 4 * c;

	if (discriminant < 0.0f)
		return false;

	float sqrtDiscriminant = std::sqrt(discriminant);

	// 광선과 구의 교차점 (t가 작을 수록 더 가까운 교차점)
	float t1 = (-b - sqrtDiscriminant) / (2.0f);
	float t2 = (-b + sqrtDiscriminant) / (2.0f);

	// 양수 t만 유효 (광선의 앞쪽에서 교차)
	if (t1 >= 0.0f) {
		outDist = t1;
		return true;
	}
	else if (t2 >= 0.0f) {
		outDist = t2;
		return true;
	}

	return false; // 둘 다 음수면 교차 지점이 광선 뒤에 있음
}

struct Scene
{
	std::vector<Sphere> spheres;
};

// 카메라 위치 항상 0,0,0 기준.
struct Camera
{
	Camera(float inFocalLength, float inHorizontalFov, float inVerticalFov)
		: focalLength(inFocalLength), pos(0.0f, 0.0f, 0.0f)
	{
		screenSize.x = static_cast<float>(std::tan(degreesToRadians(inHorizontalFov * 0.5f))) * focalLength;
		screenSize.y = static_cast<float>(std::tan(degreesToRadians(inVerticalFov * 0.5f))) * focalLength;
	}

	float focalLength;
	Vector2 screenSize;
	Vector3 pos;
};

void LoadScene(Scene& outScene)
{
	outScene.spheres = {
		{ {0, 0, 600}, 200, Color::Red },
		{ {100, 200, 800}, 200, Color::Green },
		{ {-300, -100, 700}, 100, Color::Blue }
	};
}

Color CastRay(const Ray& ray, const Scene& scene)
{
	const Sphere* intersectSphere = nullptr;
	float minT = std::numeric_limits<float>::max();
	for (const Sphere& sphere : scene.spheres)
	{
		float t = std::numeric_limits<float>::max();
		if (IsIntersect(ray, sphere, t))
		{
			if (t < minT)
			{
				minT = t;
				intersectSphere = &sphere;
			}
		}
	}

	if (intersectSphere)
	{
		return intersectSphere->color;
	}
	else
	{
		return Color::Gray;
	}
}

int main() {
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
			ray.dir.z = camera.focalLength;
			ray.dir.Normalize();
			ray.pos = camera.pos;

			frameBuffer[index] = CastRay(ray, scene);
		}
	}

	std::ofstream stream("./out.ppm", std::ios::binary);
	stream << "P6\n" << width << " " << height << "\n255\n";
	for (Color& pixel : frameBuffer) {
		stream << pixel;
	}

	return 0;
}