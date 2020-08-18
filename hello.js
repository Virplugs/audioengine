var addon = require('bindings')('virplugs-audioengine');

console.log(addon.hello()); // 'world'

console.log(addon.printDeviceInfo());
