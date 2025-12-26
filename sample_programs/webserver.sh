#!/bin/sh
#
# Simple "web server" simulator for testing long-running containers
# This simulates a server that runs indefinitely and logs requests
#

echo "[SERVER] Starting uDock Web Server on port 8080..."
echo "[SERVER] Server PID: $$"
echo "[SERVER] Environment: $SERVER_ENV"
echo "[SERVER] Debug mode: $DEBUG"
echo ""

# Simulate server initialization
echo "[SERVER] Loading configuration..."
sleep 1
echo "[SERVER] Binding to port 8080..."
sleep 1
echo "[SERVER] Server ready to accept connections"
echo ""

# Simulate incoming requests
REQUEST_COUNT=0

while true; do
    # Simulate a request every 5 seconds
    sleep 5
    REQUEST_COUNT=$((REQUEST_COUNT + 1))
    
    # Random request simulation
    RANDOM_NUM=$((REQUEST_COUNT % 4))
    
    case $RANDOM_NUM in
        0)
            echo "[SERVER] GET /index.html - 200 OK (Request #$REQUEST_COUNT)"
            ;;
        1)
            echo "[SERVER] GET /api/users - 200 OK (Request #$REQUEST_COUNT)"
            ;;
        2)
            echo "[SERVER] POST /api/login - 201 Created (Request #$REQUEST_COUNT)"
            ;;
        3)
            echo "[SERVER] GET /static/style.css - 200 OK (Request #$REQUEST_COUNT)"
            ;;
    esac
    
    # Every 10 requests, show stats
    if [ $((REQUEST_COUNT % 10)) -eq 0 ]; then
        echo "[SERVER] Statistics: Handled $REQUEST_COUNT requests so far"
        echo "[SERVER] Memory usage: OK, CPU usage: Normal"
        echo ""
    fi
done
