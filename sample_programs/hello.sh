#!/bin/sh
#
# Simple Hello World program for testing uDock containers
#

echo "========================================="
echo "  Hello from uDock Container!"
echo "========================================="
echo ""
echo "Container Information:"
echo "  - Running inside isolated filesystem"
echo "  - Process ID: $$"
echo "  - Working directory: $(pwd)"
echo ""
echo "Environment:"
echo "  - PATH: $PATH"
echo "  - HOME: $HOME"
echo "  - TERM: $TERM"
echo ""

if [ -n "$CUSTOM_MESSAGE" ]; then
    echo "Custom Message: $CUSTOM_MESSAGE"
    echo ""
fi

echo "Directory contents:"
ls -la /

echo ""
echo "Sleeping for 3 seconds..."
sleep 3
echo "Done! Goodbye from container."
