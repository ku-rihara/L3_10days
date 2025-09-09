#include "Route.h"
#include "3d/ViewProjection.h"

#include <cassert>
#include <cmath>
#include <random>
#include <algorithm>

//============================= Unit ===================================//

/////////////////////////////////////////////////////////////////////////////////////////
//      ファイルロード
/////////////////////////////////////////////////////////////////////////////////////////
void Route::RouteUnit::Load(const std::string& filePath){
	spline = std::make_unique<Spline>();
	spline->Load(filePath);

	// 元CPを保持（オフセット適用の基準）
	originalCps_.clear();
	const auto& cpsConst = spline->GetControlPoints();
	originalCps_.assign(cpsConst.begin(),cpsConst.end());
	preBasePosition = basePosition;
}

/////////////////////////////////////////////////////////////////////////////////////////
//      更新
/////////////////////////////////////////////////////////////////////////////////////////
void Route::RouteUnit::Update(){
	if (!spline) return;

	if (preBasePosition != basePosition){
		auto& cps = spline->GetControlPointsMutable();
		cps.resize(originalCps_.size());
		for (size_t i = 0; i < originalCps_.size(); ++i){ cps[i] = originalCps_[i] + basePosition; }
		preBasePosition = basePosition;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//      デバッグ描画
/////////////////////////////////////////////////////////////////////////////////////////
void Route::RouteUnit::DrawDebug(const ViewProjection& vp) const{ if (spline) spline->DebugDraw(vp); }

//============================= Route 本体 ===============================//

Route::~Route() = default;

/////////////////////////////////////////////////////////////////////////////////////////
//      enum名を文字列化
/////////////////////////////////////////////////////////////////////////////////////////
std::string Route::EnumName(RouteType t){
	switch (t){
	case RouteType::AllyDifence: return "AllyDifence";
	case RouteType::AllyAttack: return "AllyAttack";
	case RouteType::EnemyDirence: return "EnemyDirence";
	case RouteType::EnemyAttack: return "EnemyAttack";
	default: return "Unknown";
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//      ディレクトリから対象ファイルを探す
/////////////////////////////////////////////////////////////////////////////////////////
std::vector<std::filesystem::path>
	Route::FindFilesForType_(RouteType t, const std::string& dir) const{
	std::vector<std::filesystem::path> out;
	const std::string prefix = "Route_" + EnumName(t) + "_";
	const std::string ext = ".json";

	std::filesystem::path p(dir);
	if (!std::filesystem::exists(p)) return out;

	for (auto& entry : std::filesystem::directory_iterator(p)){
		if (!entry.is_regular_file()) continue;
		const auto& path = entry.path();
		const auto fname = path.filename().string();
		if (fname.rfind(prefix,0) == 0 && path.extension() == ext){ out.push_back(path); }
	}
	std::sort(out.begin(),out.end());
	return out;
}

/////////////////////////////////////////////////////////////////////////////////////////
//      初期化
/////////////////////////////////////////////////////////////////////////////////////////
void Route::Init(RouteType type, const std::string& dir) {
	type_ = type;
	variants_.clear();
	activeIndex_ = -1;

	std::filesystem::path base(dir);
	std::filesystem::path subdir = base / EnumName(type);

	if (!std::filesystem::exists(subdir)) {
		return;
	}

	// subdir 以下のすべての .json ファイルを拾う
	std::vector<std::filesystem::path> files;
	for (auto& entry : std::filesystem::directory_iterator(subdir)) {
		if (!entry.is_regular_file()) continue;
		if (entry.path().extension() == ".json") {
			files.push_back(entry.path());
		}
	}

	// 1件もなければ終了
	if (files.empty()) return;

	// 安定性のためソート
	std::sort(files.begin(), files.end());

	// すべてロード
	for (const auto& f : files) {
		RouteUnit u{};
		u.Load(f.string());
		variants_.push_back(std::move(u));
	}

	// 最初の候補をアクティブに
	if (!variants_.empty()) activeIndex_ = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
//      更新
/////////////////////////////////////////////////////////////////////////////////////////
void Route::Update(){ for (auto& v : variants_) v.Update(); }

/////////////////////////////////////////////////////////////////////////////////////////
//      デバッグ描画
/////////////////////////////////////////////////////////////////////////////////////////
void Route::DrawDebug(const ViewProjection& vp) const{ for (const auto& v : variants_) v.DrawDebug(vp); }

/////////////////////////////////////////////////////////////////////////////////////////
//      ランダム選択
/////////////////////////////////////////////////////////////////////////////////////////
void Route::ChooseRandomVariant(std::optional<uint32_t> seed){
	if (variants_.empty()){
		activeIndex_ = -1;
		return;
	}
	std::mt19937 rng(seed.value_or(std::random_device{}()));
	std::uniform_int_distribution<int> dist(0,static_cast<int>(variants_.size()) - 1);
	activeIndex_ = dist(rng);
}

/////////////////////////////////////////////////////////////////////////////////////////
//      ランダム切替（同じuを維持）
/////////////////////////////////////////////////////////////////////////////////////////
void Route::SwitchVariantKeepU(float u, std::optional<uint32_t> seed){
	if (variants_.size() <= 1) return;
	std::mt19937 rng(seed.value_or(std::random_device{}()));
	std::uniform_int_distribution<int> dist(0,static_cast<int>(variants_.size()) - 2);
	int idx = dist(rng);
	if (activeIndex_ >= 0 && idx >= activeIndex_) idx += 1;
	activeIndex_ = idx;
	(void)u; // uは呼び出し側が保持
}

/////////////////////////////////////////////////////////////////////////////////////////
//      サンプリング
/////////////////////////////////////////////////////////////////////////////////////////
Vector3 Route::Sample(float u) const{
	if (activeIndex_ < 0 || activeIndex_ >= static_cast<int>(variants_.size())) return {};
	const auto& unit = variants_[activeIndex_];
	if (!unit.spline) return {};
	return unit.spline->Evaluate(u); //< ★Spline側のAPIに合わせて修正
}

/////////////////////////////////////////////////////////////////////////////////////////
//      インデックス設定
/////////////////////////////////////////////////////////////////////////////////////////
void Route::SetActiveIndex(int idx){
	if (variants_.empty()){
		activeIndex_ = -1;
		return;
	}
	idx = std::clamp(idx,0,static_cast<int>(variants_.size()) - 1);
	activeIndex_ = idx;
}