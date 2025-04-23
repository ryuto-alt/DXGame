// src/Engine/Graphics/Model.cpp
#include "Model.h"
#include "TextureManager.h"
#include <fstream>
#include <sstream>
#include <cassert>
#include <unordered_map>
#include <cmath>

Model::Model() : dxCommon_(nullptr) {}

Model::~Model() {}

void Model::Initialize(DirectXCommon* dxCommon) {
    assert(dxCommon);
    dxCommon_ = dxCommon;
}

void Model::LoadFromObj(const std::string& directoryPath, const std::string& filename) {
    // モデルデータの読み込み
    modelData_ = LoadObjFile(directoryPath, filename);

    // モデルデータを最適化（UV球などの表示品質向上のため）
    // ファイル名も渡すように修正
    OptimizeTriangles(modelData_, filename);

    // テクスチャの読み込み
    if (!modelData_.material.textureFilePath.empty()) {
        // テクスチャパスをログに出力
        OutputDebugStringA(("Model: Texture path from MTL: " + modelData_.material.textureFilePath + "\n").c_str());

        // テクスチャが存在するかチェック
        DWORD fileAttributes = GetFileAttributesA(modelData_.material.textureFilePath.c_str());
        if (fileAttributes != INVALID_FILE_ATTRIBUTES) {
            // テクスチャが存在する場合のみ読み込み
            TextureManager::GetInstance()->LoadTexture(modelData_.material.textureFilePath);
            OutputDebugStringA(("Model: Texture loaded - " + modelData_.material.textureFilePath + "\n").c_str());
        }
        else {
            // テクスチャが見つからない場合、別の場所を探す
            OutputDebugStringA(("WARNING: Texture file not found at: " + modelData_.material.textureFilePath + "\n").c_str());

            // ファイル名のみを抽出
            std::string filenameOnly = modelData_.material.textureFilePath;
            size_t lastSlash = filenameOnly.find_last_of("/\\");
            if (lastSlash != std::string::npos) {
                filenameOnly = filenameOnly.substr(lastSlash + 1);
            }

            // 複数の可能性のある場所を探索
            std::vector<std::string> possiblePaths = {
                "Resources/textures/" + filenameOnly,
                directoryPath + "/" + filenameOnly,
                "Resources/" + filenameOnly,
                "Resources/models/" + filenameOnly
            };

            bool found = false;
            for (const auto& path : possiblePaths) {
                OutputDebugStringA(("Model: Trying alternative path: " + path + "\n").c_str());
                if (GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES) {
                    // 見つかった場合はパスを更新して読み込み
                    modelData_.material.textureFilePath = path;
                    TextureManager::GetInstance()->LoadTexture(path);
                    OutputDebugStringA(("Model: Texture found and loaded from: " + path + "\n").c_str());
                    found = true;
                    break;
                }
            }

            if (!found) {
                // どこにも見つからない場合
                OutputDebugStringA("WARNING: Texture file not found in any location. Clearing texture path.\n");
                modelData_.material.textureFilePath = ""; // パスをクリア
            }
        }
    }
    else {
        OutputDebugStringA("Model: No texture specified in MTL file\n");
    }

    // 頂点バッファの作成
    vertexResource_ = dxCommon_->CreateBufferResource(sizeof(VertexData) * modelData_.vertices.size());

    // 頂点バッファビューの設定
    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * modelData_.vertices.size());
    vertexBufferView_.StrideInBytes = sizeof(VertexData);

    // 頂点データの書き込み
    VertexData* vertexData = nullptr;
    vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
    std::memcpy(vertexData, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());

    // デバッグ情報
    OutputDebugStringA(("Model: Loaded " + std::to_string(modelData_.vertices.size()) + " vertices from " + filename + "\n").c_str());
}

// UV球などの表示品質を向上させるためのモデルデータ最適化関数
void Model::OptimizeTriangles(ModelData& modelData, const std::string& filename) {
    // 最適化前の頂点数を保存
    size_t originalVertexCount = modelData.vertices.size();

    // 重複頂点の検出と削除のためのデータ構造
    std::vector<VertexData> optimizedVertices;
    std::vector<uint32_t> indices;
    std::unordered_map<std::string, uint32_t> vertexMap;

    // 各頂点を処理
    for (const auto& vertex : modelData.vertices) {
        // 頂点のハッシュキーを作成（類似頂点の統合を強化）
        // 精度を大幅に下げて頂点統合を促進（小数点以下2桁に制限）
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f",
            vertex.position.x, vertex.position.y, vertex.position.z,
            vertex.normal.x, vertex.normal.y, vertex.normal.z,
            vertex.texcoord.x, vertex.texcoord.y);
        std::string key = buffer;

        // この頂点がまだ追加されていなければ追加する
        if (vertexMap.find(key) == vertexMap.end()) {
            vertexMap[key] = static_cast<uint32_t>(optimizedVertices.size());

            // 法線を正規化して品質を向上
            VertexData normalizedVertex = vertex;
            float length = std::sqrt(
                normalizedVertex.normal.x * normalizedVertex.normal.x +
                normalizedVertex.normal.y * normalizedVertex.normal.y +
                normalizedVertex.normal.z * normalizedVertex.normal.z
            );

            if (length > 0.0001f) {
                normalizedVertex.normal.x /= length;
                normalizedVertex.normal.y /= length;
                normalizedVertex.normal.z /= length;
            }

            optimizedVertices.push_back(normalizedVertex);
        }

        // インデックスを追加
        indices.push_back(vertexMap[key]);
    }

    // 法線平均化処理を追加（共有頂点の法線を平均化して滑らかにする）
    std::vector<Vector3> smoothedNormals(optimizedVertices.size(), { 0.0f, 0.0f, 0.0f });
    std::vector<int> normalCount(optimizedVertices.size(), 0);

    // 各三角形の法線を集計
    for (size_t i = 0; i < indices.size(); i += 3) {
        if (i + 2 < indices.size()) {
            // 三角形の頂点インデックス
            uint32_t idx0 = indices[i];
            uint32_t idx1 = indices[i + 1];
            uint32_t idx2 = indices[i + 2];

            // 三角形の辺ベクトル
            Vector3 edge1 = {
                optimizedVertices[idx1].position.x - optimizedVertices[idx0].position.x,
                optimizedVertices[idx1].position.y - optimizedVertices[idx0].position.y,
                optimizedVertices[idx1].position.z - optimizedVertices[idx0].position.z
            };

            Vector3 edge2 = {
                optimizedVertices[idx2].position.x - optimizedVertices[idx0].position.x,
                optimizedVertices[idx2].position.y - optimizedVertices[idx0].position.y,
                optimizedVertices[idx2].position.z - optimizedVertices[idx0].position.z
            };

            // 外積で面法線を計算
            Vector3 faceNormal = {
                edge1.y * edge2.z - edge1.z * edge2.y,
                edge1.z * edge2.x - edge1.x * edge2.z,
                edge1.x * edge2.y - edge1.y * edge2.x
            };

            // 法線の長さを計算
            float length = std::sqrt(
                faceNormal.x * faceNormal.x +
                faceNormal.y * faceNormal.y +
                faceNormal.z * faceNormal.z
            );

            // 法線を正規化
            if (length > 0.0001f) {
                faceNormal.x /= length;
                faceNormal.y /= length;
                faceNormal.z /= length;

                // 各頂点に面法線を加算
                smoothedNormals[idx0].x += faceNormal.x;
                smoothedNormals[idx0].y += faceNormal.y;
                smoothedNormals[idx0].z += faceNormal.z;
                normalCount[idx0]++;

                smoothedNormals[idx1].x += faceNormal.x;
                smoothedNormals[idx1].y += faceNormal.y;
                smoothedNormals[idx1].z += faceNormal.z;
                normalCount[idx1]++;

                smoothedNormals[idx2].x += faceNormal.x;
                smoothedNormals[idx2].y += faceNormal.y;
                smoothedNormals[idx2].z += faceNormal.z;
                normalCount[idx2]++;
            }
        }
    }

    // 法線を平均化
    for (size_t i = 0; i < optimizedVertices.size(); i++) {
        if (normalCount[i] > 0) {
            smoothedNormals[i].x /= normalCount[i];
            smoothedNormals[i].y /= normalCount[i];
            smoothedNormals[i].z /= normalCount[i];

            // 長さを正規化
            float length = std::sqrt(
                smoothedNormals[i].x * smoothedNormals[i].x +
                smoothedNormals[i].y * smoothedNormals[i].y +
                smoothedNormals[i].z * smoothedNormals[i].z
            );

            if (length > 0.0001f) {
                smoothedNormals[i].x /= length;
                smoothedNormals[i].y /= length;
                smoothedNormals[i].z /= length;
            }

            // 平滑化された法線を適用
            optimizedVertices[i].normal = smoothedNormals[i];
        }
    }

    // UV球の場合は特別な処理（UV座標から理論的な法線を計算）
    bool isSphere = (filename.find("sphere") != std::string::npos) ||
        (filename.find("ball") != std::string::npos) ||
        (filename.find("globe") != std::string::npos);

    if (isSphere) {
        for (size_t i = 0; i < optimizedVertices.size(); i++) {
            // UV座標から球面上の位置を計算
            float u = optimizedVertices[i].texcoord.x;
            float v = optimizedVertices[i].texcoord.y;

            // 球面座標
            float phi = u * 2.0f * 3.14159265f;  // 0～2π
            float theta = v * 3.14159265f;       // 0～π

            // 球面座標から理論的な法線を計算
            Vector3 theoreticalNormal;
            theoreticalNormal.x = std::sin(theta) * std::cos(phi);
            theoreticalNormal.y = std::cos(theta);
            theoreticalNormal.z = std::sin(theta) * std::sin(phi);

            // 99%理論的な法線を使用して完全な球面を強制
            Vector3 blendedNormal;
            blendedNormal.x = 0.99f * theoreticalNormal.x + 0.01f * optimizedVertices[i].normal.x;
            blendedNormal.y = 0.99f * theoreticalNormal.y + 0.01f * optimizedVertices[i].normal.y;
            blendedNormal.z = 0.99f * theoreticalNormal.z + 0.01f * optimizedVertices[i].normal.z;

            // 長さを正規化
            float length = std::sqrt(
                blendedNormal.x * blendedNormal.x +
                blendedNormal.y * blendedNormal.y +
                blendedNormal.z * blendedNormal.z
            );

            if (length > 0.0001f) {
                blendedNormal.x /= length;
                blendedNormal.y /= length;
                blendedNormal.z /= length;
            }

            // 混合された法線を適用
            optimizedVertices[i].normal = blendedNormal;
        }
    }

    // インデックスリストから頂点配列を再構築
    std::vector<VertexData> rebuiltVertices;
    for (size_t i = 0; i < indices.size(); i += 3) {
        // 三角形の頂点を追加（順序を維持）
        if (i + 2 < indices.size()) {
            rebuiltVertices.push_back(optimizedVertices[indices[i + 2]]);
            rebuiltVertices.push_back(optimizedVertices[indices[i + 1]]);
            rebuiltVertices.push_back(optimizedVertices[indices[i]]);
        }
    }

    // 最適化された頂点配列で元の配列を置き換え
    if (!rebuiltVertices.empty()) {
        modelData.vertices = rebuiltVertices;
    }
}

ModelData Model::LoadObjFile(const std::string& directoryPath, const std::string& filename) {
    ModelData modelData; // 構築するModelData
    std::vector<Vector4> positions; // 位置
    std::vector<Vector3> normals; // 法線
    std::vector<Vector2> texcoords; // テクスチャ座標
    std::string line; // ファイルから読んだ1行を格納するもの

    // ファイル読み込み
    std::ifstream file(directoryPath + "/" + filename); // fileを開く
    assert(file.is_open()); // 開けなかったら止める

    OutputDebugStringA(("Model: Loading OBJ file: " + directoryPath + "/" + filename + "\n").c_str());

    while (std::getline(file, line)) {
        std::string identifier;
        std::istringstream s(line);
        s >> identifier; // 先頭の識別子を読む

        if (identifier == "v") {
            Vector4 position;
            s >> position.x >> position.y >> position.z;
            position.w = 1.0f;
            position.x *= -1;
            positions.push_back(position);
        }
        else if (identifier == "vt") {
            Vector2 texcoord;
            s >> texcoord.x >> texcoord.y;
            texcoord.y = 1 - texcoord.y;
            texcoords.push_back(texcoord);
        }
        else if (identifier == "vn") {
            Vector3 normal;
            s >> normal.x >> normal.y >> normal.z;
            normal.x *= -1;
            normals.push_back(normal);
        }
        else if (identifier == "f") {
            VertexData triangle[3];
            // 面は三角形限定。その他は未対応
            for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
                std::string vertexDefinition;
                s >> vertexDefinition;
                // 頂点の要素へのIndexは「位置・UV・法線」で格納されているので、分解してIndexを取得する
                std::istringstream v(vertexDefinition);
                uint32_t elementIndices[3];
                for (int32_t element = 0; element < 3; ++element) {
                    std::string index;
                    std::getline(v, index, '/'); // 区切りでインデックスを読んでいく
                    elementIndices[element] = std::stoi(index);
                }
                // 要素へのIndexから、実際の要素の値を取得して、頂点を構築する
                Vector4 position = positions[elementIndices[0] - 1];
                Vector2 texcoord = texcoords[elementIndices[1] - 1];
                Vector3 normal = normals[elementIndices[2] - 1];

                triangle[faceVertex] = { position, texcoord, normal };
            }
            // 頂点を逆順で登録することで、周り順を逆にする
            modelData.vertices.push_back(triangle[2]);
            modelData.vertices.push_back(triangle[1]);
            modelData.vertices.push_back(triangle[0]);
        }
        else if (identifier == "mtllib") {
            // materialTemplateLibraryファイルの名前を取得する
            std::string materialFilename;
            s >> materialFilename;

            // MTLファイル名をログに出力
            OutputDebugStringA(("Model: Found MTL reference: " + materialFilename + "\n").c_str());

            // 基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
            modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
        }
    }
    return modelData;
}

MaterialData Model::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
    MaterialData materialData; // 構築するMaterialData
    std::string line; // ファイルから読んだ1行を格納するもの

    // ファイルのフルパス
    std::string mtlPath = directoryPath + "/" + filename;
    std::ifstream file(mtlPath); // ファイルを開く

    // ファイルが開けなかった場合は警告を出力して、デフォルト値を返す
    if (!file.is_open()) {
        OutputDebugStringA(("WARNING: Failed to open MTL file - " + mtlPath + "\n").c_str());
        return materialData;
    }

    OutputDebugStringA(("Model: Successfully opened MTL file - " + mtlPath + "\n").c_str());

    while (std::getline(file, line)) {
        std::string identifier;
        std::stringstream s(line);
        s >> identifier;

        // identifierの応じた処理
        if (identifier == "map_Kd") {
            std::string textureFilename;
            s >> textureFilename;

            // テクスチャファイル名をログに出力
            OutputDebugStringA(("MTL Parser: Found texture reference: " + textureFilename + "\n").c_str());

            // 連結してファイルパスにする
            materialData.textureFilePath = directoryPath + "/" + textureFilename;

            // フルパスをログに出力
            OutputDebugStringA(("MTL Parser: Full texture path constructed: " + materialData.textureFilePath + "\n").c_str());
        }
        else if (identifier == "Ka") {
            // ambient color
            s >> materialData.ambient.x >> materialData.ambient.y >> materialData.ambient.z;
            materialData.ambient.w = 1.0f;
        }
        else if (identifier == "Kd") {
            // diffuse color
            s >> materialData.diffuse.x >> materialData.diffuse.y >> materialData.diffuse.z;
            materialData.diffuse.w = 1.0f;
        }
        else if (identifier == "Ks") {
            // specular color
            s >> materialData.specular.x >> materialData.specular.y >> materialData.specular.z;
            materialData.specular.w = 1.0f;
        }
        else if (identifier == "Ns") {
            // shininess
            s >> materialData.shininess;
        }
        else if (identifier == "d" || identifier == "Tr") {
            // transparency (d) or transparency inverted (Tr)
            if (identifier == "d") {
                s >> materialData.alpha;
            }
            else { // Tr (transparency inverted)
                float tr;
                s >> tr;
                materialData.alpha = 1.0f - tr;
            }
        }
    }

    return materialData;
}