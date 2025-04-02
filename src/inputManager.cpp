#include "inputManager.hpp"
#include "IOManager.hpp"
#include "chatManager.hpp"
#include "commandHandler.hpp"
#include "log.hpp"
#include "renderManager.hpp"
#include "scroller.hpp"

CInputManager::CInputManager()
	: m_input(ftxui::Input(&m_szInput, "Type here...", {.transform = [](ftxui::InputState state) {
  if (state.focused) {
    state.element |= ftxui::bgcolor(ftxui::Color::HSVA(0, 0, 0, 0));
    state.element |= ftxui::color(ftxui::Color::White);
  } else if (state.hovered && !state.focused)
    state.element |= ftxui::bgcolor(ftxui::Color::GrayDark);
  else
    state.element |= ftxui::bgcolor(ftxui::Color::Black);
  return state.element; }, .multiline = false, .on_enter = [&] {
											if (m_szInput.empty())
												return;

											g_pCommandHandler->isCommand(m_szInput) ? g_pCommandHandler->handleCommand(m_szInput) : g_pChatManager->newMessage({.msg = m_szInput, .username = "Server"});
											m_szInput.clear(); }}) // NOLINT
	  )
	, inputComponent(ftxui::Renderer(m_input, [&] {
		return ftxui::hbox({
			ftxui::text("> "),
			m_input->Render(),
		});
	}))
	, logComponent(ftxui::Renderer([] {
		std::vector<ftxui::Element> messages;
		for (const auto &[chat, log] : g_pIOManager->getIO()) {
			if (chat && !chat->msg.empty()) {
				std::string username = chat->username;
				if (username.back() == '\n')
					username.pop_back();
				messages.push_back(ftxui::window(ftxui::text(username), ftxui::paragraph(chat->msg)));
			} else if (log && !log->log.empty())
				messages.push_back(ftxui::window(ftxui::text(NFormatter::fmt(log->type, "")), ftxui::paragraph(log->log)));
		}

		return ftxui::vbox(messages);
	})) {
	log(LOG, "InputManager: initialized");
}

CInputManager::~CInputManager() {
	log(SYS, "InputManager: bye");
}

void CInputManager::inputLoop() {
	container = logComponent |= ftxui::Scroller;
	container = ftxui::ResizableSplitBottom(inputComponent, container, &bottomSize);

	g_pRenderManager->setRenderer(ftxui::Renderer(container, [this] {
		return container->Render() | ftxui::border;
	}));
	g_pRenderManager->enterLoop();
}
