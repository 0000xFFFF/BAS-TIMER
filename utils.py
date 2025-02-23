from datetime import datetime
import subprocess


def strip_ansi(text: str) -> str:
    """Removes ANSI escape sequences manually for performance optimization."""
    result = []
    in_escape = False

    i = 0
    while i < len(text):
        if text[i] == "\x1b":  # Start of ANSI sequence
            in_escape = True
        elif in_escape:
            if text[i] in "@-_ABCDEFGHJKSTfmnsulh":
                in_escape = False  # End of ANSI sequence
        else:
            result.append(text[i])  # Normal character

        i += 1

    return "".join(result)


def timestamp():
    return datetime.now().strftime("%Y-%m-%d %H:%M:%S")


def time_to_str(t):
    return datetime.fromtimestamp(t).strftime("%Y-%m-%d %H:%M:%S")


def get_local_ips():
    """Get all IPv4 addresses excluding 127.0.0.1."""
    try:
        result = subprocess.run(
            "ip -o -4 addr show | awk '{print $4}' | cut -d/ -f1 | grep -v '127.0.0.1'",
            shell=True,
            capture_output=True,
            text=True,
        )
        return (
            result.stdout.strip().replace("\n", " ") if result.stdout else "No IP found"
        )
    except Exception as e:
        return f"Error: {e}"
