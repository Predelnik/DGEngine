#include "ParseLevelObject.h"
#include "Game/SimpleLevelObject.h"
#include "Parser/ParseAction.h"
#include "Parser/Utils/ParseUtils.h"

namespace Parser
{
	using namespace rapidjson;

	void parseLevelObject(Game& game, const Value& elem)
	{
		if (isValidString(elem, "id") == false)
		{
			return;
		}
		std::string id(elem["id"].GetString());
		if (isValidId(id) == false)
		{
			return;
		}

		auto level = game.Resources().getLevel(getStringKey(elem, "level"));
		if (level == nullptr)
		{
			return;
		}

		auto mapPos = getVector2uKey<MapCoord>(elem, "mapPosition");
		auto mapSize = level->Map().MapSize();
		if (mapPos.x >= mapSize.x || mapPos.y >= mapSize.y)
		{
			return;
		}
		auto& mapCell = level->Map()[mapPos];

		if (mapCell.getObject<SimpleLevelObject>() != nullptr)
		{
			return;
		}

		std::unique_ptr<SimpleLevelObject> levelObj;

		if (isValidString(elem, "texture") == true)
		{
			auto texture = game.Resources().getTexture(elem["texture"].GetString());
			if (texture == nullptr)
			{
				return;
			}
			levelObj = std::make_unique<SimpleLevelObject>(*texture);

			if (elem.HasMember("textureRect") == true)
			{
				sf::IntRect rect(0, 0, 32, 32);
				levelObj->setTextureRect(getIntRectKey(elem, "textureRect", rect));
			}
		}
		else if (isValidString(elem, "texturePack") == true)
		{
			auto texPack = game.Resources().getTexturePack(elem["texturePack"].GetString());
			if (texPack == nullptr)
			{
				return;
			}
			auto frames = std::make_pair(0u, texPack->size() - 1);
			frames = getFramesKey(elem, "frames", frames);
			auto refresh = getTimeKey(elem, "refresh", sf::milliseconds(50));
			levelObj = std::make_unique<SimpleLevelObject>(
				*texPack, frames, refresh, AnimationType::Looped);
		}
		else
		{
			levelObj = std::make_unique<SimpleLevelObject>();
		}

		levelObj->MapPosition(mapPos);

		levelObj->Hoverable(getBoolKey(elem, "enableHover", true));

		levelObj->Id(id);
		levelObj->Name(getStringKey(elem, "name"));

		if (elem.HasMember("action") == true)
		{
			levelObj->setAction(parseAction(game, elem["action"]));
		}

		level->addLevelObject(std::move(levelObj), true);
	}
}
