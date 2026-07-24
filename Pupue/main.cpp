#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <windows.h>

int main() {

	sf::RenderWindow window(sf::VideoMode({ 351u, 351u }), "Pupue", sf::Style::Titlebar | sf::Style::Close);

	HWND hwnd = window.getNativeHandle();
	SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	sf::Image icon;
	if (icon.loadFromFile("frame1_idle.png")) {
		window.setIcon(icon);
	}

	sf::Texture textureIdle;
	sf::Texture texturePlaying;
	if (!textureIdle.loadFromFile("frame1_idle.png")) {
		MessageBoxA(NULL, "Failed to load frame1_idle.png", "Error", MB_OK | MB_ICONERROR);
		return -1;
	}
	if (!texturePlaying.loadFromFile("frame2_playing.png")) {
		MessageBoxA(NULL, "Failed to load frame2_playing.png", "Error", MB_OK | MB_ICONERROR);
		return -1;
	}

	sf::Sprite buttonSprite(textureIdle);


	sf::SoundBuffer soundBuffer;
	if (!soundBuffer.loadFromFile("sound.wav")) {
		MessageBoxA(NULL, "Failed to load sound.wav", "Error", MB_OK | MB_ICONERROR);
		return -1;
	}
	sf::Sound sound(soundBuffer);
	

	while (window.isOpen()) {
		
		while (const std::optional<sf::Event> event = window.pollEvent()) {

			if (event->is<sf::Event::Closed>()) {
				window.close();
			}

			if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>()) {

				if (mousePressed->button == sf::Mouse::Button::Left) {

					sf::Vector2i mousePos = sf::Mouse::getPosition(window);

					if (buttonSprite.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos))) {
						sound.play();
					}
				}
			}
		}

		if (sound.getStatus() == sf::Sound::Status::Playing) {
			buttonSprite.setTexture(texturePlaying);
		}
		else {
			buttonSprite.setTexture(textureIdle);
		}

		window.clear(sf::Color::Black);
		window.draw(buttonSprite);
		window.display();
	}

	return 0;
}

