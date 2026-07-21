#include "AudioManager.h"

void AudioManager::Initialize() {
	Shutdown();

	m_audioEngine = std::make_unique<DirectX::AudioEngine>();

	m_shootSound = LoadSoundEffectOrNull(L"Assets/Audios/shoot.wav");
	m_hitSound = LoadSoundEffectOrNull(L"Assets/Audios/hit.wav");
	m_damageSound = LoadSoundEffectOrNull(L"Assets/Audios/damage.wav");
}

void AudioManager::Shutdown() noexcept {
	m_damageSound.reset();
	m_hitSound.reset();
	m_shootSound.reset();

	m_audioEngine.reset();
}

void AudioManager::Update() {
	if (!m_audioEngine) return;

	if (!m_audioEngine->Update()) {
		if (m_audioEngine->IsCriticalError()) {
			// ! ignore for now
			// TODO: later, show an error or try to reset the audio
		}
	}
}

void AudioManager::PlayShoot() {
	if (m_shootSound)
		m_shootSound->Play();
}

void AudioManager::PlayHit() {
	if (m_hitSound)
		m_hitSound->Play();
}

void AudioManager::PlayDamage() {
	if (m_damageSound)
		m_damageSound->Play();
}

std::unique_ptr<DirectX::SoundEffect> AudioManager::LoadSoundEffectOrNull(const wchar_t* filePath) {
	if (!m_audioEngine || !filePath) return nullptr;

	try {
		return std::make_unique<DirectX::SoundEffect>(
			m_audioEngine.get(),
			filePath
		);
	}
	catch (const std::exception) {
		return nullptr;
	}
}