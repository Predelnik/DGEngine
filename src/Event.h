#pragma once

#include "Actions/Action.h"
#include <memory>
#include <SFML/System/Time.hpp>
#include <string>

class Event : public Action
{
private:
	std::string id;
	std::shared_ptr<Action> action;
	sf::Time timeout;
	sf::Time currentTime;

public:
	explicit Event(const std::shared_ptr<Action>& action_,
		const sf::Time& timeout_ = sf::Time::Zero)
		: action(action_), timeout(timeout_) {}

	const std::string& getId() const noexcept { return id; }
	void setId(const std::string& id_) { id = id_; }

	void setAction(const std::shared_ptr<Action>& action_) noexcept { action = action_; }

	void resetTime() noexcept { currentTime = sf::Time::Zero; }

	virtual bool execute(Game& game);
};
