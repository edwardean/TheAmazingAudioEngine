//
//  AEAudioController.h
//  The Amazing Audio Engine
//
//  Created by Michael Tyson on 25/11/2011.
//  Copyright (c) 2011 A Tasty Pixel. All rights reserved.

#import <AudioToolbox/AudioToolbox.h>
#import <AudioUnit/AudioUnit.h>

#pragma mark - Notifications and constants

/*!
 * @var AEAudioControllerSessionInterruptionBeganNotification
 *      Notification that the audio session has been interrupted.
 *
 * @var AEAudioControllerSessionInterruptionEndedNotification
 *      Notification that the audio session interrupted has ended, and control
 *      has been passed back to the application.
 */
extern NSString * AEAudioControllerSessionInterruptionBeganNotification;
extern NSString * AEAudioControllerSessionInterruptionEndedNotification;

/*!
 * @enum ABInputMode
 *      Input mode
 *
 *      How to handle incoming audio
 *
 * @var ABInputModeStereoOrBridgedMono
 *      If input is stereo and system audio description is stereo, provide stereo audio; 
 *      if input is mono, bridge the mono audio to stereo audio
 *
 * @var ABInputModeStereoOrMono
 *      If input is stereo and system audio description is stereo, provide stereo audio; 
 *      if input is mono, provide mono audio
 *
 * @var ABInputModeDualMonoOrMono
 *      If input is stereo, treat this as two separate mono channels; 
 *      if input is mono, provide mono audio
 *
 */
typedef enum {
    ABInputModeStereoOrBridgedMono,
    ABInputModeStereoOrMono,
    ABInputModeDualMonoOrMono
} ABInputMode;

#pragma mark - Callbacks and protocols

/*!
 * Render callback
 *
 *      This is called when audio for the channel is required. As this is called from Core Audio's
 *      realtime thread, you should not wait on locks, allocate memory, or call any Objective-C or BSD
 *      code from this callback.
 *
 *      The channel object is passed through as a parameter.  You should not send it Objective-C
 *      messages, but if you implement the callback within your channel's \@implementation block, you 
 *      can gain direct access to the instance variables of the channel ("((MyChannel*)channel)->myInstanceVariable").
 *
 * @param channel           The channel object
 * @param time              The time the buffer will be played
 * @param frames            The number of frames required
 * @param audio             The audio buffer list - audio should be copied into the provided buffers
 */
typedef OSStatus (*AEAudioControllerRenderCallback) (id                        channel,
                                                     const AudioTimeStamp     *time,
                                                     UInt32                    frames,
                                                     AudioBufferList          *audio);

/*!
 * AEAudioPlayable protocol
 *
 *      The interface that a channel object must implement - this includes 'renderCallback',
 *      which is a @link AEAudioControllerRenderCallback C callback @endlink to be called when 
 *      audio is required.  The callback will be passed a reference to this object, so you should
 *      implement it from within the \@implementation block to gain access to your
 *      instance variables.
 */
@protocol AEAudioPlayable <NSObject>

/*!
 * Reference to the render callback
 *
 *      This method must return a pointer to the render callback function that provides
 *      the channel audio.  Always return the same pointer - this must not change over time.
 *
 * @return Pointer to a render callback function
 */
@property (nonatomic, readonly) AEAudioControllerRenderCallback renderCallback;

@optional

/*!
 * Track volume
 *
 *      Changes are tracked by Key-Value Observing, so be sure to send KVO notifications
 *      when the value changes (or use a readwrite property).
 *
 *      Range: 0.0 to 1.0
 */
@property (nonatomic, readonly) float volume;

/*!
 * Track pan
 *
 *      Changes are tracked by Key-Value Observing, so be sure to send KVO notifications
 *      when the value changes (or use a readwrite property).
 *
 *      Range: -1.0 (left) to 1.0 (right)
 */
@property (nonatomic, readonly) float pan;

/*
 * Whether track is currently playing
 *
 *      If this is NO, then the track will be silenced and no further render callbacks
 *      will be performed until set to YES again.
 *
 *      Changes are tracked by Key-Value Observing, so be sure to send KVO notifications
 *      when the value changes (or use a readwrite property).
 */
@property (nonatomic, readonly) BOOL playing;

/*
 * Whether track is muted
 *
 *      If YES, track will be silenced, but render callbacks will continue to be performed.
 *      
 *      Changes are tracked by Key-Value Observing, so be sure to send KVO notifications
 *      when the value changes (or use a readwrite property).
 */
@property (nonatomic, readonly) BOOL muted;

@end

/*!
 * @var AEAudioSourceInput
 *      Main audio input
 *
 * @var AEAudioSourceMainOutput
 *      Main audio output
 *
 * @var AEAudioSourceInputAlternate
 *      If selected @link AEAudioController::inputMode input mode @endlink is @link ABInputModeDualMonoOrMono @endlink, this corresponds to the second input channel
 */
#define AEAudioSourceInput           ((void*)0x01)
#define AEAudioSourceMainOutput      ((void*)0x02)
#define AEAudioSourceInputAlternate  ((void*)0x03)

/*!
 * Audio callback
 *
 *      This callback is used for notifying you of incoming audio (either from 
 *      the built-in microphone, or another input device), and outgoing audio that
 *      is about to be played by the system.  It's also used for audio filtering.
 *
 *      The receiver object is passed through as a parameter.  You should not send it Objective-C
 *      messages, but if you implement the callback within your receiver's \@implementation block, you 
 *      can gain direct access to the instance variables of the receiver ("((MyReceiver*)receiver)->myInstanceVariable").
 *
 *      Do not wait on locks, allocate memory, or call any Objective-C or BSD code.
 *
 * @param receiver   The receiver object
 * @param source     The source of the audio: @link AEAudioSourceInput @endlink, @link AEAudioSourceMainOutput @endlink, @link AEAudioSourceInputAlternate @endlink, an AEChannelGroup or an id<AEAudioPlayable>.
 * @param time       The time the audio was received (for input), or the time it will be played (for output)
 * @param frames     The length of the audio, in frames
 * @param audio      The audio buffer list
 */
typedef void (*AEAudioControllerAudioCallback) (id                        receiver,
                                                void                     *source,
                                                const AudioTimeStamp     *time,
                                                UInt32                    frames,
                                                AudioBufferList          *audio);


/*!
 * AEAudioReceiver protocol
 *
 *      The interface that a object must implement to receive incoming or outgoing output audio.
 *      This includes 'receiverCallback', which is a @link AEAudioControllerAudioCallback C callback @endlink 
 *      to be called when audio is available.  The callback will be passed a reference to this object, so you 
 *      should implement it from within the \@implementation block to gain access to your instance variables.
 */
@protocol AEAudioReceiver <NSObject>

/*!
 * Reference to the receiver callback
 *
 *      This method must return a pointer to the receiver callback function that accepts received
 *      audio.  Always return the same pointer - this must not change over time.
 *
 * @return Pointer to an audio callback
 */
@property (nonatomic, readonly) AEAudioControllerAudioCallback receiverCallback;

@end

/*!
 * AEAudioFilter protocol
 *
 *      The interface that a filter object must implement - this includes 'filterCallback',
 *      which is a @link AEAudioControllerAudioCallback C callback @endlink to be called when 
 *      audio is to be filtered.  The callback will be passed a reference to this object, so you should
 *      implement it from within the \@implementation block to gain access to your
 *      instance variables.
 */
@protocol AEAudioFilter <NSObject>

/*!
 * Reference to the filter callback
 *
 *      This method must return a pointer to the filter callback function that performs
 *      audio manipulation.  Always return the same pointer - this must not change over time.
 *
 * @return Pointer to an audio callback
 */
@property (nonatomic, readonly) AEAudioControllerAudioCallback filterCallback;

@end


/*!
 * Variable speed filter producer
 *
 *      This defines the function passed to a AEAudioControllerVariableSpeedFilterCallback,
 *      which is used to produce input audio to be processed by the filter.
 *
 * @param producerToken    An opaque pointer to be passed to the function
 * @param audio            Audio buffer list to be written to
 * @param frames           Number of frames to produce
 * @return A status code
 */
typedef OSStatus (*AEAudioControllerVariableSpeedFilterProducer)(void            *producerToken, 
                                                                 AudioBufferList *audio, 
                                                                 UInt32           frames);


/*!
 * Variable speed filter callback
 *
 *      This callback is used for variable speed audio filters - that is, filters that
 *      have a playback rate that is not 1:1.  The system provides as an argument
 *      a function pointer that is used to produce input audio.
 *
 *      The filter object is passed through as a parameter.  You should not send it Objective-C
 *      messages, but if you implement the callback within your filter's \@implementation block, you 
 *      can gain direct access to the instance variables of the filter ("((MyFilter*)filter)->myInstanceVariable").
 *
 *      Do not wait on locks, allocate memory, or call any Objective-C or BSD code.
 *
 * @param filter    The filter object
 * @param producer  A function pointer to be used to produce input audio
 * @param producerToken An opaque pointer to be passed to the producer as the first argument
 * @param time      The time the output audio will be played
 * @param frames    The length of the required audio, in frames
 * @param audio     The audio buffer list to write output audio to
 */
typedef void (*AEAudioControllerVariableSpeedFilterCallback) (id                        filter,
                                                              AEAudioControllerVariableSpeedFilterProducer producer,
                                                              void                     *producerToken,
                                                              const AudioTimeStamp     *time,
                                                              UInt32                    frames,
                                                              AudioBufferList          *audio);

/*!
 * AEAudioVariableSpeedFilter protocol
 *
 *      The interface that a variable speed filter object must implement - this includes 'filterCallback',
 *      which is a @link AEAudioControllerVariableSpeedFilterCallback C callback @endlink to be called when 
 *      audio is to be filtered.  The callback will be passed a reference to this object, so you should
 *      implement it from within the \@implementation block to gain access to your
 *      instance variables.
 */
@protocol AEAudioVariableSpeedFilter <NSObject>

/*!
 * Reference to the filter callback
 *
 *      This method must return a pointer to the filter callback function that performs
 *      audio manipulation.  Always return the same pointer - this must not change over time.
 *
 * @return Pointer to a variable speed filter callback
 */
@property (nonatomic, readonly) AEAudioControllerVariableSpeedFilterCallback filterCallback;

@end


/*!
 * @enum AEAudioTimingContext
 *      Timing contexts
 *
 *      Used to indicate which context the audio system is in when a timing receiver
 *      is called.
 *
 * @var AEAudioTimingContextInput
 *      Input context: Audio system is about to process some incoming audio (from microphone, etc).
 *
 * @var AEAudioTimingContextOutput
 *      Output context: Audio system is about to render the next buffer for playback.
 *
 */
typedef enum {
    AEAudioTimingContextInput,
    AEAudioTimingContextOutput
} AEAudioTimingContext;

/*!
 * Timing callback
 *
 *      This callback used to notify you when the system time advances.  When called
 *      from an input context, it occurs before any input receiver calls are performed.
 *      When called from an output context, it occurs before any output receivers are
 *      performed.
 *
 *      The receiver object is passed through as a parameter.  You should not send it Objective-C
 *      messages, but if you implement the callback within your receiver's \@implementation block, you 
 *      can gain direct access to the instance variables of the receiver ("((MyReceiver*)receiver)->myInstanceVariable").
 *
 *      Do not wait on locks, allocate memory, or call any Objective-C or BSD code.
 *
 * @param receiver  The receiver object
 * @param time      The time the audio was received (for input), or the time it will be played (for output)
 * @param context   The timing context - either input, or output
 */
typedef void (*AEAudioControllerTimingCallback) (id                        receiver,
                                                 const AudioTimeStamp     *time,
                                                 AEAudioTimingContext      context);

/*!
 * AEAudioTimingReceiver protocol
 *
 *      The interface that a object must implement to receive system time advance notices.
 *      This includes 'timingReceiver', which is a @link AEAudioControllerTimingCallback C callback @endlink 
 *      to be called when the system time advances.  The callback will be passed a reference to this object, so you 
 *      should implement it from within the \@implementation block to gain access to your instance variables.
 */
@protocol AEAudioTimingReceiver <NSObject>

/*!
 * Reference to the receiver callback
 *
 *      This method must return a pointer to the receiver callback function that accepts received
 *      audio.  Always return the same pointer - this must not change over time.
 *
 * @return Pointer to an audio callback
 */
@property (nonatomic, readonly) AEAudioControllerTimingCallback timingReceiverCallback;

@end


/*!
 * Channel group identifier
 *
 *      See @link AEAudioController::createChannelGroup @endlink for more info.
 */
typedef struct _channel_group_t* AEChannelGroup;

@class AEAudioController;

/*!
 * Message handler function
 */
typedef long (*AEAudioControllerMessageHandler) (AEAudioController *audioController, long *ioParameter1, long *ioParameter2, long *ioParameter3, void *ioOpaquePtr);

#pragma mark -

/*!
 * Main controller class
 *
 *      Use:
 *
 *      <ol>
 *      <li>@link initWithAudioDescription: Initialise@endlink, with the desired audio format.</li>
 *      <li>Set required parameters.</li>
 *      <li>Add channels, input receivers, output receivers, timing receivers and filters, as required.
 *         Note that all these can be added/removed during operation as well.</li>
 *      <li>Call @link start @endlink to begin processing audio.</li>
 *      </ol>
 */
@interface AEAudioController : NSObject

#pragma mark - Setup and start/stop
/** @name Setup and start/stop */
///@{

/*!
 * Canonical Audio Unit audio description
 *
 *      This is the non-interleaved audio description associated with the kAudioFormatFlagsAudioUnitCanonical flag,
 *      at 44.1kHz that can be used with @link initWithAudioDescription: @endlink.
 *
 *      This is the 8.24 fixed-point audio format recommended by Apple, although it is relatively 
 *      inconvenient to work with individual samples without converting.
 */
+ (AudioStreamBasicDescription)audioUnitCanonicalAudioDescription;

/*!
 * 16-bit stereo audio description, interleaved
 *
 *      This is a 16-bit signed PCM, stereo, interleaved format at 44.1kHz that can be used
 *      with @link initWithAudioDescription: @endlink.
 */
+ (AudioStreamBasicDescription)interleaved16BitStereoAudioDescription;

/*!
 * 16-bit stereo audio description, non-interleaved
 *
 *      This is a 16-bit signed PCM, stereo, non-interleaved format at 44.1kHz that can be used
 *      with @link initWithAudioDescription: @endlink.
 */
+ (AudioStreamBasicDescription)nonInterleaved16BitStereoAudioDescription;

/*!
 * Determine whether voice processing is available on this device
 *
 *      Older devices are not able to perform voice processing - this determines
 *      whether it's available.  See @link voiceProcessingEnabled @endlink for info.
 */
+ (BOOL)voiceProcessingAvailable;

/*!
 * Initialize the audio controller system, with the audio description you provide.
 *
 *      Creates and configures the audio unit and initial mixer audio unit.
 *
 *      This initialises the audio system without input (from microphone, etc) enabled. If
 *      you desire audio input, use @link initWithAudioDescription:inputEnabled:useVoiceProcessing: @endlink.
 *
 * @param audioDescription  Audio description to use for all audio
 */
- (id)initWithAudioDescription:(AudioStreamBasicDescription)audioDescription;

/*!
 * Initialize the audio controller system, with the audio description you provide.
 *
 *      Creates and configures the input/output audio unit and initial mixer audio unit.
 *
 * @param audioDescription    Audio description to use for all audio
 * @param enableInput         Whether to enable audio input from the microphone or another input device
 * @param useVoiceProcessing  Whether to use the voice processing unit (see @link voiceProcessingEnabled @endlink and @link voiceProcessingAvailable @endlink).
 */
- (id)initWithAudioDescription:(AudioStreamBasicDescription)audioDescription inputEnabled:(BOOL)enableInput useVoiceProcessing:(BOOL)useVoiceProcessing;

/*!
 * Start audio engine
 */
- (void)start;

/*!
 * Stop audio engine
 */
- (void)stop;

///@}
#pragma mark - Channel and channel group management
/** @name Channel and channel group management */
///@{

/*!
 * Add channels
 *
 *      Takes an array of one or more objects that implement the @link AEAudioPlayable @endlink protocol.
 *
 * @param channels An array of id<AEAudioPlayable> objects
 */
- (void)addChannels:(NSArray*)channels;

/*!
 * Add channels to a channel group
 *
 * @param channels Array of id<AEAudioPlayable> objects
 * @param group    Group identifier
 */
- (void)addChannels:(NSArray*)channels toChannelGroup:(AEChannelGroup)group;

/*!
 * Remove channels
 *
 *      Takes an array of one or more objects that implement the @link AEAudioPlayable @endlink protocol.
 *
 * @param channels An array of id<AEAudioPlayable> objects
 */
- (void)removeChannels:(NSArray*)channels;

/*!
 * Remove channels from a channel group
 *
 * @param channels Array of id<AEAudioPlayable> objects
 * @param group    Group identifier
 */
- (void)removeChannels:(NSArray*)channels fromChannelGroup:(AEChannelGroup)group;

/*!
 * Obtain a list of all channels, across all channel groups
 */
- (NSArray*)channels;

/*!
 * Get a list of channels within a channel group
 *
 * @param group Group identifier
 * @return Array of id<AEAudioPlayable> objects contained within the group
 */
- (NSArray*)channelsInChannelGroup:(AEChannelGroup)group;

/*!
 * Create a channel group
 *
 *      Channel groups cause the channels within the group to be pre-mixed together, so that one filter
 *      can be applied to several channels without the added performance impact.
 *
 *      You can create trees of channel groups using @link addChannels:toChannelGroup: @endlink, with
 *      filtering at each branch, for complex filter chaining.
 *
 * @return An identifier for the created group
 */
- (AEChannelGroup)createChannelGroup;

/*!
 * Create a channel sub-group within an existing channel group
 *
 *      With this method, you can create trees of channel groups, with filtering steps at
 *      each branch of the tree.
 *
 * @param group Group identifier
 * @return An identifier for the created group
 */
- (AEChannelGroup)createChannelGroupWithinChannelGroup:(AEChannelGroup)group;

/*!
 * Remove a channel group
 *
 *      Removes channels from the group and releases associated resources.
 *
 * @param group Group identifier
 */
- (void)removeChannelGroup:(AEChannelGroup)group;

/*!
 * Get a list of top-level channel groups
 *
 * @return Array of NSValues containing pointers (group identifiers)
 */
- (NSArray*)topLevelChannelGroups;

/*!
 * Get a list of sub-groups contained within a group
 *
 * @param group Group identifier
 * @return Array of NSNumber containing sub-group identifiers
 */
- (NSArray*)channelGroupsInChannelGroup:(AEChannelGroup)group;

/*!
 * Set the volume level of a channel group
 *
 * @param volume    Group volume (0 - 1)
 * @param group     Group identifier
 */
- (void)setVolume:(float)volume forChannelGroup:(AEChannelGroup)group;

/*!
 * Set the pan of a channel group
 *
 * @param pan       Group pan (-1.0, left to 1.0, right)
 * @param group     Group identifier
 */
- (void)setPan:(float)pan forChannelGroup:(AEChannelGroup)group;

/*!
 * Set the mute status of a channel group
 *
 * @param muted     Whether group is muted
 * @param group     Group identifier
 */
- (void)setMuted:(BOOL)muted forChannelGroup:(AEChannelGroup)group;

///@}
#pragma mark - Filters
/** @name Filters */
///@{

/*!
 * Add an audio filter to the system output
 *
 *      Audio filters are used to process live audio before playback.
 *
 * @param filter An object that implements the AEAudioFilter protocol
 */
- (void)addFilter:(id<AEAudioFilter>)filter;

/*!
 * Add an audio filter to a channel
 *
 *      Audio filters are used to process live audio before playback.
 *
 *      You can apply audio filters to one or more channels - use channel groups to do so
 *      without the extra performance overhead by pre-mixing channels together first. See
 *      @link createChannelGroup @endlink.
 *
 *      You can also apply more than one audio filter to a channel - each audio filter will
 *      be performed on the audio in the order in which the filters were added using this
 *      method.
 *
 * @param filter  An object that implements the AEAudioFilter protocol
 * @param channel The channel on which to perform audio processing
 */
- (void)addFilter:(id<AEAudioFilter>)filter toChannel:(id<AEAudioPlayable>)channel;

/*!
 * Add an audio filter to a channel group
 *
 *      Audio filters are used to process live audio before playback.
 *
 *      Create and add filters to a channel group to process multiple channels with one filter,
 *      without the performance hit of processing each channel individually.
 *
 * @param filter An object that implements the AEAudioFilter protocol
 * @param group  The channel group on which to perform audio processing
 */
- (void)addFilter:(id<AEAudioFilter>)filter toChannelGroup:(AEChannelGroup)group;

/*!
 * Remove a filter from system output
 *
 * @param filter The filter to remove
 */
- (void)removeFilter:(id<AEAudioFilter>)filter;

/*!
 * Remove a filter from a channel
 *
 * @param filter  The filter to remove
 * @param channel The channel to stop filtering
 */
- (void)removeFilter:(id<AEAudioFilter>)filter fromChannel:(id<AEAudioPlayable>)channel;

/*!
 * Remove a filter from a channel group
 *
 * @param filter The filter to remove
 * @param group  The group to stop filtering
 */
- (void)removeFilter:(id<AEAudioFilter>)filter fromChannelGroup:(AEChannelGroup)group;

/*!
 * Get a list of all top-level filters
 */
- (NSArray*)filters;

/*!
 * Get a list of all filters currently operating on the channel
 *
 * @param channel Channel to get filters for
 */
- (NSArray*)filtersForChannel:(id<AEAudioPlayable>)channel;

/*!
 * Get a list of all filters currently operating on the channel group
 *
 * @param group Channel group to get filters for
 */
- (NSArray*)filtersForChannelGroup:(AEChannelGroup)group;

/*!
 * Set variable speed audio filter for the system output
 *
 *      Variable audio filters are used to process live audio before playback, at a
 *      playback rate other than 1:1. You can provide one variable audio filter per
 *      node (channel, group, the root system node).
 *
 *      See @link AEAudioControllerVariableSpeedFilterCallback @endlink for more info.
 *
 * @param filter An object that implements the AEAudioVariableSpeedFilter protocol, or nil
 */
- (void)setVariableSpeedFilter:(id<AEAudioVariableSpeedFilter>)filter;

/*!
 * Set variable speed audio filter for the system output
 *
 *      Variable audio filters are used to process live audio before playback, at a
 *      playback rate other than 1:1. You can provide one variable audio filter per
 *      node (channel, group, the root system node).
 *
 *      See @link AEAudioControllerVariableSpeedFilterCallback @endlink for more info.
 *
 * @param filter An object that implements the AEAudioVariableSpeedFilter protocol, or nil
 * @param channel Channel to assign filter to
 */
- (void)setVariableSpeedFilter:(id<AEAudioVariableSpeedFilter>)filter forChannel:(id<AEAudioPlayable>)channel;

/*!
 * Set variable speed audio filter for the system output
 *
 *      Variable audio filters are used to process live audio before playback, at a
 *      playback rate other than 1:1. You can provide one variable audio filter per
 *      node (channel, group, the root system node).
 *
 *      See @link AEAudioControllerVariableSpeedFilterCallback @endlink for more info.
 *
 * @param filter An object that implements the AEAudioVariableSpeedFilter protocol, or nil
 * @param group Group to assign filter to
 */
- (void)setVariableSpeedFilter:(id<AEAudioVariableSpeedFilter>)filter forChannelGroup:(AEChannelGroup)group;

///@}
#pragma mark - Output receivers
/** @name Output receivers */
///@{

/*!
 * Add an output receiver
 *
 *      Output receivers receive audio that is being played by the system.  Use this
 *      method to add a receiver to receive audio that consists of all the playing channels
 *      mixed together.
 *
 * @param receiver An object that implements the AEAudioReceiver protocol
 */
- (void)addOutputReceiver:(id<AEAudioReceiver>)receiver;

/*!
 * Add an output receiver
 *
 *      Output receivers receive audio that is being played by the system.  Use this
 *      method to add a callback to receive audio from a particular channel.
 *
 * @param receiver An object that implements the AEAudioReceiver protocol
 * @param channel  A channel
 */
- (void)addOutputReceiver:(id<AEAudioReceiver>)receiver forChannel:(id<AEAudioPlayable>)channel;

/*!
 * Add an output receiver for a particular channel group
 *
 *      Output receivers receive audio that is being played by the system.  By registering
 *      a callback for a particular channel group, you can receive the mixed audio of only that
 *      group.
 *
 * @param receiver An object that implements the AEAudioReceiver protocol
 * @param group    A channel group identifier
 */
- (void)addOutputReceiver:(id<AEAudioReceiver>)receiver forChannelGroup:(AEChannelGroup)group;

/*!
 * Remove an output receiver
 *
 * @param receiver The receiver to remove
 */
- (void)removeOutputReceiver:(id<AEAudioReceiver>)receiver;

/*!
 * Remove an output receiver from a channel
 *
 * @param receiver The receiver to remove
 * @param channel  Channel to remove receiver from
 */
- (void)removeOutputReceiver:(id<AEAudioReceiver>)receiver fromChannel:(id<AEAudioPlayable>)channel;

/*!
 * Remove an output receiver from a particular channel group
 *
 * @param receiver The receiver to remove
 * @param group    A channel group identifier
 */
- (void)removeOutputReceiver:(id<AEAudioReceiver>)receiver fromChannelGroup:(AEChannelGroup)group;

/*!
 * Obtain a list of all top-level output receivers
 */
- (NSArray*)outputReceivers;

/*!
 * Obtain a list of all output receivers for the specified channel
 *
 * @param channel A channel
 */
- (NSArray*)outputReceiversForChannel:(id<AEAudioPlayable>)channel;

/*!
 * Obtain a list of all output receivers for the specified group
 *
 * @param group A channel group identifier
 */
- (NSArray*)outputReceiversForChannelGroup:(AEChannelGroup)group;

///@}
#pragma mark - Input receivers
/** @name Input receivers */
///@{

/*!
 * Add an input receiver
 *
 *      Input receivers receive audio that is being received by the microphone or another input device.
 *
 *      Note that the audio format provided to input receivers depends on the value of @link inputMode @endlink.
 *      Check the audio buffer list parameters to determine the kind of audio you are receiving (for example, if
 *      you are using an interleaved format such as @link interleaved16BitStereoAudioDescription @endlink
 *      then the audio->mBuffers[0].mNumberOfChannels field will be 1 for mono, and 2 for stereo audio).  If you
 *      are using a non-interleaved format such as @link nonInterleaved16BitStereoAudioDescription @endlink, then
 *      audio->mNumberBuffers will be 1 for mono, and 2 for stereo.
 *
 * @param receiver An object that implements the AEAudioReceiver protocol
 */
- (void)addInputReceiver:(id<AEAudioReceiver>)receiver;

/*!
 * Remove an input receiver
 *
 * @param receiver Receiver to remove
 */
- (void)removeInputReceiver:(id<AEAudioReceiver>)receiver;

/*!
 * Obtain a list of all input receivers
 */
- (NSArray*)inputReceivers;

///@}
#pragma mark - Timing receivers
/** @name Timing receivers */
///@{

/*!
 * Add a timing receiver
 *
 *      Timing receivers receive notifications for when time has advanced.  When called
 *      from an input context, the call occurs before any input receiver calls are performed.
 *      When called from an output context, it occurs before any output receivers are
 *      performed.
 *
 *      This mechanism can be used to trigger time-dependent events.
 *
 * @param receiver An object that implements the AEAudioTimingReceiver protocol
 */
- (void)addTimingReceiver:(id<AEAudioTimingReceiver>)receiver;

/*!
 * Remove a timing receiver
 *
 * @param receiver An object that implements the AEAudioTimingReceiver protocol
 */
- (void)removeTimingReceiver:(id<AEAudioTimingReceiver>)receiver;

/*!
 * Obtain a list of all timing receivers
 */
- (NSArray*)timingReceivers;

///@}
#pragma mark - Realtime/Main thread messaging system
/** @name Realtime/Main thread messaging system */
///@{

/*!
 * Send a message to the realtime thread asynchronously, optionally receiving a response via a block
 *
 *      This is a synchronization mechanism that allows you to schedule actions to be performed 
 *      on the realtime audio thread without any locking mechanism required.  Pass in a function pointer
 *      and a number of arguments, and the function will be called on the realtime thread at the next
 *      polling interval.
 *
 *      If provided, the response block will be called on the main thread after the message has
 *      been sent, and will be passed the parameters and result code from the handler.
 *
 * @param handler       A pointer to a function to call on the realtime thread
 * @param parameter1    First parameter, usage up to the developer
 * @param parameter2    Second parameter
 * @param parameter3    Third parameter
 * @param ioOpaquePtr   An opaque pointer
 * @param responseBlock A block to be performed on the main thread after the handler has been run
 */
- (void)performAsynchronousMessageExchangeWithHandler:(AEAudioControllerMessageHandler)handler 
                                           parameter1:(long)parameter1 
                                           parameter2:(long)parameter2
                                           parameter3:(long)parameter3
                                          ioOpaquePtr:(void*)ioOpaquePtr 
                                        responseBlock:(void (^)(long result, long parameter1, long parameter2, long parameter3, void *ioOpaquePtr))responseBlock;

/*!
 * Send a message to the realtime thread synchronously
 *
 *      This is a synchronization mechanism that allows you to schedule actions to be performed 
 *      on the realtime audio thread without any locking mechanism required.  Pass in a function pointer
 *      and a number of arguments, and the function will be called on the realtime thread at the next
 *      polling interval.
 *
 *      This method will block on the main thread until the handler has been called, and a response
 *      received.
 *
 * @param handler       A pointer to a function to call on the realtime thread
 * @param parameter1    First parameter, usage up to the developer
 * @param parameter2    Second parameter
 * @param parameter3    Third parameter
 * @param ioOpaquePtr   An opaque pointer
 * @return The result returned from the handler
 */
- (long)performSynchronousMessageExchangeWithHandler:(AEAudioControllerMessageHandler)handler 
                                          parameter1:(long)parameter1 
                                          parameter2:(long)parameter2 
                                          parameter3:(long)parameter3
                                         ioOpaquePtr:(void*)ioOpaquePtr;

/*!
 * Send a message to the main thread asynchronously
 *
 *      This is a synchronization mechanism that allows you to schedule actions to be performed 
 *      on the main thread, without any locking or memory allocation.  Pass in a function pointer
 *      and a number of arguments, and the function will be called on the main thread at the next
 *      polling interval.
 *
 * @param audioController The audio controller
 * @param handler       A pointer to a function to call on the main thread
 * @param parameter1    First parameter, usage up to the developer
 * @param parameter2    Second parameter
 * @param parameter3    Third parameter
 * @param ioOpaquePtr   An opaque pointer
 */
void AEAudioControllerSendAsynchronousMessageToMainThread(AEAudioController* audioController, 
                                                          AEAudioControllerMessageHandler handler, 
                                                          long parameter1, 
                                                          long parameter2,
                                                          long parameter3,
                                                          void *ioOpaquePtr);

///@}
#pragma mark - Properties

/*!
 * Enable audio input
 *
 *      Set to YES to enable recording from an input device.
 *
 *      Note that setting this parameter will cause the entire audio system to be shut down and 
 *      restarted with the new setting, which will result in a break in audio playback.
 *
 *      Default is NO.
 */
@property (nonatomic, assign) BOOL enableInput;

/*! 
 * Mute output
 *
 *      Set to YES to mute all system output. Note that even if this is YES, playback
 *      callbacks will still receive audio, as the silencing happens after output receiver
 *      callbacks are called.
 */
@property (nonatomic, assign) BOOL muteOutput;

/*!
 * Enable audio input from Bluetooth devices
 *
 *      Default is YES.
 */
@property (nonatomic, assign) BOOL enableBluetoothInput;

/*!
 * Determine whether input gain is available
 */
@property (nonatomic, readonly) BOOL inputGainAvailable;

/*!
 * Set audio input gain (if input gain is available)
 *
 *      Value must be in the range 0-1
 */
@property (nonatomic, assign) float inputGain;

/*!
 * Whether to use the built-in voice processing system
 *
 *      This can be useful for removing echo/feedback when playing through the speaker
 *      while simultaneously recording through the microphone.  Not suitable for music,
 *      but works adequately well for speech.
 *
 *      Note that changing this value will cause the entire audio system to be shut down 
 *      and restarted with the new setting, which will result in a break in audio playback.
 *
 *      Enabling voice processing in short buffer duration environments (< 0.01s) may cause
 *      stuttering.
 *
 *      Default is NO.
 */
@property (nonatomic, assign) BOOL voiceProcessingEnabled;

/*!
 * Whether to only perform voice processing for the SpeakerAndMicrophone route
 *
 *      This causes voice processing to only be enabled in the classic echo removal
 *      scenario, when audio is being played through the device speaker and recorded
 *      by the device microphone.
 *
 *      Default is YES.
 */
@property (nonatomic, assign) BOOL voiceProcessingOnlyForSpeakerAndMicrophone;

/*! 
 * Input mode: How to handle incoming audio
 *
 *      If you are using a stereo audio format, this setting defines how the system
 *      receives incoming audio.
 *
 *      See @link ABInputMode @endlink for a description of the available options.
 *
 *      Default is ABInputModeStereoOrBridgedMono.
 */
@property (nonatomic, assign) ABInputMode inputMode;

/*!
 * Preferred buffer duration (in seconds)
 *
 *      Set this to low values for better latency, but more processing overhead, or higher
 *      values for greater latency with lower processing overhead.  This parameter affects
 *      the length of the audio buffers received by the various callbacks.
 *
 *      Default is 0.005.
 */
@property (nonatomic, assign) float preferredBufferDuration;

/*!
 * Determine whether the audio engine is running
 *
 *      This is affected by calling start and stop on the audio controller.
 */
@property (nonatomic, readonly) BOOL running;

/*!
 * Determine whether audio is currently being played through the device's speaker
 *
 *      This property is observable
 */
@property (nonatomic, readonly) BOOL playingThroughDeviceSpeaker;

/*!
 * Whether audio input is currently available
 *
 *      Note: This property is observable
 */
@property (nonatomic, readonly) BOOL audioInputAvailable;

/*!
 * The number of audio channels that the current audio input device provides
 *
 *      Note: This property is observable
 */
@property (nonatomic, readonly) NSUInteger numberOfInputChannels;

/*!
 * The audio description that the audio controller was setup with
 */
@property (nonatomic, readonly) AudioStreamBasicDescription audioDescription;

/*!
 * The Remote IO audio unit used for input and output
 */
@property (nonatomic, readonly) AudioUnit audioUnit;

@end
