#!/bin/sh

# ---- HTTP response headers ----
echo 'Content-Type: text/plain; charset=UTF-8'
echo ''

# ---- Body echo ----
# If CONTENT_LENGTH is provided (typical for POST), read exactly that many bytes.
# Otherwise, read until EOF (works for servers that dechunk / close stdin).
if [ -n "${CONTENT_LENGTH:-}" ]; then
  dd bs=1 count="$CONTENT_LENGTH" 2>/dev/null
else
  cat
fi
