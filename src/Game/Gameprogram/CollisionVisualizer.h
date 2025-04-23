#pragma once

#include "UnoEngine.h"
#include "CollisionDetection.h"

// Class to visualize collision boundaries for debugging
class CollisionVisualizer {
public:
    // Constructor/Destructor
    CollisionVisualizer();
    ~CollisionVisualizer();

    // Initialize the visualizer
    void Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon);

    // Update visual elements
    void Update();

    // Draw collision boundaries
    void Draw();

    // Toggle visibility
    void SetVisible(bool visible) { isVisible_ = visible; }
    bool IsVisible() const { return isVisible_; }

private:
    // Create a simple cube model for visualizing bounding boxes
    void CreateCubeModel();

    // DirectX resources
    DirectXCommon* dxCommon_ = nullptr;
    SpriteCommon* spriteCommon_ = nullptr;

    // Cube model for visualizing bounding boxes
    std::unique_ptr<Model> cubeModel_;
    std::vector<std::unique_ptr<Object3d>> boundingBoxes_;

    // Visibility flag
    bool isVisible_ = false;

    // Color for different types of collision objects
    Vector4 stageBoundsColor_ = { 0.0f, 1.0f, 0.0f, 0.3f }; // Green, semi-transparent
    Vector4 obstacleColor_ = { 1.0f, 0.0f, 0.0f, 0.3f };    // Red, semi-transparent
    Vector4 playerBoundsColor_ = { 0.0f, 0.0f, 1.0f, 0.3f }; // Blue, semi-transparent
};