#include "CollisionVisualizer.h"

CollisionVisualizer::CollisionVisualizer() {
    // Constructor
}

CollisionVisualizer::~CollisionVisualizer() {
    // Destructor - resources cleaned up automatically by unique_ptr
}

void CollisionVisualizer::Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon) {
    // Store dependencies
    dxCommon_ = dxCommon;
    spriteCommon_ = spriteCommon;

    // Create a simple cube model for visualization
    CreateCubeModel();

    // Create visualization objects for stage bounds
    const BoundingBox& stageBounds = CollisionDetection::GetStageBounds();

    // Create a bounding box visualizer for the stage bounds
    auto stageBox = std::make_unique<Object3d>();
    stageBox->Initialize(dxCommon_, spriteCommon_);
    stageBox->SetModel(cubeModel_.get());

    // Calculate center position and scale for the bounding box
    Vector3 center;
    center.x = (stageBounds.min.x + stageBounds.max.x) * 0.5f;
    center.y = (stageBounds.min.y + stageBounds.max.y) * 0.5f;
    center.z = (stageBounds.min.z + stageBounds.max.z) * 0.5f;

    Vector3 scale;
    scale.x = stageBounds.max.x - stageBounds.min.x;
    scale.y = stageBounds.max.y - stageBounds.min.y;
    scale.z = stageBounds.max.z - stageBounds.min.z;

    stageBox->SetPosition(center);
    stageBox->SetScale(scale);
    stageBox->SetColor(stageBoundsColor_);
    stageBox->SetEnableLighting(false); // Disable lighting for visualization

    boundingBoxes_.push_back(std::move(stageBox));

    // Create bounding box visualizers for each obstacle
    for (size_t i = 0; i < CollisionDetection::GetObstacleCount(); i++) {
        const BoundingBox& obstacle = CollisionDetection::GetObstacle(i);

        auto obstacleBox = std::make_unique<Object3d>();
        obstacleBox->Initialize(dxCommon_, spriteCommon_);
        obstacleBox->SetModel(cubeModel_.get());

        // Calculate center position and scale for the obstacle
        Vector3 obsCenter;
        obsCenter.x = (obstacle.min.x + obstacle.max.x) * 0.5f;
        obsCenter.y = (obstacle.min.y + obstacle.max.y) * 0.5f;
        obsCenter.z = (obstacle.min.z + obstacle.max.z) * 0.5f;

        Vector3 obsScale;
        obsScale.x = obstacle.max.x - obstacle.min.x;
        obsScale.y = obstacle.max.y - obstacle.min.y;
        obsScale.z = obstacle.max.z - obstacle.min.z;

        obstacleBox->SetPosition(obsCenter);
        obstacleBox->SetScale(obsScale);
        obstacleBox->SetColor(obstacleColor_);
        obstacleBox->SetEnableLighting(false); // Disable lighting for visualization

        boundingBoxes_.push_back(std::move(obstacleBox));
    }

    // Initialize as hidden
    isVisible_ = false;

    OutputDebugStringA("CollisionVisualizer: Successfully initialized\n");
}

void CollisionVisualizer::Update() {
    // Update all bounding box visualizers
    for (auto& box : boundingBoxes_) {
        box->Update();
    }
}

void CollisionVisualizer::Draw() {
    // Skip drawing if not visible
    if (!isVisible_) {
        return;
    }

    // Draw all bounding box visualizers
    for (auto& box : boundingBoxes_) {
        box->Draw();
    }
}

void CollisionVisualizer::CreateCubeModel() {
    // Create a simple unit cube model for visualization
    cubeModel_ = std::make_unique<Model>();
    cubeModel_->Initialize(dxCommon_);

    // We would normally load from a file, but for simplicity, let's create a temporary
    // cube model programmatically

    // For a real implementation, you would:
    // 1. Load a simple cube model from OBJ file
    // 2. Or create a procedural cube mesh with vertices and faces

    // For now, we'll just load a simple cube model from a predefined path
    // Assuming there's a basic cube model available
    cubeModel_->LoadFromObj("Resources/models", "cube.obj");

    // If the above fails, you'd need to implement a procedural cube creation method
    // or ensure a cube.obj file exists in the resources folder

    OutputDebugStringA("CollisionVisualizer: Cube model created\n");
}