// DONE
void Window::implCtor();
void Window::implDtor();
void Window::implDestroy();
bool Window::implCreate();
void Window::destroyAll();
bool Window::created();
// NOT DONE
void Window::implSetDimensions();
void Window::implSetCursor();
void Window::implSetCursorHide();
void Window::implSetFPS();
void Window::implSetFullScreen();
void Window::implSetTitle();
void Window::implSetVSync();
Window& Window::hide();
Window& Window::iconify();
Window& Window::show();

// AL_GRAPHICS
// enum ATTRIBUTEBIT commented out (hpp)

// AL_BUFFEROBJECT
// VBO CBO PBO object implementation commented out (cpp)
// not used anywhere?

// AL_DISPLAYLIST
// erased out almost everything

// AL_LIGHT
// a lot of fixed pipeline functions

// AL_SHADER
// extension related functions needs to be updated

// EasyFBO yet included