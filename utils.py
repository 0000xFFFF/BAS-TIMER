from datetime import datetime
import subprocess


def timestamp():
    return datetime.now().strftime("%Y-%m-%d %H:%M:%S")


def get_local_ips():
    """Get all IPv4 addresses excluding 127.0.0.1."""
    try:
        result = subprocess.run(
            "ip -o -4 addr show | awk '{print $4}' | cut -d/ -f1 | grep -v '127.0.0.1'",
            shell=True,
            capture_output=True,
            text=True
        )
        return result.stdout.strip().replace("\n", " ") if result.stdout else "No IP found"
    except Exception as e:
        return f"Error: {e}"
