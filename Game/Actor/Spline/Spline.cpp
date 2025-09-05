#include "Spline.h"

/// std
#include <filesystem>
#include <fstream>
#include <cassert>

/// externals/assimp
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <json.hpp>


Spline::Spline() {
	line3D_ = std::make_unique<Line3D>();
	line3D_->Init(static_cast<size_t>(std::pow(2, 16)));
}
Spline::~Spline() {}

void Spline::Load(const std::string& _filepath) {

	/// ファイルが存在しなかったらreturn
	if (!std::filesystem::exists(_filepath)) {
		return; ///< 読み込み失敗
	}

	std::ifstream file(_filepath);
	if (!file.is_open()) {
		assert(false);
	}

	nlohmann::json j;
	file >> j;

	controlPoints_.clear();

	for (auto& v : j) {
		// Blender (X,Y,Z) -> DirectX (X,Z,Y)
		float bx = v[0].get<float>();
		float by = v[1].get<float>();
		float bz = v[2].get<float>();

		// 右手系→左手系に変換（必要ならZ反転）
		controlPoints_.push_back({ -bx, -bz, by });
	}

	controlPointCount_ = controlPoints_.size();
}

void Spline::DebugDraw(const ViewProjection& _vp) {
	if (controlPointCount_ == 0) {
		return;
	}

	/// 曲線を描画する
	line3D_->Reset();

	for (size_t i = 0; i < controlPointCount_ - 1; i++) {
		line3D_->SetLine(controlPoints_[i], controlPoints_[(i + 1) % controlPointCount_], { 1.0f, 0.0f, 0.0f, 1.0f });
	}

	line3D_->Draw(_vp);
}

size_t Spline::GetControlPointCount() const {
	return controlPointCount_;
}

const std::vector<Vector3>& Spline::GetControlPoints() const {
	return controlPoints_;
}
