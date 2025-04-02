class AssertError extends Error {
    constructor(msg) {
        super(msg);
    }
}

function throwErr(msg) {
    throw new AssertError(`Assert:${msg}`);
}

export function equal(actual, expected) {
    if (actual !== expected) {
        throwErr(`"${actual}" is not equal to "${expected}"`);
    }
}
