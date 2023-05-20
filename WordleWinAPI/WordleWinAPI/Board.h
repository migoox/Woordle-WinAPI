#pragma once
#define NOMINMAX
#include <array>
#include <windows.h>
#include "Difficulty.h"

struct Tile
{
	RECT position;
	letter_state state;
};

class Board
{
public:
	// define parameters
	static constexpr LONG margin = 6;
	static constexpr LONG tile_size = 55;

	static constexpr LONG cols = 5;

	static constexpr LONG max_rows = 10;
	static constexpr LONG easy_rows = 6;
	static constexpr LONG medium_rows = 8;
	static constexpr LONG hard_rows = 10;

	static constexpr LONG max_tile_count = max_rows * cols;
	static constexpr LONG width = cols * (tile_size + margin) + margin;

	static constexpr LONG max_height = max_rows * (tile_size + margin) + margin;
	static constexpr LONG easy_height = easy_rows * (tile_size + margin) + margin;
	static constexpr LONG medium_height = medium_rows * (tile_size + margin) + margin;
	static constexpr LONG hard_height = hard_rows * (tile_size + margin) + margin;

	std::array<Tile, max_tile_count>& Tiles() { return mTiles; }
	const std::array<Tile, max_tile_count>& Tiles() const { return mTiles; }

	Board()
	{
		// Iterate through board tiles
		for (LONG row = 0; row < max_rows; ++row)
		{
			for (LONG column = 0; column < cols; ++column)
			{
				// Take current board tile
				auto& tile = mTiles[row * cols + column];

				tile.position.top = row * (tile_size + margin) + margin;
				tile.position.left = column * (tile_size + margin) + margin;
				tile.position.bottom = tile.position.top + tile_size;
				tile.position.right = tile.position.left + tile_size;

				tile.state = letter_state::none;
			}
		}
	}

private:

	std::array<Tile, max_tile_count> mTiles;
};