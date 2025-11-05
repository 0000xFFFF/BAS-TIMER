# need to make new UI
- use colors for statuses (red if it happened now, blue if it's stale)
    - gradually change the color as time passes (use TEMP_COLORS)
- add solar anim, when on, next to temp (like check/warn)

# app state struct
    - contains bas_info
    - contains wttrin_info
- save app state every 15 min
- load app state on boot

# web ui
    - fix mobile:
        - fixed term size
        - fix font for Dusk Dawn emojis are the same
    - make on/off buttons look like this:
        icon  [on | off]
              [*  |    ]  -- make this a color
               ^     ^-- clicking on this side of button turns it off
               |-------- clicking on this side will turn it on
    - need to fix mobile padding/margin for buttons
        - when i click them they become huge hitting the border of other buttons
