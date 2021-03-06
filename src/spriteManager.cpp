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

#include "spriteManager.h"
#include "game.h"
#include "json/json.h"
#include "lzma/LzmaLib.h"

SpriteManager g_spriteManager;

extern Game g_game;
extern Uint32 g_datRevision;
extern Uint32 g_sprRevision;
extern Uint32 g_spriteCounts;

SpriteManager::SpriteManager()
{
	m_cachedSprites = NULL;
	m_sprLoaded = false;
}

SpriteManager::~SpriteManager()
{
	unloadSprites();
}

void SpriteManager::unloadSprites()
{
	if(m_cachedSprites)
	{
		SDL_free(m_cachedSprites->hidden.mem.base);//Our pointer saved in rwops - let's free it
		SDL_RWclose(m_cachedSprites);
		m_cachedSprites = NULL;
	}
	for(std::map<Uint32, SpriteData>::iterator it = m_spriteData.begin(), end = m_spriteData.end(); it != end; ++it)
	{
		SpriteData& spriteData = it->second;
		if(spriteData.data)
			SDL_free(spriteData.data);
	}
	m_spriteData.clear();
	m_spriteSheets.clear();
	m_spriteOffsets.clear();
	m_sprLoaded = false;
}

void SpriteManager::manageSprites(std::vector<Uint32>& fromSprites, std::vector<Uint32>& toSprites, Uint8& width, Uint8& height)
{
	if(fromSprites.empty())
		return;
	if(fromSprites[0] < m_spriteOffsets.size())
	{
		SpriteOffset& currentOffset = m_spriteOffsets[fromSprites[0]];
		switch(currentOffset.spriteType)
		{
			case 0:
			{
				width = 1;
				height = 1;
			}
			break;
			case 1:
			{
				width = 1;
				height = 2;
			}
			break;
			case 2:
			{
				width = 2;
				height = 1;
			}
			break;
			case 3:
			{
				width = 2;
				height = 2;
			}
			break;
		}
	}
	for(std::vector<Uint32>::iterator it = fromSprites.begin(), end = fromSprites.end(); it != end; ++it)
	{
		Uint32 currentSprite = (*it);
		if(currentSprite < m_spriteOffsets.size())
		{
			SpriteOffset& currentOffset = m_spriteOffsets[currentSprite];
			switch(currentOffset.spriteType)
			{
				case 0:
				{
					toSprites.push_back(currentOffset.spriteIndex);
				}
				break;
				case 1:
				{
					toSprites.push_back(currentOffset.spriteIndex + 0);
					toSprites.push_back(currentOffset.spriteIndex + 1);
				}
				break;
				case 2:
				{
					toSprites.push_back(currentOffset.spriteIndex + 0);
					toSprites.push_back(currentOffset.spriteIndex + 1);
				}
				break;
				case 3:
				{
					toSprites.push_back(currentOffset.spriteIndex + 0);
					toSprites.push_back(currentOffset.spriteIndex + 1);
					toSprites.push_back(currentOffset.spriteIndex + 2);
					toSprites.push_back(currentOffset.spriteIndex + 3);
				}
				break;
			}
		}
	}
}

unsigned char* SpriteManager::LoadSpriteSheet_BMP(const std::string& spriteFile, size_t& outputSize)
{
	#if CLIENT_OVVERIDE_VERSION > 0
	SDL_snprintf(g_buffer, sizeof(g_buffer), "%s%s%c%s", g_basePath.c_str(), ASSETS_CATALOG, PATH_PLATFORM_SLASH, spriteFile.c_str());
	#else
	SDL_snprintf(g_buffer, sizeof(g_buffer), "%s%s%c%u%c%s", g_basePath.c_str(), ASSETS_CATALOG, PATH_PLATFORM_SLASH, g_clientVersion, PATH_PLATFORM_SLASH, spriteFile.c_str());
	#endif
	SDL_RWops* fp = SDL_RWFromFile(g_buffer, "rb");
	if(!fp)
		return NULL;

	SDL_RWseek(fp, 0, RW_SEEK_END);
	size_t sizeData = SDL_static_cast(size_t, SDL_RWtell(fp));
	if(sizeData <= 43)
	{
		SDL_RWclose(fp);
		return NULL;
	}

	unsigned char* srcData = SDL_reinterpret_cast(unsigned char*, SDL_malloc(sizeData));
	if(!srcData)
	{
		SDL_RWclose(fp);
		return NULL;
	}

	SDL_RWseek(fp, 0, RW_SEEK_SET);
	SDL_RWread(fp, srcData, 1, sizeData);
	SDL_RWclose(fp);

	//Cipsoft specific header
	size_t position = 30;
	while((srcData[position++] & 0x80) == 0x80) {}

	//LZMA file
	position += 13;
	unsigned char* destData = SDL_reinterpret_cast(unsigned char*, SDL_malloc(1048576));
	if(!destData)
	{
		SDL_free(srcData);
		return NULL;
	}

	SizeT procOutSize = 1048576, procInSize = SDL_static_cast(SizeT, sizeData - position);
	Sint32 status = LzmaUncompress(destData, &procOutSize, &srcData[position], &procInSize, &srcData[position - 13], LZMA_PROPS_SIZE);
	if(status == SZ_OK)
	{
		outputSize = SDL_static_cast(size_t, procOutSize);
		SDL_free(srcData);
		return destData;
	}
	SDL_free(destData);
	SDL_free(srcData);
	return NULL;
}

unsigned char* SpriteManager::SplitSpriteSheet(SDL_Surface* sheet, Sint32 x, Sint32 y)
{
	unsigned char* destData = SDL_reinterpret_cast(unsigned char*, SDL_malloc(4096));
	if(destData)
	{
		Uint32* dp = SDL_reinterpret_cast(Uint32*, SDL_reinterpret_cast(Uint8*, sheet->pixels) + (y*sheet->w + x) * 4);
		Sint32 dgap = (sheet->pitch / 4);
		for(Sint32 h = 0; h < 32; h += 4)
		{
			UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, destData + ((h + 0) * 128)), SDL_reinterpret_cast(const Uint8*, dp), 128); dp += dgap;
			UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, destData + ((h + 1) * 128)), SDL_reinterpret_cast(const Uint8*, dp), 128); dp += dgap;
			UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, destData + ((h + 2) * 128)), SDL_reinterpret_cast(const Uint8*, dp), 128); dp += dgap;
			UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, destData + ((h + 3) * 128)), SDL_reinterpret_cast(const Uint8*, dp), 128); dp += dgap;
		}
		return destData;
	}
	return NULL;
}

bool SpriteManager::LoadSpriteSheet(Uint32 spriteId, bool bgra)
{
	for(std::vector<SpriteSheet>::iterator it = m_spriteSheets.begin(), end = m_spriteSheets.end(); it != end; ++it)
	{
		SpriteSheet& spriteSheet = (*it);
		if(spriteId >= spriteSheet.firstSpriteId && spriteId <= spriteSheet.lastSpriteId)
		{
			size_t bmpSize = 0;
			unsigned char* bmpData = LoadSpriteSheet_BMP(spriteSheet.spriteFile, bmpSize);
			if(!bmpData)
				return false;

			SDL_Surface* bmpSurface = SDL_LoadBMP_RW(SDL_RWFromMem(bmpData, SDL_static_cast(Sint32, bmpSize)), 1);
			SDL_free(bmpData);
			if(!bmpSurface)
				return false;

			Uint32 wantedFormat = (bgra ? SDL_PIXELFORMAT_BGRA32 : SDL_PIXELFORMAT_RGBA32);
			if(wantedFormat != bmpSurface->format->format)
			{
				SDL_Surface* s = SDL_ConvertSurfaceFormat(bmpSurface, wantedFormat, SDL_SWSURFACE);
				SDL_FreeSurface(bmpSurface);
				if(!s)
					return false;

				bmpSurface = s;
			}

			Uint32 currentSprite = spriteSheet.firstSpriteId;
			Uint32 spriteCount = currentSprite+(spriteSheet.lastSpriteId-spriteSheet.firstSpriteId);
			switch(spriteSheet.spriteType)
			{
				case 0://1x1
				{
					for(Sint32 y = 0; y < 12; ++y)
					{
						for(Sint32 x = 0; x < 12; ++x)
						{
							if(currentSprite > spriteCount)
								goto Exit_Nest_Loop;

							SpriteData newSpriteData0;
							newSpriteData0.data = SplitSpriteSheet(bmpSurface, x*32, y*32);
							newSpriteData0.bgra = bgra;
							m_spriteData[currentSprite++] = newSpriteData0;
						}
					}
				}
				break;
				case 1://1x2
				{
					for(Sint32 y = 0; y < 12; y += 2)
					{
						for(Sint32 x = 0; x < 12; ++x)
						{
							if(currentSprite > spriteCount)
								goto Exit_Nest_Loop;

							SpriteData newSpriteData0, newSpriteData1;
							newSpriteData0.data = SplitSpriteSheet(bmpSurface, x*32, (y+1)*32);
							newSpriteData0.bgra = bgra;
							m_spriteData[currentSprite++] = newSpriteData0;
							newSpriteData1.data = SplitSpriteSheet(bmpSurface, x*32, y*32);
							newSpriteData1.bgra = bgra;
							m_spriteData[currentSprite++] = newSpriteData1;
						}
					}
				}
				break;
				case 2://2x1
				{
					for(Sint32 y = 0; y < 12; ++y)
					{
						for(Sint32 x = 0; x < 12; x += 2)
						{
							if(currentSprite > spriteCount)
								goto Exit_Nest_Loop;

							SpriteData newSpriteData0, newSpriteData1;
							newSpriteData0.data = SplitSpriteSheet(bmpSurface, (x+1)*32, y*32);
							newSpriteData0.bgra = bgra;
							m_spriteData[currentSprite++] = newSpriteData0;
							newSpriteData1.data = SplitSpriteSheet(bmpSurface, x*32, y*32);
							newSpriteData1.bgra = bgra;
							m_spriteData[currentSprite++] = newSpriteData1;
						}
					}
				}
				break;
				case 3://2x2
				{
					for(Sint32 y = 0; y < 12; y += 2)
					{
						for(Sint32 x = 0; x < 12; x += 2)
						{
							if(currentSprite > spriteCount)
								goto Exit_Nest_Loop;

							SpriteData newSpriteData0, newSpriteData1, newSpriteData2, newSpriteData3;
							newSpriteData0.data = SplitSpriteSheet(bmpSurface, (x+1)*32, (y+1)*32);
							newSpriteData0.bgra = bgra;
							m_spriteData[currentSprite++] = newSpriteData0;
							newSpriteData1.data = SplitSpriteSheet(bmpSurface, x*32, (y+1)*32);
							newSpriteData1.bgra = bgra;
							m_spriteData[currentSprite++] = newSpriteData1;
							newSpriteData2.data = SplitSpriteSheet(bmpSurface, (x+1)*32, y*32);
							newSpriteData2.bgra = bgra;
							m_spriteData[currentSprite++] = newSpriteData2;
							newSpriteData3.data = SplitSpriteSheet(bmpSurface, x*32, y*32);
							newSpriteData3.bgra = bgra;
							m_spriteData[currentSprite++] = newSpriteData3;
						}
					}
				}
				break;
			}
			Exit_Nest_Loop:
			SDL_FreeSurface(bmpSurface);
			return true;
		}
	}
	return false;
}

unsigned char* SpriteManager::LoadSprite_NEW(Uint32 spriteId, bool bgra)
{
	//Not the best solution - let's think of better way of caching later
	std::map<Uint32, SpriteData>::iterator it = m_spriteData.find(spriteId);
	if(it == m_spriteData.end())
	{
		if(!LoadSpriteSheet(spriteId, bgra))
			return false;

		it = m_spriteData.find(spriteId);
	}
	SpriteData& spriteData = it->second;
	if(!spriteData.data)
		return NULL;
	if(spriteData.bgra != bgra)
	{
		for(Sint32 i = 0; i < 4096; i += 4)
		{
			#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			unsigned char temp1 = spriteData.data[1];
			unsigned char temp2 = spriteData.data[3];
			spriteData.data[1] = temp2;
			spriteData.data[3] = temp1;
			#else
			unsigned char temp1 = spriteData.data[0];
			unsigned char temp2 = spriteData.data[2];
			spriteData.data[0] = temp2;
			spriteData.data[2] = temp1;
			#endif
		}
		spriteData.bgra = bgra;
	}
	unsigned char* newSpriteData = SDL_reinterpret_cast(unsigned char*, SDL_malloc(4096));
	if(newSpriteData)
	{
		UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, newSpriteData), SDL_reinterpret_cast(const Uint8*, spriteData.data), 4096);
		return newSpriteData;
	}
	return NULL;
}

unsigned char* SpriteManager::LoadSprite_RGBA(Uint32 spriteId)
{
	SDL_RWops* sprites = m_cachedSprites;
	if(!sprites)
		return LoadSprite_NEW(spriteId, false);

	unsigned char* pixels = SDL_reinterpret_cast(unsigned char*, SDL_malloc(4096));
	if(!pixels)
		return NULL;

	memset(pixels, 0x00, 4096);

	Sint64 offset = (g_game.hasGameFeature(GAME_FEATURE_EXTENDED_SPRITES) ? 8 : 6);
	SDL_RWseek(sprites, (spriteId-1)*4+offset, RW_SEEK_SET);

	Uint32 sprLoc = SDL_ReadLE32(sprites);
	SDL_RWseek(sprites, sprLoc, RW_SEEK_SET);
	SDL_RWseek(sprites, 3, RW_SEEK_CUR);//ignore colorkey

	Uint16 sprSize = SDL_ReadLE16(sprites);
	if(sprSize == 0)
		return pixels;

	Uint32 writeData = 0, readData = 0;
	Uint16 numPix;
	bool state = false;
	while(readData < sprSize)
	{
		numPix = SDL_ReadLE16(sprites);
		readData += 2;
		if(state)
		{
			for(Uint16 i = 0; i < numPix && writeData <= 4092; ++i)
			{
				pixels[writeData] = SDL_ReadU8(sprites);
				pixels[writeData+1] = SDL_ReadU8(sprites);
				pixels[writeData+2] = SDL_ReadU8(sprites);
				#if defined(__ALPHA_SPRITES__)
				pixels[writeData+3] = SDL_ReadU8(sprites);
				readData += 4;
				#else
				pixels[writeData+3] = SDL_ALPHA_OPAQUE;
				readData += 3;
				#endif
				writeData += 4;
			}
			state = false;
		}
		else
		{
			writeData += numPix*4;
			state = true;
		}
	}
	return pixels;
}

unsigned char* SpriteManager::LoadSprite_BGRA(Uint32 spriteId)
{
	SDL_RWops* sprites = m_cachedSprites;
	if(!sprites)
		return LoadSprite_NEW(spriteId, true);

	unsigned char* pixels = SDL_reinterpret_cast(unsigned char*, SDL_malloc(4096));
	if(!pixels)
		return NULL;

	memset(pixels, 0x00, 4096);

	Sint64 offset = (g_game.hasGameFeature(GAME_FEATURE_EXTENDED_SPRITES) ? 8 : 6);
	SDL_RWseek(sprites, (spriteId-1)*4+offset, RW_SEEK_SET);

	Uint32 sprLoc = SDL_ReadLE32(sprites);
	SDL_RWseek(sprites, sprLoc, RW_SEEK_SET);
	SDL_RWseek(sprites, 3, RW_SEEK_CUR);//ignore colorkey

	Uint16 sprSize = SDL_ReadLE16(sprites);
	if(sprSize == 0)
		return pixels;

	Uint32 writeData = 0, readData = 0;
	Uint16 numPix;
	bool state = false;
	while(readData < sprSize)
	{
		numPix = SDL_ReadLE16(sprites);
		readData += 2;
		if(state)
		{
			for(Uint16 i = 0; i < numPix && writeData <= 4092; ++i)
			{
				pixels[writeData+2] = SDL_ReadU8(sprites);
				pixels[writeData+1] = SDL_ReadU8(sprites);
				pixels[writeData] = SDL_ReadU8(sprites);
				#if defined(__ALPHA_SPRITES__)
				pixels[writeData+3] = SDL_ReadU8(sprites);
				readData += 4;
				#else
				pixels[writeData+3] = SDL_ALPHA_OPAQUE;
				readData += 3;
				#endif
				writeData += 4;
			}
			state = false;
		}
		else
		{
			writeData += numPix*4;
			state = true;
		}
	}
	return pixels;
}

bool SpriteManager::loadSprites(const char* filename)
{
	SDL_RWops* sprites = SDL_RWFromFile(filename, "rb");
	if(!sprites)
		return false;

	SDL_RWseek(sprites, 0, RW_SEEK_END);
	Sint32 sizeSprites = SDL_static_cast(Sint32, SDL_RWtell(sprites));
	unsigned char* data = SDL_reinterpret_cast(unsigned char*, SDL_malloc(sizeSprites));
	if(!data)
	{
		SDL_RWseek(sprites, 0, RW_SEEK_SET);
		m_cachedSprites = sprites;
		m_sprLoaded = true;
		return true;
	}

	SDL_RWseek(sprites, 0, RW_SEEK_SET);
	SDL_RWread(sprites, data, 1, SDL_static_cast(size_t, sizeSprites));
	SDL_RWclose(sprites);
	
	m_cachedSprites = SDL_RWFromMem(data, sizeSprites);
	if(!m_cachedSprites)
	{
		SDL_free(data);
		return false;
	}
	g_sprRevision = SDL_ReadLE32(m_cachedSprites);
	g_spriteCounts = (g_game.hasGameFeature(GAME_FEATURE_EXTENDED_SPRITES) ? SDL_ReadLE32(m_cachedSprites) : SDL_static_cast(Uint32, SDL_ReadLE16(m_cachedSprites)));
	m_sprLoaded = true;
	return true;
}

bool SpriteManager::loadCatalog(const char* filename)
{
	Uint32 startSpriteIndex = 1;
	SDL_RWops* fp = SDL_RWFromFile(filename, "rb");
	if(!fp)
		return false;

	SDL_RWseek(fp, 0, RW_SEEK_END);
	size_t sizeData = SDL_static_cast(size_t, SDL_RWtell(fp));
	if(sizeData == 0)
		return false;

	char* msgData = SDL_reinterpret_cast(char*, SDL_malloc(sizeData));
	if(!msgData)
		return false;

	SDL_RWseek(fp, 0, RW_SEEK_SET);
	SDL_RWread(fp, msgData, 1, sizeData);
	SDL_RWclose(fp);

	JSON_VALUE* catalogJson = JSON_VALUE::decode(msgData);
	SDL_free(msgData);
	if(catalogJson && catalogJson->IsArray())
	{
		size_t catalogSize = catalogJson->size();
		for(size_t i = 0; i < catalogSize; ++i)
		{
			JSON_VALUE* fileJson = catalogJson->find(i);
			if(fileJson && fileJson->IsObject())
			{
				JSON_VALUE* fileType = fileJson->find("type");
				JSON_VALUE* fileName = fileJson->find("file");
				if(fileType && fileName && fileType->IsString() && fileName->IsString())
				{
					const std::string& fileTypeName = fileType->AsString();
					if(fileTypeName == "appearances")
					{
						g_datPath = fileName->AsString();

						JSON_VALUE* fileVersion = fileJson->find("version");
						if(fileVersion && fileVersion->IsNumber())
							g_datRevision = SDL_static_cast(Uint32, fileVersion->AsNumber());
						else
							g_datRevision = 0x64434654;
					}
					else if(fileTypeName == "staticdata")
					{
						//save staticdata filename to parse it later
					}
					else if(fileTypeName == "map")
					{
						//save map filename to parse it later
					}
					else if(fileTypeName == "sprite")
					{
						JSON_VALUE* spriteTypeJson = fileJson->find("spritetype");
						JSON_VALUE* firstSpriteIdJson = fileJson->find("firstspriteid");
						JSON_VALUE* lastSpriteIdJson = fileJson->find("lastspriteid");
						if(spriteTypeJson && firstSpriteIdJson && lastSpriteIdJson && spriteTypeJson->IsNumber() && firstSpriteIdJson->IsNumber() && lastSpriteIdJson->IsNumber())
						{
							Uint32 spriteIndex = startSpriteIndex;
							Uint32 spriteType = SDL_static_cast(Uint32, spriteTypeJson->AsNumber());
							Uint32 firstSpriteId = SDL_static_cast(Uint32, firstSpriteIdJson->AsNumber());
							Uint32 lastSpriteId = SDL_static_cast(Uint32, lastSpriteIdJson->AsNumber());
							for(Uint32 j = 0; j <= (lastSpriteId-firstSpriteId); ++j)
							{
								Uint32 spriteOffset = firstSpriteId + j;
								if(spriteOffset >= m_spriteOffsets.size())
									m_spriteOffsets.resize(lastSpriteId + j + 1);

								m_spriteOffsets[spriteOffset].spriteIndex = startSpriteIndex;
								m_spriteOffsets[spriteOffset].spriteType = spriteType;
								switch(spriteType)
								{
									case 0: startSpriteIndex += 1; break;
									case 1: startSpriteIndex += 2; break;
									case 2: startSpriteIndex += 2; break;
									case 3: startSpriteIndex += 4; break;
								}
							}

							SpriteSheet newSpriteSheet;
							newSpriteSheet.spriteFile = fileName->AsString();
							newSpriteSheet.firstSpriteId = spriteIndex;
							newSpriteSheet.lastSpriteId = startSpriteIndex - 1;
							newSpriteSheet.spriteType = spriteType;
							m_spriteSheets.push_back(newSpriteSheet);
						}
					}
				}
				else
					return false;
			}
			else
				return false;
		}
	}
	else
		return false;

	g_sprRevision = 0x73434654;
	g_spriteCounts = startSpriteIndex - 1;
	m_sprLoaded = true;
	return true;
}
