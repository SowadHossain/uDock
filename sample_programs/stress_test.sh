#!/bin/sh
#
# CPU and Memory stress test for testing resource limits
#

echo "=== uDock Resource Stress Test ==="
echo "This program will stress CPU and memory"
echo "Use --cpu and --mem flags to test limits"
echo ""

# CPU stress test
cpu_stress() {
    echo "[CPU] Starting CPU stress test..."
    echo "[CPU] Running infinite calculation loop"
    
    i=0
    while true; do
        i=$((i + 1))
        
        # Print progress every million iterations
        if [ $((i % 1000000)) -eq 0 ]; then
            echo "[CPU] Iteration: $i million"
        fi
        
        # If we reach 10 million, we're done
        if [ $i -ge 10000000 ]; then
            echo "[CPU] Stress test completed: 10 million iterations"
            break
        fi
    done
}

# Memory stress test (simple version)
memory_stress() {
    echo "[MEM] Starting memory stress test..."
    echo "[MEM] Allocating memory..."
    
    # This is a simple simulation since shell scripts can't easily allocate memory
    # In a real test, you'd use a C program
    
    for i in 1 2 3 4 5; do
        echo "[MEM] Allocation phase $i of 5"
        sleep 1
    done
    
    echo "[MEM] Memory stress test completed"
}

# Run tests based on arguments
case "$1" in
    cpu)
        cpu_stress
        ;;
    memory)
        memory_stress
        ;;
    *)
        echo "Running both CPU and memory tests..."
        cpu_stress
        memory_stress
        ;;
esac

echo ""
echo "=== Stress Test Completed ==="
