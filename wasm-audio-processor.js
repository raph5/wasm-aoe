
// If you change AL_RING_BUFFER_CAPACITY you need change it also in index.html
const AL_RING_BUFFER_CAPACITY = 65;

function al_ring_buffer_read(ring_buffer, first_channel, second_channel) {
  const cap = AL_RING_BUFFER_CAPACITY;

  const head = new Uint32Array(ring_buffer, 0, 2);
  const body = new Float32Array(ring_buffer, 8, 256 * cap);
  const start = Atomics.load(head, 0);
  const end = Atomics.load(head, 1);

  if (start != end) {
    const first_chunk = body.subarray(256 * start + 0, 256 * start + 128);
    const second_chunk = body.subarray(256 * start + 128, 256 * start + 256);
    first_channel.set(first_chunk);
    second_channel.set(second_chunk);
    Atomics.store(head, 0, (start + 1) % cap);
  }
}

class WasmAudioProcessor extends AudioWorkletProcessor {
  constructor() {
    super();
    this.initialized = false;
    this.ring_buffer = null;
    this.port.onmessage = this.init.bind(this);
  }

  init(message) {
    this.ring_buffer = message.data;
    this.initialized = true;
  }

  process(inputs, outputs, parameters) {
    if (this.initialized) {
      al_ring_buffer_read(this.ring_buffer, outputs[0][0], outputs[0][1]);
    }
    return true;
  }
}

registerProcessor('wasm-audio-processor', WasmAudioProcessor);
