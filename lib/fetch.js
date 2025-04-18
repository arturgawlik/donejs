
/**
 * @param {RequestInfo | URL} input 
 * @param {RequestInit} init 
 * @implements {Response}
 */
export default function fetch(input, init) {
    if (typeof input !== 'string') {
        throw new TypeError('In "fetch" currently only type "string" is supported in "input" argument.');
    }

    const parsedUrl = parseUrl(input);

    let hostForSysCall = parsedUrl.address;
    if (hostForSysCall === "localhost") {
        // this is special case where for `getaddrinfo` syscall we are sending `null`
        // if we want this syscall to resolve "localhost" address
        hostForSysCall = null
    }
    let port = parsedUrl.port;
    let protocol = parsedUrl.protocol;
    let portForSysCall = protocol;
    if (port) {
        portForSysCall = port;
    }

    const resGetAddrInfo = {};
    globalThis.done.syscall.getaddrinfo(hostForSysCall, portForSysCall, { ai_family: 'AF_UNSPEC', ai_socktype: 'SOCK_STREAM' }, resGetAddrInfo);

    const socketFd = globalThis.done.syscall.socket(resGetAddrInfo.ai_family, resGetAddrInfo.ai_socktype, resGetAddrInfo.ai_protocol);
    const connectRes = globalThis.done.syscall.connect(socketFd, resGetAddrInfo.ai_addr, resGetAddrInfo.ai_addrlen);

    // http servers before sending some data expects something
    const encoder = new TextEncoder();
    const encodedText = encoder.encode('GET /\r\n\r\n');
    const encodedTextSize = encodedText.byteLength;
    const sendBytes = send(socketFd, encodedText, encodedTextSize, 0);

    const bufferSize = 1024;
    const buffer = new ArrayBuffer(bufferSize);
    const recvConnect = globalThis.done.syscall.recv(socketFd, buffer, bufferSize, 0);

    const closeRes = globalThis.done.syscall.close(socketFd);

    const decoder = new TextDecoder();
    const decodedText = decoder.decode(buffer);

    // TODO: return `Promise` here 
    return decodedText;
}

/**
 * @param {string} url 
 */
function parseUrl(url) {
    // currently this is very naive implementation which fulfils
    // basic requirements of `fetch`

    // eg. "www.google.com"
    let address;
    // eg. "https"
    let protocol;
    // eg. "5000"
    let port;

    const addressStartIndex = url.indexOf('www.');

    if (addressStartIndex >= 0) {
        address = url.slice(addressStartIndex)
    } else if (url.includes("localhost")) {
        address = "localhost";
    } else {
        throw new Error(`Url parse error: Can not find "www." in ${url}.`);
    }

    const protocolEndIndex = url.indexOf('://');
    if (protocolEndIndex >= 0) {
        protocol = url.slice(0, protocolEndIndex);
    } else {
        throw new Error(`Url parse error: Can not find "://" in ${url}.`);
    }

    // we're searching port from address begin
    const portStartIndex = url.lastIndexOf(':');
    if (portStartIndex >= 0) {
        // +1 because we don't want to have ":"
        port = url.slice(portStartIndex + 1);

        // check is port valid
        if (!Number.parseInt(port, 10)) {
            // this can be thrown when e.g. it contains some path in url
            // or query params - those are currently not supported
            throw new Error(`Url parse error: Can not parse port. Parsed value: ${port}`);
        }
    }

    return {
        address,
        protocol,
        port
    }
}
