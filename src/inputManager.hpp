#pragma once

#include <memory>
#include <ncpp/NotCurses.hh>

class CInputManagerImpl {
public:
    CInputManagerImpl();
    ~CInputManagerImpl();
    
    void writeMessage(const std::string& msg);
    void inputLoop();
    
private:
    std::unique_ptr<ncpp::NotCurses> nc;
    struct ncplane* stdplane;
    struct ncplane* msgPlane;
    struct ncplane* sepPlane;
    struct ncplane* inputPlane;
    struct ncreader* reader;
};

class CInputManager {
public:
    CInputManager();
    ~CInputManager();

    void run();
    void inputLoop();
    void writeToConsole(const std::string& msg);
    
private:
    std::unique_ptr<CInputManagerImpl> pImpl;
};

inline std::unique_ptr<CInputManager> g_pInputManager;