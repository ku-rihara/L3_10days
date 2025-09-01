#pragma once
#include "3d/Object3d.h"
#include "3d/ViewProjection.h"
#include "3d/WorldTransform.h"
#include "Vector3.h"
#include <json.hpp>
#include <string>
#include <vector>

class ControlPosManager {

private:
    using json = nlohmann::json; // 別名を定義
private:
    ///=====================================================
    /// private variants
    ///=====================================================

    std::vector<WorldTransform> transforms_;
    std::vector<Vector3> movePosies_; // 移動座標リスト
    std::vector<std::unique_ptr<Object3d>> obj3ds_; // モデルデータ
    std::string filePath_; // 座標データの保存先
    Vector3 tempAddPosition_; // ImGuiで座標を一時入力する変数
    WorldTransform* parentTransform_;

    bool isDraw_=false;

    const std::string dyrectrypath_ = "Resources/ControlPoint/";

public:
    ControlPosManager()  = default;
    ~ControlPosManager() = default;

    ///=====================================================
    /// pbulic method
    ///=====================================================

    void Init();
    void Update(const Vector3& Direction = {1.0f, 1.0f, 1.0f},const bool&isDraw=false);
  
    void SetParent(WorldTransform* parent);

    // 座標管理
    void AddPoint(const Vector3& position);
    void RemovePoint(size_t index);

    // ファイル操作
    void SaveToFile(const std::string& filename);
    void LoadFromFile(const std::string& filename);

    // ImGui用UI
    void ImGuiUpdate(const std::string& filename);

    ///=====================================================
    /// getter method
    ///=====================================================
    std::vector<Vector3> GetWorldPositions() const;
    std::vector<Vector3> GetLocalPositions() const;

    void SetIsDraw(const bool& is) {  isDraw_ = is; }
};
