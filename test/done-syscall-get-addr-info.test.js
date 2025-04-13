import assert from "done:assert.js";

const getaddrinfo = globalThis.done.syscall.getaddrinfo;

// should return basic information about www.google.com addr
(() => {
    const host = "www.google.com";
    const port = "443";
    const hints = { ai_family: 'AF_UNSPEC', ai_socktype: 'SOCK_STREAM' };
    const result = {};

    getaddrinfo(host, port, hints, result);

    assert.equal(result.ai_family, 2 /* PF_INET */);
    assert.equal(result.ai_socktype, 1 /* SOCK_STREAM */);
    assert.equal(result.ai_protocol, 6 /* IPPROTO_TCP */);
})();
