#include "inputManager.hpp"
#include "chatManager.hpp"
#include "commandHandler.hpp"
#include <memory>
#include <ncpp/NotCurses.hh>
#include <ncpp/Plane.hh>
#include <ncpp/Reader.hh>

CInputManagerImpl::CInputManagerImpl() {
	notcurses_options nopts{};
	nopts.flags = NCOPTION_NO_ALTERNATE_SCREEN;

	nc = std::make_unique<ncpp::NotCurses>(nopts);

	unsigned dimx, dimy;
	nc->get_term_dim(&dimy, &dimx);

	struct notcurses *nc_ptr = nc->get_notcurses();
	stdplane				 = notcurses_stdplane(nc_ptr);

	struct ncplane_options msgOpts = {
		.y		  = static_cast<int>(dimy - 3),
		.x		  = 0,
		.rows	  = dimy - 3,
		.cols	  = dimx,
		.userptr  = nullptr,
		.name	  = "messages",
		.resizecb = nullptr,
		.flags	  = 0,
		.margin_b = 0,
		.margin_r = 0,
	};
	msgPlane = ncplane_create(stdplane, &msgOpts);

	struct ncplane_options sepOpts = {
		.y		  = static_cast<int>(dimy - 3),
		.x		  = 0,
		.rows	  = 1,
		.cols	  = dimx,
		.userptr  = nullptr,
		.name	  = "separator",
		.resizecb = nullptr,
		.flags	  = 0,
		.margin_b = 0,
		.margin_r = 0,
	};
	sepPlane = ncplane_create(stdplane, &sepOpts);

	ncplane_putstr_yx(sepPlane, 0, 0, "────────────────── Type your message below ──────────────────");

	struct ncplane_options inputOpts = {
		.y		  = static_cast<int>(dimy - 2),
		.x		  = 0,
		.rows	  = 2,
		.cols	  = dimx,
		.userptr  = nullptr,
		.name	  = "input",
		.resizecb = nullptr,
		.flags	  = 0,
		.margin_b = 0,
		.margin_r = 0,
	};
	inputPlane = ncplane_create(stdplane, &inputOpts);

	ncreader_options ropts{};
	ropts.flags = NCREADER_OPTION_CURSOR;
	reader		= ncreader_create(inputPlane, &ropts);

	ncplane_putstr_yx(inputPlane, 0, 0, "> ");
	ncplane_cursor_move_yx(inputPlane, 0, 2);

	nc->render();
}

CInputManagerImpl::~CInputManagerImpl() {
	if (reader)
		ncreader_destroy(reader, nullptr);
	if (msgPlane)
		ncplane_destroy(msgPlane);
	if (sepPlane)
		ncplane_destroy(sepPlane);
	if (inputPlane)
		ncplane_destroy(inputPlane);
}

void CInputManagerImpl::writeMessage(const std::string &msg) {
	ncplane_putstr(msgPlane, msg.c_str());
	ncplane_putchar(msgPlane, '\n');
	nc->render();
}

void CInputManagerImpl::inputLoop() {
	bool running = true;

	writeMessage("Chat server started. Type /help for commands.");

	while (running) {
		ncinput	 ni;
		uint32_t input = notcurses_get_blocking(nc->get_notcurses(), &ni);

		if (input == NCKEY_ENTER) {
			std::unique_ptr<char, decltype(&free)> content(ncreader_contents(reader), free);
			if (content && *content) {
				std::string message(content.get());

				ncreader_clear(reader);
				ncplane_putstr_yx(inputPlane, 0, 0, "> ");
				ncplane_cursor_move_yx(inputPlane, 0, 2);

				if (g_pCommandHandler->isCommand(message))
					g_pCommandHandler->handleCommand(message);
				else
					g_pChatManager->newMessage({.msg = message, .username = "Server"});
			}
		} else if (input == NCKEY_ESC) {
			running = false;
		} else {

			ncreader_offer_input(reader, &ni);
		}

		nc->render();
	}
}

CInputManager::CInputManager() {
	log(LOG, "InputManager: initialized");
	pImpl = std::make_unique<CInputManagerImpl>();
}

CInputManager::~CInputManager() {
	pImpl.reset();
	log(SYS, "InputManager: bye");
}

void CInputManager::run() {
	pImpl->inputLoop();
}

void CInputManager::inputLoop() {
	pImpl->inputLoop();
}

void CInputManager::writeToConsole(const std::string &msg) {
	pImpl->writeMessage(msg);
}
