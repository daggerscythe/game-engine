#pragma once
#include "../ECS/EntityManager.h"
#include "../ECS/Components.h"
#include "../utils/AudioUtils.h"
#include "../utils/AudioFile.h"

class AudioSystem
{
public:
	void Init();
	void Update(EntityManager& entityManager);
	void LoadSound(uint32_t id, const std::string& path);
	void PlaySound();
	void Shutdown();

private:
	ALCdevice* m_openALDevice;
	ALCcontext* m_openALContext;
	std::unordered_map<uint32_t, ALuint> m_buffers;
	std::unordered_map<uint32_t, ALuint> m_sources;
	ALCboolean m_closed;
	ALCboolean m_contextMadeCurrent;
};