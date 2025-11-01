# need to make new UI
    old ui problems:
        - need to cap term width to 50 (so it won't resize web ui)
        - last status gets left behind (timer status/gas status)
    todo:
        - remove labels, zoom in hella, icons only
        - rearange temps go up, pumps go down (2 rows)

        - use colors for statuses (red if it happened now, blue if it's stale)
            - gradually change the color as time passes (use TEMP_COLORS)
        - make a marquee terminal scrolling col for weather?
            - also maybe make a version which scrolls left stops then scrolls back
                - kinda like a neon sign
        - add solar anim, when on, next to temp (like check/warn)

# app state struct
    - contains bas_info
    - contains wttrin_info
- save app state every 15 min
- load app state on boot

# web ui
    - make on/off buttons look like this:
        icon  [on | off]
              [*  |    ]  -- make this a color
               ^     ^-- clicking on this side of button turns it off
               |-------- clicking on this side will turn it on
    - need to fix mobile padding/margin for buttons
        - when i click them they become huge hitting the border of other buttons
