class HttpParserError extends Error {
    constructor(msg) {
        super(msg);
    }
}

const kNewLine = '\n';
const kActionsToCall = Object.freeze({
    'REQUEST_HEAD': 'REQUEST_HEAD',
    'RESPONSE_HEAD': 'RESPONSE_HEAD',
    'HEADER': 'HEADER'
});

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

    #parsingResults = Object.seal({
        url: null,
        versionMajor: null,
        versionMinor: null,
        headers: [],
        statusCode: null,
        statusMsg: null
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
            const line = this.#readLine();
            this.#parseLine(line);
        }
    }

    /**
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
                throw new HttpParser(`Internal error of parser - no action to run was found on new line of chunk. Called action "${this.#nextActionToCall}".`)
        }
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
        this.#parsingResults.versionMajor = match[1];
        this.#parsingResults.versionMinor = match[2];
        this.#parsingResults.statusCode = match[3];
        this.#parsingResults.statusMsg = match[4];

        this.#nextActionToCall = kActionsToCall.HEADER;
    }

    static #headerRegex = /^([^: \t]+):[ \t]*((?:.*[^ \t])|)/;
    /**
     * @param {string} line 
     */
    #parseHeader(line) {
        if (!line) {
            return;
        }
        const match = HttpParser.#headerRegex.exec(line);
        this.#parsingResults.headers.push(match[1], match[2]);

        this.#nextActionToCall = kActionsToCall.HEADER;
    }

    /**
     * @param {string} line 
     */
    #parseBody() {
        throw new Error('Not implemented yet.');
    }
}
