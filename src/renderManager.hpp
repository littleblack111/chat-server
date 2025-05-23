#pragma once

#include <memory>

#ifndef NO_UI
#include <ftxui/component/screen_interactive.hpp>
#endif

class CRenderManager {
public:
    CRenderManager();
    ~CRenderManager();

    void enterLoop();
    void exitLoop();
    
#ifndef NO_UI
    void setRenderer(ftxui::Component component);
    
private:
    ftxui::ScreenInteractive screen;
    ftxui::Component renderer;
#else
private:
    bool running = false;
#endif

    friend class CInputManager;
};
inline std::unique_ptr<CRenderManager> g_pRenderManager;
