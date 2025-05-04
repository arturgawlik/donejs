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
    'BODY_CHUNK_HEAD': 'BODY_CHUNK_HEAD',
    'BODY_SIZED': 'BODY_SIZED',
    'BODY_RAW': 'BODY_RAW'
});
const kNeedsNewLine = Object.freeze([
    kActionsToCall.REQUEST_HEAD, kActionsToCall.RESPONSE_HEAD, kActionsToCall.HEADER, kActionsToCall.BODY_CHUNK_HEAD
]);

export const Kind = Object.freeze({
    'REQUEST': 'REQUEST',
    'RESPONSE': 'RESPONSE'
});

export class HttpParser {
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
     * Kind of HTTP stuff to parse.
     * @type {'REQUEST' | 'RESPONSE'}
     */
    #kind;

    /**
     * What next parser should be called.
     * @type { keyof kActionsToCall }
     */
    #nextActionToCall;

    /**
     * @type { {
     *  url: string | null,
     *  versionMajor: number | null,
     *  versionMinor: number | null,
     *  headers: string[],
     *  statusCode: number | null,
     *  statusMsg: string | null,
     *  hasBodyChunked: boolean,
     *  hasContentLength: boolean,
     *  contentLength: number,
     *  body: string,
     * } }
     */
    #parsingResults = Object.seal({
        url: null,
        versionMajor: null,
        versionMinor: null,
        headers: [],
        statusCode: null,
        statusMsg: null,
        hasBodyChunked: false,
        hasContentLength: false,
        contentLength: 0,
        body: ''
    });

    /**
     * 
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

        this.#kind = kind;
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
            if (kNeedsNewLine.includes(this.#nextActionToCall)) {
                const line = this.#readLine();
                this.#parseLine(line);
            } else {
                this.#parseWithoutLine();
            }
        }
    }

    /**
     * Calls next parser
     */
    #parseWithoutLine() {
        switch (this.#nextActionToCall) {
            case kActionsToCall.BODY_RAW:
                this.#parseBodyRaw();
                break;
            case kActionsToCall.BODY_SIZED:
                this.#parseBodySized();
                break;
            default:
                this.#throwNoParserError();
        }
    }

    /**
     * Calls next parser with line
     * @param {string} line 
     */
    #parseLine(line) {
        switch (this.#nextActionToCall) {
            case kActionsToCall.RESPONSE_HEAD:
                this.#parseResponseHead(line);
                break;
            case kActionsToCall.REQUEST_HEAD:
                this.#parseRequestHead(line);
                break;
            case kActionsToCall.HEADER:
                this.#parseHeader(line);
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
            this.#offset = nextNewLineIndex + 1;
        } else {
            line = this.#chunk.slice(this.#offset);
            this.#offset = this.#end;
        }

        return line;
    }

    /**
     * @param {string} line 
     */
    #parseRequestHead() {
        throw new Error('Not implemented yet.');
    }

    static #responseHeadRegex = /^HTTP\/(\d)\.(\d) (\d{3}) ?(.*)$/;
    /**
     * @param {string} line 
     */
    #parseResponseHead(line) {
        if (!line) {
            return;
        }
        const match = HttpParser.#responseHeadRegex.exec(line);
        this.#parsingResults.versionMajor = Number(match[1]);
        this.#parsingResults.versionMinor = Number(match[2]);
        this.#parsingResults.statusCode = match[3];
        this.#parsingResults.statusMsg = match[4];

        this.#nextActionToCall = kActionsToCall.HEADER;
    }

    static #headerRegex = /^([^: \t]+):[ \t]*((?:.*[^ \t])|)/;
    /**
     * @param {string} line 
     */
    #parseHeader(line) {
        if (line) {
            const match = HttpParser.#headerRegex.exec(line);
            this.#parsingResults.headers.push(match[1], match[2]);
        } else {
            /**
             * There is a couple option that body can be send through HTTP:
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
    }

    #parseBodySized() {
        const bodyEnd = this.#offset + this.#parsingResults.contentLength
        this.#parsingResults.body = this.#chunk.slice(this.#offset, bodyEnd);
        this.#offset = bodyEnd;
    }
}
