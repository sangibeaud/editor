/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_AUDIOIODEVICE_JUCEHEADER__
#define __JUCE_AUDIOIODEVICE_JUCEHEADER__

#include "../../text/juce_StringArray.h"
#include "../../maths/juce_BigInteger.h"
class AudioIODevice;


//==============================================================================
/**
    One of these is passed to an AudioIODevice object to stream the audio data
    in and out.

    The AudioIODevice will repeatedly call this class's audioDeviceIOCallback()
    method on its own high-priority audio thread, when it needs to send or receive
    the next block of data.

    @see AudioIODevice, AudioDeviceManager
*/
class JUCE_API  AudioIODeviceCallback
{
public:
    /** Destructor. */
    virtual ~AudioIODeviceCallback()  {}

    /** Processes a block of incoming and outgoing audio data.

        The subclass's implementation should use the incoming audio for whatever
        purposes it needs to, and must fill all the output channels with the next
        block of output data before returning.

        The channel data is arranged with the same array indices as the channel name
        array returned by AudioIODevice::getOutputChannelNames(), but those channels
        that aren't specified in AudioIODevice::open() will have a null pointer for their
        associated channel, so remember to check for this.

        @param inputChannelData     a set of arrays containing the audio data for each
                                    incoming channel - this data is valid until the function
                                    returns. There will be one channel of data for each input
                                    channel that was enabled when the audio device was opened
                                    (see AudioIODevice::open())
        @param numInputChannels     the number of pointers to channel data in the
                                    inputChannelData array.
        @param outputChannelData    a set of arrays which need to be filled with the data
                                    that should be sent to each outgoing channel of the device.
                                    There will be one channel of data for each output channel
                                    that was enabled when the audio device was opened (see
                                    AudioIODevice::open())
                                    The initial contents of the array is undefined, so the
                                    callback function must fill all the channels with zeros if
                                    its output is silence. Failing to do this could cause quite
                                    an unpleasant noise!
        @param numOutputChannels    the number of pointers to channel data in the
                                    outputChannelData array.
        @param numSamples           the number of samples in each channel of the input and
                                    output arrays. The number of samples will depend on the
                                    audio device's buffer size and will usually remain constant,
                                    although this isn't guaranteed, so make sure your code can
                                    cope with reasonable changes in the buffer size from one
                                    callback to the next.
    */
    virtual void audioDeviceIOCallback (const float** inputChannelData,
                                        int numInputChannels,
                                        float** outputChannelData,
                                        int numOutputChannels,
                                        int numSamples) = 0;

    /** Called to indicate that the device is about to start calling back.

        This will be called just before the audio callbacks begin, either when this
        callback has just been added to an audio device, or after the device has been
        restarted because of a sample-rate or block-size change.

        You can use this opportunity to find out the sample rate and block size
        that the device is going to use by calling the AudioIODevice::getCurrentSampleRate()
        and AudioIODevice::getCurrentBufferSizeSamples() on the supplied pointer.

        @param device       the audio IO device that will be used to drive the callback.
                            Note that if you're going to store this this pointer, it is
                            only valid until the next time that audioDeviceStopped is called.
    */
    virtual void audioDeviceAboutToStart (AudioIODevice* device) = 0;

    /** Called to indicate that the device has stopped. */
    virtual void audioDeviceStopped() = 0;

    /** This can be overridden to be told if the device generates an error while operating.
        Be aware that this could be called by any thread! And not all devices perform
        this callback.
    */
    virtual void audioDeviceError (const String& errorMessage);
};


//==============================================================================
/**
    Base class for an audio device with synchronised input and output channels.

    Subclasses of this are used to implement different protocols such as DirectSound,
    ASIO, CoreAudio, etc.

    To create one of these, you'll need to use the AudioIODeviceType class - see the
    documentation for that class for more info.

    For an easier way of managing audio devices and their settings, have a look at the
    AudioDeviceManager class.

    @see AudioIODeviceType, AudioDeviceManager
*/
class JUCE_API  AudioIODevice
{
public:
    /** Destructor. */
    virtual ~AudioIODevice();

    //==============================================================================
    /** Returns the device's name, (as set in the constructor). */
    const String& getName() const noexcept                          { return name; }

    /** Returns the type of the device.

        E.g. "CoreAudio", "ASIO", etc. - this comes from the AudioIODeviceType that created it.
    */
    const String& getTypeName() const noexcept                      { return typeName; }

    //==============================================================================
    /** Returns the names of all the available output channels on this device.
        To find out which of these are currently in use, call getActiveOutputChannels().
    */
    virtual StringArray getOutputChannelNames() = 0;

    /** Returns the names of all the available input channels on this device.
        To find out which of these are currently in use, call getActiveInputChannels().
    */
    virtual StringArray getInputChannelNames() = 0;

    //==============================================================================
    /** Returns the number of sample-rates this device supports.

        To find out which rates are available on this device, use this method to
        find out how many there are, and getSampleRate() to get the rates.

        @see getSampleRate
    */
    virtual int getNumSampleRates() = 0;

    /** Returns one of the sample-rates this device supports.

        To find out which rates are available on this device, use getNumSampleRates() to
        find out how many there are, and getSampleRate() to get the individual rates.

        The sample rate is set by the open() method.

        (Note that for DirectSound some rates might not work, depending on combinations
        of i/o channels that are being opened).

        @see getNumSampleRates
    */
    virtual double getSampleRate (int index) = 0;

    /** Returns the number of sizes of buffer that are available.

        @see getBufferSizeSamples, getDefaultBufferSize
    */
    virtual int getNumBufferSizesAvailable() = 0;

    /** Returns one of the possible buffer-sizes.

        @param index    the index of the buffer-size to use, from 0 to getNumBufferSizesAvailable() - 1
        @returns a number of samples
        @see getNumBufferSizesAvailable, getDefaultBufferSize
    */
    virtual int getBufferSizeSamples (int index) = 0;

    /** Returns the default buffer-size to use.

        @returns a number of samples
        @see getNumBufferSizesAvailable, getBufferSizeSamples
    */
    virtual int getDefaultBufferSize() = 0;

    //==============================================================================
    /** Tries to open the device ready to play.

        @param inputChannels        a BigInteger in which a set bit indicates that the corresponding
                                    input channel should be enabled
        @param outputChannels       a BigInteger in which a set bit indicates that the corresponding
                                    output channel should be enabled
        @param sampleRate           the sample rate to try to use - to find out which rates are
                                    available, see getNumSampleRates() and getSampleRate()
        @param bufferSizeSamples    the size of i/o buffer to use - to find out the available buffer
                                    sizes, see getNumBufferSizesAvailable() and getBufferSizeSamples()
        @returns    an error description if there's a problem, or an empty string if it succeeds in
                    opening the device
        @see close
    */
    virtual const String open (const BigInteger& inputChannels,
                               const BigInteger& outputChannels,
                               double sampleRate,
                               int bufferSizeSamples) = 0;

    /** Closes and releases the device if it's open. */
    virtual void close() = 0;

    /** Returns true if the device is still open.

        A device might spontaneously close itself if something goes wrong, so this checks if
        it's still open.
    */
    virtual bool isOpen() = 0;

    /** Starts the device actually playing.

        This must be called after the device has been opened.

        @param callback     the callback to use for streaming the data.
        @see AudioIODeviceCallback, open
    */
    virtual void start (AudioIODeviceCallback* callback) = 0;

    /** Stops the device playing.

        Once a device has been started, this will stop it. Any pending calls to the
        callback class will be flushed before this method returns.
    */
    virtual void stop() = 0;

    /** Returns true if the device is still calling back.

        The device might mysteriously stop, so this checks whether it's
        still playing.
    */
    virtual bool isPlaying() = 0;

    /** Returns the last error that happened if anything went wrong. */
    virtual const String getLastError() = 0;

    //==============================================================================
    /** Returns the buffer size that the device is currently using.

        If the device isn't actually open, this value doesn't really mean much.
    */
    virtual int getCurrentBufferSizeSamples() = 0;

    /** Returns the sample rate that the device is currently using.

        If the device isn't actually open, this value doesn't really mean much.
    */
    virtual double getCurrentSampleRate() = 0;

    /** Returns the device's current physical bit-depth.

        If the device isn't actually open, this value doesn't really mean much.
    */
    virtual int getCurrentBitDepth() = 0;

    /** Returns a mask showing which of the available output channels are currently
        enabled.
        @see getOutputChannelNames
    */
    virtual const BigInteger getActiveOutputChannels() const = 0;

    /** Returns a mask showing which of the available input channels are currently
        enabled.
        @see getInputChannelNames
    */
    virtual const BigInteger getActiveInputChannels() const = 0;

    /** Returns the device's output latency.

        This is the delay in samples between a callback getting a block of data, and
        that data actually getting played.
    */
    virtual int getOutputLatencyInSamples() = 0;

    /** Returns the device's input latency.

        This is the delay in samples between some audio actually arriving at the soundcard,
        and the callback getting passed this block of data.
    */
    virtual int getInputLatencyInSamples() = 0;


    //==============================================================================
    /** True if this device can show a pop-up control panel for editing its settings.

        This is generally just true of ASIO devices. If true, you can call showControlPanel()
        to display it.
    */
    virtual bool hasControlPanel() const;

    /** Shows a device-specific control panel if there is one.

        This should only be called for devices which return true from hasControlPanel().
    */
    virtual bool showControlPanel();


    //==============================================================================
protected:
    /** Creates a device, setting its name and type member variables. */
    AudioIODevice (const String& deviceName,
                   const String& typeName);

    /** @internal */
    String name, typeName;
};


#endif   // __JUCE_AUDIOIODEVICE_JUCEHEADER__
