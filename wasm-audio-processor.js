class WasmAudioProcessor extends AudioWorkletProcessor {
  process(inputs, outputs, parameters) {
    const output = outputs[0];
    for (let channel = 0; channel < output.length; ++channel) {
      const outputChannel = output[channel];
      for (let i = 0; i < outputChannel.length; ++i) {
        // Pull PCM from WASM memory
        outputChannel[i] = wasmModule.getNextSample(channel); 
      }
    }
    return true;
  }
}
registerProcessor('wasm-audio-processor', WasmAudioProcessor);
