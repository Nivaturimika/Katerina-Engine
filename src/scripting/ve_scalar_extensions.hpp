#pragma once

#include "ve.hpp"

namespace ve {
	template<typename T>
	inline T select(bool v, T a, T b) {
		return v ? a : b;
	}

	inline float max(float a, float b) {
		return a > b ? a : b;
	}

	inline float min(float a, float b) {
		return a < b ? a : b;
	}

	// Both ceil and floor are taken from glibc
	// https://www.gnu.org/software/libc/
	// https://sourceware.org/git/?p=glibc.git;a=blob;f=sysdeps/ieee754/flt-32/s_floorf.c;h=a5c38818f1f23ad495f560c673b41796511f9363;hb=HEAD
	// https://sourceware.org/git/?p=glibc.git;a=blob;f=sysdeps/ieee754/flt-32/s_ceilf.c;h=0263552deeda45fa05152a7ae49d911c622d3c20;hb=HEAD
	inline float ceil(float x) {
		/* Use generic implementation.  */
		int32_t i0 = *reinterpret_cast<const int32_t*>(&x);
		int32_t j0 = ((i0 >> 23) & 0xff) - 0x7f;
		if (j0 < 23) {
			if (j0 < 0) {
				/* return 0 * sign (x) if |x| < 1  */
				if (i0 < 0) {
					i0 = 0x80000000;
				} else if (i0 != 0) {
					i0 = 0x3f800000;
				}
			} else {
				uint32_t i = (0x007fffff) >> j0;
				if ((i0 & i) == 0) {
					return x;		/* x is integral  */
				}
				if (i0 > 0) {
					i0 += (0x00800000) >> j0;
				}
				i0 &= (~i);
			}
		} else {
			if (j0 != 0x80) { /* x is integral  */
				return x;
			}
			return x + x; /* inf or NaN  */
		}
		return *reinterpret_cast<const float*>(&i0);
	}

	inline float floor(float x) {
		/* Use generic implementation.  */
		int32_t i0 = *reinterpret_cast<const int32_t*>(&x);
		int32_t j0 = ((i0 >> 23) & 0xff) - 0x7f;
		if(j0 < 23) {
			if(j0 < 0) {
				/* return 0 * sign (x) if |x| < 1  */
				if (i0 >= 0) {
					i0 = 0;
				} else if ((i0 & 0x7fffffff) != 0) {
					i0 = 0xbf800000;
				}
			} else {
				uint32_t i = (0x007fffff) >> j0;
				if ((i0 & i) == 0) {
					return x;		/* x is integral  */
				}
				if (i0 < 0) {
					i0 += (0x00800000) >> j0;
				}
				i0 &= (~i);
			}
		} else {
			if (j0 != 0x80) { /* x is integral  */
				return x;
			}
			return x + x; /* inf or NaN  */
		}
		return *reinterpret_cast<const float*>(&i0);
	}

	inline float to_float(int32_t a) {
		return float(a);
	}
	inline float to_float(uint32_t a) {
		return float(a);
	}
	inline float to_float(uint16_t a) {
		return float(a);
	}
	inline float to_float(int16_t a) {
		return float(a);
	}
	inline float to_float(uint8_t a) {
		return float(a);
	}
	inline float to_float(int8_t a) {
		return float(a);
	}

	inline int32_t to_int(int32_t a) {
		return int32_t(a);
	}
	inline int32_t to_int(uint32_t a) {
		return int32_t(a);
	}
	inline int32_t to_int(uint16_t a) {
		return int32_t(a);
	}
	inline int32_t to_int(int16_t a) {
		return int32_t(a);
	}
	inline int32_t to_int(uint8_t a) {
		return int32_t(a);
	}
	inline int32_t to_int(int8_t a) {
		return int32_t(a);
	}
	inline int32_t to_int(float a) {
		return int32_t(a);
	}

	inline bool compress_mask(bool v) {
		return v;
	}

	template<typename tag_type>
	mask_vector operator==(contiguous_tags_base<tag_type> a, tag_type b) {
		return a == tagged_vector<tag_type>(b);
	}
	template<typename tag_type>
	mask_vector operator!=(contiguous_tags_base<tag_type> a, tag_type b) {
		return a != tagged_vector<tag_type>(b);
	}

	template<typename tag_type>
	mask_vector operator==(tag_type b, contiguous_tags_base<tag_type> a) {
		return a == tagged_vector<tag_type>(b);
	}
	template<typename tag_type>
	mask_vector operator!=(tag_type b, contiguous_tags_base<tag_type> a) {
		return a != tagged_vector<tag_type>(b);
	}

} // namespace ve
