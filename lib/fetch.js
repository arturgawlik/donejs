import doneURL from 'done:url.js';
// import doneURL from './url.js';

class FetchError extends Error {
    super(msg) {
        this.super(msg);
    }
}

const kProtocolVersion = 'HTTP/1.1'

/**
 * @param {RequestInfo | doneURL} input 
 * @param {RequestInit} init 
 * @implements {Response}
 */
export default function fetch(input, init) {
    return new Promise((resolve, reject) => {
        /**
         * @type {URL}
         */
        let url;

        if (input instanceof doneURL) {
            url = input;
        } else if (typeof input === 'string') {
            url = new doneURL(input);
        } else {
            return reject(new FetchError(`"input" must be instance of "URL" or type of "string". Got "${input}".`));
        }

        let hostForSysCall = url.hostname;
        if (hostForSysCall === "localhost") {
            // this is special case where for `getaddrinfo` syscall we are sending `null`
            // if we want this syscall to resolve "localhost" address
            hostForSysCall = null
        }
        let port = url.port;
        let protocolWithoutColon = url.protocol.replace(':', '');
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

        const bufferSize = 1024;
        const buffer = new ArrayBuffer(bufferSize);

        let endRegex = /0\r\n\r\n?/;
        let end = false;
        let wholeResponse = '';
        do {
            const recvConnect = globalThis.done.syscall.recv(socketFd, buffer, bufferSize, 0);

            const decoder = new TextDecoder();
            const decodedChunk = decoder.decode(buffer);
            wholeResponse += decodedChunk;

            end = endRegex.test(decodedChunk)
        } while (!end)
        const closeRes = globalThis.done.syscall.close(socketFd);

        return resolve(wholeResponse);
    });
}
