# Irrelon Emitter
A C++ library for creating and using an event emitter.

## Usage
```c++
#include <IrrelonEmitter.h>

Irrelon::Emitter emitter;
emitter.on("myEventName", [](DynaVal args){
    // do something, like read args[0]
});

DynaVal args;
args.becomeArray();
args.push("Hello");
emitter.emit("myEventName", args);
```