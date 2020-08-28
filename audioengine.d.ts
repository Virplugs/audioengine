export function getDeviceInfo(): any;
export function openAsioControlPanel(): any;
export function readAudioFileInfo(filename: string, callback: (err, info) => void): any;
export function readAudioFileWaveform(filename: string, window: number, callback: (err, data) => void): any;
export function requestLatencyInfo(): { input: number, output: number, total: number, samplerate: number };
export function setEventsCallback(callback: (name: string, data: any) => void): any;
export function startAudioEngine(api: string, inputDeviceId: number, outputDeviceId: number, samplerate: number, bufferFrames: number): boolean;
export function stopAudioEngine(): boolean;

export function setActiveTransport(transport: NativeTransport);
declare class NativeTransport {
	constructor();
	bpm: number;
	readonly masterTrack: NativeTrack;
	readonly cueTrack: NativeTrack;
}

declare class NativeTrack {
	name: string;
	inputChannels: number[];
	outputChannels: number[];
	constructor(name: string, inputChannels: number[], outputChannels: number[]);
	playAudioEvent(event: NativeAudioEvent);
	stopAudioEvent(event: NativeAudioEvent);
	readonly subTracks: NativeTrack[];
	addSubTrack(track: NativeTrack, index?: Number);
	removeSubTrack(track: NativeTrack);
	readonly levels: number[];
	volume: number;
}

declare class NativeAudioEvent {
	name: string;
	readonly lastFrameOffset: number;
	readonly duration: number;
	readonly totalFrames: number;
	constructor(name: string, filename: string);
}
