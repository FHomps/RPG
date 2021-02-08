#include <chrono>
#include "SceneEditor.h"

int main() {
	const int w_x = 1600;
	const int w_y = 900;

	sf::RenderWindow window(sf::VideoMode(w_x, w_y), "Main window");
	//window.setFramerateLimit(60);

	TileSet const& set = TileSet::get("grasslands");
	Scene s(set);

	const int zoom = 4;
	const int n_x = w_x / (24 * zoom) + 1;
	const int n_y = w_y / (24 * zoom) + 1;
	sf::View view(sf::FloatRect(0, 0, n_x, n_y));
	view.setViewport(sf::FloatRect(0, 0, (float)(n_x * 24 * zoom) / w_x, (float)(n_y * 24 * zoom) / w_y));
	window.setView(view);

	sf::Event haps;

	int height = 0;
	TerrainTool tool;
	tool.setScene(s);
	tool.top = "grass top";
	tool.wall = "grass wall";
	tool.foot = "grass foot";
	tool.lowestHeight = -2 * n_y;

	sf::Font font;
	font.loadFromFile("resources/OpenSans.ttf");
	sf::Text FPSCounter;
	FPSCounter.setFont(font);
	FPSCounter.setFillColor(sf::Color::White);
	FPSCounter.setOutlineColor(sf::Color::Black);
	FPSCounter.setOutlineThickness(2);
	FPSCounter.setPosition(10, 0);


	srand(6);

	const int b_x = 3;
	const int b_y = 2;
	for (int x = b_x; x < n_x - b_x; x++) {
		for (int y = b_y; y < n_y - b_y; y++) {
			tool.use(x, y, height);
		}
	}

	for (int x = 2; x < n_x; x+=3) {
		for (int y = 2; y < n_y; y+=3) {
			tool.use(x, y, height + 1 + rand() % 4);
		}
	}

	auto start = std::chrono::steady_clock::now();
	uint frames = 0;

	while (window.isOpen()) {
		while (window.pollEvent(haps)) {
			switch (haps.type) {
			case sf::Event::Closed:
				window.close(); break;
			case sf::Event::KeyPressed: {
				switch (haps.key.code) {
				case sf::Keyboard::Up:
					height++;
					break;
				case sf::Keyboard::Down:
					height--;
					break;
				default: break;
				}
				break;
			}
			case sf::Event::MouseButtonPressed: {
				sf::Vector2i coords = sf::Vector2i(window.mapPixelToCoords(sf::Mouse::getPosition(window), view));
				if (haps.mouseButton.button == sf::Mouse::Left) {
					tool.use(coords.x, coords.y, height);
				}
				break;
			}
			default: break;
			}
		}

		window.clear(sf::Color(20, 20, 30));
		window.setView(view);
		window.draw(s);
		window.setView(window.getDefaultView());
		window.draw(FPSCounter);
		window.display();

		frames++;
		auto stop = std::chrono::steady_clock::now();
		int ms_elapsed = (int)std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		if (ms_elapsed > 500) {
			FPSCounter.setString(std::to_string((int)((float)frames / ms_elapsed * 1000 + 0.5f)));
			start = stop;
			frames = 0;
		}
	}

	TileSet::unload_all();
}