#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <string>
#include <vector>

#include <SDL2/SDL.h>

enum class MoveDir { None, Up, Down, Left, Right };

enum class InputActionType {
    Quit,
    OpenChat,
    ToggleHelp,
    ToggleMusic,
    DismissOverlay,
    Cheat,
    ClickPrimary,
    ClickSecondary,
    ChatText,
    ChatBackspace,
    ChatSubmit,
    ChatCancel,
};

struct InputAction {
    InputActionType type;
    int x = 0;
    int y = 0;
    int cheat_slot = 0;
    std::string text;

    explicit InputAction(InputActionType t): type(t) {}
};

struct InputContext {
    bool chat_active = false;
    bool overlay_visible = false;
};

class InputHandler {
 public:
    InputHandler() = default;

    static std::vector<InputAction> poll_events(const InputContext& context);
    static MoveDir poll_movement(bool chat_active);
};

#endif
