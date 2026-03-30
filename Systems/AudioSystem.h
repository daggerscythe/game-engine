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
	uint32_t LoadSound(const std::string& path);
	void RegisterSource(EntityID entity, uint32_t soundID, bool spatial, bool isLooping);
	void PlaySound(EntityID entity);
	void Shutdown();

private:
	ALCdevice* m_openALDevice;
	ALCcontext* m_openALContext;
	uint32_t m_nextID = 1;
	std::unordered_map<uint32_t, ALuint> m_buffers;
	std::unordered_map<uint32_t, ALuint> m_sources;
	float m_volume;
	ALCboolean m_closed;
	ALCboolean m_contextMadeCurrent;
};