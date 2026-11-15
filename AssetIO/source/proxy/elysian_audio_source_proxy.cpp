#include <AssetIO/proxy/elysian_audio_source_proxy.hpp>
/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#include <AssetIO/proxy/elysian_entity_proxy.hpp>

/*!
    \qmltype GYWav
    \since 1.6.0
    \inmodule Engine
    \brief Contains a loaded WAV file buffer for sound effects.

    GYWav is a simple libGyro class containing a buffer for
    a wave file. GYWav can be passed to an \l{AudioSource} \l{Component}
    for positional SFX playback or to \l{AudioSystem} for nonpositional
    or positional SFX playback.

    \section2 Creating a WAV
    WAVs can be loaded through the \l{gyro} API with gyro::createWav():
    \code
    wav = gyro.createWav("/assets/sfx/victory.wav")
    \endcode
    LibGyro maintains a cache of all of the WAVs currently loaded within memory, so a subsequent call
    to gyro::createWav() with the same file path will result in a shared reference to the WAV being
    returned, rather than allocating a duplicate copy. For this reason, don't waste time worrying about
    whether the same WAV has already been loaded elsewhere or not.

    Because sound RAM is limited on some platforms, be sure to only load WAVs when they're needed and
    destroy them immediatley after. You must manually control their lifespan.

    \section2 Destroying a WAV
    Destroying a GYWav object is also done through the \l{gyro} API with gyro::destroyWav():
    \code
    gyro.destroyWav(wav)
    \endcode
    Because libGyro maintains a cache of shared objects for every WAV, the WAV object is only
    actually freed if this is the last GYWav object referencing it. Otherwise, the internal
    reference count is decremented, and it will be deleted later when all other references to
    the same file have been destroyed.

    Because of this, do not waste time worrying about whether or not the WAV is being used
    elsewhere before destroying it. libGyro handles that for you.

    \section2 Playing a WAV
    For playback via the \l{AudioSystem}, see \l{Playing a Sound Effect through Lua}. For playback via an \l{AudioSource}, see
    \l{AudioSource::playWav}.

    \sa AudioSource, AudioSystem, gyro
*/







/*!
    \qmltype AudioSource
    \inherits Component
    \since 1.6.0
    \inmodule Engine
    \brief Component for adding sound effects to an Entity.

    An AudioSource is a Component that can be attached to an Entity, providing it with an audio emitter
    enabling it to play WAV sound effects in 3D positional space. The sound is emitted from the entity's
    position and will be relative to the audio listener at the party lead's location.
    Sounds started through an AudioSource will automatically claim a free channel from the AudioSystem
    and release it once the sound is complete, so audio channels do not have to be managed manually.

    AudioSources dynamically request audio channels from the AudioSystem as needed for playback. Up
    to 15 AudioSources can be playing WAVs simultaneously before a channel must be recycled and its
    playback is interrupted for the next sound. For more information, see \l{Audio Channels}.

    \sa {AudioSystem}
*/
/*!
 * \qmlmethod void AudioSource::stop()
 *
 * Stops the WAV currently being played by the AudioSource (if there is one playing).
 * \sa play()
 */
/*!
 * \qmlmethod void AudioSource::play()
 *
 * Claims a free channel from the AudioSystem or the oldest channel if none are available
 * and plays the current WAV if one has been set.
 * \sa setWav(), stop()
 */
/*!
 * \qmlmethod void AudioSource::setGain(float factor)
 *
 * Sets the gain of the AudioSource to the value specified by \a{factor}. Gain can be thought of
 * as a volume multiplier. Possible gain values:
 * \list
 * \li 0.0   - muted
 * \li < 1.0 - decreased volume
 * \li 1.0   - normal volume
 * \li > 1.0 - increased volume
 * \endlist
 * \sa getGain()
 */
/*!
 * \qmlmethod float AudioSource::getGain()
 *
 * Returns the floating point gain of the emitter.
 * \sa setGain()
 */
/*!
 * \qmlmethod void AudioSource::setPitch(float pitch)
 *
 * Sets the pitch of the emitter to \a{pitch}.
 * \sa getPitch()
 */
/*!
 * \qmlmethod float AudioSource::getPitch()
 *
 * Returns the floating point pitch of the emitter.
 * \sa setPitch()
 */
/*!
 * \qmlmethod bool AudioSource::getLooping()
 *
 * Returns whether or not sound effects started on the AudioSource are played continuously in a loop.
 * \sa setLooping()
 */
/*!
 * \qmlmethod void AudioSource::setLooping(bool value)
 *
 * Sets the looping value of the AudioSource to \a{value}. If \a{value} is true, all sound effects played on the
 * AudioSource are played continuously in a loop. If \a{value} is false, they are only played once before stopping.
 * \sa getLooping()
 */
/*!
 * \qmlmethod void AudioSource::setWav(GYWav* wav)
 *
 * Sets the given \a{wav} object as the wave to be used internally for playback, but does not begin playback.
 * \sa playWav()
 */
/*!
 * \qmlmethod void AudioSource::playWav(GYWav* wav)
 *
 * Begins playback of the given \a{wav}.
 * \sa setWav()
 */


namespace elysian::lua {
using namespace elysian;
using AudioSource = elysian::AudioSource;

void registerGYWav(sol::state_view state) {
    if (state["GYWav"].valid()) return;
    using T = GYWav;
    auto type = registerType<T>(state, "GYWav");
}

void registerAudioSource(sol::state_view state) {
    if (state["AudioSource"].valid()) return;
    registerComponent(state);
    using T = elysian::AudioSource;
    auto type = registerType<T, elysian::Component>(state, "AudioSource");
    type["setWavFile"] = method<T>(&T::setWavFile);
    type["getWavFile"] = method<T>(&T::getWavFile);
    type["setBasePath"] = method<T>(&T::setBasePath);
    type["getBasePath"] = method<T>(&T::getBasePath);
    type["setRolloffFactor"] = method<T>(&T::setRolloffFactor);
    type["getRolloffFactor"] = method<T>(&T::getRolloffFactor);
    type["isPlaying"] = method<T>(&T::isPlaying);
    type["setWav"] = method<T>(&T::setWav);
    type["playWav"] = method<T>(&T::playWav);
    type["playLua"] = method<T, void(sol::object)>(&T::playLua);
    type["setStartLooping"] = method<T>(&T::setStartLooping);
    type["getStartLooping"] = method<T>(&T::getStartLooping);
    type["setStartWavFile"] = method<T>(&T::setStartWavFile);
    type["getStartWavFile"] = method<T>(&T::getStartWavFile);
    type["stop"] = method<T, void()>(&T::stop);
    type["setGain"] = method<T, void(float)>(&T::setGain);
    type["setPitch"] = method<T, void(float)>(&T::setPitch);
    type["setLooping"] = method<T, void(bool)>(&T::setLooping);
    type["getPitch"] = method<T, float() const>(&T::getPitch);
    type["getGain"] = method<T, float() const>(&T::getGain);
    type["getLooping"] = method<T, bool() const>(&T::getLooping);
    type["play"] = method<T, void(sol::object)>(&T::playLua);
    property(type, "wavFile", method<T>(&T::getWavFile), method<T>(&T::setWavFile));
    property(type, "basePath", method<T>(&T::getBasePath), method<T>(&T::setBasePath));
    property(type, "gain", method<T, float() const>(&T::getGain), method<T, void(float)>(&T::setGain));
    property(type, "pitch", method<T, float() const>(&T::getPitch), method<T, void(float)>(&T::setPitch));
    property(type, "looping", method<T, bool() const>(&T::getLooping), method<T, void(bool)>(&T::setLooping));
    property(type, "rolloffFactor", method<T>(&T::getRolloffFactor), method<T>(&T::setRolloffFactor));
    property(type, "startLooping", method<T>(&T::getStartLooping), method<T>(&T::setStartLooping));
    property(type, "startWavFile", method<T>(&T::getStartWavFile), method<T>(&T::setStartWavFile));
    inheritProperties<T, elysian::Component>();
    finish<T>(type, "AudioSource", [] { return LuaManager::isSerializing(); });
    initializeEditor<T>(type);
}
}
