
const { textDecoder } = globalThis.done;

const kUtf8Encodings = ['utf-8', 'utf8'];
const kAllSupportedEncodings = [...kUtf8Encodings];

class TextDecoder {
    #encoding;

    constructor(encoding = 'utf-8') {
        if (!kAllSupportedEncodings.includes(encoding)) {
            throw new Error(`Currently ${encoding} is not supported.`);
        }
        this.#encoding = encoding;
    }

    decode(buffer) {
        if (kUtf8Encodings.includes(this.#encoding)) {
            return textDecoder.encode(buffer, this.#encoding);
        }
    }

}


export default TextDecoder;
