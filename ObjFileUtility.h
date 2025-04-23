#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>  // stringstream用に追加
#include <Windows.h> // OutputDebugStringA用
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

// OBJファイルの検証と修正を行うユーティリティクラス
class ObjFileUtility {
public:
    // OBJファイルの基本情報を取得する
    static void GetObjFileInfo(const std::string& filePath, bool verbose = false) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            OutputDebugStringA(("Failed to open file: " + filePath + "\n").c_str());
            return;
        }

        int vertexCount = 0;
        int faceCount = 0;
        int normalCount = 0;
        int texcoordCount = 0;
        std::string line;

        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            if (line[0] == 'v') {
                if (line[1] == ' ') vertexCount++;
                else if (line[1] == 'n') normalCount++;
                else if (line[1] == 't') texcoordCount++;
            }
            else if (line[0] == 'f') {
                faceCount++;
            }
        }

        std::string info = "OBJ File Info: " + filePath + "\n";
        info += "  Vertices: " + std::to_string(vertexCount) + "\n";
        info += "  Normals: " + std::to_string(normalCount) + "\n";
        info += "  TexCoords: " + std::to_string(texcoordCount) + "\n";
        info += "  Faces: " + std::to_string(faceCount) + "\n";

        OutputDebugStringA(info.c_str());

        if (verbose) {
            // 詳細情報（最初の数頂点のみ）
            file.clear();
            file.seekg(0, std::ios::beg);

            int count = 0;
            std::string detailInfo = "First vertices:\n";

            while (std::getline(file, line) && count < 10) {
                if (line.empty() || line[0] == '#') continue;
                if (line[0] == 'v' && line[1] == ' ') {
                    detailInfo += "  " + line + "\n";
                    count++;
                }
            }

            OutputDebugStringA(detailInfo.c_str());
        }
    }

    // OBJファイルを検証し、必要に応じて修正する
    static bool VerifyAndFixObjFile(const std::string& sourceFilePath, const std::string& destFilePath) {
        std::ifstream source(sourceFilePath);
        if (!source.is_open()) {
            OutputDebugStringA(("Failed to open source file: " + sourceFilePath + "\n").c_str());
            return false;
        }

        std::ofstream dest(destFilePath);
        if (!dest.is_open()) {
            OutputDebugStringA(("Failed to create destination file: " + destFilePath + "\n").c_str());
            return false;
        }

        std::string line;
        int lineCount = 0;
        int fixedIssues = 0;

        while (std::getline(source, line)) {
            lineCount++;
            bool modified = false;

            // 行の先頭と末尾の空白を削除
            line = TrimString(line);

            // 空行をスキップ
            if (line.empty()) continue;

            // コメント行はそのまま書き込む
            if (line[0] == '#') {
                dest << line << std::endl;
                continue;
            }

            // 頂点データの修正チェック
            if (line[0] == 'v' && line[1] == ' ') {
                // 頂点座標の処理
                std::vector<float> coords = ParseFloats(line.substr(1));
                if (coords.size() >= 3) {
                    // 座標系の変換（必要に応じて）
                    // 例: X座標を反転しない
                    // coords[0] = -coords[0];

                    // 修正した頂点データを書き込む
                    dest << "v " << coords[0] << " " << coords[1] << " " << coords[2] << std::endl;
                    modified = true;
                }
            }
            else if (line[0] == 'v' && line[1] == 'n') {
                // 法線ベクトルの処理
                std::vector<float> coords = ParseFloats(line.substr(2));
                if (coords.size() >= 3) {
                    // 法線ベクトルを反転しない
                    // coords[0] = -coords[0];

                    // 修正した法線データを書き込む
                    dest << "vn " << coords[0] << " " << coords[1] << " " << coords[2] << std::endl;
                    modified = true;
                }
            }
            else if (line[0] == 'f') {
                // 面の頂点順序は変更しない
                dest << line << std::endl;
                continue;
            }

            // 修正していない行はそのまま書き込む
            if (!modified) {
                dest << line << std::endl;
            }
            else {
                fixedIssues++;
            }
        }

        OutputDebugStringA(("Processed " + std::to_string(lineCount) + " lines, fixed " +
            std::to_string(fixedIssues) + " issues.\n").c_str());
        return true;
    }

private:
    // 文字列からfloat値の配列を解析する
    static std::vector<float> ParseFloats(const std::string& str) {
        std::vector<float> result;
        std::istringstream ss(str);  // stringstream の代わりに istringstream を使用
        std::string token;

        // 空白で区切られた各トークンを処理
        while (ss >> token) {
            try {
                // 文字列をfloatに変換
                float value = std::stof(token);
                result.push_back(value);
            }
            catch (const std::exception&) {
                // 変換エラーの場合はスキップ
                continue;
            }
        }

        return result;
    }

    // 文字列の前後の空白を削除する
    static std::string TrimString(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\n\r\f\v");
        if (first == std::string::npos) return "";

        size_t last = str.find_last_not_of(" \t\n\r\f\v");
        return str.substr(first, (last - first + 1));
    }
};