#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <optional>
#include <fstream>
#include <string>
#include <atomic>
#include <vector>

std::atomic<bool> triggerAction(false);

unsigned int loadFramerateFromIni(const std::string& filename) {
	unsigned int framerate = 60;
	std::ifstream file(filename);
	std::string line;
	while (std::getline(file, line)) {
		if (line.find("Framerate=") != std::string::npos) {
			try {
				int parsedValue = std::stoi(line.substr(10));

				if (parsedValue > 0) {
					framerate = static_cast<unsigned int>(parsedValue);
					std::cerr << "Set framerate limit.\n";
				}
				else {
					std::cerr << "Framerate must be positive. Using default (60).\n";
				}
			}
			catch (...) {
				std::cerr << "Failed to parse framerate from ini. Using default (60).\n";
			}
		}
	}
	return framerate;
}


#ifdef _WIN32
#include <windows.h>
#include <CommCtrl.h>
#pragma comment(lib, "comctl32.lib")

	bool isTopmost = true;
	bool isHotkeyEnabled = true;

	std::vector<int> currentHotkeys;

	HHOOK hKeyboardHook = NULL;

	const UINT MENU_TOPMOST_ID = 0x8880;
	const UINT MENU_HOTKEY_ID = 0x8890;
	const UINT MENU_EDIT_CONFIG_ID = 0x88A0;

	std::vector<int> loadHotkeysFromIni(const std::string& filename) {
		std::vector<int> keys;
		std::ifstream file(filename);
		std::string line;
		while (std::getline(file, line)) {
			if (line.find("Hotkey=") != std::string::npos) {
				try {
					keys.push_back(std::stoi(line.substr(7), nullptr, 16));
				}
				catch (...) {
					std::cerr << "Failed to parse a hotkey from ini.\n";
				}
			}
		}

		if (keys.empty()) {
			keys.push_back(0x5F);
		}
		return keys;
	}

	LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
		if (nCode == HC_ACTION && isHotkeyEnabled) {
			if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
				KBDLLHOOKSTRUCT* pKeyBoard = (KBDLLHOOKSTRUCT*)lParam;

				for (int vk : currentHotkeys) {
					if (pKeyBoard->vkCode == vk) {
						triggerAction = true;
						break;
					}
				}
			}
		}
		return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
	}


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

			else if ((wParam & 0xFFF0) == MENU_HOTKEY_ID) {
				isHotkeyEnabled = !isHotkeyEnabled;

				HMENU hMenu = GetSystemMenu(hWnd, FALSE);
				CheckMenuItem(hMenu, MENU_HOTKEY_ID, MF_BYCOMMAND | (isHotkeyEnabled ? MF_CHECKED : MF_UNCHECKED));

				return 0;
			}

			else if ((wParam & 0xFFF0) == MENU_EDIT_CONFIG_ID) {
				ShellExecuteW(NULL, L"open", L"notepad.exe", L"config.ini", NULL, SW_SHOW);
				return 0;
			}

		}
		return DefSubclassProc(hWnd, uMsg, wParam, lParam);
	}
#endif

int main() {


	sf::Texture textureIdle;
	sf::Texture texturePlaying;
	if (!textureIdle.loadFromFile("Assets/frame1_idle.png")) {
		//MessageBoxA(NULL, "Failed to load frame1_idle.png", "Error", MB_OK | MB_ICONERROR);
		std::cerr << "Failed to load frame1_idle.png\n";
		return -1;
	}
	if (!texturePlaying.loadFromFile("Assets/frame2_playing.png")) {
		//MessageBoxA(NULL, "Failed to load frame2_playing.png", "Error", MB_OK | MB_ICONERROR);
		std::cerr << "Failed to load frame2_playing.png\n";
		return -1;
	}

	sf::Vector2u imageSize = textureIdle.getSize();

	sf::RenderWindow window(sf::VideoMode(imageSize), "Pupue", sf::Style::Titlebar | sf::Style::Close);

	unsigned int targetFPS = loadFramerateFromIni("config.ini");
	window.setFramerateLimit(targetFPS);

#ifdef _WIN32

		currentHotkeys = loadHotkeysFromIni("config.ini");

		HWND hwnd = window.getNativeHandle();
		SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

		HMENU hMenu = GetSystemMenu(hwnd, FALSE);
		AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
		AppendMenuW(hMenu, MF_STRING, MENU_TOPMOST_ID, L"Always on Top");
		CheckMenuItem(hMenu, MENU_TOPMOST_ID, MF_BYCOMMAND | MF_CHECKED);

		AppendMenuW(hMenu, MF_STRING, MENU_HOTKEY_ID, L"Global Hotkey Enabled");
		CheckMenuItem(hMenu, MENU_HOTKEY_ID, MF_BYCOMMAND | MF_CHECKED);

		AppendMenuW(hMenu, MF_STRING, MENU_EDIT_CONFIG_ID, L"Open Config File");

		SetWindowSubclass(hwnd, MyWindowSubclass, 1, 0);

		hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
#else
		std::cout << "Running on Linux. Window pinning or Global Hotkeys not supported.\n";
		std::cout << "Tested on WSL. I don't have a spare computer to natively test this on.";
#endif

	sf::Image icon = textureIdle.copyToImage();
	window.setIcon(icon);
	

	sf::Sprite buttonSprite(textureIdle);


	sf::SoundBuffer soundBuffer;
	if (!soundBuffer.loadFromFile("Assets/sound.wav")) {
		std::cerr << "Failed to load sound.wav\n";
		return -1;
	}
	sf::Sound sound(soundBuffer);

#ifdef _WIN32
	SetProcessWorkingSetSize(GetCurrentProcess(), (SIZE_T)-1, (SIZE_T)-1);
#endif
	
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

		if (triggerAction) {
			sound.play();
			triggerAction = false;
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

#ifdef _WIN32
	if (hKeyboardHook != NULL) {
		UnhookWindowsHookEx(hKeyboardHook);
	}
#endif

	return 0;
}