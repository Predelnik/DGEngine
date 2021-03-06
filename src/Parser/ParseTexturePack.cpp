#include "ParseTexturePack.h"
#include "Game/LevelHelper.h"
#include "ImageContainers/CelImageContainer.h"
#include "Min.h"
#include "ParseImageContainer.h"
#include "ParseTexture.h"
#include "TexturePacks/CachedTexturePack.h"
#include "TexturePacks/IndexedTexturePack.h"
#include "TexturePacks/RectTexturePack.h"
#include "TexturePacks/SimpleTexturePack.h"
#include "Utils/ParseUtils.h"

namespace Parser
{
	using namespace rapidjson;

	bool parseTexturePackFromId(Game& game, const Value& elem)
	{
		if (isValidString(elem, "fromId") == true)
		{
			if (isValidString(elem, "id") == true)
			{
				std::string fromId(elem["fromId"].GetString());
				std::string id(elem["id"].GetString());
				if (fromId != id && isValidId(id) == true)
				{
					auto obj = game.Resources().getTexturePack(fromId);
					if (obj != nullptr)
					{
						game.Resources().addTexturePack(id, obj);
					}
				}
			}
			return true;
		}
		return false;
	}

	std::unique_ptr<TexturePack> parseImageContainerTexturePack(
		Game& game, const Value& elem)
	{
		std::vector<std::shared_ptr<ImageContainer>> imgVec;

		const auto& imgElem = elem["imageContainer"];
		if (imgElem.IsString() == true)
		{
			std::shared_ptr<ImageContainer> imgCont;
			getOrParseImageContainer(game, elem, "imageContainer", imgCont);
			if (imgCont != nullptr)
			{
				imgVec.push_back(imgCont);
			}
		}
		else if (imgElem.IsArray() == true)
		{
			for (const auto& val : imgElem)
			{
				auto imgCont = game.Resources().getImageContainer(getStringVal(val));
				if (imgCont != nullptr)
				{
					imgVec.push_back(imgCont);
				}
			}
		}
		if (imgVec.empty() == true)
		{
			return nullptr;
		}

		std::shared_ptr<Palette> pal;
		if (elem.HasMember("palette") == true)
		{
			pal = game.Resources().getPalette(elem["palette"].GetString());
		}

		bool useIndexedImages = pal != nullptr && Shaders::supportsPalettes();

		if (imgVec.size() == 1)
		{
			return std::make_unique<CachedTexturePack>(imgVec.front(), pal, useIndexedImages);
		}
		else
		{
			return std::make_unique<CachedMultiTexturePack>(imgVec, pal, useIndexedImages);
		}
	}

	std::unique_ptr<TexturePack> parseSingleTextureTexturePack(
		Game& game, const Value& elem, const char* textureIdKey)
	{
		std::shared_ptr<sf::Texture> texture;
		getOrParseTexture(game, elem, textureIdKey, texture);
		if (texture == nullptr)
		{
			return nullptr;
		}
		std::shared_ptr<Palette> palette;
		if (isValidString(elem, "palette") == true)
		{
			palette = game.Resources().getPalette(elem["palette"].GetString());
		}
		auto isIndexed = palette != nullptr && getBoolKey(elem, "indexed");

		auto frames = getFramesKey(elem, "frames");
		if (frames.first == 0 || frames.second == 0)
		{
			frames.first = frames.second = 1;
		}
		auto offset = getVector2fKey<sf::Vector2f>(elem, "offset");
		auto startIndex = getUIntKey(elem, "startIndex");
		bool isHorizontal = getStringKey(elem, "direction") == "horizontal";

		auto texturePack = std::make_unique<SimpleTexturePack>(
			texture, frames, offset, startIndex, isHorizontal, palette, isIndexed);

		if (texturePack->size() == 0)
		{
			return nullptr;
		}
		return texturePack;
	}

	bool parseMultiTextureTexturePackHelper(Game& game, const Value& elem,
		SimpleMultiTexturePack* texturePack)
	{
		auto globalOffset = getVector2fKey<sf::Vector2f>(elem, "offset");
		ImageCache cache;
		for (const auto& val : elem["texture"])
		{
			std::shared_ptr<sf::Texture> texture;
			getOrParseTexture(game, val, "id", texture, &cache);
			if (texture == nullptr)
			{
				return false;
			}
			if (val.HasMember("frames") == true)
			{
				auto frames = getFramesKey(val, "frames");
				if (frames.first == 0 || frames.second == 0)
				{
					frames.first = frames.second = 1;
				}
				auto offset = globalOffset + getVector2fKey<sf::Vector2f>(val, "offset");
				auto startIndex = getUIntKey(val, "startIndex");
				bool isHorizontal = getStringKey(val, "direction") == "horizontal";
				texturePack->addTexturePack(
					texture, frames, offset, startIndex, isHorizontal);
			}
		}
		if (texturePack->getTextureCount() == 0)
		{
			return false;
		}
		return true;
	}

	std::unique_ptr<TexturePack> parseMultiTextureTexturePack(
		Game& game, const Value& elem)
	{
		std::shared_ptr<Palette> palette;
		if (isValidString(elem, "palette") == true)
		{
			palette = game.Resources().getPalette(elem["palette"].GetString());
		}

		auto isIndexed = palette != nullptr && getBoolKey(elem, "indexed");

		auto texturePack = std::make_unique<SimpleMultiTexturePack>(palette, isIndexed);
		if (parseMultiTextureTexturePackHelper(game, elem, texturePack.get()) == false)
		{
			return nullptr;
		}
		return texturePack;
	}

	void parseTexturePack(Game& game, const Value& elem)
	{
		if (parseTexturePackFromId(game, elem) == true)
		{
			return;
		}
		std::string id;
		if (isValidString(elem, "id") == true)
		{
			id = elem["id"].GetString();
		}
		else
		{
			if (isValidString(elem, "file") == false)
			{
				return;
			}
			std::string file(elem["file"].GetString());
			if (getIdFromFile(file, id) == false)
			{
				return;
			}
		}
		if (isValidId(id) == false)
		{
			return;
		}
		if (game.Resources().hasTexturePack(id) == true)
		{
			return;
		}

		std::unique_ptr<TexturePack> texturePack;

		if (elem.HasMember("imageContainer") == true)
		{
			texturePack = parseImageContainerTexturePack(game, elem);
		}
		else if (elem.HasMember("texture") == true)
		{
			if (elem["texture"].IsArray() == true)
			{
				texturePack = parseMultiTextureTexturePack(game, elem);
			}
			else if (elem["texture"].IsObject() == true)
			{
				texturePack = parseSingleTextureTexturePack(game, elem["texture"], "id");
			}
			else
			{
				texturePack = parseSingleTextureTexturePack(game, elem, "texture");
			}
		}
		if (texturePack == nullptr)
		{
			return;
		}
		if (isValidArray(elem, "rects") == true)
		{
			bool hasRefSize = elem.HasMember("referenceTextureSize");
			auto refSize = getVector2fKey<sf::Vector2f>(elem, "referenceTextureSize");
			auto refTileHeight = (float)getUIntKey(elem, "referenceTileHeight", 32);
			auto globalOffset = getVector2fKey<sf::Vector2f>(elem, "offset");
			auto texturePack2 = std::make_unique<RectTexturePack>(std::move(texturePack));
			for (const auto& val : elem["rects"])
			{
				if (val.IsArray() == true)
				{
					auto rect = getIntRectVal(val);
					if (rect.width > 0 && rect.height > 0)
					{
						texturePack2->addRect(0, rect, {});
					}
				}
				else if (val.IsObject() == true)
				{
					auto rect = getIntRectKey(val, "rect");
					if (rect.width > 0 && rect.height > 0)
					{
						auto index = getUIntKey(val, "index");
						auto offset = globalOffset + getVector2fKey<sf::Vector2f>(val, "offset");
						if (hasRefSize == true)
						{
							offset.x = std::round(refSize.x / 2.f) - offset.x;
							offset.y = (refSize.y - std::round(refTileHeight / 2.f) - offset.y);

							offset.x = -(std::round(refSize.x / 2.f) - std::round((float)rect.width / 2.f) - offset.x);
							offset.y = offset.y - (refSize.y - (float)rect.height);
						}
						texturePack2->addRect(index, rect, offset);
					}
				}
			}
			texturePack = std::move(texturePack2);
		}
		if (isValidArray(elem, "textureIndexes") == true)
		{
			auto texturePack2 = std::make_unique<IndexedTexturePack>(
				std::move(texturePack), getBoolKey(elem, "onlyUseIndexed", true));
			for (const auto& val : elem["textureIndexes"])
			{
				if (val.IsUint() == true)
				{
					texturePack2->mapTextureIndex(val.GetUint());
				}
				else if (val.IsArray() == true &&
					val.Size() == 2 &&
					val[0].IsUint() &&
					val[1].IsUint())
				{
					texturePack2->mapTextureIndex(val[0].GetUint(), val[1].GetUint());
				}
			}
			texturePack = std::move(texturePack2);
		}
		game.Resources().addTexturePack(id, std::move(texturePack));
	}

	void getOrParseLevelTexturePack(Game& game, const Value& elem,
		const char* idKeyBottom, const char* idKeyTop,
		std::shared_ptr<TexturePack>& texturePackBottom,
		std::shared_ptr<TexturePack>& texturePackTop,
		std::pair<uint32_t, uint32_t>& tileSize)
	{
		if (isValidString(elem, "min") == true)
		{
			// l4.min and town.min contain 16 blocks, all others 10.
			auto minBlocks = getUIntKey(elem, "minBlocks", 10);
			if (minBlocks != 10 && minBlocks != 16)
			{
				minBlocks = 10;
			}
			Min min(elem["min"].GetString(), minBlocks);
			if (min.size() == 0)
			{
				return;
			}

			auto pal = game.Resources().getPalette(elem["palette"].GetString());
			if (pal == nullptr)
			{
				return;
			}

			std::shared_ptr<ImageContainer> imgCont;
			getOrParseImageContainer(game, elem, "imageContainer", imgCont);
			if (imgCont == nullptr)
			{
				return;
			}

			bool useIndexedImages = Shaders::supportsPalettes();
			CachedImagePack imgPack(imgCont.get(), pal, useIndexedImages);

			texturePackBottom = LevelHelper::loadTilesetSprite(imgPack, min, false, false, true);
			texturePackTop = LevelHelper::loadTilesetSprite(imgPack, min, true, true, true);

			tileSize.first = 64;
			tileSize.second = 32;
		}
		else
		{
			if (isValidString(elem, idKeyBottom) == true)
			{
				texturePackBottom = game.Resources().getTexturePack(elem[idKeyBottom].GetString());
			}
			if (isValidString(elem, idKeyTop) == true)
			{
				texturePackTop = game.Resources().getTexturePack(elem[idKeyTop].GetString());
			}

			tileSize = getVector2uKey<std::pair<uint32_t, uint32_t>>(
				elem, "tileSize", std::make_pair(64u, 32u));
		}
	}
}
