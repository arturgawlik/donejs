const { textEncoder } = globalThis.done;

class TextEncoder {
    encode(string) {
        return textEncoder.encode(string);
    }

}

export default TextEncoder;
