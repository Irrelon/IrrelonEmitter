#include <Irrelon/DynaVal.h>
#include "Irrelon/Emitter.h"

void setup() {
	Irrelon::Emitter emitter;
	emitter.on("event", [](const Irrelon::DynaVal& args) {

	});

	emitter.emit("event");
}

void loop () {

}