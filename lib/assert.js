class AssertError extends Error {
    constructor(msg) {
        super(msg);
    }
}

function throwErr(msg) {
    throw new AssertError(`Assert:${msg}`);
}

export function equal(actual, expected, assertMessage) {
    if (actual !== expected) {
        let msg = '';
        if (assertMessage) {
            msg += `${assertMessage}. `
        }
        msg += `"${actual}" is not equal to "${expected}"`;

        throwErr(msg);
    }
}

export default {
    equal
}
