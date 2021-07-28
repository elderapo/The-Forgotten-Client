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

#include "rsa.h"

RSA::RSA()
{
	#if defined(__linux__)
		mpz_init2(m_mod, 1024);
		mpz_init2(m_e, 1024);
		mpz_set_str(m_mod, CLIENT_RSA_KEY, 10);
		mpz_set_str(m_e, "65537", 10);
	#else
		m_mod.fromString(CLIENT_RSA_KEY, 10);
	#endif
}

void RSA::setKey(const char* publicKey)
{
	#if defined(__linux__)
		mpz_set_str(m_mod, publicKey, 10);
	#else
		m_mod.fromString(publicKey, 10);
	#endif
}

void RSA::encrypt(Uint8* msg)
{
	#if defined(__linux__)
		mpz_t plain, c;
		mpz_init2(plain, 1024);
		mpz_init2(c, 1024);

		mpz_import(plain, 128, 1, 1, 0, 0, msg);
		mpz_powm(c, plain, m_e, m_mod);

		size_t count = (mpz_sizeinbase(c, 2) + 7) / 8;
		memset(msg, 0, 128 - count);
		mpz_export(&msg[128 - count], NULL, 1, 1, 0, 0, c);

		mpz_clear(c);
		mpz_clear(plain);
	#else
		Uint1024 plain;
		plain.importData(msg);

		Uint1024 encrypted = base_uint_powm<1024>(plain, m_e, m_mod);
		encrypted.exportData(msg);
	#endif
}
