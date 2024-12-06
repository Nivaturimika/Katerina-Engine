#include <cstdint>
#include "color.hpp"

namespace ogl {
	// borrowed from http://www.burtleburtle.net/bob/hash/doobs.html
	uint32_t color_from_hash(uint32_t color) {
		uint32_t m1 = 0x1337CAFE;
		uint32_t m2 = 0xDEADBEEF;
		m1 -= m2;
		m1 -= color;
		m1 ^= (color >> 13);
		m2 -= color;
		m2 -= m1;
		m2 ^= (m1 << 8);
		color -= m1;
		color -= m2;
		color ^= (m2 >> 13);
		m1 -= m2;
		m1 -= color;
		m1 ^= (color >> 12);
		m2 -= color;
		m2 -= m1;
		m2 ^= (m1 << 16);
		color -= m1;
		color -= m2;
		color ^= (m2 >> 5);
		m1 -= m2;
		m1 -= color;
		m1 ^= (color >> 3);
		m2 -= color;
		m2 -= m1;
		m2 ^= (m1 << 10);
		color -= m1;
		color -= m2;
		color ^= (m2 >> 15);
		return color;
	}

	uint32_t color_gradient(float percent, uint32_t top_color, uint32_t bot_color) {
		uint32_t color = 0;
		for(uint32_t i = 0; i <= 16; i += 8) {
			auto diff = int32_t(top_color >> i & 0xFF) - int32_t(bot_color >> i & 0xFF);
			color |= uint32_t(int32_t(bot_color >> i & 0xFF) + diff * percent) << i;
		}
		return color;
	}

	// CC0 from matplotlib - https://github.com/BIDS/colormap/blob/master/colormaps.py
	uint32_t gradient_viridis(float x) {
		//(-3.9741952365322133*x^4 + 10.34582562913701*x^3 - 6.5526233193290189*x^2 + 0.90723692672422241*x + 0.267004)/(2.2204460492503131e-15*x^4 - 4.1078251911130793e-15*x^3 + 3.3306690738754696e-15*x^2 - 7.4940054162198071e-16*x + 1.0)
		static float coeff_r[] = {
			-3.9741952365322133,
			+10.34582562913701,
			-6.5526233193290189,
			+0.90723692672422241,
			+0.267004,
			// div
			+2.2204460492503131e-15,
			-4.1078251911130793e-15,
			+ 3.3306690738754696e-15,
			-7.4940054162198071e-16,
			+1.0
		};
		//(-1.3646031386525587*x^4 + 2.5789437805030973*x^3 - 1.9187321592388973*x^2 + 1.6056745173883582*x + 0.0048739999999999996)/(2.2204460492503131e-15*x^4 - 4.1078251911130793e-15*x^3 + 3.3306690738754696e-15*x^2 - 7.4940054162198071e-16*x + 1.0)
		static float coeff_g[] = {
			-1.3646031386525587,
			+2.5789437805030973,
			-1.9187321592388973,
			+1.6056745173883582,
			+0.0048739999999999996,
			//div
			+2.2204460492503131e-15,
			-4.1078251911130793e-15,
			+3.3306690738754696e-15,
			-7.4940054162198071e-16,
			+1.0
		};
		//1.0*(0.86215771119943041*x^4 - 0.94878618510330309*x^3 - 1.3441167454446439*x^2 + 1.2452662193485168*x + 0.32941499999999999)/(2.2204460492503131e-15*x^4 - 4.1078251911130793e-15*x^3 + 3.3306690738754696e-15*x^2 - 7.4940054162198071e-16*x + 1.0)
		static float coeff_b[] = {
			0.86215771119943041, //x4
			-0.94878618510330309, //x3
			-1.3441167454446439, //x2
			+1.2452662193485168, //x1
			+0.32941499999999999, //x0
			//div
			2.2204460492503131e-15,
			-4.1078251911130793e-15,
			+3.3306690738754696e-15,
			-7.4940054162198071e-16,
			1.0
		};
		//from 0 to 1
		float x2 = x * x;
		float x3 = x2 * x;
		float x4 = x3 * x;
		auto color = ogl::color3f{
			.r = (coeff_r[0] * x4 + coeff_r[1] * x3 + coeff_r[2] * x2 + coeff_r[3] * x + coeff_r[4])
				/ (coeff_r[5] * x4 + coeff_r[6] * x3 + coeff_r[7] * x2 + coeff_r[8] * x + coeff_r[9]),
			.g = (coeff_g[0] * x4 + coeff_g[1] * x3 + coeff_g[2] * x2 + coeff_g[3] * x + coeff_g[4])
				/ (coeff_g[5] * x4 + coeff_g[6] * x3 + coeff_g[7] * x2 + coeff_g[8] * x + coeff_g[9]),
			.b = (coeff_b[0] * x4 + coeff_b[1] * x3 + coeff_b[2] * x2 + coeff_b[3] * x + coeff_b[4])
				/ (coeff_b[5] * x4 + coeff_b[6] * x3 + coeff_b[7] * x2 + coeff_b[8] * x + coeff_b[9]),
		};
		return (uint8_t(color.r * 255.f) << 0)
			| (uint8_t(color.g * 255.f) << 8)
			| (uint8_t(color.b * 255.f) << 16);
	}
}
