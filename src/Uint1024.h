/*
  Tibia CLient
  Copyright (C) 2019 Saiyans King

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#ifndef __FILE_UINT1024_h_
#define __FILE_UINT1024_h_

#include "defines.h"

#if !defined(HAVE_VSSCANF) || !defined(HAVE_STRTOL) || !defined(HAVE_STRTOUL)  || !defined(HAVE_STRTOLL) || !defined(HAVE_STRTOULL) || !defined(HAVE_STRTOD)
#define SDL_isupperhex(X)   (((X) >= 'A') && ((X) <= 'F'))
#define SDL_islowerhex(X)   (((X) >= 'a') && ((X) <= 'f'))
#endif

#if !defined(HAVE__LTOA) || !defined(HAVE__I64TOA) || !defined(HAVE__ULTOA) || !defined(HAVE__UI64TOA)
static const char ntoa_table[] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
	'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
	'U', 'V', 'W', 'X', 'Y', 'Z'
};
#endif

template <unsigned int BITS>
class base_uint
{
	protected:
		enum {WIDTH = BITS/32};
		Uint32 pn[WIDTH];

	public:
		base_uint()
		{
			memset(pn, 0, sizeof(pn));
		}
		base_uint(const base_uint& b)
		{
			for(Sint32 i = 0; i < WIDTH; ++i)
				pn[i] = b.pn[i];
		}
		base_uint(Uint64 b)
		{
			pn[0] = SDL_static_cast(Uint32, b);
			pn[1] = SDL_static_cast(Uint32, (b >> 32));
			for(Sint32 i = 2; i < WIDTH; ++i)
				pn[i] = 0;
		}

		base_uint& operator=(const base_uint& b)
		{
			for(Sint32 i = 0; i < WIDTH; ++i)
				pn[i] = b.pn[i];
			return *this;
		}

		bool operator!() const
		{
			for(Sint32 i = 0; i < WIDTH; ++i)
				if(pn[i] != 0)
					return false;
			return true;
		}

		const base_uint operator~() const
		{
			base_uint ret;
			for(Sint32 i = 0; i < WIDTH; ++i)
				ret.pn[i] = ~pn[i];
			return ret;
		}

		const base_uint operator-() const
		{
			base_uint ret;
			for(Sint32 i = 0; i < WIDTH; ++i)
				ret.pn[i] = ~pn[i];
			ret++;
			return ret;
		}

		base_uint& operator^=(const base_uint& b)
		{
			for(Sint32 i = 0; i < WIDTH; ++i)
				pn[i] ^= b.pn[i];
			return *this;
		}

		base_uint& operator&=(const base_uint& b)
		{
			for(Sint32 i = 0; i < WIDTH; ++i)
				pn[i] &= b.pn[i];
			return *this;
		}

		base_uint& operator|=(const base_uint& b)
		{
			for(Sint32 i = 0; i < WIDTH; ++i)
				pn[i] |= b.pn[i];
			return *this;
		}

		base_uint& operator+=(const base_uint& b)
		{
			Uint64 carry = 0;
			for(Sint32 i = 0; i < WIDTH; ++i)
			{
				carry += SDL_static_cast(Uint64, pn[i]) + b.pn[i];
				pn[i] = SDL_static_cast(Uint32, carry);
				carry >>= 32;
			}
			return *this;
		}

		base_uint& operator-=(const base_uint& b)
		{
			*this += -b;
			return *this;
		}

		base_uint& operator++()
		{
			Sint32 i = 0;
			while(++pn[i] == 0 && i < WIDTH-1)
				++i;
			return *this;
		}

		const base_uint operator++(int)
		{
			const base_uint ret = *this;
			++(*this);
			return ret;
		}

		base_uint& operator--()
		{
			Sint32 i = 0;
			while(--pn[i] == SDL_static_cast(Uint32, -1) && i < WIDTH-1)
				++i;
			return *this;
		}

		const base_uint operator--(int)
		{
			const base_uint ret = *this;
			--(*this);
			return ret;
		}

		base_uint& operator*=(const base_uint& b)
		{
			base_uint a = *this;
			memset(pn, 0, sizeof(pn));
			for(Sint32 j = 0; j < WIDTH; ++j)
			{
				Uint64 carry = 0;
				for(Sint32 i = 0; i+j < WIDTH; ++i)
				{
					carry += pn[i+j] + SDL_static_cast(Uint64, a.pn[j]) * b.pn[i];
					pn[i+j] = SDL_static_cast(Uint32, carry);
					carry >>= 32;
				}
			}
			return *this;
		}

		base_uint& operator/=(const base_uint& b)
		{
			base_uint div = b;
			base_uint num = *this;
			memset(pn, 0, sizeof(pn));
			Sint32 num_bits = num.bits();
			Sint32 div_bits = div.bits();
			if(div_bits == 0 || div_bits > num_bits)
				return *this;
			Sint32 shift = num_bits-div_bits;
			div <<= shift;
			while(shift >= 0)
			{
				if(num >= div)
				{
					num -= div;
					pn[shift/32] |= (1 << (shift & 31));
				}
				div >>= 1;
				--shift;
			}
			return *this;
		}

		base_uint& operator%=(const base_uint& b)
		{
			base_uint div = b;
			Sint32 num_bits = bits();
			Sint32 div_bits = div.bits();
			if(div_bits == 0 || div_bits > num_bits)
				return *this;
			Sint32 shift = num_bits-div_bits;
			div <<= shift;
			while(shift >= 0)
			{
				if(*this >= div)
					*this -= div;
				div >>= 1;
				--shift;
			}
			return *this;
		}

		base_uint& operator<<=(Sint32 shift)
		{
			base_uint a(*this);
			memset(pn, 0, sizeof(pn));
			Sint32 k = shift / 32;
			shift = shift % 32;
			for(Sint32 i = 0; i < WIDTH; ++i)
			{
				if(i+k+1 < WIDTH && shift != 0)
					pn[i+k+1] |= (a.pn[i] >> (32 - shift));
				if(i+k < WIDTH)
					pn[i+k] |= (a.pn[i] << shift);
			}
			return *this;
		}

		base_uint& operator>>=(Sint32 shift)
		{
			base_uint a(*this);
			memset(pn, 0, sizeof(pn));
			Sint32 k = shift / 32;
			shift = shift % 32;
			for(Sint32 i = 0; i < WIDTH; ++i)
			{
				if(i-k-1 >= 0 && shift != 0)
					pn[i-k-1] |= (a.pn[i] << (32 - shift));
				if(i-k >= 0)
					pn[i-k] |= (a.pn[i] >> shift);
			}
			return *this;
		}

		friend inline const base_uint operator+(const base_uint& a, const base_uint& b) {return base_uint(a) += b;}
		friend inline const base_uint operator-(const base_uint& a, const base_uint& b) {return base_uint(a) -= b;}
		friend inline const base_uint operator*(const base_uint& a, const base_uint& b) {return base_uint(a) *= b;}
		friend inline const base_uint operator/(const base_uint& a, const base_uint& b) {return base_uint(a) /= b;}
		friend inline const base_uint operator%(const base_uint& a, const base_uint& b) {return base_uint(a) %= b;}
		friend inline const base_uint operator|(const base_uint& a, const base_uint& b) {return base_uint(a) |= b;}
		friend inline const base_uint operator&(const base_uint& a, const base_uint& b) {return base_uint(a) &= b;}
		friend inline const base_uint operator^(const base_uint& a, const base_uint& b) {return base_uint(a) ^= b;}
		friend inline const base_uint operator>>(const base_uint& a, Sint32 shift) {return base_uint(a) >>= shift;}
		friend inline const base_uint operator<<(const base_uint& a, Sint32 shift) {return base_uint(a) <<= shift;}
		friend inline const base_uint operator*(const base_uint& a, Uint32 b) {return base_uint(a) *= b;}
		friend inline bool operator==(const base_uint& a, const base_uint& b) {return a.CompareTo(b) == 0;}
		friend inline bool operator!=(const base_uint& a, const base_uint& b) {return a.CompareTo(b) != 0;}
		friend inline bool operator>(const base_uint& a, const base_uint& b) {return a.CompareTo(b) > 0;}
		friend inline bool operator<(const base_uint& a, const base_uint& b) {return a.CompareTo(b) < 0;}
		friend inline bool operator>=(const base_uint& a, const base_uint& b) {return a.CompareTo(b) >= 0;}
		friend inline bool operator<=(const base_uint& a, const base_uint& b) {return a.CompareTo(b) <= 0;}
		friend inline bool operator==(const base_uint& a, Uint64 b) {return a.EqualTo(b);}
		friend inline bool operator!=(const base_uint& a, Uint64 b) {return !a.EqualTo(b);}

		base_uint& operator=(Uint64 b)
		{
			pn[0] = SDL_static_cast(Uint32, b);
			pn[1] = SDL_static_cast(Uint32, (b >> 32));
			for(Sint32 i = 2; i < WIDTH; ++i)
				pn[i] = 0;
			return *this;
		}

		base_uint& operator^=(Uint64 b)
		{
			pn[0] ^= SDL_static_cast(Uint32, b);
			pn[1] ^= SDL_static_cast(Uint32, (b >> 32));
			return *this;
		}

		base_uint& operator|=(Uint64 b)
		{
			pn[0] |= SDL_static_cast(Uint32, b);
			pn[1] |= SDL_static_cast(Uint32, (b >> 32));
			return *this;
		}

		base_uint& operator*=(Uint32 b32)
		{
			Uint64 carry = 0;
			for(Sint32 i = 0; i < WIDTH; ++i)
			{
				carry += SDL_static_cast(Uint64, b32) * pn[i];
				pn[i] = SDL_static_cast(Uint32, carry);
				carry >>= 32;
			}
			return *this;
		}

		Sint32 CompareTo(const base_uint& b) const
		{
			for(Sint32 i = WIDTH-1; i >= 0; --i)
			{
				if(pn[i] < b.pn[i])
					return -1;
				if(pn[i] > b.pn[i])
					return 1;
			}
			return 0;
		}

		bool EqualTo(Uint64 b) const
		{
			for(Sint32 i = 2; i < WIDTH; ++i)
			{
				if(pn[i])
					return false;
			}
			if(pn[1] != (b >> 32))
				return false;
			if(pn[0] != (b & 0xFFFFFFFFul))
				return false;
			return true;
		}

		Uint32 GetLow32() const {return pn[0];}
		Uint64 GetLow64() const {return SDL_static_cast(Uint64, pn[0]) | SDL_static_cast(Uint64, pn[1]) << 32;}

		Sint32 bits() const
		{
			for(Sint32 pos = WIDTH-1; pos >= 0; --pos)
			{
				if(pn[pos])
				{
					for(Sint32 bits = 31; bits > 0; --bits)
					{
						if(pn[pos] & (1 << bits))
							return 32*pos+bits+1;
					}
					return 32*pos+1;
				}
			}
			return 0;
		}

		void fromString(const char* psz, Sint32 radix)
		{
			memset(pn, 0, sizeof(pn));
			if(radix == 16 && SDL_strncmp(psz, "0x", 2) == 0)
				psz += 2;

			while(true)
			{
				Sint32 v;
				if(SDL_isdigit(*psz))
					v = *psz-'0';
				else if(radix == 16 && SDL_isupperhex(*psz))
					v = 10+(*psz-'A');
				else if(radix == 16 && SDL_islowerhex(*psz))
					v = 10+(*psz-'a');
				else
					break;
				*this *= radix;
				*this += base_uint(v);
				++psz;
			}
		}
		void fromString(std::string& str, Sint32 radix) {fromString(str.c_str(), radix);}

		std::string ToString(Sint32 radixs)
		{
			std::string newStr;
			base_uint zero(0), radix(radixs), a(*this);
			while(a > zero)
			{
				char chr = ntoa_table[(a % radix).GetLow32()];
				newStr.insert(newStr.begin(), chr);
				a /= radix;
			}
			return newStr;
		}

		void* getData() {return SDL_reinterpret_cast(void*, &pn[0]);}
		void setData(void* data, Sint32 dataSize) {memcpy(pn, data, dataSize);}
		void importData(Uint8* data)
		{
			for(Sint32 i = 0; i < WIDTH; ++i)
			{
				Uint32 index = i * 4;
				Uint32 value = SDL_static_cast(Uint32, data[index + 3]) | (SDL_static_cast(Uint32, data[index + 2]) << 8)
					| (SDL_static_cast(Uint32, data[index + 1]) << 16) | (SDL_static_cast(Uint32, data[index + 0]) << 24);
				pn[WIDTH-1-i] = value;
			}
		}
		void exportData(Uint8* data)
		{
			for(Sint32 i = 0; i < WIDTH; ++i)
			{
				Uint32 index = (WIDTH-1-i) * 4;
				data[index + 3] = SDL_static_cast(Uint8, pn[i]);
				data[index + 2] = SDL_static_cast(Uint8, pn[i] >> 8);
				data[index + 1] = SDL_static_cast(Uint8, pn[i] >> 16);
				data[index + 0] = SDL_static_cast(Uint8, pn[i] >> 24);
			}
		}
};

class Uint1024 : public base_uint<1024>
{
	public:
		Uint1024() {}
		Uint1024(const base_uint<1024>& b) : base_uint<1024>(b) {}
		Uint1024(Uint64 b) : base_uint<1024>(b) {}
};

template <unsigned int BITS>
const base_uint<BITS> base_uint_powm(base_uint<BITS> a, Uint32 e, base_uint<BITS> m)
{
	base_uint<BITS*2> x(1), y, tmp_m;
	y.setData(a.getData(), BITS/8);
	tmp_m.setData(m.getData(), BITS/8);
	while(e > 0)
	{
		if(e & 1)
			x = (x * y) % tmp_m;

		y = (y * y) % tmp_m;
		e >>= 1;
	}
	a.setData(x.getData(), BITS/8);
	return a;
}

#endif
