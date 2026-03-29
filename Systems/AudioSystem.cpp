#include "AudioSystem.h"

void AudioSystem::Init()
{
	// initialize openAL device
	m_openALDevice = alcOpenDevice(nullptr); // nullptr sets the default device
	if (!m_openALDevice) {
		std::cerr << "ERROR: Failed to initialize OpenAL device" << std::endl;
		return;
	}

	// create openAL context
	if (!alcCall(alcCreateContext, m_openALContext, m_openALDevice, m_openALDevice, nullptr)
		|| !m_openALContext) {
		std::cerr << "ERROR: Failed to create audio context" << std::endl;
		return;
	}

	// make a context current
	m_contextMadeCurrent = false;
	if (!alcCall(alcMakeContextCurrent, m_contextMadeCurrent, m_openALDevice, m_openALContext)
		|| m_contextMadeCurrent != ALC_TRUE) {
		std::cerr << "ERROR: Could not make audio context current" << std::endl;
		return;
	}
}

void AudioSystem::Update(EntityManager& entityManager)
{
	auto cameras = entityManager.GetEntitiesWith<CameraComponent>();
	CameraComponent camera = entityManager.GetComponent<CameraComponent>(cameras[0]);
	glm::vec3 camPos = entityManager.GetComponent<TransformComponent>(cameras[0]).position;

	// orietation of listener (camera)
	ALfloat orientation[] = {
		camera.front.x, camera.front.y, camera.front.z,
		camera.up.x, camera.up.y, camera.up.z
	};

	// update listener pos to match camera pos
	alCall(alListener3f, AL_POSITION, camPos.x, camPos.y, camPos.z);
	alCall(alListener3f, AL_VELOCITY, 0, 0, 0); // assign proper values later for Doppler effect
	alCall(alListenerfv, AL_ORIENTATION, orientation);

	auto entities = entityManager.GetEntitiesWith<AudioSourceComponent>();
	for (EntityID entity : entities) {
		glm::vec3 sourcePos = entityManager.HasComponent<TransformComponent>(entity) ?
			entityManager.GetComponent<TransformComponent>(entity).position :
			glm::vec3(0.0f);
		glm::vec3 sourceVel = entityManager.HasComponent<RigidBodyComponent>(entity) ?
			entityManager.GetComponent<RigidBodyComponent>(entity).velocity :
			glm::vec3(0.0f);
		auto asc = entityManager.GetComponent<AudioSourceComponent>(entity);
		auto source = m_sources[asc.id];

		// update source to match TC, RBC, ASC
		alCall(alSource3f, source, AL_POSITION, sourcePos.x, sourcePos.y, sourcePos.z);
		alCall(alSource3f, source, AL_VELOCITY, sourceVel.x, sourceVel.y, sourceVel.z);
		alCall(alSourcei, source, AL_LOOPING, (asc.isLooping ? AL_TRUE : AL_FALSE));

		if (asc.isPlaying) {
			ALint state;
			alCall(alGetSourcei, source, AL_SOURCE_STATE, &state);
			// don't play if already playing
			if (state != AL_PLAYING) {
				alCall(alSourcePlay, source);
			}
		}
	}
}

void AudioSystem::LoadSound(uint32_t id, const std::string& path)
{
	AudioFile<double> audioFile;
	std::uint8_t channels;
	std::int32_t sampleRate;
	std::uint8_t bitsPerSample;
	std::vector<int16_t> soundData;
	if (!audioFile.load(path)) {
		std::cerr << "ERROR: Failed to load audio file at path " << path << std::endl;
		return;
	}
	channels = audioFile.getNumChannels();
	sampleRate = audioFile.getSampleRate();
	bitsPerSample = audioFile.getBitDepth();

	if (audioFile.isMono()) {
		for (double sample : audioFile.samples[0]) {
			soundData.push_back(static_cast<int16_t>(sample * 32767)); // convert to 16-bit int
		}
	}
	else {
		for (int i = 0; i < audioFile.getNumSamplesPerChannel(); i++) {
			soundData.push_back(static_cast<int16_t>(audioFile.samples[0][i] * 32767));
			soundData.push_back(static_cast<int16_t>(audioFile.samples[1][i] * 32767));
		}
	}

	ALenum format;
	if (channels == 1 && bitsPerSample == 8) format = AL_FORMAT_MONO8;
	else if (channels == 1 && bitsPerSample == 16) format = AL_FORMAT_MONO16;
	else if (channels == 2 && bitsPerSample == 8) format = AL_FORMAT_STEREO8;
	else if (channels == 2 && bitsPerSample == 16) format = AL_FORMAT_STEREO16;
	else {
		std::cerr << "ERROR: Unsupported channel count: "
			<< channels << " channels, "
			<< bitsPerSample << " bps" << std::endl;
		return;
	}

	// create a buffer
	ALuint buffer;
	alCall(alGenBuffers, 1, &buffer);
	alCall(alBufferData, buffer, format, soundData.data(), soundData.size() * sizeof(int16_t), sampleRate);
	soundData.clear(); // erase sound in RAM

	// add to list of buffers
	m_buffers[id] = buffer;

	// create a source
	ALuint source;
	alCall(alGenSources, 1, &source);
	alCall(alSourcef, source, AL_PITCH, 1);
	alCall(alSourcef, source, AL_GAIN, 0.05f); // volume
	alCall(alSource3f, source, AL_POSITION, 0, 0, 0);
	alCall(alSource3f, source, AL_VELOCITY, 0, 0, 0);
	alCall(alSourcei, source, AL_LOOPING, AL_FALSE);
	alCall(alSourcei, source, AL_BUFFER, buffer);

	// multiple sources can use the same buffer

	// add to list of sources
	m_sources[id] = source;

	// DEBUG
	std::cout << "DEBUG: Loaded sound: " << path << std::endl;
}

void AudioSystem::PlaySound()
{
}

void AudioSystem::Shutdown()
{
	// collect all source IDs into a vector
	std::vector<ALuint> sourceIDs;
	for (auto& [id, source] : m_sources)
		sourceIDs.push_back(source);

	// delete all at once
	if (!sourceIDs.empty())
		alCall(alDeleteSources, sourceIDs.size(), sourceIDs.data());

	m_sources.clear();

	// collect all buffer IDs into a vector
	std::vector<ALuint> bufferIDs;
	for (auto& [id, buffer] : m_buffers)
		bufferIDs.push_back(buffer);

	// delete all at once
	if (!bufferIDs.empty())
		alCall(alDeleteBuffers, bufferIDs.size(), bufferIDs.data());

	m_buffers.clear();

	// stop the context being current
	alcCall(alcMakeContextCurrent, m_contextMadeCurrent, m_openALDevice, nullptr);

	// destroy context
	alcCall(alcDestroyContext, m_openALDevice, m_openALContext);

	// close openAL device
	alcCall(alcCloseDevice, m_closed, m_openALDevice, m_openALDevice);
}
