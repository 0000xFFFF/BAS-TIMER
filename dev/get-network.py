#!/usr/bin/env python

import subprocess

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

print(f"IP Addresses: {get_local_ips()}")


def get_active_connections():
    """Get active network connections using nmcli."""
    try:
        result = subprocess.run(
            ["nmcli", "-t", "-f", "DEVICE,TYPE,STATE", "connection", "show", "--active"],
            capture_output=True,
            text=True
        )
        
        if result.returncode != 0 or not result.stdout.strip():
            return "No active network connections"

        connections = result.stdout.strip().split("\n")
        formatted_connections = [line.replace(":", " | ") for line in connections]
        return "\n".join(formatted_connections)

    except Exception as e:
        return f"Error: {e}"

print(f"Active Network Connections:\n{get_active_connections()}")

