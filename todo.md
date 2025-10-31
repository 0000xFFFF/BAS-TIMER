
# weather emoji:
    - struct wttrin_info:
        - has weather buff
        - enum weather { Clear, Rain, ... } -- which anim emoji to use
    - draw_ui weather -> weather enum to color + spinner

# app state struct
    - contains bas_info
    - contains wttrin_info

- write to g_term_buffer and then do 50 cols space padding before printf
