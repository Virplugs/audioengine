var addon = require('bindings')('virplugs-audioengine');

console.dir(addon, { depth: null });

console.dir(addon.getDeviceInfo(), { depth: null });

try {
	console.dir(addon.readAudioFileInfo("drumLookp.wav", function(err, info) {
		console.log(err, info);
	}));
} catch(e) {
	console.log("hio");
	console.error("errrr");
}
//console.dir(addon.readAudioFileWaveform("drumLoop.wav", 128));
