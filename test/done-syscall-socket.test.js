import assert from "done:assert.js";

const socket = globalThis.done.syscall.socket;
const close = globalThis.done.syscall.close;

// should return file descriptor for basic socket creation
(() => {
    const domain = 2; /* PF_INET */
    const type = 1; /* SOCK_STREAM */
    const protocol = 0; /* IPPROTO_IP */

    const fd = socket(domain, type, protocol);

    assert.check(fd > 0, 'File descriptor with "socket" syscall was not created.');

    // after test
    close(fd);
})();
