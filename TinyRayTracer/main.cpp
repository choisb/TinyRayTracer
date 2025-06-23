#include <vector>
#include <fstream>
#include <cmath>

struct color
{
	float r = 0;
	float g = 0;
	float b = 0;
};

std::ostream& operator<<(std::ostream& strean, const color& color) {
	strean << static_cast<char>(color.r * 255) << static_cast<char>(color.g * 255) << static_cast<char>(color.b * 255);
	return strean;
}

int main() {
	constexpr int width = 1024;
	constexpr int height = 768;

	std::vector<color> framebuffer(width * height);

	for (int i = 0; i < width * height; i++)
	{
		framebuffer[i] = color{ 1.0f, 1.0f ,1.0f };
	}

	std::ofstream stream("./out.ppm", std::ios::binary);
	stream << "P6\n" << width << " " << height << "\n255\n";
	for (color& pixel : framebuffer) {
		stream << pixel;
	}

	return 0;
}