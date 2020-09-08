type WaveformOverview = Array<{
	max: number;
	min: number;
}>;

type DeviceInfo = Record<string, {
	displayName: string;
	numDevices: number;
	devices: Array<{
		name: string;
		outputChannels: number;
		inputChannels: number;
		duplexChannels: number;
		isDefaultInput: boolean;
		isDefaultOutput: boolean;
		preferredSampleRate: number;
		sampleRates: number[];
		nativeFormats: Array<"SINT8" | "SINT16" | "SINT24" | "SINT32" | "FLOAT32" | "FLOAT64">;
	}>
}>;

type FileInfo = {
	filename: string;
	channels: number;
	samplerate: number;
	frames: number;
	sections: number;
	seekable: boolean;
	format: number;
};

type TimingInfo = {
	projectTimeSamples: number;
	projectTimeMusic: number;
	projectTimeMusicBars: number;
	barPositionMusic: number;
	tempo: number;
	timeSigNumerator: number;
	timeSigDenominator: number;
	samplerate: number;
};

export function getDeviceInfo(): DeviceInfo;
export function openAsioControlPanel(): void;
export function readAudioFileInfo(filename: string, callback: (err: any, info: FileInfo) => void): void;
export function readAudioFileWaveform(filename: string, window: number, callback: (err: any, data: WaveformOverview[]) => void): void;
export function requestLatencyInfo(): { input: number, output: number, total: number, samplerate: number };
export function setEventsCallback(callback: (name: string, data: any) => void): any;
export function startAudioEngine(api: string, inputDeviceId: number, outputDeviceId: number, samplerate: number, bufferFrames: number): boolean;
export function stopAudioEngine(): boolean;

export function setActiveTransport(transport: NativeTransport);
declare class NativeTransport {
	constructor();
	bpm: number;
	masterTrack: NativeTrack;
	cueTrack: NativeTrack;
	readonly isPlaying: boolean;
	readonly timingInfo: TimingInfo;
	readonly totalRuntimeSamples: number;
	start();
	stop();
	setTimeInSamples(time: number);
}

declare class NativeTrack {
	name: string;
	inputChannels: number[];
	outputChannels: number[];
	constructor(name: string, inputChannels: number[], outputChannels: number[]);
	playAudioEvent(event: NativeAudioEvent, time?: number);
	stopAudioEvent(event: NativeAudioEvent);
	readonly subTracks: NativeTrack[];
	addSubTrack(track: NativeTrack, index?: number);
	removeSubTrack(track: NativeTrack);
	readonly levels: number[];
	volume: number;
}

declare class NativeAudioEvent {
	name: string;
	readonly lastFrameOffset: number;
	readonly duration: number;
	readonly totalFrames: number;
	gain: number;
	constructor(name: string, filename: string);
}
