#include "inputManager.hpp"
#include "IOManager.hpp"
#include "../core/chatManager.hpp"
#include "../command/commandHandler.hpp"
#include "../debug/log.hpp"
#include "renderManager.hpp"
#include "scroller.hpp"

CInputManager::CInputManager()
	: m_input(ftxui::Input(&m_szInput, "Type here...", {.transform = [](ftxui::InputState state) {
    state.element |= ftxui::bgcolor(ftxui::Color::HSVA(0, 0, 0, 0));
    state.element |= ftxui::color(ftxui::Color::White);
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
		for (const auto &[chat, log, custom] : g_pIOManager->getIO()) {
			if (chat && !chat->msg.empty()) {
				std::string username = chat->username;
				if (username.back() == '\n')
					username.pop_back();
				messages.push_back(ftxui::window(ftxui::text(username), ftxui::paragraph(chat->msg)));
			} else if (log && !log->log.empty())
				messages.push_back(ftxui::window(ftxui::text(NFormatter::fmt(log->type, "")), ftxui::paragraph(log->log)));
			else if (custom)
				messages.push_back(ftxui::window(ftxui::text(custom->first), ftxui::paragraph(custom->second)));
		}

		return ftxui::vbox(messages);
	})) {

	log(LOG, "InputManager: initialized");
}

CInputManager::~CInputManager() {
	log(SYS, "InputManager: bye");
}

void CInputManager::updateIO() {
	logComponent->OnEvent(ftxui::Event::Custom);
	g_pRenderManager->screen.PostEvent(ftxui::Event::Custom);
}

void CInputManager::inputLoop() {
	container = logComponent |= ftxui::Scroller;
	container = ftxui::ResizableSplitBottom(inputComponent, container, &bottomSize);

	// FIXME: this shit spams TakeFocus(not ideal for power etc reasons)
	// consider: custom loop, or just only allow focus to be inputComponent somehow
	container |= ftxui::CatchEvent([&](ftxui::Event) {
		inputComponent->TakeFocus();
		return false;
	});
	g_pRenderManager->setRenderer(ftxui::Renderer(container, [this] {
		return container->Render() | ftxui::border;
	}));
	inputComponent->TakeFocus();
	g_pRenderManager->enterLoop();
}
