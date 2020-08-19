var addon = require('bindings')('virplugs-audioengine');

console.dir(addon, { depth: null });
//console.dir(addon.getDeviceInfo(), { depth: null });

console.dir(addon.readAudioFileInfo("d:\\drumLoop.wav"));
console.dir(addon.readAudioFileWaveform("d:\\drumLoop.wav", 128));
console.dir(addon.readAudioFileWaveform("d:\\drumLoop.wav", 512)[0].length);
