class HttpParserError extends Error {
    constructor(msg) {
        super(msg);
    }
}

const kNewLine = '\n';
const kActionsToCall = Object.freeze({
    'REQUEST_HEAD': 'REQUEST_HEAD',
    'RESPONSE_HEAD': 'RESPONSE_HEAD',
    'HEADER': 'HEADER',
    'BODY_SIZED': 'BODY_SIZED',
    'BODY_RAW': 'BODY_RAW',
    'BODY_CHUNK_HEAD': 'BODY_CHUNK_HEAD',
    'BODY_CHUNK': 'BODY_CHUNK',
});

export const Kind = Object.freeze({
    'REQUEST': 'REQUEST',
    'RESPONSE': 'RESPONSE'
});

export default class HttpParser {
    /**
     * Size of whole content to parse.
     * @type {number}
     */
    #end = 0;

    /**
     * Offset of already parsed content.
     * @type {number}
     */
    #offset = 0;

    /**
     * Chunk (or whole) data to parse.
     * @type {string[]}
     */
    #chunk = '';

    /**
     * What next parser should be called.
     * @type { keyof kActionsToCall }
     */
    #nextActionToCall;

    #parsingResults = Object.seal({
        url: '',
        versionMajor: 0,
        versionMinor: 0,
        headers: [],
        statusCode: 0,
        statusMsg: '',
        hasBodyChunked: false,
        hasContentLength: false,
        contentLength: 0,
        hasChunkLength: false,
        chunkLength: 0,
        hasBody: false,
        body: ''
    });

    /**
     * @param {keyof Kind} kind 
     */
    constructor(kind) {
        if (kind === Kind.REQUEST) {
            this.#nextActionToCall = kActionsToCall.REQUEST_HEAD
        } else if (kind === Kind.RESPONSE) {
            this.#nextActionToCall = kActionsToCall.RESPONSE_HEAD
        } else {
            throw new HttpParserError('"kind" argument must have one of "Kind" object value.')
        }
    }

    /**
     * Runs parser for chunk of data.
     * @param {string} chunk 
     */
    parse(chunk) {
        if (!chunk) {
            throw new HttpParserError('chunk to parse can not be empty.')
        }

        this.#chunk += chunk;
        this.#end += chunk.length;

        while (this.#offset < this.#end) {
            this.#callFragmentParser();
        }

        if (this.#parsingResults.hasBodyChunked && this.#parsingResults.hasBody) {
            // PS1. when body is chunked the `this.body` was not resolved yet
            //      so it's must be resolved now
            // PS2. calling in next microtask to not interrupt parser
            Promise.resolve().then(() => this.#bodyRes(this.#parsingResults.body));
        }
    }

    #statusCodeRes;
    #statusCodeRej;
    statusCode = new Promise((res, rej) => {
        this.#statusCodeRes = res;
        this.#statusCodeRej = rej;
    });

    #bodyRes;
    #bodyRej;
    body = new Promise((res, rej) => {
        this.#bodyRes = res;
        this.#bodyRej = rej;
    });

    #callFragmentParser() {
        switch (this.#nextActionToCall) {
            case kActionsToCall.RESPONSE_HEAD:
                this.#parseResponseHead();
                break;
            case kActionsToCall.REQUEST_HEAD:
                this.#parseRequestHead();
                break;
            case kActionsToCall.HEADER:
                this.#parseHeader();
                break;
            case kActionsToCall.BODY_RAW:
                this.#parseBodyRaw();
                break;
            case kActionsToCall.BODY_SIZED:
                this.#parseBodySized();
                break;
            case kActionsToCall.BODY_CHUNK_HEAD:
                this.#parseBodyChunkedHead();
                break;
            case kActionsToCall.BODY_CHUNK:
                this.#parseBodyChunked();
                break;
            default:
                this.#throwNoParserError();
        }
    }

    #throwNoParserError() {
        throw new HttpParser(`Internal error of parser - no action to run was found on new line of chunk. Called action "${this.#nextActionToCall}".`)
    }

    /**
     * @param {string} line 
     */
    #readLine() {
        const nextNewLineIndex = this.#chunk.indexOf(kNewLine, this.#offset);
        let line;
        if (nextNewLineIndex > 0) {
            line = this.#chunk.slice(this.#offset, nextNewLineIndex);

            if (line.charAt(line.length - 1) === '\r') {
                // if '/r' stays at the end regex will not match
                line = line.slice(0, line.length - 1);
            }

            this.#offset = nextNewLineIndex + 1;
        } else {
            line = this.#chunk.slice(this.#offset);
            this.#offset = this.#end;
        }

        return line;
    }

    #parseRequestHead() {
        throw new Error('Not implemented yet.');
    }

    static #responseHeadRegex = /^HTTP\/(\d)\.(\d) (\d{3}) ?(.*)$/;
    #parseResponseHead() {
        const line = this.#readLine();
        if (!line) {
            return;
        }
        const match = HttpParser.#responseHeadRegex.exec(line);
        if (!match) {
            throw new HttpParserError(`No regex match when parsing response head for line "${line}"`);
        }
        this.#parsingResults.versionMajor = Number(match[1]);
        this.#parsingResults.versionMinor = Number(match[2]);
        this.#parsingResults.statusCode = Number(match[3]);
        Promise.resolve().then(() => this.#statusCodeRes(this.#parsingResults.statusCode));
        this.#parsingResults.statusMsg = match[4];

        this.#nextActionToCall = kActionsToCall.HEADER;
    }

    static #headerRegex = /^([^: \t]+):[ \t]*((?:.*[^ \t])|)/;
    #parseHeader() {
        const line = this.#readLine();
        if (line) {
            const match = HttpParser.#headerRegex.exec(line);
            this.#parsingResults.headers.push(match[1], match[2]);
        } else {
            /**
             * There is a couple options that body can be send through HTTP:
             * 
             * 1. Chunked - when there is "Transfer-Encoding: chunked".
             * Size of an body chunk is defined by number that is written in hex
             * system in the first line of the chunk. 
             * This should be recalled for each chunk.
             * (BODY_CHUNKHEAD => BODY_CHUNKTRAILERS | BODY_CHUNK => BODY_CHUNKTRAILERS).
             * 
             * 2. Content-Length: value.
             * Where content length is explicity set in header. 
             * (BODY_SIZED)
             * 
             * 3. No indication about size.
             * This just simple read to the end of available content.
             * (BODY_RAW)
             */
            const headers = this.#parsingResults.headers
            for (let i = 0; i < headers.length; i += 2) {
                const headerName = headers[i].toLowerCase();
                const headerValue = headers[i + 1].toLowerCase();
                switch (headerName) {
                    case 'transfer-encoding':
                        this.#parsingResults.hasBodyChunked = headerValue === 'chunked';
                        break;
                    case 'content-length':
                        this.#parsingResults.contentLength = Number(headerValue);
                        this.#parsingResults.hasContentLength = true;
                        break;
                    case 'connection':
                        throw new Error('Connection header support not implemented.');
                    case 'upgrade':
                        throw new Error('Upgrade header support not implemented.');
                }
            }

            if (this.#parsingResults.hasBodyChunked) {
                this.#nextActionToCall = kActionsToCall.BODY_CHUNK_HEAD;
            } else if (this.#parsingResults.hasContentLength) {
                this.#nextActionToCall = kActionsToCall.BODY_SIZED;
            } else {
                this.#nextActionToCall = kActionsToCall.BODY_RAW;
            }
        }
    }

    #parseBodyRaw() {
        this.#parsingResults.body = this.#chunk.slice(this.#offset, this.#end);
        this.#offset = this.#end;
        Promise.resolve().then(() => this.#bodyRes(this.#parsingResults.body));
    }

    #parseBodySized() {
        const bodyEnd = this.#offset + this.#parsingResults.contentLength;
        this.#parsingResults.body = this.#chunk.slice(this.#offset, bodyEnd);
        this.#offset = bodyEnd;
        Promise.resolve().then(() => this.#bodyRes(this.#parsingResults.body));
    }

    #parseBodyChunkedHead() {
        const line = this.#readLine();
        if (!line) {
            return;
        }

        this.#parsingResults.chunkLength = parseInt(line, 16);
        this.#parsingResults.hasChunkLength = true;
        this.#nextActionToCall = kActionsToCall.BODY_CHUNK;
    }

    #parseBodyChunked() {
        if (!this.#parsingResults.hasChunkLength) {
            return;
        }

        const bodyEnd = this.#offset + this.#parsingResults.chunkLength;
        this.#parsingResults.body += this.#chunk.slice(this.#offset, bodyEnd);
        this.#offset = bodyEnd;

        this.#nextActionToCall = kActionsToCall.BODY_CHUNK_HEAD;
        this.#parsingResults.hasBody = true;
    }
}
