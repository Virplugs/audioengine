export function getDeviceInfo(): any;
export function openAsioControlPanel(): any;
export function readAudioFileInfo(filename: string, callback: (err, info) => void): any;
export function readAudioFileWaveform(filename: string, window: number, callback: (err, data) => void): any;
export function requestLatencyInfo(): { input: number, output: number, total: number, samplerate: number };
export function setEventsCallback(callback: (name: string, data: any) => void): any;
export function startAudioEngine(api: string, inputDeviceId: number, outputDeviceId: number, samplerate: number, bufferFrames: number): boolean;
export function stopAudioEngine(): boolean;
