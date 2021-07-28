/*
  The Forgotten Client
  Copyright (C) 2020 Saiyans King

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

#ifndef __FILE_RSA_h_
#define __FILE_RSA_h_

#include "defines.h"

#if defined(__linux__)
	#include <gmp.h>
#else
	#include "Uint1024.h"
#endif


class RSA
{
	public:
		RSA();

		// non-copyable
		RSA(const RSA&) = delete;
		RSA& operator=(const RSA&) = delete;

		// non-moveable
		RSA(RSA&&) = delete;
		RSA& operator=(RSA&&) = delete;

		void setKey(const char* publicKey);
		void encrypt(Uint8* msg);

	private:
	#if defined(__linux__)
		mpz_t m_mod, m_e;
	#else
		Uint1024 m_mod;
		Uint32 m_e = 65537;
	#endif
};

#endif /* __FILE_RSA_h_ */
