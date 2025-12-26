#!/bin/sh
#
# Simple counter program - counts from 1 to N
# Good for testing log following (mdock logs -f)
#

# Get count limit from environment or use default
COUNT=${COUNT:-10}
DELAY=${DELAY:-1}

echo "=== Counter Program ==="
echo "Counting from 1 to $COUNT with ${DELAY}s delay"
echo ""

i=1
while [ $i -le $COUNT ]; do
    echo "[$(date +%H:%M:%S)] Count: $i/$COUNT"
    sleep $DELAY
    i=$((i + 1))
done

echo ""
echo "=== Counter Complete! ==="
echo "Final count: $COUNT"
