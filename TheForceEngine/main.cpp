// main.cpp : Defines the entry point for the application.
#include <SDL.h>
#include <TFE_System/types.h>
#include <TFE_ScriptSystem/scriptSystem.h>
#include <TFE_InfSystem/infSystem.h>
#include <TFE_Editor/editor.h>
#include <TFE_Game/level.h>
#include <TFE_Game/gameMain.h>
#include <TFE_Game/GameUI/gameUi.h>
#include <TFE_Audio/audioSystem.h>
#include <TFE_FileSystem/paths.h>
#include <TFE_Polygon/polygon.h>
#include <TFE_RenderBackend/renderBackend.h>
#include <TFE_Input/input.h>
#include <TFE_Renderer/renderer.h>
#include <TFE_Settings/settings.h>
#include <TFE_System/system.h>
#include <TFE_Asset/paletteAsset.h>
#include <TFE_Asset/imageAsset.h>
#include <TFE_Ui/ui.h>
#include <TFE_FrontEndUI/frontEndUi.h>
#include <algorithm>

// Replace with music system
#include <TFE_Audio/midiPlayer.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#ifdef min
#undef min
#undef max
#endif
#endif

#define PROGRAM_ERROR   1
#define PROGRAM_SUCCESS 0

#pragma comment(lib, "SDL2main.lib")

// Replace with settings.
static bool s_fullscreen = false;
static bool s_vsync = true;
static bool s_loop  = true;
static f32  s_refreshRate = 0;
static u32  s_baseWindowWidth = 1280;
static u32  s_baseWindowHeight = 720;
static u32  s_displayWidth = s_baseWindowWidth;
static u32  s_displayHeight = s_baseWindowHeight;
static u32  s_monitorWidth = 1280;
static u32  s_monitorHeight = 720;

void handleEvent(SDL_Event& Event)
{
	TFE_Ui::setUiInput(&Event);

	switch (Event.type)
	{
		case SDL_QUIT:
		{
			s_loop = false;
		} break;
		case SDL_WINDOWEVENT:
		{
			if (Event.window.event == SDL_WINDOWEVENT_RESIZED || Event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
			{
				TFE_RenderBackend::resize(Event.window.data1, Event.window.data2);
			}
		} break;
		case SDL_CONTROLLERDEVICEADDED:
		{
			const s32 cIdx = Event.cdevice.which;
			if (SDL_IsGameController(cIdx))
			{
				SDL_GameController* controller = SDL_GameControllerOpen(cIdx);
				SDL_Joystick* j = SDL_GameControllerGetJoystick(controller);
				SDL_JoystickID joyId = SDL_JoystickInstanceID(j);

				//Save the joystick id to used in the future events
				SDL_GameControllerOpen(0);
			}
		} break;
		case SDL_MOUSEBUTTONDOWN:
		{
			TFE_Input::setMouseButtonDown(MouseButton(Event.button.button - SDL_BUTTON_LEFT));
		} break;
		case SDL_MOUSEBUTTONUP:
		{
			TFE_Input::setMouseButtonUp(MouseButton(Event.button.button - SDL_BUTTON_LEFT));
		} break;
		case SDL_MOUSEWHEEL:
		{
			TFE_Input::setMouseWheel(Event.wheel.x, Event.wheel.y);
		} break;
		case SDL_KEYDOWN:
		{
			if (Event.key.keysym.scancode && Event.key.repeat == 0)
			{
				TFE_Input::setKeyDown(KeyboardCode(Event.key.keysym.scancode));
			}

			if (Event.key.keysym.scancode)
			{
				TFE_Input::setBufferedKey(KeyboardCode(Event.key.keysym.scancode));
			}
		} break;
		case SDL_KEYUP:
		{
			if (Event.key.keysym.scancode)
			{
				const KeyboardCode code = KeyboardCode(Event.key.keysym.scancode);
				TFE_Input::setKeyUp(KeyboardCode(Event.key.keysym.scancode));

				// Fullscreen toggle.
				if (code == KeyboardCode::KEY_F11)
				{
					s_fullscreen = !s_fullscreen;
					TFE_RenderBackend::enableFullscreen(s_fullscreen);
				}
			}
		} break;
		case SDL_TEXTINPUT:
		{
			TFE_Input::setBufferedInput(Event.text.text);
		} break;
		case SDL_CONTROLLERAXISMOTION:
		{
			const s32 deadzone = 3200;
			if ((Event.caxis.value < -deadzone) || (Event.caxis.value > deadzone))
			{
				if (Event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX)
				{ TFE_Input::setAxis(AXIS_LEFT_X, f32(Event.caxis.value) / 32768.0f); }
				else if (Event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY)
				{ TFE_Input::setAxis(AXIS_LEFT_Y, -f32(Event.caxis.value) / 32768.0f); }

				if (Event.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTX)
				{ TFE_Input::setAxis(AXIS_RIGHT_X, f32(Event.caxis.value) / 32768.0f); }
				else if (Event.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTY)
				{ TFE_Input::setAxis(AXIS_RIGHT_Y, -f32(Event.caxis.value) / 32768.0f); }

				if (Event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT)
				{ TFE_Input::setAxis(AXIS_LEFT_TRIGGER, f32(Event.caxis.value) / 32768.0f); }
				if (Event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
				{ TFE_Input::setAxis(AXIS_RIGHT_TRIGGER, -f32(Event.caxis.value) / 32768.0f); }
			}
			else
			{
				if (Event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX)
				{ TFE_Input::setAxis(AXIS_LEFT_X, 0.0f); }
				else if (Event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY)
				{ TFE_Input::setAxis(AXIS_LEFT_Y, 0.0f); }

				if (Event.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTX)
				{ TFE_Input::setAxis(AXIS_RIGHT_X, 0.0f); }
				else if (Event.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTY)
				{ TFE_Input::setAxis(AXIS_RIGHT_Y, 0.0f); }

				if (Event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT)
				{ TFE_Input::setAxis(AXIS_LEFT_TRIGGER, 0.0f); }
				if (Event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
				{ TFE_Input::setAxis(AXIS_RIGHT_TRIGGER, 0.0f); }
			}
		} break;
		case SDL_CONTROLLERBUTTONDOWN:
		{
			if (Event.cbutton.button < CONTROLLER_BUTTON_COUNT)
			{
				TFE_Input::setButtonDown(Button(Event.cbutton.button));
			}
		} break;
		case SDL_CONTROLLERBUTTONUP:
		{
			if (Event.cbutton.button < CONTROLLER_BUTTON_COUNT)
			{
				TFE_Input::setButtonUp(Button(Event.cbutton.button));
			}
		} break;
		default:
		{
		} break;
	}
}

bool sdlInit()
{
	// Audio is handled outside of SDL2.
	// Using the Force Engine Audio system for sound mixing, FluidSynth for Midi handling and rtAudio for audio I/O.
	const int code = SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_GAMECONTROLLER);
	if (code != 0) { return false; }

	// Determine the display mode settings based on the desktop.
	SDL_DisplayMode mode = {};
	SDL_GetDesktopDisplayMode(0, &mode);
	s_refreshRate = (f32)mode.refresh_rate;

	TFE_Settings_Window* windowSettings = TFE_Settings::getWindowSettings();
	s_fullscreen = windowSettings->fullscreen;
	s_displayWidth = windowSettings->width;
	s_displayHeight = windowSettings->height;
	s_baseWindowWidth = windowSettings->baseWidth;
	s_baseWindowHeight = windowSettings->baseHeight;

	if (s_fullscreen)
	{
		s_displayWidth = mode.w;
		s_displayHeight = mode.h;
	}
	else
	{
		s_displayWidth = std::min(s_displayWidth, (u32)mode.w);
		s_displayHeight = std::min(s_displayHeight, (u32)mode.h);
	}

	s_monitorWidth = mode.w;
	s_monitorHeight = mode.h;

	return true;
}

void setAppState(AppState newState, TFE_Renderer* renderer)
{
	const TFE_Settings_Graphics* config = TFE_Settings::getGraphicsSettings();

	if (newState != APP_STATE_EDITOR)
	{
		TFE_Editor::disable();
	}

	switch (newState)
	{
	case APP_STATE_MENU:
		break;
	case APP_STATE_EDITOR:
		renderer->changeResolution(640, 480);
		TFE_Editor::enable(renderer);
		break;
	case APP_STATE_DARK_FORCES:
		renderer->changeResolution(config->gameResolution.x, config->gameResolution.z);
		renderer->enableScreenClear(false);
		TFE_Input::enableRelativeMode(true);
		TFE_GameMain::init(renderer);
		break;
	};
}

static AppState s_curState = APP_STATE_UNINIT;

int main(int argc, char* argv[])
{
	// Paths
	bool pathsSet = true;
	pathsSet &= TFE_Paths::setProgramPath();
	pathsSet &= TFE_Paths::setProgramDataPath("TheForceEngine");
	pathsSet &= TFE_Paths::setUserDocumentsPath("TheForceEngine");
	if (!pathsSet)
	{
		return PROGRAM_ERROR;
	}

	// Initialize settings so that the paths can be read.
	if (!TFE_Settings::init())
	{
		return PROGRAM_ERROR;
	}

	// Setup game paths.
	// Get the current game.
	const TFE_Game* game = TFE_Settings::getGame();
	const TFE_Settings_Game* gameSettings = TFE_Settings::getGameSettings(game->game);
	TFE_Paths::setPath(PATH_SOURCE_DATA, gameSettings->sourcePath);
	TFE_Paths::setPath(PATH_EMULATOR, gameSettings->emulatorPath);

	TFE_System::logOpen("the_force_engine_log.txt");
	TFE_System::logWrite(LOG_MSG, "Paths", "Program Path: \"%s\"",   TFE_Paths::getPath(PATH_PROGRAM));
	TFE_System::logWrite(LOG_MSG, "Paths", "Program Data: \"%s\"",   TFE_Paths::getPath(PATH_PROGRAM_DATA));
	TFE_System::logWrite(LOG_MSG, "Paths", "User Documents: \"%s\"", TFE_Paths::getPath(PATH_USER_DOCUMENTS));
	TFE_System::logWrite(LOG_MSG, "Paths", "Source Data: \"%s\"",    TFE_Paths::getPath(PATH_SOURCE_DATA));

	// Initialize SDL
	if (!sdlInit())
	{
		TFE_System::logWrite(LOG_CRITICAL, "SDL", "Cannot initialize SDL.");
		TFE_System::logClose();
		return PROGRAM_ERROR;
	}

	// Setup the GPU Device and Window.
	u32 windowFlags = 0;
	if (s_fullscreen) { TFE_System::logWrite(LOG_MSG, "Display", "Fullscreen enabled.");    windowFlags |= WINFLAG_FULLSCREEN; }
	if (s_vsync)      { TFE_System::logWrite(LOG_MSG, "Display", "Vertical Sync enabled."); windowFlags |= WINFLAG_VSYNC; }

	const WindowState windowState =
	{
		"The Force Engine",
		s_displayWidth,
		s_displayHeight,
		s_baseWindowWidth,
		s_baseWindowHeight,
		s_monitorWidth,
		s_monitorHeight,
		windowFlags,
		s_refreshRate
	};
	if (!TFE_RenderBackend::init(windowState))
	{
		TFE_System::logWrite(LOG_CRITICAL, "GPU", "Cannot initialize GPU/Window.");
		TFE_System::logClose();
		return PROGRAM_ERROR;
	}
	TFE_System::init(s_refreshRate, s_vsync);
	TFE_Audio::init();
	TFE_Polygon::init();
	TFE_Image::init();
	TFE_ScriptSystem::init();
	TFE_InfSystem::init();
	TFE_Level::init();
	TFE_Palette::createDefault256();
	TFE_FrontEndUI::init();

	// Replace with TFE_Music
	TFE_MidiPlayer::init();
		
	TFE_Renderer* renderer = TFE_Renderer::create(TFE_RENDERER_SOFTWARE_CPU);
	if (!renderer)
	{
		TFE_System::logWrite(LOG_CRITICAL, "Renderer", "Cannot create software renderer.");
		TFE_System::logClose();
		return PROGRAM_ERROR;
	}
	if (!renderer->init())
	{
		TFE_System::logWrite(LOG_CRITICAL, "Renderer", "Cannot initialize software renderer.");
		TFE_System::logClose();
		return PROGRAM_ERROR;
	}

	// Game loop
	TFE_GameUi::init(renderer);
		
	u32 frame = 0u;
	bool showPerf = false;
	bool relativeMode = false;
	TFE_System::logWrite(LOG_MSG, "Progam Flow", "The Force Engine Game Loop Started");
	while (s_loop)
	{
		bool enableRelative = TFE_Input::relativeModeEnabled();
		if (enableRelative != relativeMode)
		{
			relativeMode = enableRelative;
			SDL_SetRelativeMouseMode(relativeMode ? SDL_TRUE : SDL_FALSE);
		}

		// System events
		SDL_Event event;
		while (SDL_PollEvent(&event)) { handleEvent(event); }

		// Handle mouse state.
		s32 mouseX, mouseY;
		s32 mouseAbsX, mouseAbsY;
		u32 state = SDL_GetRelativeMouseState(&mouseX, &mouseY);
		SDL_GetMouseState(&mouseAbsX, &mouseAbsY);
		TFE_Input::setRelativeMousePos(mouseX, mouseY);
		TFE_Input::setMousePos(mouseAbsX, mouseAbsY);

		const AppState appState = TFE_FrontEndUI::update();
		if (appState == APP_STATE_QUIT)
		{
			s_loop = false;
		}
		else if (appState != s_curState)
		{
			s_curState = appState;
			setAppState(s_curState, renderer);
		}

		TFE_Ui::begin();
						
		// Update
		if (TFE_Input::keyPressed(KEY_F9))
		{
			showPerf = !showPerf;
		}

		TFE_System::update();
		if (showPerf)
		{
			TFE_Editor::showPerf(frame);
		}

		if (appState == APP_STATE_MENU)
		{
			TFE_FrontEndUI::draw();
		}
		else if (appState == APP_STATE_EDITOR)
		{
			if (TFE_Editor::update())
			{
				TFE_FrontEndUI::setAppState(APP_STATE_MENU);
			}
		}
		else if (appState == APP_STATE_DARK_FORCES)
		{
			if (TFE_GameMain::loop() == TRANS_QUIT)
			{
				s_loop = false;
			}
		}

		// Render
		renderer->begin();
		// Do stuff
		bool swap = appState != APP_STATE_EDITOR;
		if (appState == APP_STATE_EDITOR)
		{
			swap = TFE_Editor::render();
		}
		renderer->end();

		// Blit the frame to the window and draw UI.
		TFE_RenderBackend::swap(swap);

		// Clear transitory input state.
		TFE_Input::endFrame();
		frame++;
	}

	// Cleanup
	TFE_FrontEndUI::shutdown();
	TFE_Audio::shutdown();
	TFE_Polygon::shutdown();
	TFE_Image::shutdown();
	TFE_Level::shutdown();
	TFE_InfSystem::shutdown();
	TFE_ScriptSystem::shutdown();
	TFE_Palette::freeAll();
	TFE_RenderBackend::updateSettings();
	TFE_Settings::shutdown();
	TFE_Renderer::destroy(renderer);
	TFE_RenderBackend::destroy();
	SDL_Quit();

	// Replace with TFE_Music
	TFE_MidiPlayer::destroy();

	TFE_System::logWrite(LOG_MSG, "Progam Flow", "The Force Engine Game Loop Ended.");
	TFE_System::logClose();
	return PROGRAM_SUCCESS;
}