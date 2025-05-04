import doneURL from 'done:url.js';
import doneHttpParser from 'done:http-parser.js';

class FetchError extends Error {
    super(msg) {
        this.super(msg);
    }
}

class Response {
    /**
     * @type {URL}
     */
    #url;

    /**
     * @type {string}
     */
    #rawWholeResponseStr;

    #wholeResponseFetched = false;

    #httpResponseParser = new doneHttpParser('RESPONSE');

    /**
     * @returns {Promise<string>}
     */
    text() {
        return new Promise(async (resolve, reject) => {
            if (!this.#wholeResponseFetched) {
                this.#fetchResponse();
                this.#httpResponseParser.parse(this.#rawWholeResponseStr);
            }
            const body = await this.#httpResponseParser.body;
            resolve(body);
        });
    }

    /**
     * @returns {Promise<object>}
     */
    json() {
        return new Promise(async (resolve, reject) => {
            if (!this.#wholeResponseFetched) {
                this.#fetchResponse();
                this.#httpResponseParser.parse(this.#rawWholeResponseStr);
            }
            try {
                const body = await this.#httpResponseParser.body;
                resolve(body);
            } catch (e) {
                reject(e);
            }
        });
    }

    constructor(url) {
        this.#url = url
    }

    #fetchResponse() {
        let hostForSysCall = this.#url.hostname;
        if (hostForSysCall === "localhost") {
            // this is special case where for `getaddrinfo` syscall we are sending `null`
            // if we want this syscall to resolve "localhost" address
            hostForSysCall = null;
        }
        let port = this.#url.port;
        let protocolWithoutColon = this.#url.protocol.replace(':', '');
        let portForSysCall = protocolWithoutColon;
        if (port) {
            portForSysCall = port;
        }

        const resGetAddrInfo = {};
        globalThis.done.syscall.getaddrinfo(hostForSysCall, portForSysCall, { ai_family: 'AF_UNSPEC', ai_socktype: 'SOCK_STREAM' }, resGetAddrInfo);

        const socketFd = globalThis.done.syscall.socket(resGetAddrInfo.ai_family, resGetAddrInfo.ai_socktype, resGetAddrInfo.ai_protocol);
        const connectRes = globalThis.done.syscall.connect(socketFd, resGetAddrInfo.ai_addr, resGetAddrInfo.ai_addrlen);

        // http servers before sending some data expects something
        const encoder = new TextEncoder();
        const encodedText = encoder.encode(`GET / ${kProtocolVersion}\r\n\r\n`);
        const encodedTextSize = encodedText.byteLength;
        const sendBytes = globalThis.done.syscall.send(socketFd, encodedText, encodedTextSize, 0);

        const buffer = new ArrayBuffer(kBufferSize);

        let endRegex = /0\r\n\r\n?/;
        let end = false;
        let wholeResponse = '';
        do {
            const recvSize = globalThis.done.syscall.recv(socketFd, buffer, kBufferSize, 0);

            const decoder = new TextDecoder();
            const bufferToDecode = new DataView(buffer, 0, recvSize);
            const decodedChunk = decoder.decode(bufferToDecode);
            wholeResponse += decodedChunk;

            end = endRegex.test(decodedChunk);
        } while (!end)
        const closeRes = globalThis.done.syscall.close(socketFd);

        this.#rawWholeResponseStr = wholeResponse;
        this.#wholeResponseFetched = true;
    }
}

const kProtocolVersion = 'HTTP/1.1'
const kBufferSize = 1024;

/**
 * @param {RequestInfo | doneURL} input 
 * @param {RequestInit} init 
 * @implements {Response}
 */
export default function fetch(input, init) {
    return new Promise((resolve, reject) => {
        let url;
        if (input instanceof doneURL) {
            url = input;
        } else if (typeof input === 'string') {
            url = new doneURL(input);
        } else {
            return reject(new FetchError(`"input" must be instance of "URL" or type of "string". Got "${input}".`));
        }

        const response = new Response(url);
        return resolve(response);
    });
}
