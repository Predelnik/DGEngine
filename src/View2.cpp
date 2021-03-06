#include "View2.h"
#include "Game.h"
#include "GameUtils.h"

sf::Vector2f View2::getPosition(const sf::Vector2f& point) const
{
	sf::Vector2f mousePosition =
		view.getCenter() -
		(view.getSize() / 2.f) +
		(point * zoomFactor) -
		(position * zoomFactor);
	mousePosition.x = std::round(mousePosition.x);
	mousePosition.y = std::round(mousePosition.y);
	return mousePosition;
}

void View2::setViewport(const Game& game)
{
	const auto& windowSize = game.WindowTex().getSize();
	const auto& gameSize = game.WindowSize();
	auto x = position.x;
	if (windowSize.x > gameSize.x)
	{
		x += ((float)windowSize.x / 2.f) - ((float)gameSize.x / 2.f);
	}
	x /= (float)windowSize.x;
	auto y = position.y;
	if (windowSize.y > gameSize.y)
	{
		y += ((float)windowSize.y / 2.f) - ((float)gameSize.y / 2.f);
	}
	y /= (float)windowSize.y;

	auto w = sizeNoZoom.x / (float)windowSize.x;
	auto h = sizeNoZoom.y / (float)windowSize.y;

	view.setViewport(sf::FloatRect(x, y, w, h));
}

void View2::updateSize(const Game& game)
{
	if (game.StretchToFit() == true)
	{
		setViewport(game);
		return;
	}
	auto pos2 = getPosition();
	auto size2 = sizeNoZoom;
	GameUtils::setAnchorPosSize(anchor, pos2, size2, game.OldWindowSize(), game.WindowSize());
	setPosition(pos2);
	sizeNoZoom = size2;
	view.setSize(size2);
	setViewport(game);
	view.zoom(zoomFactor);
}

void View2::updateViewport(const Game& game)
{
	view.setSize(sizeNoZoom);
	setViewport(game);
	view.zoom(zoomFactor);
}

void View2::reset()
{
	const auto& vSize = view.getSize();
	view.reset(sf::FloatRect(0, 0, vSize.x, vSize.y));
}
