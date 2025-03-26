#include "inputManager.hpp"
#include "chatManager.hpp"
#include "commandHandler.hpp"
#include "log.hpp"
#include <ncpp/NotCurses.hh>
#include <ncpp/Plane.hh>
#include <ncpp/Reader.hh>
#include <iostream>

CInputManagerImpl::CInputManagerImpl() {
    // Initialize notcurses with options
    notcurses_options nopts{};
    nopts.flags = NCOPTION_NO_ALTERNATE_SCREEN;
    
    // Create NotCurses instance
    nc = std::make_unique<ncpp::NotCurses>(nopts);
    
    // Get dimensions and standard plane
    unsigned dimx, dimy;
    nc->get_term_dim(&dimy, &dimx);
    
    // Get the C API notcurses object and its standard plane
    struct notcurses* nc_ptr = nc->get_notcurses();
    stdplane = notcurses_stdplane(nc_ptr);
    
    // Set up message area (upper portion)
    struct ncplane_options msgOpts = {
        .y = static_cast<int>(dimy - 3),  // Fix narrowing conversion
        .x = 0,
        .rows = dimy - 3,
        .cols = dimx,
        .userptr = nullptr,
        .name = "messages",
        .resizecb = nullptr,
        .flags = 0,
        .margin_b = 0,
        .margin_r = 0,
    };
    msgPlane = ncplane_create(stdplane, &msgOpts);
    
    // Set up separator line
    struct ncplane_options sepOpts = {
        .y = static_cast<int>(dimy - 3),  // Fix narrowing conversion
        .x = 0,
        .rows = 1,
        .cols = dimx,
        .userptr = nullptr,
        .name = "separator",
        .resizecb = nullptr,
        .flags = 0,
        .margin_b = 0,
        .margin_r = 0,
    };
    sepPlane = ncplane_create(stdplane, &sepOpts);
    
    // Draw separator
    ncplane_putstr_yx(sepPlane, 0, 0, "────────────────── Type your message below ──────────────────");
    
    // Set up input area (bottom)
    struct ncplane_options inputOpts = {
        .y = static_cast<int>(dimy - 2),  // Fix narrowing conversion
        .x = 0,
        .rows = 2,
        .cols = dimx,
        .userptr = nullptr,
        .name = "input",
        .resizecb = nullptr,
        .flags = 0,
        .margin_b = 0,
        .margin_r = 0,
    };
    inputPlane = ncplane_create(stdplane, &inputOpts);
    
    // Create reader for input handling
    ncreader_options ropts{};
    ropts.flags = NCREADER_OPTION_CURSOR;
    reader = ncreader_create(inputPlane, &ropts);
    
    // Set prompt
    ncplane_putstr_yx(inputPlane, 0, 0, "> ");
    ncplane_cursor_move_yx(inputPlane, 0, 2);
    
    // Initial render
    nc->render();
}

CInputManagerImpl::~CInputManagerImpl() {
    if (reader) {
        ncreader_destroy(reader, nullptr);
    }
    if (msgPlane) {
        ncplane_destroy(msgPlane);
    }
    if (sepPlane) {
        ncplane_destroy(sepPlane);
    }
    if (inputPlane) {
        ncplane_destroy(inputPlane);
    }
}

void CInputManagerImpl::writeMessage(const std::string& msg) {
    ncplane_putstr(msgPlane, msg.c_str());
    ncplane_putchar(msgPlane, '\n');
    nc->render();
}

void CInputManagerImpl::inputLoop() {
    bool running = true;
    
    // Write welcome message
    writeMessage("Chat server started. Type /help for commands.");
    
    while (running) {
        // Get input
        ncinput ni;
        uint32_t input = notcurses_get_blocking(nc->get_notcurses(), &ni);
        
        // Process input
        if (input == NCKEY_ENTER) {
            char* content = ncreader_contents(reader);
            if (content && *content) { // Check if content exists and is not empty
                std::string message(content);
                
                // Clear reader for next input
                ncreader_clear(reader);
                ncplane_putstr_yx(inputPlane, 0, 0, "> ");
                ncplane_cursor_move_yx(inputPlane, 0, 2);
                
                // Process command or message
                if (g_pCommandHandler->isCommand(message)) {
                    g_pCommandHandler->handleCommand(message);
                } else {
                    g_pChatManager->newMessage({.msg = message, .username = "Server"});
                }
            }
            free(content);
        } 
        else if (input == NCKEY_ESC) {
            running = false; // Exit on ESC
        }
        else {
            // Pass other inputs to the reader
            ncreader_offer_input(reader, &ni);
        }
        
        nc->render();
    }
}

// CInputManager implementation
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

void CInputManager::writeToConsole(const std::string& msg) {
    pImpl->writeMessage(msg);
}