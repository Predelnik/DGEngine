#pragma once

#include "Anchor.h"
#include <SFML/Graphics/View.hpp>

class Game;

class View2
{
private:
	sf::View view;
	sf::Vector2f position;
	sf::Vector2f sizeNoZoom;
	float zoomFactor{ 1.f };
	Anchor anchor{ Anchor::Top | Anchor::Left };

	void setViewport(const Game& game);

public:
	void updateSize(const Game& game);
	void updateViewport(const Game& game);

	void setAnchor(const Anchor anchor_) noexcept { anchor = anchor_; }

	const sf::Vector2f& getCenter() const { return view.getCenter(); }
	void setCenter(float x, float y) { view.setCenter(x, y); }
	void setCenter(const sf::Vector2f& center) { view.setCenter(center); }

	sf::Vector2f getPosition(const sf::Vector2f& point) const;

	const sf::Vector2f& getPosition() const noexcept { return position; }
	void setPosition(const sf::Vector2f& position_) noexcept { position = position_; }

	const sf::Vector2f& getVisibleSize() const noexcept { return view.getSize(); }
	const sf::Vector2f& getSize() const noexcept { return sizeNoZoom; }
	void setSize(float width, float height)
	{
		view.setSize(width, height);
		sizeNoZoom = getSize();
	}
	void setSize(const sf::Vector2f& size_)
	{
		view.setSize(size_);
		sizeNoZoom = size_;
	}

	const sf::View& getView() const noexcept { return view; }

	float getZoom() const noexcept { return zoomFactor; }

	void setZoom(float factor) noexcept { zoomFactor = factor; }

	void reset(const sf::FloatRect& rectangle) { view.reset(rectangle); }
	void reset();
};
