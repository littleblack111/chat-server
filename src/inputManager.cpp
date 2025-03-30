#include "inputManager.hpp"
#include "chatManager.hpp"
#include "commandHandler.hpp"
#include "log.hpp"
#include "renderManager.hpp"

CInputManager::CInputManager()
	: m_input(ftxui::Input(&m_szInput, {.multiline = false, .on_enter = [&] {
											if (m_szInput.empty())
												return;

											g_pCommandHandler->isCommand(m_szInput) ? g_pCommandHandler->handleCommand(m_szInput) : g_pChatManager->newMessage({.msg = m_szInput, .username = "Server"});
											m_szInput.clear();
										}}) // NOLINT
	  )
	, inputComponent(ftxui::Renderer(m_input, [&] {
		return ftxui::hbox({
			ftxui::text("> "),
			m_input->Render(),
		});
	}))
	, logComponent(ftxui::Renderer([] {
		std::vector<ftxui::Element> messages;
		for (const auto &msg : g_pChatManager->getChat())
			messages.push_back(ftxui::window(ftxui::text(msg.username), ftxui::paragraph(msg.msg)));

		return ftxui::vbox(messages);
	})) {
	log(LOG, "InputManager: initialized");
}

CInputManager::~CInputManager() {
	log(SYS, "InputManager: bye");
}

void CInputManager::inputLoop() {
	container = logComponent;
	container = ftxui::ResizableSplitBottom(inputComponent, container, &bottomSize);

	g_pRenderManager->setRenderer(ftxui::Renderer(container, [this] {
		return container->Render() | ftxui::border;
	}));
	g_pRenderManager->enterLoop();
}
