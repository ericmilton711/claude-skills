#!/bin/bash
# Docker Setup Script
# Run this script ONCE to set up Docker with passwordless sudo

set -e  # Exit on error

echo "🐳 Docker Setup Script"
echo "====================="
echo ""

# Check if running as root
if [ "$EUID" -eq 0 ]; then
    echo "❌ Don't run this script as root (with sudo)"
    echo "   Just run: ./setup-docker.sh"
    echo "   It will ask for your password when needed."
    exit 1
fi

echo "Step 1: Installing passwordless sudo configuration..."
if sudo visudo -c -f docker-sudoers > /dev/null 2>&1; then
    sudo cp docker-sudoers /etc/sudoers.d/docker
    sudo chmod 0440 /etc/sudoers.d/docker
    echo "✅ Sudoers configuration installed"
else
    echo "❌ Sudoers file has syntax errors - skipping"
    exit 1
fi

echo ""
echo "Step 2: Starting Docker service..."
sudo systemctl start docker
echo "✅ Docker started"

echo ""
echo "Step 3: Enabling Docker on boot..."
sudo systemctl enable docker
echo "✅ Docker enabled"

echo ""
echo "Step 4: Adding user to docker group..."
sudo usermod -aG docker $USER
echo "✅ User added to docker group"

echo ""
echo "Step 5: Testing Docker service..."
if sudo systemctl is-active --quiet docker; then
    echo "✅ Docker is running"
else
    echo "❌ Docker is not running"
    exit 1
fi

echo ""
echo "╔════════════════════════════════════════════════════════════╗"
echo "║  🎉 Docker Setup Complete!                                 ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo ""
echo "⚠️  IMPORTANT: To use Docker without sudo, you need to:"
echo "   1. Log out and log back in (or reboot)"
echo "   2. OR run: newgrp docker"
echo ""
echo "After that, test with:"
echo "   docker run hello-world"
echo ""
echo "Future docker/systemctl commands won't need password!"
