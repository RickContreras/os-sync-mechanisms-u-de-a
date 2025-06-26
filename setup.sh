#!/bin/bash

# setup.sh - Script para configurar el proyecto OS Synchronization Lab
# Autor: Your Name
# Fecha: $(date)

set -e  # Exit on any error

echo "🚀 Setting up OS Synchronization Lab MVP..."

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if we're in a DevContainer
if [ -n "$REMOTE_CONTAINERS" ] || [ -n "$CODESPACES" ]; then
    print_status "Running in DevContainer/Codespaces environment"
    IN_CONTAINER=true
else
    print_status "Running in local environment"
    IN_CONTAINER=false
fi

# Create project structure
print_status "Creating project directories..."

# Main directories
mkdir -p src/task1_queue
mkdir -p src/task2_producer_consumer
mkdir -p src/task3_dining_philosophers
mkdir -p src/go_implementations/queue
mkdir -p src/go_implementations/producer_consumer
mkdir -p src/go_implementations/dining_philosophers
mkdir -p src/common
mkdir -p tests/unit
mkdir -p tests/integration
mkdir -p tests/performance
mkdir -p build
mkdir -p output/sample_outputs
mkdir -p docs
mkdir -p scripts
mkdir -p benchmarks
mkdir -p tools
mkdir -p .vscode

print_status "Project structure created successfully!"

# Create .vscode directory and basic configuration
print_status "Setting up VS Code configuration..."

# Create basic launch.json for debugging
cat > .vscode/launch.json << 'EOF'
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug Queue Test",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/queue_test",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ],
            "preLaunchTask": "build-queue-test"
        }
    ]
}
EOF

# Create basic tasks.json
cat > .vscode/tasks.json << 'EOF'
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "build-all",
            "type": "shell",
            "command": "make",
            "args": ["all"],
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "presentation": {
                "echo": true,
                "reveal": "always",
                "focus": false,
                "panel": "shared"
            }
        },
        {
            "label": "build-queue-test",
            "type": "shell",
            "command": "make",
            "args": ["queue_test"],
            "group": "build"
        },
        {
            "label": "run-tests",
            "type": "shell",
            "command": "make",
            "args": ["test"],
            "group": "test"
        },
        {
            "label": "clean",
            "type": "shell",
            "command": "make",
            "args": ["clean"],
            "group": "build"
        },
        {
            "label": "valgrind-check",
            "type": "shell",
            "command": "make",
            "args": ["valgrind"],
            "group": "test"
        }
    ]
}
EOF

print_status "VS Code configuration created!"


# Make scripts executable
chmod +x scripts/build.sh
chmod +x scripts/run_tests.sh
chmod +x tools/valgrind_check.sh

print_status "Scripts made executable!"



# Final instructions
echo ""
echo -e "${BLUE}🎉 OS Synchronization Lab MVP Setup Complete!${NC}"
echo ""
echo "Next steps:"
echo "1. Open the project in VS Code (if not already)"
echo "2. Build the project: make all"
echo "3. Run tests: make test"
echo "4. Check memory: make valgrind"
echo ""
echo "Key files:"
echo "- src/task1_queue/: Thread-safe queue implementation"
echo "- Makefile: Build configuration"
echo "- README.md: Project documentation"
echo ""
echo "Happy coding! 🚀"