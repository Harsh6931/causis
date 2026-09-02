#!/usr/bin/env python3
"""
Cursor beforeShellExecution hook for the causis project.

Blocks the agent from running any git command that would change
repository history or remote state (commit, push, tag, etc.).
Read-only git commands (status, diff, log, show, ...) are allowed.

Cursor sends a JSON payload on stdin, e.g. {"command": "git commit -m ..."}
and expects a JSON response on stdout with a "permission" field.
"""
import json
import re
import sys

# Substrings that indicate a write/history-changing git operation.
# Matched against the command with git subcommand boundaries in mind,
# so this won't block things like "git log --grep=commit".
BLOCKED_PATTERNS = [
    r"\bgit\s+commit\b",
    r"\bgit\s+push\b",
    r"\bgit\s+tag\b",
    r"\bgit\s+merge\b",
    r"\bgit\s+rebase\b",
    r"\bgit\s+reset\b",
    r"\bgit\s+cherry-pick\b",
    r"\bgit\s+commit\s+--amend\b",
]


def main() -> None:
    try:
        payload = json.load(sys.stdin)
    except Exception:
        # If we can't parse the payload, fail open (allow) rather than
        # breaking the agent's ability to run anything at all.
        print(json.dumps({"permission": "allow"}))
        return

    command = payload.get("command", "") or ""

    for pattern in BLOCKED_PATTERNS:
        if re.search(pattern, command):
            print(json.dumps({
                "permission": "deny",
                "userMessage": (
                    "Blocked: this command would change git history or "
                    "the remote (matched a commit/push/rebase-style git "
                    "command). Commits are done manually in this project."
                ),
                "agentMessage": (
                    "Do not run git commands that write history or push "
                    "to a remote (commit, push, tag, merge, rebase, reset, "
                    "cherry-pick). The user commits manually. Report what "
                    "changed and let them commit it themselves."
                ),
            }))
            return

    print(json.dumps({"permission": "allow"}))


if __name__ == "__main__":
    main()
