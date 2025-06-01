#ifndef CLIENT_HPP
#define CLIENT_HPP

namespace client {
    struct State{
        bool is_open; 
        bool is_fullscreen; 
        int width;
        int height;
    };
    struct Window{
        State state;
        int screen_number;
        int workspace;
        int mouse;
        int window;
    };
    class Client{
        public:
            Client();
            ~Client();
            void window_resize(Window&);
            void window_move_workspace(Window&);
            void window_open(Window&);
            void window_close(Window&);
    };
}

#endif //CLIENT_HPP
