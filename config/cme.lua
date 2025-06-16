-- Key bindings
local keys = {
    -- Window management
    { mod = "Mod4", key = "j", action = "focus_next" },
    { mod = "Mod4", key = "k", action = "focus_prev" },
    { mod = "Mod4", key = "h", action = "shrink_master" },
    { mod = "Mod4", key = "l", action = "grow_master" },
    { mod = "Mod4", key = "Return", action = "swap_master" },
    { mod = "Mod4", key = "space", action = "toggle_float" },
    { mod = "Mod4", key = "q", action = "kill_window" },
    
    -- Layout control
    { mod = "Mod4", key = "Tab", action = "next_layout" },
    { mod = "Mod4", key = "f", action = "toggle_fullscreen" },
    
    -- Program launcher
    { mod = "Mod4", key = "d", action = "spawn", command = "dmenu_run" },
    { mod = "Mod4", key = "Return", action = "spawn", command = "xterm" },
}

-- Window rules
local rules = {
    -- Make certain windows float by default
    { class = "Gimp", floating = true },
    { class = "Firefox", floating = false },
    { class = "XTerm", floating = false },
}

-- Layouts
local layouts = {
    "tile",    -- Default tiling layout
    "float",   -- Floating layout
    "monocle"  -- Fullscreen layout
}

-- Theme settings
local theme = {
    border_width = 2,
    border_color_focused = "#4c7899",
    border_color_unfocused = "#333333",
    background_color = "#222222"
}

-- Export configuration
return {
    keys = keys,
    rules = rules,
    layouts = layouts,
    theme = theme
}
