#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <windows.h>
#include <CommCtrl.h>
#include <thread>

#pragma comment(lib, "comctl32.lib")

bool isTopmost = true;

const UINT MENU_TOPMOST_ID = 0x8880;

LRESULT CALLBACK MyWindowSubclass(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
	if (uMsg == WM_SYSCOMMAND) {
		if ((wParam & 0xFFF0) == MENU_TOPMOST_ID) {
			isTopmost = !isTopmost;
			
			SetWindowPos(
				hWnd,
				isTopmost ? HWND_TOPMOST : HWND_NOTOPMOST,
				0, 0, 0, 0,
				SWP_NOMOVE | SWP_NOSIZE
			);

			HMENU hMenu = GetSystemMenu(hWnd, FALSE);
			CheckMenuItem(hMenu, MENU_TOPMOST_ID, MF_BYCOMMAND | (isTopmost ? MF_CHECKED : MF_UNCHECKED));

			return 0;
		}
	}
	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

void app() {

}

int main() {

	/*if (SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS)) {
		MessageBoxA(NULL, "Priority set to High.", "Success", MB_OK | MB_ICONINFORMATION);
	}
	else {
		MessageBoxA(NULL, "Failed to change priority.", "Error", MB_OK | MB_ICONERROR);
	}*/

	sf::Texture textureIdle;
	sf::Texture texturePlaying;
	if (!textureIdle.loadFromFile("Assets/frame1_idle.png")) {
		MessageBoxA(NULL, "Failed to load frame1_idle.png", "Error", MB_OK | MB_ICONERROR);
		return -1;
	}
	if (!texturePlaying.loadFromFile("Assets/frame2_playing.png")) {
		MessageBoxA(NULL, "Failed to load frame2_playing.png", "Error", MB_OK | MB_ICONERROR);
		return -1;
	}

	sf::Vector2u imageSize = textureIdle.getSize();

	sf::RenderWindow window(sf::VideoMode(imageSize), "Pupue", sf::Style::Titlebar | sf::Style::Close);

	window.setFramerateLimit(60);

	HWND hwnd = window.getNativeHandle();
	SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	HMENU hMenu = GetSystemMenu(hwnd, FALSE);
	AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
	AppendMenuW(hMenu, MF_STRING, MENU_TOPMOST_ID, L"Always on Top");
	CheckMenuItem(hMenu, MENU_TOPMOST_ID, MF_BYCOMMAND | MF_CHECKED);

	SetWindowSubclass(hwnd, MyWindowSubclass, 1, 0);

	sf::Image icon;
	if (icon.loadFromFile("frame1_idle.png")) {
		window.setIcon(icon);
	}

	sf::Sprite buttonSprite(textureIdle);


	sf::SoundBuffer soundBuffer;
	if (!soundBuffer.loadFromFile("Assets/sound.wav")) {
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