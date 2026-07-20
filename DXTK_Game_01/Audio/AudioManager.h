#pragma once

#include<Audio.h>
#include<memory>

class AudioManager {
public:
	void Initialize();

	void Update();

	void PlayShoot();
	void PlayHit();
	void PlayDamage();

private:
	std::unique_ptr<DirectX::SoundEffect> LoadSoundEffectOrNull(const wchar_t* filePath);

private:
	std::unique_ptr<DirectX::AudioEngine> m_audioEngine;

	std::unique_ptr<DirectX::SoundEffect> m_shootSound;
	std::unique_ptr<DirectX::SoundEffect> m_hitSound;
	std::unique_ptr<DirectX::SoundEffect> m_damageSound;
};