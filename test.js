var addon = require('bindings')('virplugs-audioengine');

console.dir(addon, { depth: null });
//console.dir(addon.getDeviceInfo(), { depth: null });

console.dir(addon.readAudioFileInfo("drumLoop.wav"));
console.dir(addon.readAudioFileWaveform("drumLoop.wav", 128));
