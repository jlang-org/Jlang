#!/bin/bash

set -euo pipefail

fetch_main_branch() {
  echo "[INFO] Fetching latest 'main' branch..."
  git fetch origin main --depth=1
}

get_commit_messages() {
  git log origin/main..HEAD --pretty=format:"%s"
}

validate_commit_messages() {
  local invalid_found=0

  while read -r msg; do
    if [[ ! "$msg" =~ ^\[.+\] ]]; then
      echo "[ERROR] Invalid commit message:"
      echo "        $msg"
      invalid_found=1
    fi
  done

  if [[ $invalid_found -eq 1 ]]; then
    echo
    echo "[FAIL] Commit messages must start with [SOME_TAG]"
    exit 1
  fi
}

main() {
  echo "[CHECK] Validating commit messages..."

  fetch_main_branch
  local commits
  commits=$(get_commit_messages)

  if [[ -z "$commits" ]]; then
    echo "[INFO] No new commits found."
    exit 0
  fi

  echo "$commits" | validate_commit_messages

  echo "[SUCCESS] All commit messages are valid."
}

main "$@"

