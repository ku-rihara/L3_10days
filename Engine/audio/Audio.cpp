#include "Audio.h"

#include<fstream>
#include<assert.h>

/// game option
#include "Option/GameOption.h"

#define ATTENUATION_TIME_MS 50
#define ATTENUATION_FACTOR 0.01f // 音量を0.5%にする

#pragma comment(lib,"xaudio2.lib")


Audio* Audio::GetInstance() {
	static Audio instance;
	return &instance;
}

void Audio::Init() {
	//XAudioエンジンのインスタンスを生成
	HRESULT hr = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	// マスターボイスを作成
	hr = xAudio2_->CreateMasteringVoice(&masterVoice_);
}

void Audio::Update() {
	// 再生が終了した音声を解放
	auto it = playingAudios_.begin();
	while (it != playingAudios_.end()) {
		XAUDIO2_VOICE_STATE state;
		(*it)->GetState(&state);
		if (state.BuffersQueued == 0) {
			(*it)->Stop();
			(*it)->DestroyVoice();
			it = playingAudios_.erase(it);
		} else {
			++it;
		}
	}
}

void Audio::Finalize() {
	for (auto& soundData : soundDatas_) {
		delete[] soundData.pBuffer;
		soundData.pBuffer = nullptr;
	}

	/// すべて停止
	for(auto& sound : playingAudios_) {
		sound->Stop();
		sound->DestroyVoice();
	}

	soundDatas_.clear();
	xAudio2_.Reset();
}


int Audio::LoadWave(const std::string& filename) {
	// ファイルがすでに読み込まれている場合
	auto it = soundIndexMap_.find(filename);

	if (it != soundIndexMap_.end()) {
		return it->second; // 既存のインデックスを返す
	}

	// ファイルオープン------------------------
	std::ifstream file;
	file.open(filename, std::ios_base::binary);
	assert(file.is_open());

	//// RIFFヘッダーの読み込み
	//RiffHeader riff;
	//file.read((char*)&riff, sizeof(riff));
	//if (strncmp(riff.chunk.id, "RIFF", 4) != 0 || strncmp(riff.type, "WAVE", 4) != 0) {
	//    assert(0);
	//}

	//// Formatチャンクの読み込み
	//FormatChunk format = {};
	//file.read((char*)&format, sizeof(ChunkHeader));
	//assert(strncmp(format.chunk.id, "fmt ", 4) == 0);
	//assert(format.chunk.size <= sizeof(format.fmt));
	//file.read((char*)&format.fmt, format.chunk.size);

	//// Dataチャンクの読み込み (汎用チャンクスキップ対応)
	//ChunkHeader chunk;
	//char* pBuffer = nullptr;
	//size_t dataSize = 0;

	//while (file.read((char*)&chunk, sizeof(chunk))) {
	//    if (strncmp(chunk.id, "data", 4) == 0) {
	//        // 波形データの読み込み
	//        pBuffer = new char[chunk.size];
	//        file.read(pBuffer, chunk.size);
	//        dataSize = chunk.size;
	//        break;
	//    } else {
	//        // その他チャンクをスキップ
	//        file.seekg(chunk.size, std::ios_base::cur);
	//    }
	//}
	//assert(pBuffer != nullptr && dataSize > 0);
	//file.close();

	//// 読み込んだ音声データを保存
	//SoundData soundData = {};
	//soundData.wfex = format.fmt;
	//soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	//soundData.bufferSize = int(dataSize);

	//// soundDatas_に追加
	//soundDatas_.push_back(soundData);
	//int index = int(soundDatas_.size()) - 1;
	SoundData audioClip{};

	// WAVヘッダの読み込み
	char chunkId[4];
	file.read(chunkId, 4);
	if (std::strncmp(chunkId, "RIFF", 4) != 0) {
		assert(false);
	}

	file.ignore(4); // ChunkSizeをスキップ

	file.read(chunkId, 4);
	if (std::strncmp(chunkId, "WAVE", 4) != 0) {
		assert(false);
	}

	// fmtチャンクを探す
	bool fmtFound = false;
	while (file.read(chunkId, 4)) {
		uint32_t chunkSize;
		file.read(reinterpret_cast<char*>(&chunkSize), sizeof(chunkSize));

		if (std::strncmp(chunkId, "fmt ", 4) == 0) {
			fmtFound = true;
			if (chunkSize >= sizeof(WAVEFORMATEX)) {
				file.read(reinterpret_cast<char*>(&audioClip.wfex), sizeof(WAVEFORMATEX));

				// fmtChunkSize が WAVEFORMATEX よりも大きい場合は、余分なデータをスキップ
				if (chunkSize > sizeof(WAVEFORMATEX)) {
					file.ignore(chunkSize - sizeof(WAVEFORMATEX));
				}
			} else if (chunkSize == 40) {
				file.read(reinterpret_cast<char*>(&audioClip.wfex), sizeof(WAVEFORMATEX));
				// 追加の16バイト（WAVEFORMATEXを超える部分）をスキップ
				file.ignore(chunkSize - sizeof(WAVEFORMATEX));
			} else if (chunkSize == 16) {
				// 16バイトのfmtチャンクの場合
				file.read(reinterpret_cast<char*>(&audioClip.wfex), 16);
			}
			break;
		} else {
			// チャンクをスキップ
			file.ignore(chunkSize);
		}
	}

	if (!fmtFound) {
		assert(false);
	}

	// dataチャンクまでスキップ
	file.read(chunkId, 4);
	while (std::strncmp(chunkId, "data", 4) != 0) {
		uint32_t chunkSize;
		file.read(reinterpret_cast<char*>(&chunkSize), sizeof(chunkSize));
		file.ignore(chunkSize);
		file.read(chunkId, 4);
	}

	uint32_t dataSize;
	file.read(reinterpret_cast<char*>(&dataSize), sizeof(dataSize));

	audioClip.pBuffer = new BYTE[dataSize];
	file.read(reinterpret_cast<char*>(audioClip.pBuffer), dataSize);

	audioClip.bufferSize = dataSize;

	file.close();
	// soundDatas_に追加
	soundDatas_.push_back(audioClip);
	int index = int(soundDatas_.size()) - 1;
	//return audioClip;
	// ファイル名とインデックスをマップに保存
	soundIndexMap_[filename] = index;

	return index;
}


void Audio::PlayWave(const int& soundId, const float& volume, float _pitch) {
	if (soundId < 0 || soundId >= soundDatas_.size()) {
		return;
	}

	const SoundData& soundData = soundDatas_[soundId];

	HRESULT result;
	IXAudio2SourceVoice* pSourceVoice = nullptr;

	result = xAudio2_->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));

	// **ボリューム調整**
	float adjustedVolume = volume; // デフォルトはそのまま
	auto now = std::chrono::steady_clock::now();

	if (lastPlayTimes_.find(soundId) != lastPlayTimes_.end()) {
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastPlayTimes_[soundId]).count();
		if (elapsed < ATTENUATION_TIME_MS) {
			// **直前に同じ音が鳴った場合、ボリュームを下げる**
			float factor = 1.0f - (1.0f - ATTENUATION_FACTOR) * (1.0f - float(elapsed) / ATTENUATION_TIME_MS);
			adjustedVolume *= factor; // 重なり具合に応じてボリュームを調整
		}
	}
	lastPlayTimes_[soundId] = now; // 最新の再生時間を記録

	GameOption* option = GameOption::GetInstance();
	float volumeScale = option->GetMasterVolume() * option->GetSEVolume();
	result = pSourceVoice->SetVolume(adjustedVolume * volumeScale);
	assert(SUCCEEDED(result));

	result = pSourceVoice->SetFrequencyRatio(_pitch); // ピッチ変更
	assert(SUCCEEDED(result));

	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;

	result = pSourceVoice->SubmitSourceBuffer(&buf);
	assert(SUCCEEDED(result));

	result = pSourceVoice->Start();
	assert(SUCCEEDED(result));

	playingAudios_.push_back(pSourceVoice);
}

int Audio::PlayBGM(const int& soundId, const float& volume) {
	if (soundId < 0 || soundId >= soundDatas_.size()) {
		return -1;
	}

	/// すでに同じBGMが再生されている場合は停止
	auto it = bgmVoices_.find(soundId);
	if (it != bgmVoices_.end()) {
		StopBGM(soundId);
	}

	const SoundData& soundData = soundDatas_[soundId];

	HRESULT result;
	IXAudio2SourceVoice* pSourceVoice = nullptr;

	result = xAudio2_->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));

	// **ボリューム調整**
	float adjustedVolume = volume; // デフォルトはそのまま
	auto now = std::chrono::steady_clock::now();

	if (lastPlayTimes_.find(soundId) != lastPlayTimes_.end()) {
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastPlayTimes_[soundId]).count();
		if (elapsed < ATTENUATION_TIME_MS) {
			// **直前に同じ音が鳴った場合、ボリュームを下げる**
			float factor = 1.0f - (1.0f - ATTENUATION_FACTOR) * (1.0f - float(elapsed) / ATTENUATION_TIME_MS);
			adjustedVolume *= factor; // 重なり具合に応じてボリュームを調整
		}
	}
	lastPlayTimes_[soundId] = now; // 最新の再生時間を記録

	GameOption* option = GameOption::GetInstance();
	float volumeScale = option->GetMasterVolume() * option->GetBGMVolume();
	result = pSourceVoice->SetVolume(adjustedVolume * volumeScale);
	assert(SUCCEEDED(result));

	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;
	buf.LoopCount = XAUDIO2_LOOP_INFINITE; // ループ再生

	result = pSourceVoice->SubmitSourceBuffer(&buf);
	assert(SUCCEEDED(result));

	result = pSourceVoice->Start();
	assert(SUCCEEDED(result));

	bgmVoices_[soundId] = pSourceVoice; // BGMのボイスを保存
	playingAudios_.push_back(pSourceVoice);


	return soundId;
}

void Audio::StopBGM(const int& soundId) {
	auto it = bgmVoices_.find(soundId);
	auto audioIt = std::find(playingAudios_.begin(), playingAudios_.end(), it->second);
	if (audioIt != playingAudios_.end()) {
		playingAudios_.erase(audioIt);
	}

	if (it != bgmVoices_.end()) {
		it->second->Stop();
		it->second->DestroyVoice();
		bgmVoices_.erase(it);
	}


}

void Audio::Unload(const int& soundId) {
	if (soundId >= 0 && soundId < soundDatas_.size()) {
		SoundData& soundData = soundDatas_[soundId];
		delete[] soundData.pBuffer;
		soundData.pBuffer = nullptr;
		soundData.bufferSize = 0;
		soundData.wfex = {};
	}
}


//
//Audio* Audio::GetInstance() {
//	static Audio instance;
//	return &instance;
//}
//
//void Audio::Init() {
//	//XAudioエンジンのインスタンスを生成
//	HRESULT hr = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
//	// マスターボイスを作成
//	hr = xAudio2_->CreateMasteringVoice(&masterVoice_);
//}
//
//SoundData Audio::SoundLoadWave(const char* filename) {
//	/*HRESULT result;*/
//	//①ファイルオープン------------------------
//	//ファイル入力ストリームのインスタンス
//	std::ifstream file;
//	//.wavファイルをバイナリモードで開く
//	file.open(filename, std::ios_base::binary);
//	//ファイルオープン失敗を検出する
//	assert(file.is_open());
//	//②.wavデータ読み込み------------------------
//	//RIFFヘッダーの読み込み
//	RiffHeader riff;
//	file.read((char*)&riff, sizeof(riff));
//	//ファイルがRIFFかチェック
//	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
//		assert(0);
//	}
//	//タイプがWAVEかチェック
//	if (strncmp(riff.type, "WAVE", 4) != 0) {
//		assert(0);
//	}
//	//Formatチャンクの読み込み
//	FormatChunk format = {};
//	//チャンクヘッダーの確認
//	file.read((char*)&format, sizeof(ChunkHeader));
//	if (strncmp(format.chunk.id, "fmt ", 4) != 0) {
//		assert(0);
//	}
//	//チャンク本体の読み込み
//	assert(format.chunk.size <= sizeof(format.fmt));
//	file.read((char*)&format.fmt, format.chunk.size);
//	//Dataチャンクの読み込み
//	ChunkHeader data;
//	file.read((char*)&data, sizeof(data));
//	//JUNKチャンクを検出した場合
//	if (strncmp(data.id, "JUNK", 4) == 0) {
//		//読み取り位置をJUNKチャンクの終わりまで進める
//		file.seekg(data.size, std::ios_base::cur);
//		//再読み込み
//		file.read((char*) &data, sizeof(data));
//	}
//	if (strncmp(data.id, "data", 4) != 0) {
//		assert(0);
//	}
//	//Dataチャンクのデータ部(波形データ)の読み込み
//	char* pBuffer = new char[data.size];
//	file.read(pBuffer, data.size);
//	//waveファイルを閉じる
//	file.close();
//	//④読み込んだ音声データをreturn------------------------
//	//returnする為の音声データ
//	SoundData soundData = {};
//
//	soundData.wfex = format.fmt;
//	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
//	soundData.bufferSize = data.size;
//	return soundData;
// }
//
//void  Audio::SoundUnload(SoundData* soundData) {
//	//バッファのメモリを解放
//	delete[]soundData->pBuffer;
//
//	soundData->pBuffer = 0;
//	soundData->bufferSize = 0;
//	soundData->wfex = {};
//}
//
//void Audio::SoundPlayWave(IXAudio2* xAudio2,const SoundData&soundData) {
//
//	HRESULT result;
//	//波形フォーマットを元にSourceVoiceの生成
//	IXAudio2SourceVoice* pSourceVoice = nullptr;
//	result = xAudio2->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
//	assert(SUCCEEDED(result));
//
//	//再生する波形データの設定
//	XAUDIO2_BUFFER buf{};
//	buf.pAudioData = soundData.pBuffer;
//	buf.AudioBytes = soundData.bufferSize;
//	buf.Flags = XAUDIO2_END_OF_STREAM;
//
//	//波形データの再生
//	result = pSourceVoice->SubmitSourceBuffer(&buf);
//	result = pSourceVoice->Start();
//}
//
//void Audio::Finalizer() {	
//	xAudio2_.Reset();
//}