.\"_ ECE3321 project
.\"_
.nr N 2
.BIB testdoc.rbd
.\"_
.\"_ heading at level three (and below) start a new line
.nr Hb 3
.\"_
.\"_
.EQ
delim $$
define linbo3 % Ti:LiNbO sub 3 %
.EN
.\"_
.PF "'5 June 1998'MPEG Layer III Audio Compression'David Morano'"
.\"_
.\"_
.SP 1
.\"_
.DS C
.B
\s+2MPEG Layer III Audio Compression\s-2
.R
.SP 2
\s+2\fIDavid Morano\fP\s-2
5 June 1998
.DE
.\"_
.SP 2
.DS C
\fIAbstract\fP
.DE
.SP
.DS CB F 5
.in +5
This paper presents an overview of the MPEG Layer III Audio
compression standard.  Some background and context for the compression
standard is given.  The main features of MPEG Audio and Layer III
in particular are presented and a more detailed explanation of
the components of the Layer III compression encoding process is
provided.
.in -5
.DE
.SP 2
.\"_
.H 1 "Introduction"
.P
This paper will present an overview of the best of the first major group
of Motion Picture Experts Group (MPEG) audio compression schemes known
as Layer III audio compression.  MPEG audio, in general, is the first
significant method to provide moderate to low bitrate data streams
that are \fItransparently\fP as good quality as the original audio.
This is achieved even when the original audio is sampled at rates as
high as 48,000 samples per second.  It is the first audio standard at
all to incorporate psychoacoustic techniques to dramatically reduce
the required bandwidth to store or transmit the compressed audio data.
Audio that is transparently as good as the original has come to mean that
it is not necessarily a good waveform reconstruction of the original
audio but rather a reconstruction of a sort that sounds perceptually
identical to the original.  The fact that psychoacoustic technology (to
be discussed much more later) was used in the development of MPEG audio
has actually seemed to provide a fascination to the public who has been
familiar with digital audio for some time now (audio CD-ROMs) but who
would have never thought that the details of how humans perceive sound
would have played such a roll in the format of audio media.
.P
Why is MPEG Layer III important?  This is a good question, especially since
more modern and somewhat better audio compression schemes are
available and becoming available.
The various MPEG standards, in general, are finding
wide acceptance for use in a variety of consumer
applications around the world \cite {chiariglione95mpeg}
.BK chiariglione MPEG technological basis multimedia applications
\&.
For example, MPEG video is essentially the video coding used for the
HDTV standard to be implemented between now and the year
2005 in the United States.
(Interestingly, a similar to but somewhat better audio standard has been
adopted for use in HDTV called AC-3 from Dolby.)
The author does not think that this
is the reason that MPEG Audio (and MPEG Layer III Audio in particular)
is becoming as important as it seems to be.
In the opinion of this author,
MPEG Layer III audio is becoming important because of the increasing
interest by the public for the first time to store high quality
audio in digital form.
This has been facilitated by both the introduction of computers
to the public in general and the wide-spread acceptance and use of
the Internet by the public.
However, the storage size of the digital audio is a factor
in the use of MPEG audio compression, especially for the
public with their somewhat limited storage capacity.
Conventional high quality (CD-ROM quality)
digital audio would
require over 10.5 Megabytes to store just one minute of stereo audio
(using 16 bit PCM samples, at 44100 samples per second).
This is too much for the public to bear at this point and I think that
MPEG Layer III was conveniently ready to provide an alternative.
(Think about trying to load this over an Internet connection for example!)
Since MPEG Layer III compressed CD-ROM quality sound only uses
a bandwidth of approximately 112 kbps to 128 kbps, one minute of
audio can occupy as little as about 840 kbytes (compared with 10.5 Megabytes
above).
This is about a factor of 14 reduction in storage requirement.
A conventional data CD-ROM may be able to store over 12 hours of CD quality
audio rather than just about one hour's worth.
.P
I think that the rising interest by the public in storing or transferring
high quality digital audio
is what may make
MPEG Layer III much more significant a standard than its use in
otherwise hidden consumer audio applications.
Due to a rising public interest in MPEG Layer III audio, it is
likely that there may be some additional commercial use of it
in the music industry other than the planned Digital Video Disc (DVD)
introduction and some
MPEG Layer III music distribution by Phillips on CD-ROM.
If there is a market for high quality audio-only media that stores
significantly more than the current audio CD-ROM, it is likely
to use MPEG Layer III compressed audio in some way.
.P
This paper will primarily present an overview of the compression scheme
used in Layer III of the MPEG standard.  Some elements of the Layer I and
Layer II schemes are also discussed but only because all of these compression
schemes share some common elements.  Note that there is significant
latitude in the MPEG Audio standard for different encoder and
decoder implementations.  This was done wisely by design so that
better techniques for encoding and decoding could be developed over
time.  This paper discusses one possible implementation for
the encoder.

The rest of the paper is organized as follows.
Section 2 provides the background of the MPEG compression
standard and some of the related work that contributed to its
development or which proceeded roughly in parallel with its
development.
Section 3 discusses the MPEG compression approach in general
and touches on the goal of the standard, sideband filtering
and the encoder design, the idea of psychoacoustics and its
impact on the standard.
Section 4 discusses Layer III of the standard -- our
primary focus of this paper.A
The discussion on Layer III includes an overview, the frequency
transform used in the layer, pre-echo reduction management,
non-uniform quantization, the encoding method (entropy encoding with
Hoffman codes), and the bit reservoir technique used in the layer.
Section 5 provides some enhancements and alternatives to
the standard.
We conclude in Section 6.
.\"_
.H 1 "Background and Related Work"
.P
Firstly, MPEG (a now relatively famous abbreviation) refers
to the Motion Picture Experts Group, which is under the overall
standards direction of the Internation Standards Organization (ISO).
The MPEG standards started in 1989 with requests for proposals for
inclusion in what would become the MPEG
I standard.
The MPEG standards, in general, cover both
the handling of video and audio.
Later, there was interest in extending the capabilities of MPEG
I and these efforts lead to what became MPEG II.
The MPEG I standard is defined by the ISO standard IS 11172 \*(Rf
.RS
ISO/IEC JTCI/SC29. Information technology - coding of
moving pictures and associated audio for digital storage media
at about 1.5 Mbps - cd 11172 (part 3, audio). Doc. ISO/IEC JTCI/SC29 NO71.
.RF
and
the MPEG II standard by IS13818.
Much more on MPEG I compression is discussed
by Shlien et al. \cite{ shlien94mpeg}
.BK shlien guide MPEG audio standard
Much more on MPEG II compression is discussed
by Grill et al. \cite{grill94mpeg}
.BK grill improved MPEG audio encoding
.P
The MPEG I and MPEG II standards do not
in themselves completely specify the audio compression method used but
rather the intended constraints and capabilities of the audio format in
these standards.  Each has its own compression bit stream format.
MPEG I basically deals with higher sampling rates of
the original audio and was intended to provide nearly perfect sound
quality as compared with the original audio.  These sampling rates are
typically 44100 samples per second (for audio CDROM) but 32000 and
48000 samples per second are also available.  MPEG I can handle two
audio channels making up the familiar stereo presentation.
MPEG II was introduced
to handle audio for which lower sampling rates are adequate (24000,
22050, and 16000 samples per second).
MPEG II also introduced handling of
more than two audio channels
(for surround sound and the like),
and support for a still more advanced
audio compression than Layer III (mentioned later).
.P
Now within each of these MPEG standards (I and II) there can be
different types of audio coding.  These different coding schemes are
referred to as \fILayers\fP.
The term \fIlayer\fP denotes that higher layers make
use of some of the same types of coding techniques as the lower layers but it
does not mean that all of the layers are there at the same time as
might be suggested by a typical data networking model.
Further, higher
layer implementations are required to decode any lower layer
implementation.
Higher compression Layer implementations are required by the
standard to also encode or decode any lower compression layers.
Each higher layer makes use of some of the same techniques and components
of a lower layer also.
.P
Target compression bitrates of 8000 bps to 256 kbps are
supported across the MPEG I and MPEG II standards.
Only fixed target bitrates are supported by MPEG I but both fixed
and variable bitrates are supported by MPEG II (variable bitrates
being only supported when additionally using Layer III compression).
The lowest target bitrate for stereo is usually 32 kbps using
the MPEG II standard bit stream format.
In general, the distinction between whether the MPEG I standard
is used in a given application or MPEG II is used is often completely
hidden from the end user and the proper standard is used to achieve
the desired compression objectives.
.P
In general, the three compression layers are available in order to provide
a time \- quality tradeoff for primarily the encoding process.
Layer I provides the fastest encoding time but poorer quality
compressed audio.  It is suitable for higher compression bitrates
like around 128 bps or higher and is currently used commercially
at a compressed bitrate of 192 bps.
It is meant for applications that can afford
to compress the audio stream moderately to what are still relatively high
target bitrates.
Layer II compression is more complex than Layer I (more time consuming)
but will produce better quality audio at the same target bitrate
or roughly equivalent quality sound (as Layer I) at some lower
bitrate.  It is often used to achieve a target bitrate of 128 kbps
with excellent quality audio.
Layer III provides the best quality compressed audio but can take
significantly longer to encode.  This compression scheme is
capable of providing target compressed bitrates as low as 64 kbps
with still very good sound.  This is important for use in transmitting
high fidelity audio over ISDN bearer (B) channels or any other 64kbps
telephony type digital channel.
.P
It should be noted that care
and fortune are both involved in keeping the decoder implementations
always as relatively simple as possible.  Decoders are always much
more prevalent than encoders so it is a good trait for a compression
scheme to shift complexity from the decoders to the encoder.
It is conjectured that an
encoding process can either afford to perform the encoding out of
real time, due to the extreme computational complexity of the encoding
process, or it can afford to have expensive (sometimes massive) parallel
oriented hardware to perform the encoding process in real time.
Over time, it is assumed that technology advances will make encoding both
faster and cheaper.
.P
There is an additional audio
compression scheme defined for use in MPEG II.
It is called \fIAdvanced Audio Coding\fP (AAC).
More information on AAC is covered by
Bosi and others from their work on the
ISO AAC standard \cite{ bosi96aac,bosi97aac}.
.BK bosi advanced audio encoding
The AAC compression is not backward
compatible with these first three Layers and therefore is often referred
to as "Not Backward Compatible"     	   	 	   
or NBC for short \cite{ johnson96nbc }    	 !   
In addition, not related directly to MPEG, there is an increasingly
popular advanced audio compression scheme known as \fIAdvanced Coding -3\fP
(AC-3) from Dolby.
Neither AAC compression nor AC-3 is further discussed in this paper.
.P
For the curious, work on MPEG IV (and MPEG 7)
is just starting and MPEG IV will be addressing almost all application
ranges
of both audio and video coding.
It will undoubtedly be
among the most significant unifications of most of the
various compression ideas developed so far.
It will also include speech related (and other very
low bit rate audio application) compression and coding methods.
(Contrary to early reports, MPEG IV is probably no
where near being standardized as early as November of 1998 as originally
planned, in the opinion of this author.)
Neither MPEG IV nor MPEG 7 will be addressed further
in this paper.
.\"_
.H 1 "MPEG Audio Compression Approach in General"
.P
Some of the basic ideas used in MPEG Audio compression are
now discussed.
MPEG Audio compression basically uses
a frequency domain coding technique.
A \fIfrequency subband encoding\fP method (or a \fItransform encoding\f
method in the case of Layer III compression)
is used
but with some important
modifications due to the psychoacoustic information
computed about the input signal.  The overall compression technique
may seem straightforward to some (DSP guys no doubt!)
and has been described as
exceedingly complicated by others.
.P
As will be discussed later, the Layer III compression scheme
introduces a number of additional important techniques that
substantially reduce the required bitrate for the encoded output
as well as increase the quality of the encoded audio.
The ideas discussed in these next few of sections apply,
in general to all compression Layers of the MPEG standard
but elements are enhanced for Layer II and Layer III respectively.
A more detailed discussion of Layer III compression, along with its
enhancements, is taken up later.
.\"_
.H 2 "Goal of MPEG Audio Compression"
.P
There are many requirements for audio compression of one sort
or another.  For example, there is a flurry of activity now
to try and find new and better audio compression and encoding
techniques for speech.  This has critical usefulness in the
current cellular and PCS mobile communications standards.
One of the most common complaints about existing digital
mobile communications standards is the nature of the low quality
speech reproduced.
In fact, the need for better speech in mobile communications
has prompted new speed coding techniques to be incorporated into
existing established and deployed standards!
.P
Accurately compressing and encoding speech was not the primary goal
of MPEG audio compression.  The goal was more specifically
to accurately compress and encode high quality audio (up to 48000
samples per second digital audio).  Although there have been many
good strategies and standards for compressing and encoding human speech
(many still argue that there is still a great need for improvement),
these methods do not work for high fidelity audio such as CD-ROM
quality music.  This is why MPEG audio took a different approach
to audio compression than the advanced speech compression strategies.
.\"_
.H 2 "Subband Filtering and the Basic Encoder Design"
.P
A block diagram of the general MPEG Audio encoder is given in
Figure \_ENCODER .
.DS CB
.PS < encoder.pic
.SP 2
.FG "General MPEG Audio Encoder Block Diagram"
.TAG ENCODER
.DE
.SP 2
The audio input signal to be encoded is sent to both the
polyphase filter bank and the psychoacoustic model block.
The filter bank filters the input signal into 32 equal spaced
subbands.
The input signal is also feed into one of several possible psychoacoustic
analysis blocks (only one typically used at a time)
that analyze the input signal for major spectral
content to a much finer degree than the polyphase filter bank.
The outputs of the polyphase filter bank are decimated by 32 such that
the number of samples output per unit time equals the number of
samples input per unit time.
.P
The input signal is processed in units of \fIframes\fP.
A frame consists of 384 input audio samples for Layer I compression
and 1152 input audio samples for Layers II and III.
This amounts to 12 consecutive subband signals (after subband decimation)
for each of the 32 outputs of the polyphase filter in Layer I.
Layers II and III process a frame consisting of three times this
amount of samples or 3 groups of 12 ( = 36) subband samples per frame.
A prototype subband filter is provided by the MPEG standard
for use in the polyphase filter bank.  This prototype filter
is conceptually modulated up (convolved with a cosine function)
to create a series of overlapping passbands for each subfilter
of the filter band.  Details of the prototype filter are provided
in the standard.
The filtering process occurs by shifting the input audio signal 32 samples at
a time (every frame) into a 512 sample-long window that is then processed
to produce the 32 subband outputs.
Although the pass bands of the 32 subband filters overlap and thus
creating aliasing,
the combination of the analysis subband filtering process with
the synthesis filtering process in an associated decoder
remove essentially all of these aliasing effects.
.P
The psychoacoustic block creates a \fIglobal masking threshold\fP
(discussed more later)
for a frame of the input signal samples being analyzed and this
information is used to dynamically adjust the scaling and the
amount of quantization
used to encode each of the 32 subband component signals in this frame.
This means that only one psychoacoustic computation is performed per
frame.  Unfortunately, the psychoacoustic computations tend
to be very expensive computationally anyway so this does not help as
much as one might want.
The global masking threshold
is computed as a function of frequency and therefore also the subband
frequencies.
The quantization levels are computed such that the amount of
quantization noise generated in each subband is just below the global masking
threshold for each subband.
Now those subband components that are quantized with less levels
will have more quantization noise.  Ah, here is the beauty of the scheme!
Since the scaling and the amount of quantization allocated to each subband is
below, or as close as possible to just below, the global masking
threshold, the quantization noise
allowed to appear in each subband will \fBnot\fP be perceived by the
human listener after signal reconstruction.  Since the minimum
amount of tolerable quantization was used in each subband, the total
number of bits needed to represent the signal has been minimized.
This is the source of the truly amazing compression ratios
that are possible with MPEG audio compression!
The amount of quantization is obviously related to how many bits
are allocated to the coding of a subband value.
Therefore the quantization process and bit allocation are joint activities
in the compression process.
This idea of dynamically adjusting the quantization
levels of the subband components to reduce or let more noise through
(decided per frequency band)
is sometimes referred to as a \fInoise shaping coding technique\fP
in some literature.
Noll claims that he and Zelinski first introduced dynamic
bit allocation techniques \cite{ noll97mpeg}
.BK noll MPEG digital audio coding
in the mid-1970s.
More advanced
audio coding schemes also build on this kind of idea.
.P
Finally the (apparently under) quantized frequency components
along with the information about the various degrees of quantization
and the scaling of each subband component
are now assembled
into a bit stream audio frame (further defined by the MPEG standards)
and is ready for input to a decoder.  The decoder uses the information
about how much scaling and quantization was applied to each subband component
to recreate the original subband signals and these are feed into a
corresponding synthesis filter in order to get the original
audio signal.
.P
Relatively early work done on transform encoding using psychoacoustics
for noise throttling
has been done
by Johnston \cite{ johnston88transform}.
.BK johnston transform coding audio signals using perceptual noise criteria
As an aside, Johnston has also done much in the area of speech compression
using transform and psychoacoustics techniques.
More information on psychoacoustics and human audio perception
can be found in the more recent works by Jayant, Johnston,
and Safranek \cite{ jayant93signal}
.BK jayant johnston safranek signal compression based models human perception
and by Wiese and Stoll \cite{ wiese90bitrate}
.BK wiese stoll bitrate reduction high quality audio signals modeling masking
\&.
.\"_
.\"_
.H 2 "Psychoacoustics"
.P
The MPEG audio compression schemes all make use of the
psychoacoustic properties of the human auditory system to
eliminate redundancy in the compression process.
This approach is often referred to as perceptual encoding since the
ultimate encoding of the original audio is one in which information
that cannot be perceived by a human listener has been removed.
MPEG Audio Compression
was the first major audio compression standard to use
psychoacoustics \cite{ stoll92codec }.
.\"_ stoll92codec
In non-psychoacoustic compression techniques,
the original audio signal
is compressed and encoded so that its waveform
can be reconstructed later as close to the original as possible.
In psychoacoustic based compression techniques, rather than
trying to reconstruct an audio signal's waveform, a waveform is
constructed that appears to \fBsound\fP the same as
the original audio signal to a human listener.  The constructed,
decoded, signal is perceived to sound the same as the original.
Since the decoded audio sounds the same as the original, the whole
scheme is often referred to as a transparent coding scheme.
The psychoacoustic properties of the human audio system
were first explored by Zwicker and Feldtkeller \cite{ zwicker67ohr}
.BK Zwicker Feldtkeller
as long ago as the late 1950s.
.P
The basic idea of using psychoacoustics is to identify
that part of a signal that can be ignored due to characteristics of the
signal itself.
More specifically, the MPEG compression schemes
make use of the fact that certain amounts of noise (generated
due to the quantization of the subband component signals coming out
of the polyphase filter) will be ignored by the auditory system.
There is an important effect that occurs in
the human auditory system which is taken advantage of by the
psychoacoustic compression techniques.
This is an \fIauditory masking\fP effect of some of the input
audio sound due to the presence of certain other elements in
the input sound.  This masking effect manifests itself in
several ways.
.P
The approximation model for the human ear
processes audio by bandpass filtering it
into what are called \fIcritical bands\fP \cite{ scharf70critical}
.BK scharf critical bands
\&.
There are twenty six of these critical bands and
these
bands are not uniform in frequency width.
They are also significantly overlapping with regard to
the frequency cutoffs of each band.
Bandwidths of these critical bands range from 50 to 100 Hz for
frequencies below 500 Hz and up to 5 kHz wide for
the higher frequencies. \cite{ noll97mpeg}
The psychoacoustic masking effect is manifested
in three different circumstances.
.P
Firstly, an input signal
to the auditory system
at a certain frequency
must be above
a certain intensity threshold in order to be perceived.
This threshold is a function of frequency and is referred to as the
\fIthreshold in quiet\fP (mentioned in the MPEG standard).
.P
Further, if there is a strong relatively narrowband
input signal at a certain
frequency (corresponding to auditory reception in one or more
critical bands), other signal components in frequencies adjacent
to the first are masked if they are below a threshold.
This phenomenon is referred to as \fIsimultaneous masking\fP
(or frequency masking).  It is assumed that this second signal
is present at the same time as the first.  The first signal is
called the \fImasker\fP and the second signal is the \fImaskee\fP.
This threshold is a function of the frequency of the masker as
well as whether the masker is tonal in its character or noise-like
in its character.  Noise-like signals tend to have a greater masking
effect than tonal-like signals.
The masking threshold is not
uniform around the masker (above and below its frequency).
This masking effect can serve to mask maskee signals that fall
in critical bands even several away from the one the masker signal is in.
There is a higher threshold
level on the higher frequency side of the masker than on its
lower frequency side.
.P
Finally, masker signals not only mask other signals that occur
at the same time as the masker but can also mask other signals
that occur before or after the masker signal.  This effect
is referred to as \fItemporal masking\fP.  Yes, you read that correctly.
Even signals that occur \fBbefore\fP a masker signal can be
masked by the masker.  How is this possible?  Isn't this a non-causal
effect from the masker signal?  The short answer is, yes (and not
really)!
Apparently, as sound is processed by the human brain, it undergoes
delays that can present an opportunity for a newly arriving sound
to interact with the previous sound.
All of this can transpire before the first sound is \fBperceived\fP
consciously.  When a signal (a maskee) is masked by a masker
which arrives later in time, this is referred to as \fIpremasking\fP.
Signals that are masked by a masker which occurred earlier in
time creates the condition referred to as \fIpostmasking\fP.
The time window for premasking, as might be expected, is rather small
and is in the range of usually 4 to 20 milliseconds.  The time
window for postmasking is usually 50 to 200 milliseconds.
.P
All of these masking effects are used in the psychoacoustic
model of an MPEG encoder to allow as much quantization noise and
signal distortion through to the decoder (and the final human)
as possible in order to minimize bits needed for subband component
quantization levels.  These masking effects are utilized in
a computation of the global masking threshold mentioned previously.
All masking effects are effectively computed in what is called
the \fPperceptual\fP domain of the input signal.  This domain
is the frequency domain of the critical bands of the auditory
system.  This domain is used because the range of masker signals
is defined more naturally in this frequency warped domain
than in the normal uniformly scaled frequency domain.
There are many details about the extent to which masker signals
affect their frequency and temporal neighborhood and the reader
is encouraged to reference the MPEG standards for more information
on the psychoacoustic computation of the global masking threshold.
Essentially, all masking effects are added (meaning there is more
masking available) whether related to frequency (simultaneous)
masking, temporal masking, or the quiet level masking.
The result is a function of frequency and time.
This threshold is computed for each input signal frame and is used
to compute the number of quantization levels used for
each of the subband signal components.
In all cases, a lower bound is used for the global masking threshold
if its computed value is too low on one subband or another.
.P
More information
on psychoacoustic models as applied to Layer I and Layer II
compression can be found in a paper
by Lanciani et al \cite{ lanciani97mpeg}
.BK lanciani schafer wang reibman psychoacoustically processing MPEG encoded
\&.
There are two psychoacoustic models proposed by the MPEG standard
but more are possible and encouraged.
Similar to the idea
of having multiple compression layers to suit different speed/quality
tradeoffs, more than one psychoacoustic model may be used.
The two models proposed in the original MPEG standard are presented
briefly next.
.\"_
.H 2 "MPEG Psychoacoustic Model 1"
.P
This model is the cheaper of the two in that it requires
less compute time to get some reasonable results for
use in the bit allocation and subband/transform component quantization
process.
This model analyses the input by taking a 1024 point Fast Fourier
Transform (FFT)
in the case of Layer II & III compression.  A 512 point FFT is
used for the Layer I compression scheme.
For all compression Layers, the
the input is multiplied by a Hanning window.
The input signal data is delayed in such a way so that the FFT
output is properly analyzed and applied for the correct subband (or transform)
data of the current input frame.
The analysis
as a result of this is used to perform bit allocation and quantization
on 1152 samples of subband data in the case of Layer II and
transform data in the case of Layer III.
An FFT is used on the input so that much better frequency resolution
is is available for searching for tonal components of the input
and noise-like components of the input.  Masking effects are computed,
non-trivially, from the frequency placement of the various components
found.  Finally a global masking threshold is computed for this
input signal frame.  As indicated earlier, this is used in the
subband or transform frequency signal quantization process.
.\"_
.H 2 "MPEG Psychoacoustic Model 2"
.P
This is a more complicated psychoacoustic model and it also
has special provisions for better qualify when used with Layer III
compression.  This model does not do the tonal/noise type of
analysis of model 1 but rather transforms the FFT spectral output
to something called a \fIpartition\fP domain.  This partition domain
is related almost linearly to the critical bands of the auditory system.
The auditory system model has twenty six critical bands where as
a partition domain of 64 bands is used.
This model also uses a 1024 point FFT on the input signal multiplied
by a Hanning window.
Again the input signal data is delayed properly
for correct
analysis to be applied to the subband (or transform)
data of the current frame.
.P
In this model, the tonality of the input signal is determined
by analyzing the \fIunpredictability\fP of the spectrum with time.
This is done by a using a linear extrapolation of the phase and
amplitude of the current frame spectrum with that of the two
previous frame spectrums.  The differences in phase and amplitude
are measured and this information ultimately is used to calculate
the global masking threshold in the partition domain.
The global masking threshold is now transformed back to the
normal frequency domain by simply spreading the threshold
from each Pariti on domain frequency band over each of the affected (covered)
normal domain frequency bands.  Again,
the global masking threshold is now used in the bit allocation
and quantization process of the frequency components (subband or transform).
.P
Finally, when this model is used in conjunction with Layer III compression,
several parameters of the model are changed to better fit the way
in which Layer III compression operates.  Further, this model can
interact with the Layer III pre-echo reduction logic to reduce
the effects of pre-echos in the output caused by large transients
in the input.  This is described more when Layer III is discussed next.
.\"_
.H 1 "Layer III Compression"
.P
This section will discuss the details of the Layer III compression scheme.
First an overview of the scheme is presented and then more details
of components of the scheme are presented in subsequent sections.
.\"_
.H 2 "Overview"
.P
As mentioned previously, this is the most
sophisticated of the original compatible MPEG
audio compression schemes and has many additional processing
steps than either Layer I or Layer II has.
This scheme makes use of the following additional (as compared with
Layers I & II) major techniques :
.BL
.LI
post-processing of the subband frequency components
into transform spectral components for better frequency
resolution
.LI
alias reduction of the frequency components
.LI
pre-echo reduction
.LI
nonuniform quantization of the frequency components
.LI
use scale factor bands to further reduce bit requirements
.LI
entropy encoding of the quantized samples
.LI
and, one of the more famous devices, the use of a bit reservoir
to better handle wild input signal changes
.LE
.sp 0.5
All of these additional techniques, over what is done in
Layers I and II, create better quality compressed audio
usually at a significantly lower bitrate also.
These will be discussed in more detail in the following sections.
.\"_
.H 2 "Frequency Transform Components and Alias Reduction"
.P
In Layer III compression (only), each of the 32 subbands is further
divided into (nominally) 18 frequency spectral lines by the application of
a Modified Discrete Cosine Transform (MDCT) to the 32 subband frequency signals.
The 32 subband signals are taken 36 samples (for the 18 spectral line
case being discussed) at a time and
multiplied by a windowing function.
Then the MDCT is applied.
This produces $ 18 times 32 $
spectral lines or 576 lines altogether.
The MDCT outputs are now also passed through an anti-aliasing
butterfly network (remember that the 32 subbands had overlapping bandpass
filters).  This alias reduction performed here is correctly reversed by the
matching Layer III decoder before passing the signal back through
its synthesis filters.  This is so that the total aliasing
from the input to the encoder's polyphase filter to the output
of the decoder's synthesis filter is properly minimized (approaches
zero net aliasing ideally).
.P
A MDCT can also be applied in this step to produce six spectral lines
per subband output component.  The MDCT producing 18 lines is
referred to as a \fIlong block\fP MDCT and the MDCT producing six lines
is referred to as a \fIshort block\fP MDCT.
When the short block MDCT is applied, the 32 subband signals are
taken 12 samples at a time and multiplied by a 12 sample window
function before the short block MDCT is applied.
When short block MDCTs are applied, $ 6 times 32 $ or 192 spectral
lines are produced at a time
(rather than the 576 as with long block MDCTs).
As mentioned already, a short block MDCT operates over 12 subband samples.
This period of 12 samples is sometimes referred to as a \fIgroup\fP.
Three of the short block MDCT periods obviously covers the
same amount of signal time and signal samples as one long block
MDCT period.  A series of three short 
block MDCTs ( $ 3 times 12 $ sample
groups) is referred to as a \fIsuper group\fP in some literature.
When the long block MDCT is used, each such application produces
a number of output line values (576 of them) referred to
as a \fIgranule\fP in some
literature.  For Layer III compression, two such
applications are applied (producing two granules
of spectral line outputs or 1152 sample values) to
cover the Layer III frame size of 1152 spectral sample values.
.P
Adjusting of the MDCT block sizes can be used (with other methods)
to dynamically adjust for
pre-echo effects in the audio signal.
Since the MDCT usually provides
better frequency resolution (with 18 spectral lines per subband output),
it correspondingly provides worse time domain resolution
(frequency/time uncertainty principle).
The quantization effects of the MDCT values will be spread out in
time more for the long block MDCT case.  This can cause pre-echos
in the output signal when the input undergoes some rapid change.
To avoid these pre-echos, the compression
algorithm can switch between using long block MDCTs to short block
MDCTs (to get back some time domain resolution) for reducing pre-echo
effects for transient-like input considerations.
Other ways to mitigate the pre-echo effects are briefly discussed
in the next section.
The choice of using either short block MDCTs or long block MDCTs is
optionally made by the Psychoacoustic Model 2 described previously.
.P
Switching between long block MDCTs and short block MDCTs does not
happen immediately.  Instead a special transition window function
is used as an intermediate step.  There is a special window function
for going from a long block to a short block MDCT as well as
a special window function for going from a short block MDCT back
to a long block MDCT.  Therefore there are four MDCT window
functions altogether, one each for short or long MDCTs and one
for each type of transition possible.  Using the special transition
window functions reduces the transient effects of switching between
the two MDCT lengths.  For reference purposes, the various
windows are assigned names according to the MPEG standard.
The long block MDCT window is a \fItype 0\fP window.
The short block MDCT window is a \fItype 2\fP window.
The long to short block transition MDCT window is a \fItype 1\fP window.
The short to long block transition MDCT window is a \fItype 3\fP window.
.P
Finally for any given frame processed all of the subband
components can be processed with short block MDCTs or
long block MDCTs.  In addition there is a mixed mode variation
available also.  In the mixed mode situation, the two lowest frequency
subbands get processed with long block MDCTs (giving better frequency
resolution -- where its needed with the low frequencies)
and the upper 30 subbands get processed with short block MDCTs.
This provides some additional flexibility for a frequency/time
tradeoff as compared with the other two MDCT processing modes.
.\"_
.H 2 "Pre-echo Reduction Management"
.P
Psychoacoustic Model 2 has the provision for adjusting
its output so that pre-echo effects are reduced by calling
for more quantization around a pre-echo effect to reduce
the associated quantization noise.
As previously discussed, the Psychoacoustic Model 2 can also
switch from using long block MDCTs to short MDCTs to mitigate
any pre-echo effects also.
Finally, if necessary, more bits can be allocated to quantizing
around a pre-echo effect by taking bits out of the bit reservoir.
The bit reservoir is discussed more in later sections.
.\"_
.H 2 "Non-uniform Quantization"
.P
The transform spectral line values are now raised to
the $ 3 over 4 $ power
before quantization is applied.  This creates a more uniform
signal to noise ratio over the range of quantized values.
Like the anti-aliasing above, this step is reversed in the matching
Layer III decoder after the full values are formulated
from the quantized values and before going through the corresponding
inverse modified discrete cosine transform (IMDCT) in the decoder.
The values are returned to linear ones in the decoder by simply raising
each value to the $ 4 over 3 $ power.
.\"_
.H 2 "Scale-factor Band Processing And Bit Allocation"
.P
Unlike Layers I and II, Layer III can group transform line values
together to share a common scale factor.  This leads to the potential
for greater thriftiness in the uses of bit for transmitting
the scale values for all transform line values to the eventual decoder.
.P
The process of allocating bits is obviously tied with scale factor
adjustment and the resulting quantization of the transform spectral line
values.
The global masking threshold function calculated by the
psychoacoustic model is used to create a signal to masking ratio (SMR).
Now an iterative \fIanalysis-by-synthesis\fP algorithm is
used to assign quantization
levels (or quantization bits) to each transform spectral line value.
A Huffman encoding of the resulting quantized values is done
and this adds complexity to the quantization process since the number
of bits needed for a particular spectral line value is also dependent
on the resulting Huffman code assigned to the value.
.P
A doubly nested iteration loop is used to find the resulting
Signal to Noise Ratio (SNR) for each conjectured amount of quantization
of a spectral line value.  A Noise to Masking Ration (NMR) is now just
the difference between the SNR and the SMR.  A negative NMR means that
the noise is below the perception level of the listener and this
is desirable.
The process starts by assigning no bits to any spectral
line component and initializing all of the scale factors
to their minimums.  In each iteration, the outer loop adjusts the scale
factors upward slightly
and one bit is allocated for the
quantization of a spectral component.
The inner loop adjusts a global scale value (applied to all spectral line
components) and checks if the number of bits available for quantization
has expired.
This process repeats until a component either
has the maximum scale value, the maximum number of quantization bits
allowed, or until all spectral line components are quantized such
that their NMR rations are just negative.
If the process stops with extra bits available for quantization
that were not used, these bits may possibly be added to the bit reservoir
discussed later.  If more bits are needed for quantization
and there are none left in the current frame's allocation, some bits
may be taken from the bit reservoir if there are any there to start with.
.\"_
.H 2 "Entropy Encoding with Huffman Codes"
.P
As mentioned previously, the spectral line component values
are quantized according to input (the global masking threshold)
calculated from the psychoacoustic model.  These quantized
values are encoded with one of three possible different Huffman codes
per frame.  The three Huffman codes used per frame are taken from
a palette of 32 different Huffman code tables available.
This procedure is used so that the maximum amount of coding
efficiency can be used with regard to the number of bits
needed for the coding of all quantized spectral component sample values.
Of course, which of the 32 Huffman codes were used (maximum of three),
is also transmitted with the quantized values (as part of the "side"
information) so that the decoder can apply the proper Huffman decoding
to the quantized bit data.
.\"_
.H 2 "Bit Reservoir"
.P
One of the most famous techniques employed in the Layer III compression
is the use of a bit reservoir for storing unused bits.
These bits are produced (or put into the reservoir) when the number of
bits needed to achieve a zero Noise-to-Masking Ratio (NMR)
in the spectral value quantization step is less than the number of
bits available for quantization bit allocation in a particular frame.
These bits can be used (or taken out of the reservoir) when
more than the number of bits available in a frame is not enough to achieve
total noise or distortion masking (the zero NMR).
.P
The number of reservoir bits is not allowed to grow without bound.
After a certain maximum, bits are wasted by sending some padding bits
in the current frame to the decoder thus using them up.
The use of these bits allows for rapid transients in the input audio
to be properly quantized when these transients may have otherwise had to
appear as noise to the end listener.
Although much of the Layer III compression scheme is impressive,
the use of this bit reservoir has gained much attention.
This attention may not be warranted based on the relative sophistication
of the overall Layer III compression scheme but it is much talked
about none the less.  This facility mimics (somewhat) a generic variable
bit encoding scheme, which has received great attention since about
1991 or 1992 for compressed audio and video transmission.
This may be why this bit reservoir is referenced in the literature
as a major advanced feature of Layer III compression.
.\"_
.H 2 "Assembly Into The Bitstream Audio Frame"
.P
Once an entire frame of input audio data has been analyzed and
quantized, it is assembled into a frame and sequenced into a
multimedia bit stream.  The type of bit stream used
is either one defined by the MPEG I standard or the MPEG II standard
and this is determined by
the original requirements given to the encoder.
These MPEG bit steams can also contain video data or ancillary data
frames.  These other types of frames are outside the scope of this paper.
Details of the exact audio frame format is not presented here (the interested
reader is referred to the MPEG audio standards) but the data
that must be transmitted in each frame will now be summarized.
The following data (among other information) is sent in each
assembled audio frame.
.BL
.LI
The quantized transform spectral line component values
are obviously transmitted to the decoder.
Remember that these values are Huffman encoded also.
.LI
The scale factors used both over the spectral lines of
a single granule or across two granules is send.
These are needed to reconstruct the quantized spectral line components.
.LI
Information on the bit sizes of the scale factor quantities
is also transmitted
since there can be scale factors with different sizes used
in a single frame.
.LI
Other information bits specifying the various parameters
and options used in the compression process are also transmitted.
Some of these include the types of MDCTs used in the frame (short, long,
or mixed) and the specific Huffman code tables (up to 3 out of 32)
that were used in the encoding of the quantizations of the transform
line values.
.LI
An optional frame header CRC or 16 bits is possibly transmitted
if desired.  This would most likely be used over channels
that which have no underlying error correction.
.LI
Each frame (frame header) contains a leading synchronization bit
sequence (all ones) to allow the receiver to synchronize to the
start of a frame.  Also a frame identification is transmitted
in the header also specifying the type of frame that this
data represents.  Remember that video or ancillary data frame
can also be present in multimedia applications.
The frame bit format was chosen so as to minimize the likelihood
of the synchronization pattern showing up in positions of the frame
other than its front.
.LE
.sp 0.5
Finally, it should be noted that each frame is self contained in the
sense that it contains all of the information needed
within itself
to be decoded properly.
.\"_
.H 1 "Implementation Alternatives and Enhancements"
.P
The MPEG standards have been designed, as many good
standards are, to be purposely vague
in specific implementation details
(I don't mean this factitiously).
This is done to leave latitude
so that new or enhanced building blocks may be used to realize
a standardized bit stream more cheaply or more quickly in the future.
Some of these implementation alternatives or improvements
are now discussed.
.P
Since a psychoacoustic model needs access to the
spectral content of the input audio signal and the polyphase filter
bank is needed to get the proper subband spectral components,
there is some computational overlap in performing these two steps
independently.
A hybrid filter bank is proposed by Liu and Lee \cite{ liu97design }
.BK liu lee design hybrid filter bank psychoacoustically mode MPEG
to reduce the amount of overall computations needed in possibly
both an encoder and a decoder.
.P
Kumar and Zubair \cite{ kumar96mpeg }
.BK Kumar Zubair high performance software implementation MPEG audio encoder
(at IBM) have shown how to organize and
manage a software implementation of MPEG Layer III compression
for very substantial speed gains (a factor or 10 or more).
There work has focused on both reducing the computational complexity
of the polyphase subband filter and also on dramatically
reducing the compute time needed to perform the bit allocation
calculations using Huffman encoding.  They store data, to facilitate
a very rapid Huffman encoding procedure, in a heap data structure.
.P
The problem of poor computational efficiency in the polyphase
subband filter has also been
addressed by Konstantinides \cite{ konstantinides94mpeg }
.BK Konstantinides fast subband filtering MPEG audio coding
in his paper on the subject as long ago as 1994.
.P
The MDCT processing of Layer III compression has also been targeted
for improvements and Chiang and Liu \cite{ chiang96mpeg }
.BK chiang liu regressive implementations forward inverse MDCT MPEG audio
have addressed this part of the compression scheme by implementing
a regressive structure for performing both the forward MDCTs and
the inverse MDCTs (used in the decoder).
.P
Novel and efficient MPEG audio decoders have been proposed
by Tsai, et al \cite{ tsai97implementation }
.BK Tsai chen ibrahim pirsch mccanny implementation strategy audio
to handle multichannel applications in parallel,
and by Tsai, and others beside those in the previous paper, for how to handle
multichannel data using techniques of intelligent data management
in the processing algorithms.
Improving the performance of software MPEG decoders
has also been worked on by Zucker, Flynn, and Lee \cite{ zucker96mpeg }
.BK zucker flynn lee improving performance MPEG software players
\&.
.P
Hans and Bhaskaran \cite{ hans97mpeg }
.BK hans bhaskaran compliant MPEG decoder arithmetic operations
have shown how to implement an MPEG decoder when only 16 bit
fixed point arithmetic is available.
This is obviously aimed at a low cost consumer market and
efforts like this has kept fixed point arithmetic in even some of the newest
families of DSP even though more and more DSP applications can be
better (and usually much more simply) handled by floating point.
.P
Since AC-3 compression (from Dolby) has gained acceptance (undoubtedly
due to its inclusion in HDTV), many have worked on making
MPEG audio compression hardware and the required AC-3 compression
hardware as similar as possible in order to make low cost
designs that handle both types of compression.
Both the work of Lau and Chwu \cite{ lau97mpeg }
.BK lau chwu common transform engine MPEG audio decoder
and also of Jhung and Park \cite{ jhung97mpeg }
.BK jhung park architecture dual mode audio filter MPEG
have achieved this with different implementations.
.P
Tsai, Chen, et al \cite{ tsai95mpeg }
.BK tsai chen MPEG audio decoder chip
also published their work an a novel fast
hardware MPEG decoder integrated circuit (IC),
a while back, where they were able to
reduce both the computation time and the storage requirements
by half as compared with previous decoders.
.\"_
.\"_
.H 1 "Conclusions"
.P
I trust that the reader has enjoyed this little whirlwind tour through
MPEG audio compression in general and Layer III compression in
particular.  There are obviously many details that were not
covered in this paper and there have not been any single papers
(to my knowledge)
that have covered every aspect of the standard, except for the standards
documents themselves.
Obviously, a great amount of effort has gone into the definition of this
standard.  Consider the video part of the MPEG II standard
which, although is not perfect in the opinion of the author
and others who were involved in its formulation,
is nonetheless a masterpiece of workmanship that utilized some of the best
engineering talent that was present in the world in the late 1980s
and the early 1990s.
The audio part of MPEG is likewise a very respectable work which has
brought together many ideas in digital signal processing,
and digital audio processing in particular.
Many of these ideas had been explored and experimented with
for parts of the 1980s and earlier in some cases.
.P
This paper has presented some background on MPEG and Layer III compression
and its possible importance in consumer applications possibly
yet to be fully determined.
An overview of MPEG audio compression in general has been presented
along with a more detailed discussion of MPEG Layer III Audio compression.
Further, some alternative implementations and enhancement ideas
for MPEG audio have also been presented.
.\"_
.\"_
.SK
.TC
.\"_
.\"_
